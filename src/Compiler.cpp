#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, string loc) {
	File = ofstream(loc);

	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.text' code readable executable\n\n\n";
	File << "  macro callerpush{\n      push rax  \n      push rcx  \n      push rdx  \n      push r8  \n      push r9  \n      push r10  \n      push r11  \n}  \n    \n  macro callerpop{\n      pop r11  \n      pop r10  \n      pop r9  \n      pop r8  \n      pop rdx  \n      pop rcx  \n      pop rax  \n}\n    \n  macro printint[int]{\n      callerpush  \n      invoke printf, intfmt,int  \n      callerpop  \n}  \n    \n  macro printstr[string]{\n      callerpush  \n      invoke printf, string  \n      callerpop  \n}  \n    \n  macro printdouble[xmm]{\n      callerpush  \n        \n      sub rsp, 8  \n      movsd QWORD[rsp], xmm0  \n        \n      movsd xmm0, xmm  \n      movq rdx, xmm0          \n      invoke printf, doublefmt  \n    \n      movsd xmm0, QWORD[rsp]  \n      add rsp, 8  \n    \n      callerpop  \n}  \n  macro printfloat[dwrd]{\n		sub rsp, 8\n		movsd QWORD[rsp], xmm0\n		cvtss2sd xmm0, dwrd\n		printdouble xmm0\n		movsd xmm0, QWORD[rsp]\n		add rsp, 8\n}\n\n\n\n";

	for (Statement* st : tree)
		compileStatement(st,nullptr);

	File << "section '.data' data readable writeable\n";
	File << data.str();


	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \\msvcrt, 'MSVCRT.DLL'\nimport kernel,\\exit,'ExitProcess'\nimport msvcrt,\\printf, 'printf'";
	File.close();
}


void Compiler::compileStatement(Statement* b, Func* fn) {
	switch (b->getType())
	{
	case FUNC_DEFINITION: {
		Func* fun = (Func*)b;

		compileStatement(fun->body, fun);

		if (fun->name.value == MAIN) {
			File << "start:" << endl;
			File << "push rbp" << endl;
			File << "mov rbp, rsp" << endl;
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
	case FUNC_CALL: {
		FuncCall* fc = (FuncCall*)b;
		if (fc->name.value == PRINT)
			for (Value* v : fc->params) {
				AsmSize sz = getSize(v, fn, false);
				CompilationToken ct = compileValue(v, fn);
				switch (sz.prec) {
				case 0:
					switch (sz.sz) {
					case 8:
						fn->fbody << "printstr " << ct.line << endl; break;
					case 4:
						fn->fbody << "printint " << ct.line << endl; break;
					}break;
				case 1: fn->fbody << "printfloat " << ct.line << endl; break;
				case 2: fn->fbody << "printdouble " << ct.line << endl; break;
				}
			}
		else {
			saveScratch(fn);
			fn->fbody << "call LABFUNC" + to_string(fc->name.value) << endl;
			restoreScratch(fn);
		}

		return;
	}


	case SCOPE: {
		prologue(fn);

		CodeBlock* scope = (CodeBlock*)b;
		for (size_t i = 0; i < scope->code.size(); i++)
			compileStatement(scope->code[i], fn);

		epilogue(fn);

		return;
	}
	case WHILE_STMT: {
		IfStatement* ifs = (IfStatement*)b;
		string label = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;
		string label2 = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;

		fn->fbody << label << ":" << endl;
		compileInstruction(CMP2, ifs->condition, new Bit(true), fn, BIT_SIZE);
		compileInstruction(JNZ1, new CompilationToken(label2), nullptr, fn, BIT_SIZE);

		compileStatement(ifs->ifBlock, fn);

		compileInstruction(JMP1, new CompilationToken(label), nullptr, fn, BIT_SIZE);

		fn->fbody << label2 << ":" << endl;
		return;
	}
	case IF_STMT: {
		IfStatement* ifs = (IfStatement*)b;
		string label = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;
		compileInstruction(CMP2, ifs->condition, new Bit(true), fn, BIT_SIZE);
		compileInstruction(JNZ1, new CompilationToken(label), nullptr, fn, BIT_SIZE);

		compileStatement(ifs->ifBlock, fn);

		fn->fbody << label << ":" << endl;
		return;
	}
	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;
		AsmSize sz = getSize(a->value, fn, false);

		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == a->name.value && fn->varsStack[i]->size.sz == sz.sz) {
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
				}

				compileInstruction(ins, new Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size), a->value, fn, sz);
				return;
			}

		//FUTURE PROOF THIS

		fn->scopesStack.back()++;

		if (fn->varsStack.size() == 0) {
			compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
			compileInstruction(SUB2, rr.rsp, new Int(8), fn, PTR_SIZE);
			rr.rspOff.back() += ALIGN;
			fn->varsStack.push_back(new Variable(sz.sz, sz, a->name));
			return;
		}
		Variable* v = fn->varsStack.back();
		if (v->size.sz == sz.sz && v->share > 0) {
			compileInstruction(MOV2, new Pointer("[rsp + "+to_string(v->share-sz.sz) + "]", sz), a->value, fn, sz);
			fn->varsStack.push_back(new Variable(v->off+sz.sz, sz, a->name, v->share));
		}
		else {
			compileInstruction(MOV2, new Pointer("[rsp-" + to_string(sz.sz) + "]", sz), a->value, fn, sz);
			compileInstruction(SUB2, rr.rsp, new Int(ALIGN), fn, PTR_SIZE);
			rr.rspOff.back() += ALIGN;
			fn->varsStack.push_back(new Variable(v->off + sz.sz + v->share, sz, a->name));
		}

		return;
	}
	default: aThrowError(2, -1);
	}
}



void Compiler::compileInstruction(INSTRUCTION i, Value* op, Value* op2, Func* fn, AsmSize sz) {

	if (op == nullptr) {
		switch (i)
		{
		case CDQ0: fn->fbody << "cdq" << endl;break;
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
		}
		return;
	}

	AsmSize o1sz = getSize(op, fn, false);
	if (o1sz.prec != sz.prec) {
		Register* r = cast(op, o1sz, sz, fn);
		op  = r;
		o1  = compileValue(op, fn);
		rr.free(r);
	}
	AsmSize o2sz = getSize(op2, fn, false);
	if (o2sz.prec != sz.prec) {
		Register* r = cast(op2, o2sz, sz, fn);
		op2 = r;
		rr.free(r);
	}


	const char* suffix = "";
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
		if (o1.type == _PTR && o2.type == _PTR) {
			Reference* op2ptr = (Reference*)op2;
			Register* reg = rr.alloc(getSize(op, fn, false));

			compileInstruction(MOV2, reg, op2, fn, sz);
			compileInstruction(MOV2, op, reg, fn, sz);
			rr.free(reg);
			return;
		}
		else {
			fn->fbody << "mov" << suffix << " " << o1.line << ", " << o2.line << endl;
		}
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
		if (o1.type == _REGISTER || o2.type == _REGISTER) {
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
	}

	return;
}



Register* Compiler::cast(Value* v, AsmSize from, AsmSize to, Func* fn){
	Register* r1 = rr.alloc(from);
	compileInstruction(MOV2, r1, v, fn, from);
	bool regRe = false;
	Register* r2 = nullptr;
	if (from.prec == 0 || to.prec == 0) {
		regRe = true;
		r2 = rr.alloc(to);
	}
	else
		r2 = rr.realloc(to);



	const char* cvt = "cvt";
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

CompilationToken Compiler::compileValue(Value* v, Func* fn) {

	switch (v->getType())
	{
	case _TOKEN: return { ((CompilationToken*)v)->line };
	case PTR: return { ((Pointer*)v)->ptr , _PTR };
	case REGISTER: return { ((Register*)v)->reg, _REGISTER };
	case INT_STMT: return { to_string(((Int*)v)->value) };
	case BIT_STMT: return{ to_string((((Bit*)v)->value) ? 1 : 0) };
	case FLOAT_STMT: {
		Float* fpt = (Float*)v;
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dd " + to_string(fpt->value));
			dataLabelIdx++;
		}
		return { "DWORD [" + label + "]",_PTR };
	}
	case DOUBLE_STMT: {
		Double* fpt = (Double*)v;
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dq " + to_string(fpt->value));
			dataLabelIdx++;
		}
		return { "QWORD [" + label + "]",_PTR };
	}
	case STRING_STMT: {
		String* fpt = (String*)v;
		string label = "LABDAT" + to_string(fpt->label);
		if (fpt->label == -1) {
			fpt->label = dataLabelIdx;
			label = "LABDAT" + to_string(fpt->label);
			addToData(label + " dq '" + fpt->value+ '\'');
			dataLabelIdx++;
		}
		return { "QWORD [" + label + "]",_PTR };
	}
	case REFERENCE: {
		Reference* id = (Reference*)v;
		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == id->value) {
				Pointer p = Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size);
				return { p.ptr, _PTR };
			}
		break;
	}
	case UN_OPERATION: {
		UnaryOperation* uo = (UnaryOperation*)v;
		switch (uo->op)
		{
		case NOT: {
			Register* reg = rr.alloc(BIT_SIZE);
			compileInstruction(MOV2, reg, uo->right, fn,reg->size);
			compileInstruction(XOR2, reg, new Bit(true), fn, reg->size);
			rr.free(reg);
			return { reg->reg,_REGISTER };
		}

		case NEGATIVE: {
			Register* reg = rr.alloc(getSize(uo, fn, false));
			compileInstruction(MOV2, reg, uo->right, fn, reg->size);
			compileInstruction(NEG1, reg, nullptr, fn, reg->size);
			rr.free(reg);
			return { reg->reg,_REGISTER };
		}
		}
		break;
	}


	case MULTI_OPERATION: {
		MultipleOperation* mo = (MultipleOperation*)v;
		AsmSize sz = getSize(mo, fn, true);
		switch (mo->op) {

		case PLUS: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn,sz);
			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(ADD2, reg, mo->operands[i], fn, sz);
			for (size_t i = 0; i < mo->invoperands.size(); i++)
				compileInstruction(SUB2, reg, mo->invoperands[i], fn, sz);
			rr.free(reg);
			return { reg->reg ,_REGISTER };
		}
		case MULTIPLY: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(MUL2, reg, mo->operands[i], fn, sz);

			if (mo->invoperands.size() > 0) {
				if (sz.prec == 0) {
					Register* Areg = rr.A(sz);
					for (size_t i = 0; i < mo->invoperands.size(); i++)
					{
						if (rr.getRegIdx() != 0) {
							compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
							compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
							compileInstruction(MOV2, Areg, reg, fn, sz);
						}
						Register* diver = rr.alloc(sz);
						if (rr.getRegIdx() == 2) {
							Register* diver2 = rr.alloc(sz);
							compileInstruction(MOV2, diver2, mo->invoperands[i], fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver2, nullptr, fn, sz);
							rr.free(diver2);
						}
						else {
							compileInstruction(MOV2, diver, mo->invoperands[i], fn, sz);
							compileInstruction(CDQ0, nullptr, nullptr, fn, sz);
							compileInstruction(IDIV1, diver, nullptr, fn, sz);
						}
						rr.free(diver);

						if (rr.getRegIdx() != 0) {
							compileInstruction(MOV2, reg, Areg, fn, sz);
							compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
							compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
						}
					}
				}
				else
					for (size_t i = 0; i < mo->invoperands.size(); i++)
						compileInstruction(DIV2, reg, mo->invoperands[i], fn, sz);
			}
			rr.free(reg);
			return { reg->reg,_REGISTER };
		}
		case MODULO: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			Register* Areg = rr.A(sz);
			Register* Dreg = rr.D(sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				if (rr.getRegIdx() != 0) {
					compileInstruction(SUB2, rr.rsp, new Int(sz.sz), fn, sz);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, sz);
					compileInstruction(MOV2, Areg, reg, fn, sz);
				}
				Register* diver = rr.alloc(sz);
				if (rr.getRegIdx() == 2) {
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

				if (rr.getRegIdx() != 0) {
					compileInstruction(MOV2, reg, Areg, fn, sz);
					compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn, sz);
					compileInstruction(ADD2, rr.rsp, new Int(sz.sz), fn, sz);
				}
			}
			rr.free(reg);
			return { reg->reg,_REGISTER };
		}

		case OR: {
			unsigned int lidx = operationLabelIdx;
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
			return { reg->reg,_REGISTER };
		}
		case AND: {
			unsigned int lidx = operationLabelIdx;
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
			return { reg->reg,_REGISTER };
		}
		case XOR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return { reg->reg ,_REGISTER };
		}
		case BITWISE_AND: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(AND2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return { reg->reg , _REGISTER };
		}
		case BITWISE_OR: {
			Register* reg = rr.alloc(sz);
			compileInstruction(MOV2, reg, mo->operands[0], fn, sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(OR2, reg, mo->operands[i], fn, sz);
			}

			rr.free(reg);
			return { reg->reg , _REGISTER };
		}
		}


		INSTRUCTION cmp;
		switch (mo->op)
		{
		case COMPARISON:cmp = SETE1; break;
		case NOT_EQUAL:cmp = SETNE1; break;
		case GREATER_THAN:cmp = SETG1; break;
		case GREATER_THAN_EQUAL:cmp = SETGE1; break;
		case SMALLER_THAN:cmp = SETL1; break;
		case SMALLER_THAN_EQUAL:cmp = SETLE1; break;
		}

		Register* reg = rr.alloc(getSize(mo, fn, true));
		Register* reg2;
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
		if (regRe) rr.free(reg2);
		return { reg2->reg  , _REGISTER };


		break;
	};
	};
	aThrowError(5, -1);
	return { "" };
}

AsmSize Compiler::getSize(Value* v, Func* fn, bool inp) {
	switch (v->getType())
	{
	case REGISTER: {
		return ((Register*)v)->size;
	}
	case PTR: {
		return ((Pointer*)v)->size;
	}
	case REFERENCE: {
		for (size_t i = 0; i < fn->varsStack.size(); i++)
		{
			if (fn->varsStack[i]->name.value == ((Reference*)v)->value) {
				return fn->varsStack[i]->size;
			}
		}
		break;
	}

	case MULTI_OPERATION: {
		MultipleOperation* mop = (MultipleOperation*)v;
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

		for (size_t i = 0; i < mop->operands.size(); i++) {
			AsmSize osz = getSize(mop->operands[i], fn, inp);
			if (osz.sz > sz.sz)
				sz = osz;
			if(osz.prec > sz.prec)
				sz = osz;
		}

		for (size_t i = 0; i < mop->invoperands.size(); i++) {
			AsmSize osz = getSize(mop->invoperands[i], fn, inp);
			if (osz.sz > sz.sz)
				sz = osz;
			if (osz.prec > sz.prec)
				sz = osz;
		}
		mop->size = sz;
		return sz;

	}
	case UN_OPERATION: {
		UnaryOperation* uop = (UnaryOperation*)v;
		switch (uop->op) {
		case NEGATIVE:return getSize(uop->right, fn, inp);
		case POSITIVE:return getSize(uop->right, fn, inp);
		case NOT:return BIT_SIZE;
		case BITWISE_NOT:return getSize(uop->right, fn, inp);
		}

		break;
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
	}

	aThrowError(2, -1);
	return VOID_SIZE;
}