// ReSharper disable CppDFATimeOver
#include "Compiler.h"

void Compiler::compile(const vector<Statement*>& tree, const string& loc)
{
	File = ofstream(loc);

	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.text' code readable executable\n\n\n";
	File <<
		// "macro callerpush{\npush rax\npush rcx\npush rdx\npush r8\npush r9\npush r10\npush r11\n}\n  "
		// "macro callerpop{\npop r11\npop r10\npop r9\npop r8\npop rdx\npop rcx\npop rax\n}\n  "
		"macro printint[int]{\ninvoke printf,intfmt,int\n}\n"
		"macro printstr[string]{\ninvoke printf, string\n}\n"
		"macro printdouble[xmm]{\nsub rsp, 8\nmovsd QWORD[rsp], xmm0\nmovsd xmm0, xmm\nmovq rdx, xmm0\ninvoke printf, doublefmt\nmovsd xmm0, QWORD[rsp]\nadd rsp, 8\n}\n"
		"macro printfloat[dwrd]{\nsub rsp, 8\nmovsd QWORD[rsp], xmm0\ncvtss2sd xmm0, dwrd\nprintdouble xmm0\nmovsd xmm0, QWORD[rsp]\nadd rsp, 8\n}\n"
		"macro scanstr[dwrd]{\ninvoke  scanf, scanfmt, dwrd\ninvoke  scanf, '%c', charfmt\n}\n"
		"macro scanint[dwrd]{\nlea rax, dwrd\ninvoke  scanf, intfmt , rax\n}\n"
		"macro strlen[ptr, ptr]{\npush rdx\npush rax\npush rcx\nmov rdx ,0\nmov rcx, ptr\n.loop:\ninc rcx\ninc rdx\nmov al, byte[rcx]\ntest al, al\njnz .loop\npush rdx\nadd rsp, 8\npop rcx\npop rax\npop rdx\nsub rsp, 32\npop reg\nadd rsp, 24\n}\n"
		"\n\n\n\n";

	for (Statement* st : tree)
		compileStatement(st,nullptr);

	File << "section '.data' data readable writeable\n";
	File << data.str();


	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \\msvcrt, 'MSVCRT.DLL'\nimport kernel,\\exit,'ExitProcess',\\beep,'Beep'\nimport msvcrt,\\printf, 'printf',\\scanf, 'scanf'";
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
		rr.regIdx.push_back(-1);
		auto regs = vector<Register*>();
		for (int i = 0; i < fun->params; i++)
		{
			auto a = dynamic_cast<Assignment*>(fun->body->code[i]);
			auto sz = dynamic_cast<Size*>(a->value)->value;

			printStatement(a);
			Register* reg = rr.alloc(sz);
			a->value = reg;
			regs.push_back(reg);
		}
		for (auto reg : regs)
			rr.free(reg);

		rr.regIdx.pop_back() ;
		compileStatement(fun->body, fun);

		if (fun->name.value == MAIN) {
			File << "start:" << endl;
			File << "push rbp" << endl;
			File << "mov rbp, rsp" << endl;
			File << "and rsp, 0xFFFFFFFFFFFFFFF0" << endl;
			savePreserved();
			File << endl;
			File << fun->fbody.str();
			File << endl;
			restorePreserved();
			File << "mov rsp, rbp" << endl;
			File << "pop rbp;" << endl;
			File << "\n\ninvoke  exit, rax" << endl;
		}
		else {
			File << "LABFUNC" + to_string(fun->name.value) << ":" << endl;
			File << "push rbp" << endl;
			File << "mov rbp, rsp" << endl;
			savePreserved();
			File << endl;
			File << fun->fbody.str();
			File << endl;
			restorePreserved();
			File << "mov rsp, rbp" << endl;
			File << "pop rbp;" << endl;
			File << "ret" << endl;
		}

		fun->fbody.str(std::string());
		fun->fbody.clear();

		return;
	}
	case FUNC_CALL:
		{
			saveScratch(fn);
			switch  (const auto fc = dynamic_cast<FuncCall*>(b); fc->name.value ){
			case PRINT:{
					for (Value* v : fc->params) {
						auto [sz, prec] = getSize(v, fn, false);
						CompilationToken ct = compileValue(v, fn);
						switch (prec) {
						case 0:
							switch (sz) {
						case 8:
							fn->fbody << "printstr " << ct.line << endl; break;
						case 4:
							fn->fbody << "printint " << ct.line << endl; break;
						default: break;
							}break;
						case 1: fn->fbody << "printfloat " << ct.line << endl; break;
						case 2: fn->fbody << "printdouble " << ct.line << endl; break;
						default:break;
						}
					}
					break;
			}
			case SCAN: {
					for (Value* v : fc->params) {
						auto [sz, prec] = getSize(v, fn, false);
						CompilationToken ct = compileValue(v, fn);
						switch (sz) {
						case 8:
							{
								auto label = "SCANSTR"+to_string(dataLabelIdx);
								dataLabelIdx++;
								addToData(label+" db 'Umm this text should not exist. i dont know why it is here? Help',0");
								fn->fbody << "scanstr " << label << endl;
								const Register *reg = rr.alloc(PTR_SIZE);
								fn->fbody << "mov " << reg->reg << "," << label << endl;
								fn->fbody << "mov " << ct.line << "," << reg->reg << endl;
								rr.free(reg);
								break;
							}
						case 4:
							fn->fbody << "scanint " << ct.line << endl; break;
						default: break;
						}
					}
					break;
			}
			case BEEP:
				{
					if(fc->params.empty())
					{
						fn->fbody << "invoke beep, 650, 350" << endl;
					}
					else if(fc->params.size() == 1)
					{
						auto [sz, prec] = getSize(fc->params[0], fn, false);
						CompilationToken ct = compileValue(fc->params[0], fn);
						if(sz == 4)
							fn->fbody << "invoke beep, " << ct.line << ", 100" << endl;
					}
					else if(fc->params.size() == 2)
					{
						auto [sz, prec] = getSize(fc->params[0], fn, false);
						CompilationToken ct = compileValue(fc->params[0], fn);
						if(sz == 4)
						{
							auto [sz2, prec2] = getSize(fc->params[1], fn, false);
							CompilationToken ct2 = compileValue(fc->params[1], fn);
							if(sz2 == 4)
								fn->fbody << "invoke beep, " << ct.line << ", " << ct2.line << endl;
						}
					}
					break;
				}
			default:{
					int i =0;
					int r = 0;
					int x = 0;
					for (auto par : fc->params)
					{
						AsmSize sz = getSize(par,fn,false);
						Register* reg;
						if(sz.prec == 0)
						{
							switch(sz.sz){
								case 1:reg = rr.regs1[r];break;
								case 2:reg = rr.regs2[r];break;
								case 4:reg = rr.regs4[r];break;
								case 8:reg = rr.regs8[r];break;
							default: aThrowError(5,5);
							}
							r++;
						}
						else
						{
							reg = rr.regsXMM[x];
							x++;
						}

						compileInstruction(MOV2,reg,par,fn,sz);
						i++;
					}
					fn->fbody << "call LABFUNC" + to_string(fc->name.value) << endl;
					break;
			}
			}
			restoreScratch(fn);

			return;
		}


	case SCOPE: {
		prologue(fn);

		for (const auto* scope = dynamic_cast<CodeBlock*>(b); const auto i : scope->code)
			compileStatement(i, fn);

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
		compileInstruction(CMP2, whils->condition, new Bit(true), fn, BIT_SIZE);
		compileInstruction(JNZ1, new CompilationToken(label2), nullptr, fn, BIT_SIZE);

		compileStatement(whils->whileBlock, fn);

		compileInstruction(JMP1, new CompilationToken(label), nullptr, fn, BIT_SIZE);

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
			compileInstruction(CMP2, ifs->condition, new Bit(true), fn, BIT_SIZE);
			compileInstruction(JNZ1, new CompilationToken(label), nullptr, fn, BIT_SIZE);

			compileStatement(ifs->ifBlock, fn);
			if(ifs->elseBlock != nullptr)
				compileInstruction(JMP1, new CompilationToken(label2), nullptr, fn, BIT_SIZE);

			fn->fbody << label << ":" << endl;
			if(ifs->elseBlock != nullptr)
			{
				compileStatement(ifs->elseBlock, fn);
				fn->fbody << label2 << ":" << endl;
			}
			return;
		}
	case ASSIGNMENT: {
		const auto* a = dynamic_cast<Assignment*>(b);
		const AsmSize sz = getSize(a->value, fn, false);

		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i].name.value == a->name.value ) {
				if(fn->varsStack[i].size.sz != sz.sz || fn->varsStack[i].size.prec != sz.prec)aThrowError(3, -1);
				INSTRUCTION ins = MOV2;
				switch (a->type)
				{
				case EQUALS: {
					ins = MOV2;
					break;
				}
				case PLUS_EQUAL: {
					ins = ADD2;
					break;
				}
				case MINUS_EQUAL: {
					ins = SUB2;
					break;
				}
				case MULTIPLY_EQUAL: {
					ins = MUL2;
					break;
				}
				case BITWISE_OR_EQUAL: {
					ins = OR2;
					break;
				}
				case BITWISE_AND_EQUAL: {
					ins = AND2;
					break;
				}
				default: break;
				}

				compileInstruction(ins, new Pointer("[rbp - " + to_string(fn->varsStack[i].off) + "]", fn->varsStack[i].size), a->value, fn, sz);
				return;
			}

		//FUTURE PROOF THIS

		const int szonstack = static_cast<int>(ceil(static_cast<double>(sz.sz)/ALIGN)*ALIGN);
		fn->scopesStack.back()++;
		if (fn->varsStack.empty()) {
			compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
			compileInstruction(SUB2, rr.rsp, new Int(szonstack), fn, PTR_SIZE);
			rr.rspOff.back() += szonstack;
			fn->varsStack.emplace_back(a->value, sz.sz, sz, a->name);
			return;
		}
		if (const Variable v = fn->varsStack.back(); v.size.sz == sz.sz && v.share > 0) {
			compileInstruction(MOV2, new Pointer("[rsp + "+to_string(v.share-sz.sz) + "]", sz), a->value, fn, sz);
			fn->varsStack.emplace_back(a->value,v.off+sz.sz, sz, a->name, v.share);
		}
		else {
			compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
			compileInstruction(SUB2, rr.rsp, new Int(szonstack), fn, PTR_SIZE);
			rr.rspOff.back() += szonstack;
			fn->varsStack.emplace_back(a->value,v.off + sz.sz + v.share, sz, a->name);
		}

		return;
	}
	default: aThrowError(2, -1);
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

	CompilationToken o1 = compileValue(op, fn);
	if (op2 == nullptr) {
		switch (i)
		{
		case IDIV1: fn->fbody << "idiv " << o1.line << endl;	break;
		case NEG1: fn->fbody << "neg " << o1.line << endl;break;
		case JNZ1: fn->fbody << "jnz " << o1.line << endl;break;
		case JZ1: fn->fbody << "jz " << o1.line << endl;break;
		case JMP1: fn->fbody << "jmp " << o1.line << endl;break;
		case SETE1: fn->fbody << "sete " << o1.line << endl;break;
		case SETNE1: fn->fbody << "setne " << o1.line << endl;break;
		case SETG1: fn->fbody << "setg " << o1.line << endl;break;
		case SETGE1: fn->fbody << "setge " << o1.line << endl;break;
		case SETL1: fn->fbody << "setl " << o1.line << endl;break;
		case SETLE1: fn->fbody << "setle " << o1.line << endl;break;
		default: break;
		}
		return;
	}

	if (const AsmSize o1sz = getSize(op, fn, false); o1sz.prec != sz.prec) {
		Register* r = cast(op, o1sz, sz, fn);
		op  = r;
		o1  = compileValue(op, fn);
		rr.free(r);
	}
	if (const AsmSize o2sz = getSize(op2, fn, false); o2sz.prec != sz.prec) {
		Register* r = cast(op2, o2sz, sz, fn);
		op2 = r;
		rr.free(r);
	}


	auto suffix = "";
	if (sz.prec == 1) {
		suffix = "ss";
	}
	else if (sz.prec == 2) {
		suffix = "sd";
	}

	CompilationToken o2 = compileValue(op2, fn);

	switch (i)
	{
	case MOV2: {
			if(o1.type == COMPILETIME_IGNORE || o2.type == COMPILETIME_IGNORE)
				return;
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
		if (o1.type == COMPILETIME_REGISTER || o2.type == COMPILETIME_REGISTER) {
			if (sz.prec == 0)
				fn->fbody << "cmp " << o1.line << ", " << o2.line << endl;
			else
				fn->fbody << "comi" << suffix << " " << o1.line << ", " << o2.line << endl;

		}
		else {
			Register* reg = rr.alloc(getSize(op, fn, false));
			compileInstruction(MOV2, reg, &o1, fn, sz);
			compileInstruction(CMP2, reg, &o2, fn, sz);
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

	fn->fbody << cvt << compileValue(r2,fn).line << ", " << compileValue(r1, fn).line << endl;
	if(regRe)
		rr.free(r1);
	return r2;
}

CompilationToken Compiler::compileValue(Value* v, Func* fn) { // NOLINT(*-no-recursion)

	switch (v->getType())
	{
	case COMPILETIME_TOKEN: return CompilationToken( dynamic_cast<CompilationToken*>(v)->line );
	case PTR: return CompilationToken{dynamic_cast<Pointer*>(v)->ptr , COMPILETIME_PTR };
	case REGISTER: return CompilationToken{ dynamic_cast<Register*>(v)->reg, COMPILETIME_REGISTER };
	case INT_STMT: return CompilationToken{ to_string(dynamic_cast<Int*>(v)->value) };
	case BIT_STMT: return CompilationToken{ to_string(dynamic_cast<Bit*>(v)->value ? 1 : 0) };
	case FLOAT_STMT: {
		auto* fpt = dynamic_cast<Float*>(v);
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dd " + to_string(fpt->value));
			dataLabelIdx++;
		}
		return CompilationToken{ "DWORD [" + label + "]",COMPILETIME_PTR };
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
		return CompilationToken{ "QWORD [" + label + "]",COMPILETIME_PTR };
	}
	case STRING_STMT: {
		auto* fpt = dynamic_cast<String*>(v);
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
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
		}
		return CompilationToken{ label };
	}
	case REFERENCE: {
		const auto* id = dynamic_cast<Reference*>(v);
		for (const auto & i : fn->varsStack)
			if (i.name.value == id->value) {
				const auto p = Pointer("[rbp - " + to_string(i.off) + "]", i.size);
				return CompilationToken{ p.ptr, COMPILETIME_PTR };
			}
		break;
	}
	case UN_OPERATION: {
		switch (auto* uo = dynamic_cast<UnaryOperation*>(v); uo->op)
		{
		case NOT: {
			Register* reg = rr.alloc(BIT_SIZE);
			compileInstruction(MOV2, reg, uo->right, fn,reg->size);
			compileInstruction(XOR2, reg, new Bit(true), fn, reg->size);
			rr.free(reg);
			return CompilationToken{ reg->reg,COMPILETIME_REGISTER };
		}

		case NEGATIVE: {
			Register* reg = rr.alloc(getSize(uo, fn, false));
			compileInstruction(MOV2, reg, uo->right, fn, reg->size);
			compileInstruction(NEG1, reg, nullptr, fn, reg->size);
			rr.free(reg);
			return CompilationToken{ reg->reg,COMPILETIME_REGISTER };
		}
		default: break;
		}
		break;
	}
	case ARRAY:
		{
			const auto arr = dynamic_cast<Array*>(v);
			if(arr->values.empty())
				aThrowError(101, -1);
			const auto sz = getSize(arr->values[0], fn, true);
			int i;
			int itlen = arr->values.size(); // NOLINT(*-narrowing-conversions)
			for(i = 0; i < itlen; i++)
			{
				compileInstruction(MOV2, new Pointer("[rsp - " + to_string(sz.sz * (itlen-i)) + "]", sz), arr->values[i], fn, sz);
			}
			return CompilationToken{ "{array}",COMPILETIME_IGNORE };
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
						return CompilationToken{  Pointer("[rbp - "+to_string(var.off - sz.sz*dynamic_cast<Int*>(aa->index)->value)+"]", sz).ptr,COMPILETIME_PTR };
					}
					Register* reg = rr.alloc(sz);
					Register* Areg = rr.A(sz);
					if (rr.regIdx.back() != 0) {
						compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
						compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
						compileInstruction(MOV2, Areg, reg, fn, sz);
					}
					Register *AregSz = rr.A(getSize(aa->index,fn, false));
					compileInstruction(MOV2, AregSz,aa->index, fn, sz);
					compileInstruction(MUL2, Areg, new Int(sz.sz),fn,sz);
					compileInstruction(SUB2, Areg,new Int(static_cast<int>(var.off)),fn,sz);
					compileInstruction(ADD2,Areg,rr.rbp,fn,sz);
					if (rr.regIdx.back() != 0) {
						compileInstruction(MOV2, reg, Areg, fn, sz);
						compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
						compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
						return CompilationToken{ Pointer("["+ string(reg->reg) + "]",sz).ptr, COMPILETIME_PTR };
					}
					return CompilationToken{ Pointer("[" + string(Areg->reg) + "]",sz).ptr, COMPILETIME_PTR };
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
			return CompilationToken{ reg->reg ,COMPILETIME_REGISTER };
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
						if (rr.regIdx.back() != 0) {
							compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
							compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
							compileInstruction(MOV2, Areg, reg, fn, sz);
						}
						Register* diver = rr.alloc(sz);
						if (rr.regIdx.back() == 2) {
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

						if (rr.regIdx.back() != 0) {
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
			return CompilationToken{ reg->reg,COMPILETIME_REGISTER };
		}
		case MODULO: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			Register* Areg = rr.A(sz);
			Register* Dreg = rr.D(sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				if (rr.regIdx.back() != 0) {
					compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
					compileInstruction(MOV2, Areg, reg, fn, sz);
				}
				Register* diver = rr.alloc(sz);
				if (rr.regIdx.back() == 2) {
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

				if (rr.regIdx.back() != 0) {
					compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
					compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
				}
			}
			rr.free(reg);
			return CompilationToken { reg->reg,COMPILETIME_REGISTER };
		}

		case OR: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JNZ1, new CompilationToken(".LABOP_S" + to_string(lidx)), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx)), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Bit(true), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return CompilationToken{ reg->reg,COMPILETIME_REGISTER };
		}
		case AND: {
			const unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register* reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, reg, s, fn, sz);
				compileInstruction(TEST2, reg, reg, fn, sz);
				compileInstruction(JZ1, new CompilationToken(".LABOP_S" + to_string(lidx)), nullptr, fn, sz);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx)), nullptr, fn, sz);
			fn->fbody << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, reg, new Bit(false), fn, sz);
			fn->fbody << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return CompilationToken{ reg->reg,COMPILETIME_REGISTER };
		}
		case XOR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg->reg ,COMPILETIME_REGISTER };
		}
		case BITWISE_AND: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(AND2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg->reg , COMPILETIME_REGISTER };
		}
		case BITWISE_OR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(OR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return CompilationToken { reg->reg , COMPILETIME_REGISTER };
		}
		default: break;
		}


		INSTRUCTION cmp = {};
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
			compileInstruction(cmp, reg2, nullptr, fn, sz);
		}
		rr.free(reg);
			if(reg2 != nullptr)
			{
				if (regRe) rr.free(reg2);
				return CompilationToken { reg2->reg  , COMPILETIME_REGISTER };}}
	default: break;}
	aThrowError(5, -1);
	return CompilationToken{ "" };
}

AsmSize Compiler::getSize(Value* v, Func* fn, const bool inp) // NOLINT(*-no-recursion)
{
	switch (v->getType())
	{
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

	case MULTI_OPERATION: {
			auto* mop = dynamic_cast<MultipleOperation*>(v);
			if (mop->op == OR || mop->op == AND) return BIT_SIZE;
			if (!inp)
				if (
					mop->op == COMPARISON ||
					mop->op == NOT_EQUAL ||
					mop->op == GREATER_THAN ||
					mop->op == SMALLER_THAN ||
					mop->op == GREATER_THAN_EQUAL ||
					mop->op == SMALLER_THAN_EQUAL
					)
					return BIT_SIZE;

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
			case NOT:return BIT_SIZE;
			case BITWISE_NOT:return getSize(uop->right, fn, inp);
			default: break;
			}
			return getSize(uop->right, fn, inp);
	}
	case INT_STMT: {
			return INT_SIZE;
	}
	case FLOAT_STMT: {
			return FLOAT_SIZE;
	}
	case DOUBLE_STMT: {
			return DOUBLE_SIZE;
	}
	case STRING_STMT: {
			return PTR_SIZE;
	}
	case BIT_STMT: {
			return BIT_SIZE;
	}
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

	aThrowError(2, -1);
	return VOID_SIZE;
}