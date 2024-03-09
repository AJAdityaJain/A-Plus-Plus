// ReSharper disable CppDFATimeOver
#include "Compiler.h"

void Compiler::compile(const vector<Statement*>& tree, const int stack, const string& loc, const string& fasmdir)
{
	globalRefs.emplace_back(nullptr, 0, INT_SIZE, IdentifierToken(LEN), 0);
	globalRefs.emplace_back(nullptr, 0, INT_SIZE, IdentifierToken(SIZEOF), 0);
	for (const auto st : tree)
		if (st->getType() == FUNC_DEFINITION)
		{
			const auto fns = dynamic_cast<Func*>(st);
			globalRefs.emplace_back(nullptr, 0, fns->returns, fns->name, 0);
		}
	File = ofstream(loc);

	File << "format PE64 console\nentry LABFUNC" + to_string(MAIN)+"\nstack "+to_string(stack*1024)+"\ninclude '" + fasmdir + "INCLUDE\\MACRO\\IMPORT64.INC'\nsection '.text' code readable executable\n\n\n";
	File <<"\nstrlen:\n	mov rdx ,0\n	lenloop:\n	inc rcx\n	inc rdx\n	mov al, byte[rcx]\n	test al, al\n	jnz lenloop\n	mov rax,rdx\nret\nstrcmp:\n	dec rcx\n	dec rdx\n	cmploop:\n		inc rcx\n		inc rdx\n		mov al, byte[rcx]\n		mov ah, byte[rdx]\n		cmp al, 0\n		je cmpsub\n		jmp cmpsubend\n		cmpsub:\n			cmp ah, 0\n			je cmpend\n		cmpsubend:\n		cmp al, ah\n	je cmploop\n	mov rax, 1  \nret\n	cmpend:\n	mov rax, 0  \nret\n\ngetchar:\nadd rcx, 8\ncmp edx, dword [rcx]\njge retnone\n	add rcx, rdx\n	add rcx,  8\n	mov al, byte [rcx]\n	ret\n	retnone:\n	mov al, -1\nret\n\naddchar:\n	sub rcx, 8\n		mov eax, dword [rcx +4]\n		sub eax, dword [rcx]\n		cmp eax, 8\n		jne recharskip\n			push rdx\n				mov eax, dword [ecx + 4]\n				imul rax, 2\n				mov dword[ecx + 4], eax\n				sub rsp, 32\n					mov r9, rax\n					mov r8, rcx\n					mov rdx, 0\n					mov rcx, [hHeap]\n					call [HeapReAlloc]\n				add rsp, 32\n				mov rcx, rax\n			pop rdx\n		recharskip:\n		mov rax, rcx	\n		mov ecx, dword [rax]\n		add rax, rcx\n		add rax, 8\n		mov byte [rax+1], 0 \n		mov byte [rax], dl	\n		sub rax, 8\n		sub rax, rcx\n		add ecx, 1\n		mov dword [rax], ecx\n		add rax, 8\nret\n\ngenstr:\n	push rcx\n	sub rsp, 32\n		mov r8, rcx\n		mov rcx, [hHeap]\n		mov rdx, 0\n		call [HeapAlloc]\n	add rsp, 32\n	pop rcx\n	mov dword [rax], 0\n	mov dword [rax+4], ecx\n	add rax, 8\nret\n\ngenstrlab: \n	push rcx\n		call strlen\n	pop rcx\n	push rcx\n		sub rsp, 32\n			mov r8, rax\n			mov rcx, [hHeap]\n			mov rdx, 0\n			call [HeapAlloc]\n		add rsp, 32\n	pop rcx\n\n	mov dword [rax], 0\n	mov dword [rax+4], ecx\n\n	add rax, 8\n	push rax\n		genloop:\n			mov dl, byte [rcx]\n			mov byte[rax], dl\n			inc rax\n			inc rcx\n			; mov dl, byte [rcx]\n			test dl, dl\n		jnz genloop\n	pop rax\n\nret\n\ndelstr:\n	sub rcx, 8\n	sub rsp, 32\n			mov r8, rcx \n			mov rdx, 0\n			mov rcx, [hHeap]\n			call [HeapFree]	\n	add rsp, 32\nret\n\n\n";

	for (Statement* st : tree)
		compileStatement(st,nullptr);

	File << "section '.data' data readable writeable\n";
	File << data.str();


	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \\msvcrt, 'MSVCRT.DLL'\nimport kernel,\\exit,'ExitProcess',\\beep,'Beep',\\HeapCreate, 'HeapCreate',\\HeapAlloc, 'HeapAlloc',\\HeapReAlloc, 'HeapReAlloc',\\HeapFree, 'HeapFree'\nimport msvcrt,\\printf, 'printf',\\scanf, 'scanf'";
	File.close();
}


void Compiler::compileStatement(Statement* b, Func* fn) { // NOLINT(*-no-recursion)
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
			File << "\nxor rcx, rcx\nxor rdx, rdx\nxor r8, r8\ncall [HeapCreate]\n mov [hHeap], rax" << endl;
		savePreserved();
		File << endl;
		File << fun->fbody.str();
		File << endl;
		File << "LABFUNCEND" + to_string(fun->name.value) << ":" << endl;
		restorePreserved();
		File << "mov rsp, rbp" << endl;
		if (fun->name.value == MAIN) File << "\nmov rcx,0\ncall [exit]" << endl;
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
						unsigned int returnvaraddrs = -1;
						if(inter->value != nullptr)
						{
							Register* returnreg = nullptr;
							if(fn->returns.prec <= 0)
							{
								switch(fn->returns.sz)
								{
								case 1:returnreg = rr.regs1[0];break;
								case 2:returnreg = rr.regs2[0];break;
								case 4:returnreg = rr.regs4[0];break;
								case 8:returnreg = rr.regs8[0];break;
								default: aThrowError(OVERSIZED_VALUE,-1);
								}
							}
							else returnreg = rr.regsXMM[0];

							compileInstruction(MOV2,returnreg,inter->value,fn,returnreg->size);

							switch(inter->value->getType()){
							case REFERENCE:
								{
									unsigned int name = dynamic_cast<Reference*>(inter->value)->value;
									for(const auto& var : fn->varsStack)
										if(var.name.value == name)
										{
											returnvaraddrs = var.off;
											break;
										}
									break;
								}
							default: break;
							}
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

			epiloguefree(fn, -1);
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
					AsmSize vsz = fn->varsStack[i].size;
					if(vsz.sz != sz.sz || vsz.prec != sz.prec)aThrowError(ASSIGNED_WRONG_TYPE, -1);
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

					compileInstruction(ins, new Pointer("[rbp - " + to_string(fn->varsStack[i].off) + "]", fn->varsStack[i].size), a->value, fn, sz);
					return;
				}


			//actually doing the changes on stack
			if(a->value->getType() == STRING_STMT)
			{
				a->value = new CompilationToken(compileValue(a->value,fn));
			}
			const int szonstack = static_cast<int>(ceil(static_cast<double>(sz.sz)/ALIGN)*ALIGN);
			fn->scopesStack.back()++;
			if (fn->varsStack.empty()) {
				compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
				compileInstruction(SUB2, rr.rsp, new Int(szonstack), fn, LONG_SIZE);
				rr.rspOff.back() += szonstack;
				fn->varsStack.emplace_back(a->value, sz.sz, sz, a->name);
				return;
			}
			const Variable v = fn->varsStack.back();
			if (v.size.sz == sz.sz && v.share > 0) {
				compileInstruction(MOV2, new Pointer("[rsp + "+to_string(v.share-sz.sz) + "]", sz), a->value, fn, sz);
				fn->varsStack.emplace_back(a->value,v.off+sz.sz, sz, a->name, v.share);
			}
			else {
				compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
				compileInstruction(SUB2, rr.rsp, new Int(szonstack), fn, LONG_SIZE);
				rr.rspOff.back() += szonstack;
				fn->varsStack.emplace_back(a->value,v.off + sz.sz + v.share, sz, a->name);
			}

			return;
		}
	default: compileValue(dynamic_cast<Value*>(b),fn);// aThrowError(UNKNOWN_STATEMENT, -1);
	}
}


void Compiler::compileInstruction(const INSTRUCTION i, Value* op, Value* op2, Func* fn, const AsmSize sz) // NOLINT(*-no-recursion)
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
		if (const AsmSize o1sz = getSize(op, fn, false); o1sz.prec != sz.prec && o1sz.prec + sz.prec > -1) {
			Register* r = cast(op, o1sz, sz, fn);
			op  = r;
			o1  = compileValue(op, fn);
			rr.free(r);
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
	auto suffix = "";
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
		if (const AsmSize o2sz = getSize(op2, fn, false); o2sz.prec != sz.prec && o2sz.prec + sz.prec > -1) {
			Register* r = cast(op2, o2sz, sz, fn);
			op2 = r;
			o2 = compileValue(op2,fn);
			rr.free(r);
		}


	switch (i)
	{
		case MOV2: {

				if (o1.type == COMPILETIME_PTR && o2.type == COMPILETIME_PTR) {
					Register* reg = rr.alloc(getSize(op, fn, false));

					compileInstruction(MOV2, reg, op2, fn, sz);
					compileInstruction(MOV2, op, reg, fn, sz);
					rr.free(reg);
					return;
				}
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
		case XOR2: {
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
				if(sz.prec == -1)
				{
					saveScratch(fn);
					compileInstruction(MOV2, rr.regs8[1], op, fn, sz);
					compileInstruction(MOV2, rr.regs8[2], op2, fn, sz);
					fn->fbody << "call strcmp" << endl;

					restoreScratch(fn);
					break;
				}
				if (o1.type == COMPILETIME_REGISTER || o2.type == COMPILETIME_REGISTER) {
					if (sz.prec == 0)
						fn->fbody << "cmp " << o1.line << ", " << o2.line << endl;
					else
						fn->fbody << "comi" << suffix << " " << o1.line << ", " << o2.line << endl;

				}
				else {
					Register* reg = rr.alloc(getSize(op, fn, false));
					compileInstruction(MOV2, reg, op, fn, sz);
					compileInstruction(CMP2, reg, op2, fn, sz);
					rr.free(reg);
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


Register* Compiler::cast(Value* v, const AsmSize from, const AsmSize to, Func* fn){ // NOLINT(*-no-recursion)
	Register* r1 = rr.alloc(from);
	compileInstruction(MOV2, r1, v, fn, from);
	bool regRe = false;
	Register* r2;
	if (from.prec == 0 || to.prec == 0) {
		regRe = true;
		r2 = rr.alloc(to);
	}
	else
		r2 = rr.realloc(to);


	auto cvt = "cvt";
	if (from.prec == 0 && to.prec == 1)
		cvt = "cvtsi2ss ";
	else if (from.prec == 0 && to.prec == 2)
		cvt = "cvtsi2sd ";
	else if (from.prec == 1 && to.prec == 0)
		cvt = "cvtss2si ";
	else if (from.prec == 1 && to.prec == 2)
		cvt = "cvtss2sd ";
	else if (from.prec == 2 && to.prec == 0)
		cvt = "cvtsd2si ";
	else if (from.prec == 2 && to.prec == 1)
		cvt = "cvtsd2ss ";
	if(from.sz == 2 )
		r1 = rr.regs4[rr.regIdx];

	fn->fbody << cvt << compileValue(r2,fn).line << ", " << compileValue(r1, fn).line << endl;
	if(regRe)
		rr.free(r1);
	return r2;
}

CompilationToken Compiler::compileValue(Value* v, Func* fn) { // NOLINT(*-no-recursion)

	switch (v->getType())
	{
	case FUNC_CALL:
		{
			Register* returnreg = rr.regs8[0];
			saveScratch(fn);
			const auto fc = dynamic_cast<FuncCall*>(v);
			switch  (fc->name.value ){
			case PRINT:{
				for (Value* params : fc->params) {
					CompilationToken cpar = compileValue(params,fn);
					compileInstruction(SUB2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
					switch (auto [sz, prec] = getSize(params, fn, false); sz)
					{
					case 2:
						{
							compileInstruction(MOV2, rr.regs8[1], new CompilationToken("intfmt", VOID_SIZE,COMPILETIME_PTR), fn, LONG_SIZE);
							compileInstruction(XOR2,	rr.regs8[2],rr.regs8[2],fn,LONG_SIZE);
							compileInstruction(MOV2, rr.regs2[2], &cpar, fn, SHORT_SIZE);
							fn->fbody << "call [printf]" << endl;
						}break;
					case 4:
						if(prec == 0)
						{
							compileInstruction(MOV2, rr.regs8[1], new CompilationToken("intfmt", VOID_SIZE,COMPILETIME_PTR), fn, LONG_SIZE);
							compileInstruction(MOV2, rr.regs4[2], &cpar, fn, INT_SIZE);
							fn->fbody << "call [printf]" << endl;
						}
						if(prec == 1)
						{
							compileInstruction(SUB2, rr.rsp, new Int(8), fn, LONG_SIZE);
							compileInstruction(MOV2, new Pointer("[rsp]", FLOAT_SIZE), rr.regsXMM[0], fn, FLOAT_SIZE);
							// compileInstruction(MOV2, rr.regsXMM[0], params, fn, FLOAT_SIZE);
							fn->fbody << "cvtss2sd xmm0, "<< cpar.line << endl;
							fn->fbody << "movq rdx, xmm0" << endl;
							compileInstruction(MOV2, rr.regs8[1], new CompilationToken("doublefmt", VOID_SIZE,COMPILETIME_PTR), fn, LONG_SIZE);
							fn->fbody << "call [printf]" << endl;
							compileInstruction(MOV2, rr.regsXMM[0], new Pointer("[rsp]", FLOAT_SIZE), fn, FLOAT_SIZE);
							compileInstruction(ADD2, rr.rsp, new Int(8), fn, LONG_SIZE);

							// fn->fbody << "printfloat " << ct.line << endl;
						}
						break;
					case 8:
						if(prec == -1)
						{
							compileInstruction(MOV2, rr.regs8[1], &cpar, fn, LONG_SIZE);
							fn->fbody << "call [printf]" << endl;
						}
						if(prec == 0)
						{
							compileInstruction(MOV2, rr.regs8[1], new CompilationToken("intfmt", VOID_SIZE,COMPILETIME_PTR), fn, LONG_SIZE);
							compileInstruction(MOV2, rr.regs8[2],&cpar , fn, LONG_SIZE);
							fn->fbody << "call [printf]" << endl;
						}
						if(prec == 2)
						{
							compileInstruction(SUB2, rr.rsp, new Int(8), fn, LONG_SIZE);
							compileInstruction(MOV2, new Pointer("[rsp]", DOUBLE_SIZE), rr.regsXMM[0], fn, DOUBLE_SIZE);
							compileInstruction(MOV2, rr.regsXMM[0], &cpar, fn, DOUBLE_SIZE);
							fn->fbody << "movq rdx, xmm0" << endl;
							compileInstruction(MOV2, rr.regs8[1], new CompilationToken("doublefmt", VOID_SIZE,COMPILETIME_PTR), fn, LONG_SIZE);
							fn->fbody << "call [printf]" << endl;
							compileInstruction(MOV2, rr.regsXMM[0], new Pointer("[rsp]", DOUBLE_SIZE), fn, DOUBLE_SIZE);
							compileInstruction(ADD2, rr.rsp, new Int(8), fn, LONG_SIZE);
						}
						break;

					default:break;
					}
					compileInstruction(ADD2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
				}
				break;
			}
			case SCAN: {
				for (Value* params : fc->params) {
				auto [sz, prec] = getSize(params, fn, false);
				CompilationToken ct = compileValue(params, fn);
				switch (sz) {
				case 8:
					{
						auto label = "SCANSTR"+to_string(dataLabelIdx);
						dataLabelIdx++;
						addToData(label+" db '????????????????????????????????????????????????????????????????',0");

						compileInstruction(SUB2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
						compileInstruction(MOV2, rr.regs8[1],new CompilationToken("scanfmt", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
						compileInstruction(MOV2, rr.regs8[2],new CompilationToken(label, STRPTR_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
						fn->fbody << "call [scanf]"<<endl;

						compileInstruction(MOV2, rr.regs8[1],new CompilationToken("charfmt", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
						compileInstruction(MOV2, rr.regs8[2],new CompilationToken("chardiscard", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
						fn->fbody << "call [scanf]"<<endl;

						compileInstruction(ADD2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
						const Register *reg = rr.alloc(STRPTR_SIZE);
						fn->fbody << "mov " << reg->reg << "," << label << endl;
						fn->fbody << "mov " << ct.line << "," << reg->reg << endl;
						rr.free(reg);
						break;
					}
				case 4:
					fn->fbody << "lea "<<rr.regs8[0]->reg<<"," << ct.line << endl;

					compileInstruction(SUB2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
					compileInstruction(MOV2, rr.regs8[1],new CompilationToken("intfmt", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
					compileInstruction(MOV2, rr.regs8[2],rr.regs8[0],fn,LONG_SIZE);
					fn->fbody << "call [scanf]"<<endl;
					compileInstruction(MOV2, rr.regs8[1],new CompilationToken("charfmt", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
					compileInstruction(MOV2, rr.regs8[2],new CompilationToken("chardiscard", VOID_SIZE,COMPILETIME_PTR),fn,LONG_SIZE);
					fn->fbody << "call [scanf]"<<endl;

					compileInstruction(ADD2, rr.rsp, new Int(0x20), fn, LONG_SIZE);

					// fn->fbody << "scanint " << ct.line << endl; break;
				default: break;
				}
				}
				break;
			}
			case BEEP:
				{
					compileInstruction(SUB2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
					if(fc->params.empty())
					{
						fn->fbody << "mov rcx, 640\nmov rdx, 350\n call [beep]"<< endl;
					}
					else if(fc->params.size() == 1)
					{
						auto [sz, prec] = getSize(fc->params[0], fn, false);
						auto ct = compileValue(fc->params[0],fn);
						if(sz == 4)
						{
							compileInstruction(MOV2, rr.regs4[1], &ct, fn, INT_SIZE);
							compileInstruction(MOV2, rr.regs4[2], new Int(350), fn, INT_SIZE);
							fn->fbody << "call [beep]" << endl;
						}
					}
					else if(fc->params.size() == 2)
					{
						auto [sz, prec] = getSize(fc->params[0], fn, false);
						auto ct = compileValue(fc->params[0],fn);
						if(sz == 4)
						{
							auto [sz2, prec2] = getSize(fc->params[1], fn, false);
						auto ct2 = compileValue(fc->params[1],fn);
							if(sz2 == 4)
							{
								compileInstruction(MOV2, rr.regs4[1], &ct, fn, INT_SIZE);
								compileInstruction(MOV2, rr.regs4[2], &ct2, fn, INT_SIZE);
								fn->fbody << "call [beep]" << endl;

							}
						}
					}
					compileInstruction(ADD2, rr.rsp, new Int(0x20), fn, LONG_SIZE);
					break;
				}
			case LEN:
				{
					compileInstruction(MOV2, rr.regs8[1], fc->params[0],fn, STRPTR_SIZE);
					returnreg = rr.regs4[0];
					fn->fbody << "	call strlen" << endl;
					Register* temp = rr.alloc(returnreg->size);
					compileInstruction(MOV2, temp, returnreg, fn, returnreg->size);
					rr.free(temp);
					returnreg = temp;
					break;
				}
				case SIZEOF:
				{
					returnreg = rr.regs4[0];
					int totalsz = 0;
					for (Value* params : fc->params) {
						auto [sz, prec] = getSize(params, fn, false);
						totalsz += sz;
					}
					compileInstruction(MOV2, returnreg, new Int(totalsz), fn, returnreg->size);
					break;
				}
			default:{

					int align = 0;
					for (int i = fc->params.size() - 1; i >= 0; --i) // NOLINT(*-narrowing-conversions)
					{
						auto [sz, prec] = getSize(fc->params[i], fn, false);
						align += sz;
					}
					int off = ((align / ALIGN) * ALIGN)+ALIGN - align;
					for (int i = fc->params.size() - 1; i >= 0; --i) // NOLINT(*-narrowing-conversions)
					{
						AsmSize sz = getSize(fc->params[i], fn, false);
						auto ct = compileValue(fc->params[i],fn);
						compileInstruction(MOV2, new Pointer("[rsp - " + to_string(off) + "]", sz), &ct, fn, sz);
						off += sz.sz;
					}
					const auto sz = getSize(fc,fn,false);
					if(sz.prec <= 0)
					{
						switch(sz.sz)
						{
						case 0:;break;
						case 1:returnreg = rr.regs1[0];break;
						case 2:returnreg = rr.regs2[0];break;
						case 4:returnreg = rr.regs4[0];break;
						case 8:returnreg = rr.regs8[0];break;
						default: aThrowError(OVERSIZED_VALUE,-1);
						}
					}
					else returnreg = rr.regsXMM[0];

					// off += returnreg->size.sz;
					if(off != 0)
						compileInstruction(SUB2, rr.rsp, new Int(off), fn, LONG_SIZE);

					fn->fbody << "	call LABFUNC" + to_string(fc->name.value) << endl;
					if((returnreg->size.prec <= 0 && rr.regIdx != -1)||(returnreg->size.prec > 0 && rr.xmmIdx != -1))
					{
						Register* temp = rr.alloc(sz);
						compileInstruction(MOV2, temp, returnreg, fn, sz);
						rr.free(temp);
						returnreg = temp;
					}

					if(off != 0)
						compileInstruction(ADD2, rr.rsp, new Int(off), fn, LONG_SIZE);



				break;
			}
			}
			restoreScratch(fn);
			return CompilationToken{ returnreg };
		}
	case COMPILETIME_TOKEN: dynamic_cast<CompilationToken*>(v);
	case PTR: return CompilationToken{dynamic_cast<Pointer*>(v) };
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
				case '\'':fmt << "',0x27,'" ;break;
				case '\n':fmt << "',0x0A,'" ;break;
				case '\r':fmt << "',0x0D,'" ;break;
				default:fmt << fpt->value[i];break;
				}
			}
			addToData(label + " db '" + fmt.str() + '\'');
			dataLabelIdx++;

			saveScratch(fn);
			fn->fbody << "mov rcx, " << label << endl;
			fn->fbody << "call genstrlab" << endl;
			fn->fbody << "sub rsp, " << ALIGN << endl;
			fn->fbody << "mov qword[rsp+8], rax" << endl;

			fpt->heapaddr = rr.rspOff.back()+ STRPTR_SIZE.sz;

			fn->varsStack.emplace_back(nullptr, rr.rspOff.back()+ STRPTR_SIZE.sz, STRPTR_SIZE, IdentifierToken()	);
			rr.rspOff.back() += ALIGN;
			rr.heapDels.back()++;
			rr.heaped.push_back(fpt);

			Register* reg = rr.regs8[0];
			if(rr.regIdx != -1)
			{
				reg = rr.alloc(STRPTR_SIZE);
				compileInstruction(MOV2, reg, rr.regs8[0], fn, STRPTR_SIZE);
				rr.free(reg);
			}
			restoreScratch(fn);


			return CompilationToken{ reg };
		}
			return CompilationToken(new Pointer("[rbp - "+to_string(fpt->heapaddr)+"]", LONG_SIZE));
	}
	case REFERENCE: {
		const auto* id = dynamic_cast<Reference*>(v);
		for (const auto & i : fn->varsStack)
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
			Register* reg = rr.alloc(BOOL_SIZE);
			compileInstruction(MOV2, reg, uo->right, fn,reg->size);
			compileInstruction(XOR2, reg, new Boolean(true), fn, reg->size);
			rr.free(reg);
			return CompilationToken{ reg };
		}

		case NEGATIVE: {
				Register* reg = rr.alloc(getSize(uo, fn, false));
				compileInstruction(MOV2, reg, uo->right, fn, reg->size);
				compileInstruction(NEG1, reg, nullptr, fn, reg->size);
				rr.free(reg);
				return CompilationToken{ reg };
		}
		case BITWISE_NOT: {
				Register* reg = rr.alloc(getSize(uo, fn, false));
				compileInstruction(MOV2, reg, uo->right, fn, reg->size);
				compileInstruction(NOT1, reg, nullptr, fn, reg->size);
				rr.free(reg);
				return CompilationToken{ reg };
		}
		default: break;
		}
		break;
	}
	case ARRAY:
		{
			const auto arr = dynamic_cast<Array*>(v);
			if(arr->values.empty())
				aThrowError(REDUNDANT_IMMUTABLE_ARRAY, -1);
			const auto sz = getSize(arr->values[0], fn, true);
			size_t i;
			size_t itlen = arr->values.size();
			for(i = 0; i < itlen; i++)
			{
				compileInstruction(MOV2, new Pointer("[rsp - " + to_string(sz.sz * (itlen-i)) + "]", sz), arr->values[i], fn, sz);
			}
			return CompilationToken{ "{array}", VOID_SIZE,COMPILETIME_IGNORE };
		}
	case ARRAY_ACCESS:{
		const auto aa = dynamic_cast<ArrayAccess*>(v);
			for (const auto& var : fn->varsStack)
				if (var.name.value == aa->name.value)
				{
					auto sz = getSize(aa, fn, false);
					if(aa->index->getType() == INT_STMT)
					{
						const auto ind = compileValue(aa->index, fn);
						return CompilationToken{ new Pointer("[rbp - "+to_string(var.off - sz.sz*dynamic_cast<Int*>(aa->index)->value)+"]", sz) };
					}
					Register* reg = rr.alloc(sz);
					Register* Areg = rr.A(sz);
					if (rr.regIdx != 0) {
						compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
						compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
						compileInstruction(MOV2, Areg, reg, fn, sz);
					}
					Register *AregSz = rr.A(getSize(aa->index,fn, false));
					compileInstruction(MOV2, AregSz,aa->index, fn, sz);
					compileInstruction(MUL2, Areg, new Int(sz.sz),fn,sz);
					compileInstruction(SUB2, Areg,new Int(static_cast<int>(var.off)),fn,sz);
					compileInstruction(ADD2,Areg,rr.rbp,fn,sz);
					if (rr.regIdx != 0) {
						compileInstruction(MOV2, reg, Areg, fn, sz);
						compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
						compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
						return CompilationToken{new  Pointer("["+ string(reg->reg) + "]",sz)};
					}
					return CompilationToken{ new Pointer("[" + string(Areg->reg) + "]",sz) };
					// rr.A()//DUMB
				}
	}
	case MULTI_OPERATION: {
		auto* mo = dynamic_cast<MultipleOperation*>(v);
		const AsmSize sz = getSize(mo, fn, true);
		switch (mo->op) {

		case PLUS: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn,sz);
			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(ADD2, reg, mo->operands[i], fn, sz);
			for (const auto & invoperand : mo->invoperands)
				compileInstruction(SUB2, reg, invoperand, fn, sz);
			rr.free(reg);
			return CompilationToken{ reg };
		}
		case MULTIPLY: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(MUL2, reg, mo->operands[i], fn, sz);

			if (!mo->invoperands.empty()) {
				if (sz.prec == 0) {
					Register* Areg = rr.A(sz);
					for (const auto & invoperand : mo->invoperands)
					{
						if (rr.regIdx != 0) {
							compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
							compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
							compileInstruction(MOV2, Areg, reg, fn, sz);
						}
						Register* diver = rr.alloc(sz);
						if (rr.regIdx == 2) {
							Register* diver2 = rr.alloc(sz);
							compileInstruction(MOV2, diver2, invoperand, fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver2, nullptr, fn, sz);
							rr.free(diver2);
						}
						else {
							compileInstruction(MOV2, diver, invoperand, fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver, nullptr, fn, sz);
						}
						rr.free(diver);

						if (rr.regIdx != 0) {
							compileInstruction(MOV2, reg, Areg, fn, sz);
							compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
							compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
						}
					}
				}
				else
					for (const auto & invoperand : mo->invoperands)
						compileInstruction(DIV2, reg, invoperand, fn, sz);
			}
			rr.free(reg);
			return CompilationToken{ reg };
		}
		case MODULO: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			Register* Areg = rr.A(sz);
			Register* Dreg = rr.D(sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				if (rr.regIdx != 0) {
					compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
					compileInstruction(MOV2, Areg, reg, fn, sz);
				}
				Register* diver = rr.alloc(sz);
				if (rr.regIdx == 2) {
					Register* diver2 = rr.alloc(sz);
					compileInstruction(MOV2, diver2, mo->operands[i], fn, sz);
					compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
					compileInstruction(IDIV1, diver2, nullptr, fn, sz);
					rr.free(diver2);
				}
				else {
					compileInstruction(MOV2, diver, mo->operands[i], fn, sz);
					compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
					compileInstruction(IDIV1, diver, nullptr, fn, sz);
				}
				compileInstruction(MOV2, reg, Dreg, fn, sz);
				rr.free(diver);

				if (rr.regIdx != 0) {
					compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
					compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
				}
			}
			rr.free(reg);
			return CompilationToken { reg };
		}

		case OR: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = rr.alloc(BOOL_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JNZ1, new CompilationToken(".LABOP_S" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Boolean(true), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return CompilationToken{ reg };
		}
		case AND: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = rr.alloc(BOOL_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JZ1, new CompilationToken(".LABOP_S" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx), VOID_SIZE), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Boolean(false), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return CompilationToken{ reg };
		}
		case XOR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg };
		}
		case BITWISE_AND: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(AND2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg };
		}
		case BITWISE_OR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(OR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg };
		}
		default: break;
		}


		INSTRUCTION cmp = {};
			if(mo->size.prec <= 0)
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

		Register* reg = rr.alloc(getSize(mo, fn, true));
		Register* reg2 = nullptr;
		bool regRe = false;
		compileInstruction(MOV2, reg, mo->operands[0], fn, sz);
		for (size_t i = 1; i < mo->operands.size(); i++) {
			compileInstruction(CMP2, reg, mo->operands[i], fn, sz);
			if (reg->size.prec == 0)
				reg2 = rr.realloc(getSize(mo, fn, false));
			else {
				regRe = true;
				reg2 = rr.alloc(getSize(mo, fn, false));
			}
			compileInstruction(cmp, reg2, nullptr, fn, BOOL_SIZE);
		}
		rr.free(reg);
			if(reg2 != nullptr)
			{
				if (regRe) rr.free(reg2);
				return CompilationToken { reg2 };}}
	default: break;}
	aThrowError(UNKNOWN_VALUE, -1);
	return CompilationToken{ "" ,VOID_SIZE};
}

AsmSize Compiler::getSize(Value* v, Func* fn, const bool inp) // NOLINT(*-no-recursion)
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

	case FUNC_CALL:
		{
			auto fc = dynamic_cast<FuncCall*>(v);
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
				if (osz.sz > sz.sz)
					sz = osz;
				if(osz.prec > sz.prec)
					sz = osz;
			}

			for (const auto & invoperand : mop->invoperands) {
				const AsmSize osz = getSize(invoperand, fn, inp);
				if (osz.sz > sz.sz)
					sz = osz;
				if (osz.prec > sz.prec)
					sz = osz;
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
	case ARRAY:{
			const auto arr = dynamic_cast<Array*>(v);
			if(!inp)
			{
				AsmSize sz{};
				sz.sz = static_cast<int>(arr->values.size()) * getSize(arr->values[0], fn, inp).sz;
				sz.prec = 0;
				return sz;
			}
			return getSize(arr->values[0],fn,inp);
	}
	case ARRAY_ACCESS:
		{
			const auto arr = dynamic_cast<ArrayAccess*>(v);
			for (const auto& var : fn->varsStack)
				if (var.name.value == arr->name.value)
				{
					const auto arrptr = static_cast<Array*>(var.val);
					return getSize(arrptr->values[0], fn, inp);
				}
		}
	default: break;
	}

	aThrowError(UNKNOWN_VALUE, -1);
	return VOID_SIZE;
}