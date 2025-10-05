// ReSharper disable CppDFATimeOver
#include "Compiler.h"

void compile(const vector<Statement*>& tree, const int stack, const string& loc, const string& fasmdir, const string& msvcrt, const string& libd)
{
	addToData("scanstr db '                                                                                                                                '");
	addToData("intfmt db '%d'");
	addToData("doublefmt db '%f'");
	addToData("scanfmt db '%[^',10,']s'");
	addToData("charfmt db '%c'");
	addToData("chardiscard db '?'");

	addToData("gc dq 0");
	addToData("gcsz dd 10");
	addToData("heapclearmessage db 10,'Removed 1 object from memory'");

	File = ofstream(loc);
	File << "format PE64 console\nentry LABFUNC" + to_string(MAIN)+"\nstack "+to_string(stack*1024)+"\ninclude '" + fasmdir + "INCLUDE\\MACRO\\IMPORT64.INC'\ninclude '"+libd+"'\n\n";

	for (Statement* st : tree)
		compileStatement(st,nullptr);

	File << "section '.data' data readable writeable\n";
	File << datarw.str();
	File.close();
}


void compileStatement(Statement* b, Func* fn) { // NOLINT(*-no-recursion)
	const StatementType statementtype = b->getType();
	if(statementtype!=FUNC_DEFINITION && fn == nullptr)
		return;
	switch (statementtype)
	{
	case FUNC_DEFINITION:
		{
			const auto fun = dynamic_cast<Func*>(b);

			auto off = 16;
			for (int i = 0; i < fun->params; i++)
			{
				const auto a = dynamic_cast<Assignment*>(fun->body->code[i]);
				const auto sz = dynamic_cast<Size*>(a->value)->value;
				off += sz.sz;
				a->value = new Pointer("[rbp+" + to_string(off)+"]",sz);
			}

			fun->body->returnPtr = "LABFUNCEND" + to_string(fun->name.value);
			compileStatement(fun->body, fun);

			File << "LABFUNC" + to_string(fun->name.value) << ":" << endl;
			if (fun->name.value == MAIN)
				File << "mov     rax,rsp\nand     rax,0FFFFFFFFFFFFFFF0h\ntest    rsp,1111b\ncmovnz  rsp,rax" << endl;

			File << "push rbp" << endl;
			File << "mov rbp, rsp" << endl;
			if (fun->name.value == MAIN)
				File << "\nxor rcx, rcx\nxor rdx, rdx\nxor r8, r8\n" << endl; // call[HeapCreate]\n mov[hHeap], rax\ncall gengc" << endl;
			savePreserved();
			File << endl;
			File << fun->fbody.str();
			File << endl;
			File << "LABFUNCEND" + to_string(fun->name.value) << ":" << endl;
			restorePreserved();
			File << "mov rsp, rbp" << endl;
			if (fun->name.value == MAIN) File << "\nmov rcx,0\ncall [ExitProcess]" << endl;
			File << "pop rbp;" << endl;
			File << "ret" << endl;

			fun->fbody.str(std::string());
			fun->fbody.clear();

			return;
		}
	case SCOPE: {
		prologue(fn);

		for (const auto* scope = dynamic_cast<CodeBlock*>(b); const auto i : scope->code)
		{
			if(i->getType() == WHILE_STMT)
				dynamic_cast<WhileStatement*>(i)->SetParentLoop(scope->returnPtr);
			if(i->getType() == IF_STMT)
				dynamic_cast<IfStatement*>(i)->SetParentLoop(scope->stopPtr,scope->skipPtr, scope->returnPtr);

			if(i->getType() == INTERUPT)
			{
				switch(const auto inter = dynamic_cast<Interupt*>(i); inter->type)
				{
				case RETURN:
					{
						Value* returnvaraddrs = nullptr;
						if(inter->value != nullptr)
						{
							Register* returnreg = nullptr;
							if(fn->returns.prec == 0 || fn->returns.prec >= 10)
							{
								switch(fn->returns.sz)
								{
								case 1:returnreg = regs1[0];break;
								case 2:returnreg = regs2[0];break;
								case 4:returnreg = regs4[0];break;
								case 8:returnreg = regs8[0];break;
								default: aThrowError(OVERSIZED_VALUE,-1);
								}
							}
							else returnreg = regsXMM[0];

							compileInstruction(MOV2,returnreg,inter->value,fn,returnreg->size);
							if(fn->returns.prec == 255)returnvaraddrs = inter->value;
						}
						epiloguefree(fn,returnvaraddrs);
						fn->fbody << "jmp " << scope->returnPtr << endl;
						break;
					}
				case STOP:fn->fbody << "jmp " << scope->stopPtr << endl;break;
				case SKIP:fn->fbody << "jmp " << scope->skipPtr << endl;break;
				default: aThrowError(UNKNOWN_STATEMENT,-1);break;
				}
			}
			else compileStatement(i, fn);
		}
		epiloguefree(fn, nullptr);
		epilogue(fn);
		return;
	}
	case WHILE_STMT: {
		const auto* whils = dynamic_cast<WhileStatement*>(b);
		const string label = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;
		const string label2 = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;

		fn->fbody << label << ":" << endl;
		compileInstruction(CMP2, whils->condition, new Boolean(true), fn, BOOL_SIZE);
		compileInstruction(JNZ1, new CompilationToken(label2, VOID_SIZE,COMPILETIME_PTR), nullptr, fn, BOOL_SIZE);

			whils->whileBlock->stopPtr = label2;
			whils->whileBlock->skipPtr = label;
		compileStatement(whils->whileBlock, fn);

		compileInstruction(JMP1, new CompilationToken(label, VOID_SIZE,COMPILETIME_PTR), nullptr, fn, BOOL_SIZE);

		fn->fbody << label2 << ":" << endl;
		return;
	}
	case IF_STMT:
		{
			const auto* ifs = dynamic_cast<IfStatement*>(b);
			const string label = ".LABBRNCH" + to_string(dataLabelIdx);
			dataLabelIdx++;
			const string label2 = ".LABBRNCH" + to_string(dataLabelIdx);
			if(ifs->elseBlock != nullptr)
				dataLabelIdx++;
			compileInstruction(CMP2, ifs->condition, new Boolean(true), fn, BOOL_SIZE);
			compileInstruction(JNZ1, new CompilationToken(label,VOID_SIZE, COMPILETIME_PTR), nullptr, fn, BOOL_SIZE);

			compileStatement(ifs->ifBlock, fn);
			if(ifs->elseBlock != nullptr)
				compileInstruction(JMP1, new CompilationToken(label2,VOID_SIZE,COMPILETIME_PTR), nullptr, fn, BOOL_SIZE);

			fn->fbody << label << ":" << endl;
			if(ifs->elseBlock != nullptr)
			{
				compileStatement(ifs->elseBlock, fn);
				fn->fbody << label2 << ":" << endl;
			}
			return;
		}
	case ASSIGNMENT:
		{
			const auto* a = dynamic_cast<Assignment*>(b);
			AsmSize sz = getSize(a->value, fn, false);
			//Re asssignment
			for (int i = 0; i < fn->varsStack.size(); i++)
				if (fn->varsStack[i].name.value == a->name.value )
				{
					if(!fn->varsStack[i].isConst)
					{
						AsmSize vsz = fn->varsStack[i].size;
						INSTRUCTION ins = MOV2;
						switch (a->type)
						{
						case EQUALS:			ins = MOV2;break;
						case PLUS_EQUAL:		ins = ADD2;break;
						case MINUS_EQUAL:		ins = SUB2;break;
						case MULTIPLY_EQUAL:	ins = MUL2;break;
						case BITWISE_OR_EQUAL:	ins = OR2; break;
						case BITWISE_AND_EQUAL: ins = AND2;break;
						default: break;
						}

						if (vsz.prec == 255 && ins == ADD2)
						{
							if (vsz.sub == nullptr) {
								compileInstruction(MOV2, regs8[2], a->value, fn, sz);
								fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
								fn->fbody << "call addstr" << endl;
								return;
							}
							else {
								switch (vsz.sub->sz)
								{
								case 1: {
									if (vsz.sub->prec == 0) {
										compileInstruction(MOV2, regs1[2], a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "call addbyte" << endl;
										return;
									}
								}
								case 2: {
									if (vsz.sub->prec == 0) {
										compileInstruction(MOV2, regs2[2], a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "call addword" << endl;
										return;
									}
								}
								case 4: {
									if (vsz.sub->prec == 0) {
										compileInstruction(MOV2, regs4[2], a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "call adddword" << endl;
										return;
									}
									else if (vsz.sub->prec == 1) {
										Register* reg = alloc(*vsz.sub);
										compileInstruction(MOV2, reg, a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "movq rdx, "<< reg->reg << endl << "call adddword" << endl;
										free(reg);
										return;
									}
								}
								case 8: {
									if (vsz.sub->prec == 0 || vsz.sub->prec == 255) {
										compileInstruction(MOV2, regs8[2], a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "call addqword" << endl;
										return;
									}
									else if (vsz.sub->prec == 2) {
										Register* reg = alloc(*vsz.sub);
										compileInstruction(MOV2, reg, a->value, fn, sz);
										fn->fbody << "mov rcx, " << "qword [rbp - " + to_string(fn->varsStack[i].off) + "]" << endl;
										fn->fbody << "movq rdx, " << reg->reg << endl << "call addqword" << endl;
										free(reg);
										return;
									}
								}
								}
								aThrowError(ASSIGNED_WRONG_TYPE, -1);
								return;
							}
						}
						if (vsz.sz != sz.sz || vsz.prec != sz.prec)aThrowError(ASSIGNED_WRONG_TYPE, -1);

						compileInstruction(ins, new Pointer("[rbp - " + to_string(fn->varsStack[i].off) + "]", fn->varsStack[i].size), a->value, fn, sz);
						return;
					}
					aThrowError(ILLEGAL_OPERATION_ONCONST, -1);
				}


			//actually doing the changes on stack
			const int szonstack = static_cast<int>(ceil(static_cast<double>(sz.sz)/ALIGN)*ALIGN);
			fn->scopesStack.back()++;
			if (fn->varsStack.empty()) {
				compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
				compileInstruction(SUB2, rsp, new Long(szonstack), fn, LONG_SIZE);
				rspOff.back() += szonstack;
				fn->varsStack.emplace_back(a->value, sz.sz, sz, a->name);
				if(sz.prec == 255)fn->varsStack.back().isHeaped = true;
				fn->varsStack.back().isConst = a->isconst;
				return;
			}
			const Variable v = fn->varsStack.back();
			if (v.size.sz == sz.sz && v.share > 0) {
				compileInstruction(MOV2, new Pointer("[rsp + "+to_string(v.share-sz.sz) + "]", sz), a->value, fn, sz);
				fn->varsStack.emplace_back(a->value,v.off+sz.sz, sz, a->name, v.share);
			}
			else {
				compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
				compileInstruction(SUB2, rsp, new Long(szonstack), fn, LONG_SIZE);
				rspOff.back() += szonstack;
				fn->varsStack.emplace_back(a->value,v.off + sz.sz + v.share, sz, a->name);
			}
			if(sz.prec == 255)fn->varsStack.back().isHeaped = true;
			fn->varsStack.back().isConst = a->isconst;

			return;
		}
	default: compileValue(dynamic_cast<Value*>(b),fn);// aThrowError(UNKNOWN_STATEMENT, -1);
	}
}

void compileInstruction(const INSTRUCTION i, Value* op, Value* op2, Func* fn, const AsmSize sz) // NOLINT(*-no-recursion)
{
	if (op == nullptr) {
		switch (i)
		{
		case CDQ0: fn->fbody << "cdq" << endl;break;
		default: break;
		}
		return;
	}

	CompilationToken o1;
	if(op->getType() == COMPILETIME_TOKEN) o1 = *dynamic_cast<CompilationToken*>(op);
	else o1 = compileValue(op, fn);

	if(o1.type == COMPILETIME_IGNORE)return;
	// if(o1.type != COMPILETIME_PTR)
	const AsmSize o1sz = getSize(op, fn, false);
	if(o1sz.sz != 0 && sz.sz != 0)
		if ((o1sz.prec != sz.prec || o1sz.sz != sz.sz) && (o1sz.prec!= 255 && sz.prec != 255)) {
			op  = cast(op, o1sz, sz, fn);
			o1  = compileValue(op, fn);
		}



	if (op2 == nullptr) {
		switch (i)
		{
		case IDIV1: fn->fbody << "idiv " << o1.line << endl;	break;
		case NEG1: fn->fbody << "neg " << o1.line << endl;break;
		case NOT1: fn->fbody << "not " << o1.line << endl;break;
		case JNZ1: fn->fbody << "jnz " << o1.line << endl;break;
		case JZ1: fn->fbody << "jz " << o1.line << endl;break;
		case JMP1: fn->fbody << "jmp " << o1.line << endl;break;
		case SETE1: fn->fbody << "sete " << o1.line << endl;break;
		case SETNE1: fn->fbody << "setne " << o1.line << endl;break;
		case SETG1: fn->fbody << "setg " << o1.line << endl;break;
		case SETGE1: fn->fbody << "setge " << o1.line << endl;break;
		case SETL1: fn->fbody << "setl " << o1.line << endl;break;
		case SETLE1: fn->fbody << "setle " << o1.line << endl;break;
			case SETA1: fn->fbody << "seta " << o1.line << endl;break;
			case SETAE1: fn->fbody << "setae " << o1.line << endl;break;
			case SETB1: fn->fbody << "setb " << o1.line << endl;break;
			case SETBE1: fn->fbody << "setbe " << o1.line << endl;break;
		default: break;
		}
		return;
	}
	auto suffix = "  ";
	if (sz.prec == 1) {
		suffix = "ss";
	}
	else if (sz.prec == 2) {
		suffix = "sd";
	}
	CompilationToken o2;
	if(op2->getType() == COMPILETIME_TOKEN) o2 = *dynamic_cast<CompilationToken*>(op2);
	else o2 = compileValue(op2, fn);

	if(o2.type == COMPILETIME_IGNORE)return;
	// if(o2.type != COMPILETIME_PTR)
	const AsmSize o2sz = getSize(op2, fn, false);
	if(o2sz.sz != 0 && sz.sz != 0)
		if ( (o2sz.prec != sz.prec|| o2sz.sz != sz.sz) && (o2sz.prec != 255 && sz.prec != 255)) {
			op2 = cast(op2, o2sz, sz, fn);
			o2 = compileValue(op2,fn);
		}

	if (o1.type == COMPILETIME_PTR && o2.type == COMPILETIME_PTR) {
		Register* reg = alloc(getSize(op, fn, false));

		compileInstruction(MOV2, reg, op2, fn, sz);
		compileInstruction(i, op, reg, fn, sz);
		free(reg);
		return;
	}

	switch (i)
	{
		case MOV2: {
				fn->fbody << "mov" << suffix << " " << o1.line << ", " << o2.line << endl;
				break;
		}
		case ADD2: {
				fn->fbody << "add" << suffix << " " << o1.line << ", " << o2.line << endl;
				break;
		}
		case SUB2: {
				fn->fbody << "sub" << suffix << " " << o1.line << ", " << o2.line << endl;
				break;
		}
		case MUL2: {
				if (sz.prec == 0)
					fn->fbody << "imul " << o1.line << ", " << o2.line << endl;
				else
					fn->fbody << "mul" << suffix << " " << o1.line << ", " << o2.line << endl;
				break;
		}
		case DIV2: {
				if (sz.prec != 0)
					fn->fbody << "div" << suffix << " " << o1.line << ", " << o2.line << endl;
				break;
		}
		case XOR2:
			{
				if(suffix[1] != ' ')fn->fbody << "xorp" << suffix[1] <<" " << o1.line << ", " << o2.line << endl;
				fn->fbody << "xor " << o1.line << ", " << o2.line << endl;
				break;
			}
		case AND2: {
				fn->fbody << "and " << o1.line << ", " << o2.line << endl;
				break;
		}
		case OR2: {
				fn->fbody << "or " << o1.line << ", " << o2.line << endl;
				break;
		}
		case CMP2: {
				if (o1.type == COMPILETIME_REGISTER || o2.type == COMPILETIME_REGISTER) {
					if (sz.prec == 0)
						fn->fbody << "cmp " << o1.line << ", " << o2.line << endl;
					else
						fn->fbody << "comi" << suffix << " " << o1.line << ", " << o2.line << endl;

				}
				else {
					Register* reg = alloc(getSize(op, fn, false));
					compileInstruction(MOV2, reg, op, fn, sz);
					compileInstruction(CMP2, reg, op2, fn, sz);
					free(reg);
				}
				break;
		}
		case TEST2: {
				fn->fbody << "test " << o1.line << ", " << o2.line << endl;
				break;
		}
		default: break;
	}
}

Register* cast(Value* v, const AsmSize from, const AsmSize to, Func* fn)
{
	if(from.sz == to.sz && from.prec == to.prec)
		aThrowError(ILLEGAL_CAST,-1);

	if(from.prec == 0)
	{
		if(to.prec == 0)
		{
			Register* regt = alloc(to);
			if(to.sz >= from.sz)
			{
				compileInstruction(XOR2, regt, regt,fn,to);;
			}
			Register* regf = realloc(from);
			compileInstruction(MOV2, regf, v, fn, from);
			free(regt);
			return regt;
		}
		if(to.prec == 1 || to.prec == 2)
		{
			Register* regt = alloc(to);
			Register* regf = alloc(LONG_SIZE);
			compileInstruction(XOR2,regf,regf,fn,LONG_SIZE);
			regf = realloc(from);
			compileInstruction(MOV2, regf, v, fn, from);
			regf = realloc(LONG_SIZE);
			if(to.prec == 1)
				fn->fbody << "cvtsi2ss " << regt->reg << ", " << regf->reg << endl;
			else
				fn->fbody << "cvtsi2sd " << regt->reg << ", " << regf->reg << endl;
			free(regf);
			free(regt);
			return regt;
		}
	}
	if(from.prec == 1)
	{
		if(to.prec == 0)
		{
			Register *regf = alloc(from);
			Register *regt = alloc(LONG_SIZE);

			compileInstruction(MOV2, regf, v, fn, from);
			fn->fbody << "cvtss2si " << regt->reg << ", " << regf->reg << endl;
			regt = realloc(to);

			free(regt);
			free(regf);
			return regt;
		}
		if(to.prec == 2)
		{
			Register* regf = alloc(from);
			Register* regt = alloc(to);
			compileInstruction(MOV2, regf, v, fn, from);
			fn->fbody << "cvtss2sd " << regt->reg << ", " << regf->reg << endl;
			free(regt);
			free(regf);
			return regt;
		}
	}
	if(from.prec == 2)
	{
		if(to.prec == 0)
		{
			Register *regf = alloc(from);
			Register *regt = alloc(LONG_SIZE);

			compileInstruction(MOV2, regf, v, fn, from);
			fn->fbody << "cvtsd2si " << regt->reg << ", " << regf->reg << endl;
			regt = realloc(to);

			free(regt);
			free(regf);
			return regt;
		}
		if(to.prec == 1)
		{
			Register* regf = alloc(from);
			Register* regt = alloc(to);
			compileInstruction(MOV2, regf, v, fn, from);
			fn->fbody << "cvtsd2ss " << regt->reg << ", " << regf->reg << endl;
			free(regt);
			free(regf);
			return regt;
		}
	}
	aThrowError(ILLEGAL_CAST,-1);
	return nullptr;
}

CompilationToken compileValue(Value* v, Func* fn) { // NOLINT(*-no-recursion)

	switch (v->getType())
	{
	case FUNC_CALL:
	{
		Register* returnreg = regs8[0];
		saveScratch(fn);
		const auto fc = dynamic_cast<FuncCall*>(v);
		switch (fc->name.value) {
		case PRINT: {
			for (Value* params : fc->params) {
				CompilationToken cpar = compileValue(params, fn);
				compileInstruction(SUB2, rsp, new Long(0x20), fn, LONG_SIZE);
				auto size = getSize(params, fn, false);
				switch (size.sz)
				{
				case 2:
				{
					compileInstruction(MOV2, regs8[1], new CompilationToken("intfmt", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
					compileInstruction(XOR2, regs8[2], regs8[2], fn, LONG_SIZE);
					compileInstruction(MOV2, regs2[2], &cpar, fn, SHORT_SIZE);
					fn->fbody << "call [printf]" << endl;
				}break;
				case 4:
					if (size.prec == 0)
					{
						compileInstruction(MOV2, regs8[1], new CompilationToken("intfmt", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
						compileInstruction(MOV2, regs4[2], &cpar, fn, INT_SIZE);
						fn->fbody << "call [printf]" << endl;
					}
					if (size.prec == 1)
					{
						fn->fbody << "sub rsp, 8" << endl
							<< "movss dword [rsp], xmm0" << endl
							<< "cvtss2sd xmm0, " << cpar.line << endl
							<< "movq rdx, xmm0" << endl
							<< "mov rcx, doublefmt" << endl
							<< "call [printf]" << endl
							<< "movss xmm0, [rsp]" << endl
							<< "add rsp, 8" << endl;

					}
					break;
				case 8:
					if (size.prec == 255)
					{
						fn->fbody
							<< "mov rcx, " << cpar.line << endl
							<< "mov rcx, [rcx]" << endl
							<< "call [printf]" << endl;
					}
					if (size.prec == 0)
					{
						compileInstruction(MOV2, regs8[1], new CompilationToken("intfmt", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
						compileInstruction(MOV2, regs8[2], &cpar, fn, LONG_SIZE);
						fn->fbody << "call [printf]" << endl;
					}
					if (size.prec == 2)
					{
						fn->fbody << "sub rsp, 8" << endl
							<< "movsd qword [rsp], xmm0" << endl
							<< "movsd xmm0, " << cpar.line << endl
							<< "movq rdx, xmm0" << endl
							<< "mov rcx, doublefmt" << endl
							<< "call [printf]" << endl
							<< "movsd xmm0, qword [rsp]" << endl
							<< "add rsp, 8" << endl;
					}
					break;

				default:break;
				}
				compileInstruction(ADD2, rsp, new Long(0x20), fn, LONG_SIZE);
			}
			break;
		}
		case SCAN: {
			for (Value* params : fc->params) {
				auto [sz, prec, sub] = getSize(params, fn, false);
				CompilationToken ct = compileValue(params, fn);
				switch (sz) {
				case 8:
				{
					fn->fbody << "sub rsp, 32" << endl
						<< "mov rcx, scanfmt" << endl
						<< "mov rdx, scanstr" << endl
						<< "call [scanf]" << endl;

					fn->fbody << "mov rcx, charfmt" << endl
						<< "mov rdx, chardiscard" << endl
						<< "call [scanf]" << endl
						<< "add rsp, 32" << endl;

					fn->fbody << "mov rcx, scanstr" << endl
						<< "call genstrlab" << endl
						<< "mov " << ct.line << ", rax" << endl;
					break;
				}
				case 4:
					fn->fbody << "lea " << regs8[0]->reg << "," << ct.line << endl;

					compileInstruction(SUB2, rsp, new Long(0x20), fn, LONG_SIZE);
					compileInstruction(MOV2, regs8[1], new CompilationToken("intfmt", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
					compileInstruction(MOV2, regs8[2], regs8[0], fn, LONG_SIZE);
					fn->fbody << "call [scanf]" << endl;
					compileInstruction(MOV2, regs8[1], new CompilationToken("charfmt", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
					compileInstruction(MOV2, regs8[2], new CompilationToken("chardiscard", VOID_SIZE, COMPILETIME_PTR), fn, LONG_SIZE);
					fn->fbody << "call [scanf]" << endl;

					compileInstruction(ADD2, rsp, new Long(0x20), fn, LONG_SIZE);

					// fn->fbody << "scanint " << ct.line << endl; break;
				default: break;
				}
			}
			break;
		}
		case BEEP:
		{
			compileInstruction(SUB2, rsp, new Long(0x20), fn, LONG_SIZE);
			if (fc->params.empty())
			{
				fn->fbody << "mov rcx, 640\nmov rdx, 350\n call [beep]" << endl;
			}
			else if (fc->params.size() == 1)
			{
				auto [sz, prec, sub] = getSize(fc->params[0], fn, false);
				auto ct = compileValue(fc->params[0], fn);
				if (sz == 4)
				{
					compileInstruction(MOV2, regs4[1], &ct, fn, INT_SIZE);
					compileInstruction(MOV2, regs4[2], new Int(350), fn, INT_SIZE);
					fn->fbody << "call [beep]" << endl;
				}
			}
			else if (fc->params.size() == 2)
			{
				auto [sz, prec, sub] = getSize(fc->params[0], fn, false);
				auto ct = compileValue(fc->params[0], fn);
				if (sz == 4)
				{
					auto [sz2, prec2, sub2] = getSize(fc->params[1], fn, false);
					auto ct2 = compileValue(fc->params[1], fn);
					if (sz2 == 4)
					{
						compileInstruction(MOV2, regs4[1], &ct, fn, INT_SIZE);
						compileInstruction(MOV2, regs4[2], &ct2, fn, INT_SIZE);
						fn->fbody << "call [beep]" << endl;

					}
				}
			}
			compileInstruction(ADD2, rsp, new Long(0x20), fn, LONG_SIZE);
			break;
		}
		case LEN:
		{
			compileInstruction(MOV2, regs8[1], fc->params[0], fn, STRPTR_SIZE);
			fn->fbody << "mov rdx, 1" << endl;
			returnreg = regs4[0];
			fn->fbody << "	call len" << endl;
			Register* temp = alloc(returnreg->size);
			compileInstruction(MOV2, temp, returnreg, fn, returnreg->size);
			free(temp);
			returnreg = temp;
			break;
		}
		case SIZEOF:
		{
			returnreg = regs4[0];
			int totalsz = 0;
			for (Value* params : fc->params) {
				auto [sz, prec, sub] = getSize(params, fn, false);
				totalsz += sz;
			}
			fn->fbody << "mov " << returnreg->reg << "," << totalsz << endl;
			break;
		}
		case COPY:
		{
			if (fc->params.size() == 1)
			{
				Register* temp = alloc(returnreg->size);
				auto cv = compileValue(fc->params[0], fn);
				fn->fbody << "mov rcx, " << cv.line << endl << "mov rcx, qword [rcx]" << endl << "call genstrlab" << endl << "mov " << temp->reg << ", rax" << endl;
				free(temp);
				returnreg = temp;
			}

			break;
		}
		case CAST:
		{
			if (fc->params.size() == 2 && fc->params[0]->getType() == SIZE)
			{
				AsmSize from = getSize(fc->params[1], fn, false);
				AsmSize to = dynamic_cast<Size*>(fc->params[0])->value;
				returnreg = cast(fc->params[1], from, to, fn);
			}
			else
			{
				aThrowError(ILLEGAL_CAST, -1);
			}
			break;
		}
		default: {
			bool found = false;
			for (const auto& g : globalRefs)
			{
				if (g.name.value == fc->name.value)
				{
					found = true;
					int align = 0;
					for (int i = fc->params.size() - 1; i >= 0; i--) // NOLINT(*-narrowing-conversions)
					{
						auto [sz, prec, sub] = getSize(fc->params[i], fn, false);
						align += sz;
					}
					int off = ((align / ALIGN) * ALIGN) + ALIGN - align;
					for (int i = fc->params.size() - 1; i >= 0; i--) // NOLINT(*-narrowing-conversions)
					{
						AsmSize sz = getSize(fc->params[i], fn, false);
						auto ct = compileValue(fc->params[i], fn);
						compileInstruction(MOV2, new Pointer("[rsp - " + to_string(off) + "]", sz), &ct, fn, sz);
						off += sz.sz;
					}
					const auto sz = getSize(fc, fn, false);
					if (sz.prec == 0 || sz.prec >= 10)
					{
						switch (sz.sz)
						{
						case 0:; break;
						case 1:returnreg = regs1[0]; break;
						case 2:returnreg = regs2[0]; break;
						case 4:returnreg = regs4[0]; break;
						case 8:returnreg = regs8[0]; break;
						default: aThrowError(OVERSIZED_VALUE, -1);
						}
					}
					else returnreg = regsXMM[0];

					// off += returnreg->size.sz;
					if (off != 0)
						compileInstruction(SUB2, rsp, new Long(off), fn, LONG_SIZE);

					fn->fbody << "	call LABFUNC" + to_string(fc->name.value) << endl;
					if (((returnreg->size.prec == 0 || returnreg->size.prec >= 10) && regIdx != -1) || (returnreg->size.prec > 0 && xmmIdx != -1))
					{
						Register* temp = alloc(sz);
						compileInstruction(MOV2, temp, returnreg, fn, sz);
						free(temp);
						returnreg = temp;
					}

					if (off != 0)
						compileInstruction(ADD2, rsp, new Long(off), fn, LONG_SIZE);
				}
			}
			if (!found)aThrowError(UNDEFINED_FUNCTION, -1);
			break;
		}
		}
		restoreScratch(fn);
		return CompilationToken{ returnreg };
	}
	case COMPILETIME_TOKEN: dynamic_cast<CompilationToken*>(v);
	case PTR: return CompilationToken{ dynamic_cast<Pointer*>(v) };
	case REGISTER:return CompilationToken{ dynamic_cast<Register*>(v) };
	case SHORT_STMT: return CompilationToken{ to_string(dynamic_cast<Short*>(v)->value), SHORT_SIZE };
	case INT_STMT: return CompilationToken{ to_string(dynamic_cast<Int*>(v)->value), INT_SIZE };
	case LONG_STMT: return CompilationToken{ to_string(dynamic_cast<Long*>(v)->value), LONG_SIZE };
	case BOOL_STMT: return CompilationToken{ to_string(dynamic_cast<Boolean*>(v)->value ? 1 : 0), BOOL_SIZE };
	case FLOAT_STMT: {
		auto* fpt = dynamic_cast<Float*>(v);
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dd " + to_string(fpt->value));
			dataLabelIdx++;
		}
		return CompilationToken{ "DWORD [" + label + "]", FLOAT_SIZE,COMPILETIME_PTR };
	}
	case DOUBLE_STMT: {
		auto* fpt = dynamic_cast<Double*>(v);
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dq " + to_string(fpt->value));
			dataLabelIdx++;
		}
		return CompilationToken{ "QWORD [" + label + "]", DOUBLE_SIZE,COMPILETIME_PTR };
	}
	case STRING_STMT: {
		auto* fpt = dynamic_cast<String*>(v);
		if (fpt->label == -1)
		{
			fpt->label = dataLabelIdx;
			string label = "LABDAT" + to_string(fpt->label);
			stringstream fmt;
			for (int i = 0; i < fpt->value.length(); i++)
			{
				switch (fpt->value[i])
				{
				case '\'':fmt << "',0x27,'"; break;
				case '\n':fmt << "',0x0A,'"; break;
				case '\r':fmt << "',0x0D,'"; break;
				default:fmt << fpt->value[i]; break;
				}
			}
			addToData(label + " db '" + fmt.str() + '\'');
			dataLabelIdx++;
			saveScratch(fn);
			fn->fbody << "mov rcx, " << label << endl;
			fn->fbody << "call genstrlab" << endl;

			Register* reg = regs8[0];
			if (regIdx != -1)
			{
				reg = alloc(STRPTR_SIZE);
				compileInstruction(MOV2, reg, regs8[0], fn, STRPTR_SIZE);
				free(reg);
			}
			restoreScratch(fn);
			return CompilationToken{ reg };
		}
		//////
		//string label = "LABDAT" + to_string(fpt->label);
		//saveScratch(fn);
		//fn->fbody << "mov rcx, " << label << endl;
		//fn->fbody << "call genstrlab" << endl;

		//Register* reg = regs8[0];
		//if (regIdx != -1)
		//{
		//	reg = alloc(STRPTR_SIZE);
		//	compileInstruction(MOV2, reg, regs8[0], fn, STRPTR_SIZE);
		//	free(reg);
		//}
		//restoreScratch(fn);
		printf("e\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\ne\n");
		return CompilationToken{ regs8[0]};

	}
	case REFERENCE: {
		const auto* id = dynamic_cast<Reference*>(v);
		for (const auto& i : fn->varsStack)
			if (i.name.value == id->value) {
				const auto p = Pointer("[rbp - " + to_string(i.off) + "]", i.size);
				return CompilationToken{ &p };
			}
		break;
	}
	case UN_OPERATION: {
		switch (auto* uo = dynamic_cast<UnaryOperation*>(v); uo->op)
		{
		case NOT: {
			Register* reg = alloc(BOOL_SIZE);
			compileInstruction(MOV2, reg, uo->right, fn, reg->size);
			compileInstruction(XOR2, reg, new Boolean(true), fn, reg->size);
			free(reg);
			return CompilationToken{ reg };
		}

		case NEGATIVE: {
			Register* reg = alloc(getSize(uo, fn, false));
			compileInstruction(MOV2, reg, uo->right, fn, reg->size);
			compileInstruction(NEG1, reg, nullptr, fn, reg->size);
			free(reg);
			return CompilationToken{ reg };
		}
		case BITWISE_NOT: {
			Register* reg = alloc(getSize(uo, fn, false));
			compileInstruction(MOV2, reg, uo->right, fn, reg->size);
			compileInstruction(NOT1, reg, nullptr, fn, reg->size);
			free(reg);
			return CompilationToken{ reg };
		}
		default: break;
		}
		break;
	}
	case MULTI_OPERATION: {
		auto* mo = dynamic_cast<MultipleOperation*>(v);
		const AsmSize sz = getSize(mo, fn, true);

		if (mo->size.prec == 255 && mo->operands.size() == 2)
		{
			saveScratch(fn);
			CompilationToken ct1 = compileValue(mo->operands[0], fn);
			fn->fbody << "mov rcx, " << ct1.line << endl << "push rcx" << endl;
			CompilationToken ct2 = compileValue(mo->operands[1], fn);
			fn->fbody << "mov rdx, " << ct2.line << endl << "pop rcx" << endl << "call strcmp" << endl;
			Register* reg = alloc(BOOL_SIZE);
			if (mo->op == NOT_EQUAL)
				fn->fbody << "xor al, 1" << endl;
			fn->fbody << "mov " << reg->reg << ", al" << endl;
			free(reg);
			restoreScratch(fn);
			return CompilationToken(reg);
		}

		switch (mo->op) {

		case PLUS: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);
			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(ADD2, reg, mo->operands[i], fn, sz);
			for (const auto& invoperand : mo->invoperands)
				compileInstruction(SUB2, reg, invoperand, fn, sz);
			free(reg);
			return CompilationToken{ reg };
		}
		case MULTIPLY: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(MUL2, reg, mo->operands[i], fn, sz);

			if (!mo->invoperands.empty()) {
				if (sz.prec == 0) {
					Register* Areg = A(sz);
					for (const auto& invoperand : mo->invoperands)
					{
						if (regIdx != 0) {
							compileInstruction(SUB2, rsp, new Long(sz.sz), fn, sz);
							compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
							compileInstruction(MOV2, Areg, reg, fn, sz);
						}
						Register* diver = alloc(sz);
						if (regIdx == 2) {
							Register* diver2 = alloc(sz);
							compileInstruction(MOV2, diver2, invoperand, fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver2, nullptr, fn, sz);
							free(diver2);
						}
						else {
							compileInstruction(MOV2, diver, invoperand, fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver, nullptr, fn, sz);
						}
						free(diver);

						if (regIdx != 0) {
							compileInstruction(MOV2, reg, Areg, fn, sz);
							compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
							compileInstruction(ADD2, rsp, new Long(sz.sz), fn, sz);
						}
					}
				}
				else
					for (const auto& invoperand : mo->invoperands)
						compileInstruction(DIV2, reg, invoperand, fn, sz);
			}
			free(reg);
			return CompilationToken{ reg };
		}
		case MODULO: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			Register* Areg = A(sz);
			Register* Dreg = D(sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				if (regIdx != 0) {
					compileInstruction(SUB2, rsp, new Long(sz.sz), fn, sz);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
					compileInstruction(MOV2, Areg, reg, fn, sz);
				}
				Register* diver = alloc(sz);
				if (regIdx == 2) {
					Register* diver2 = alloc(sz);
					compileInstruction(MOV2, diver2, mo->operands[i], fn, sz);
					compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
					compileInstruction(IDIV1, diver2, nullptr, fn, sz);
					free(diver2);
				}
				else {
					compileInstruction(MOV2, diver, mo->operands[i], fn, sz);
					compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
					compileInstruction(IDIV1, diver, nullptr, fn, sz);
				}
				compileInstruction(MOV2, reg, Dreg, fn, sz);
				free(diver);

				if (regIdx != 0) {
					compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
					compileInstruction(ADD2, rsp, new Long(sz.sz), fn, sz);
				}
			}
			free(reg);
			return CompilationToken{ reg };
		}

		case OR: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = alloc(BOOL_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JNZ1, new CompilationToken(".LABOP_S" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Boolean(true), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			free(reg);
			return CompilationToken{ reg };
		}
		case AND: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = alloc(BOOL_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JZ1, new CompilationToken(".LABOP_S" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Boolean(false), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			free(reg);
			return CompilationToken{ reg };
		}
		case XOR: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, reg, mo->operands[i], fn, sz);
			}

			free(reg);
			return CompilationToken{ reg };
		}
		case BITWISE_AND: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(AND2, reg, mo->operands[i], fn, sz);
			}

			free(reg);
			return CompilationToken{ reg };
		}
		case BITWISE_OR: {
			Register* reg = alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(OR2, reg, mo->operands[i], fn, sz);
			}

			free(reg);
			return CompilationToken{ reg };
		}
		default: break;
		}


		INSTRUCTION cmp = {};
		if (mo->size.prec == 0 || mo->size.prec >= 10)
			switch (mo->op)
			{
			case COMPARISON:cmp = SETE1; break;
			case NOT_EQUAL:cmp = SETNE1; break;
			case GREATER_THAN:cmp = SETG1; break;
			case GREATER_THAN_EQUAL:cmp = SETGE1; break;
			case SMALLER_THAN:cmp = SETL1; break;
			case SMALLER_THAN_EQUAL:cmp = SETLE1; break;
			default: break;
			}
		else
			switch (mo->op)
			{
			case COMPARISON:cmp = SETE1; break;
			case NOT_EQUAL:cmp = SETNE1; break;
			case GREATER_THAN:cmp = SETA1; break;
			case GREATER_THAN_EQUAL:cmp = SETAE1; break;
			case SMALLER_THAN:cmp = SETB1; break;
			case SMALLER_THAN_EQUAL:cmp = SETBE1; break;
			default: break;
			}

		Register* reg = alloc(getSize(mo, fn, true));
		Register* reg2 = nullptr;
		bool regRe = false;
		compileInstruction(MOV2, reg, mo->operands[0], fn, sz);
		for (size_t i = 1; i < mo->operands.size(); i++) {
			compileInstruction(CMP2, reg, mo->operands[i], fn, sz);
			if (reg->size.prec == 0)
				reg2 = realloc(getSize(mo, fn, false));
			else {
				regRe = true;
				reg2 = alloc(getSize(mo, fn, false));
			}
			compileInstruction(cmp, reg2, nullptr, fn, BOOL_SIZE);
		}
		free(reg);
		if (reg2 != nullptr)
		{
			if (regRe) free(reg2);
			return CompilationToken{ reg2 };
		}
	}
	case ACCESSOR: {
		Accessor* acc = dynamic_cast<Accessor*>(v);
		AsmSize sz = getSize(acc->accessed, fn,false);
		AsmSize indexsz = getSize(acc->index, fn, false);

		saveScratch(fn);

		compileInstruction(MOV2, regs8[1], acc->accessed, fn, sz);
		fn->fbody << "push rcx" << endl << "xor rdx, rdx" << endl;

		compileInstruction(MOV2, D(indexsz), acc->index, fn, indexsz);
		fn->fbody << "pop rcx" << endl << "mov r8, " << to_string(sz.sub->sz) << endl << "call getelement" << endl;
		Register* reg = alloc(STRPTR_SIZE);
		fn->fbody << "mov " << reg->reg << ", rax" << endl;
		reg = realloc(*sz.sub);
		free(reg);

		restoreScratch(fn);
		return CompilationToken{ reg };
	}
	case ARRAY: {
		auto* ptr = dynamic_cast<Array*>(v);
		saveScratch(fn);
		auto reg = alloc(STRPTR_SIZE);
		fn->fbody << "mov rcx, " << ptr->values.size() << endl << "mov rdx, " << (int)ptr->size.sz << endl;
		fn->fbody << "call genlist" << endl << "mov " << reg->reg << ", rax" << endl;
		free(reg);
		restoreScratch(fn);

		return CompilationToken{ reg };
	}
	default: { break; }
	}
	aThrowError(UNKNOWN_VALUE, -1);
	return CompilationToken{ "" ,VOID_SIZE };
}

AsmSize getSize(Value* v, Func* fn, const bool inp) // NOLINT(*-no-recursion)
{
	switch (v->getType())
	{
	case COMPILETIME_TOKEN:return dynamic_cast<CompilationToken*>(v)->sz;
	case REGISTER: {
			return dynamic_cast<Register*>(v)->size;
	}
	case PTR: {
			return dynamic_cast<Pointer*>(v)->size;
	}
	case REFERENCE: {
			for (const auto & i : fn->varsStack)
			{
				if (i.name.value == dynamic_cast<Reference*>(v)->value) {
					return i.size;
				}
			}
			break;
	}
	case ARRAY: {
		return AsmSize{ 8, 255, &dynamic_cast<Array*>(v)->size };
	}
	case ACCESSOR:{
		return *getSize(dynamic_cast<Accessor*>(v)->accessed, fn, inp).sub;
	}
	case FUNC_CALL:
		{
			auto fc = dynamic_cast<FuncCall*>(v);
			if(fc->name.value == CAST && fc->params.size() == 2 && fc->params[0]->getType() == SIZE)
			{
				return dynamic_cast<Size*>(fc->params[0])->value;
			}
			for (const auto& fs: globalRefs)
			{
				if(fc->name.value == fs.name.value)
				{
					return fs.size;
				}
			}
			return VOID_SIZE;
		}
	case MULTI_OPERATION: {
			auto* mop = dynamic_cast<MultipleOperation*>(v);
			if (mop->op == OR || mop->op == AND) return BOOL_SIZE;
			if (!inp)
				if (
					mop->op == COMPARISON ||
					mop->op == NOT_EQUAL ||
					mop->op == GREATER_THAN ||
					mop->op == SMALLER_THAN ||
					mop->op == GREATER_THAN_EQUAL ||
					mop->op == SMALLER_THAN_EQUAL
					)
					return BOOL_SIZE;

			AsmSize sz = VOID_SIZE;

			for (const auto & operand : mop->operands) {
				const AsmSize osz = getSize(operand, fn, inp);
				if((osz.prec > sz.prec) || (osz.prec == sz.prec && osz.sz > sz.sz))sz = osz;
			}

			for (const auto & invoperand : mop->invoperands) {
				const AsmSize osz = getSize(invoperand, fn, inp);
				if((osz.prec > sz.prec) || (osz.prec == sz.prec && osz.sz > sz.sz))sz = osz;
			}
			mop->size = sz;
			return sz;

	}
	case UN_OPERATION:{
			const auto uop = dynamic_cast<UnaryOperation*>(v);
			switch (uop->op) {
			case NOT:return BOOL_SIZE;
			case BITWISE_NOT:return getSize(uop->right, fn, inp);
			default: break;
			}
			return getSize(uop->right, fn, inp);
	}
	case INT_STMT: return INT_SIZE;
	case FLOAT_STMT: return FLOAT_SIZE;
	case SHORT_STMT : return SHORT_SIZE;
	case DOUBLE_STMT: return DOUBLE_SIZE;
	case LONG_STMT: return LONG_SIZE;
	case STRING_STMT: return STRPTR_SIZE;
	case BOOL_STMT: return BOOL_SIZE;
	default: break;
	}

	aThrowError(UNKNOWN_VALUE, -1);
	return VOID_SIZE;
}