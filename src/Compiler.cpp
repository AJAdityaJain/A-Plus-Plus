#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, char* loc) {
	File = ofstream(loc);

	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.text' code readable executable\n\n\n";
	File <<"macro printnum[number]{\npush rax\npush rdx\npush r8\n\npush r10\npush r11\ninvoke printf, ns,number\npop r11\npop r10\npop r8\npop rdx\npop rax\n}\nmacro printstr[string]{\npush rax\npush rdx\npush r8\n\npush r10\npush r11\ninvoke printf, string\npop r11\npop r10\npop r8\npop rdx\npop rax\n}";

	for (Statement* st : tree) {
		switch (st->getType()) {
		case FUNC_DEFINITION: {
			Func* fn = (Func*)st;
			if (fn->name.value == MAIN) {
				File << "start:" << endl;

				functionPrologue();
				compileStatement(fn->body, fn);
				functionEpilogue();

				File << "\n\ninvoke  exit, rax"<<endl;
			}
			break;
		}
		}
	}
	File << "\nsection '.data' data readable writeable\n";
	File << data.str();

	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \\msvcrt, 'MSVCRT.DLL'\nimport kernel,\\exit,'ExitProcess'\nimport msvcrt,\\printf, 'printf'";
	File.close();
}


void Compiler::compileStatement(Statement* b, Func* fn) {
	switch (b->getType())
	{
	case FUNC_CALL: {
		FuncCall* fc = (FuncCall*)b;
		if (fc->name.value == PRINT) 
			for (Value* v : fc->params)
				if (v->getType() == STRING_STMT) 
					compileInstruction(PRINTSTR1, v, nullptr, fn);
				else
					compileInstruction(PRINTNUM1, v, nullptr, fn);
			
		
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

		File << label << ":" << endl;
		compileInstruction(CMP2, ifs->condition, new Bit(true), fn);
		compileInstruction(JNZ1, new CompilationToken(label2), nullptr, fn);

		compileStatement(ifs->ifBlock, fn);

		compileInstruction(JMP1, new CompilationToken(label), nullptr, fn);

		File << label2 << ":" << endl;
		return;
	}	
	case IF_STMT: {
		IfStatement* ifs = (IfStatement*)b;
		string label = ".LABBRNCH" + to_string(dataLabelIdx);
		dataLabelIdx++;
		compileInstruction(CMP2, ifs->condition, new Bit(true), fn);
		compileInstruction(JNZ1, new CompilationToken(label), nullptr, fn);

		compileStatement(ifs->ifBlock, fn);

		File << label << ":" << endl;
		return;
	}
	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;
		AsmSize sz = getSize(a->value, fn, false);

		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == a->name.value && fn->varsStack[i]->size == sz) {
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
					ins = IMUL2;
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

				compileInstruction(ins, new Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size), a->value, fn, true);
				return;
			}
	
		//FUTURE PROOF THIS
		compileInstruction(MOV2, new Pointer("[rsp-"+to_string(sz) + "]", sz), a->value, fn, true);
		compileInstruction(SUB2, rr.rsp, new Int(sz), fn);

		rr.rspOff.back() += sz;
		fn->scopesStack.back()++;

		if (fn->varsStack.size() == 0)
			fn->varsStack.push_back(new Variable(sz, sz, a->name));
		else
			fn->varsStack.push_back(new Variable(fn->varsStack.back()->off + sz, sz, a->name));


		return;
	}

	}
	
	aThrowError(2, -1);
	return;

}


void Compiler::compileInstruction(INSTRUCTION i, Value* op, Value* op2, Func* fn, bool o1isptr) {

	if (op == nullptr) {
		switch (i)
		{
		case CDQ0: {
			File << "cdq" << endl;
			break;
		}
		}

		return;
	}


	CompilationToken o1 = compileValue(op, fn);
	if (op2 == nullptr) {
		switch (i)
		{
		case IDIV1: {
			File << "idiv " << o1.line << endl;
			break;
		}
		case NEG1: {
			File << "neg " << o1.line << endl;
			break;
		}
		case JNZ1: {
			File << "jnz " << o1.line << endl;
			break;
		}
		case JZ1: {
			File << "jz " << o1.line << endl;
			break;
		}
		case JMP1: {
			File << "jmp " << o1.line << endl;
			break;
		}
		case SETE1: {
			File << "sete " << o1.line << endl;
			break;

		}
		case SETNE1: {
			File << "setne " << o1.line << endl;
			break;
		}
		case SETG1: {
			File << "setg " << o1.line << endl;
			break;

		}
		case SETGE1: {
			File << "setge " << o1.line << endl;
			break;
		}
		case SETL1: {
			File << "setl " << o1.line << endl;
			break;

		}
		case SETLE1: {
			File << "setle " << o1.line << endl;
			break;
		}
		case PRINTNUM1: {
			File << "printnum " << o1.line << endl;
			break;
		}
		case PRINTSTR1: {
			File << "printstr " << o1.line << endl;
			break;
		}
		}
		return;
	}

	CompilationToken o2 = compileValue(op2, fn);

	switch (i)
	{
	case MOV2: {
		if (o1isptr && o2.type == _PTR) {
			Reference* op2ptr = (Reference*)op2;
			Register r = rr.alloc(getSize(op, fn, false));

			compileInstruction(MOV2, &r, &o2, fn);
			compileInstruction(MOV2, &o1, &r, fn);
			rr.free(r);
			return;
		}
		else {
			File << "mov " << o1.line << ", " << o2.line << endl;
		}
		break;
	}
	case ADD2: {
		File << "add " << o1.line << ", " << o2.line << endl;
		break;
	}
	case SUB2: {
		File << "sub " << o1.line << ", " << o2.line << endl;
		break;
	}
	case IMUL2: {
		File << "imul " << o1.line << ", " << o2.line << endl;
		break;
	}
	case XOR2: {
		File << "xor " << o1.line << ", " << o2.line << endl;
		break;
	}
	case AND2: {
		File << "and " << o1.line << ", " << o2.line << endl;
		break;
	}
	case OR2: {
		File << "or " << o1.line << ", " << o2.line << endl;
		break;
	}
	case CMP2: {
		if (o1.type == _REGISTER || o2.type == _REGISTER) {
			File << "cmp " << o1.line << ", " << o2.line << endl;
		}
		else {
			Register r = rr.alloc(getSize(op, fn, false));
			compileInstruction(MOV2, &r, &o1, fn);
			compileInstruction(CMP2, &r, &o2, fn);
			rr.free(r);
		}
		break;
	}
	case TEST2: {
		File << "test " << o1.line << ", " << o2.line << endl;
		break;
	}
	}

	return;
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
		string label = ".LABDAT" + to_string(dataLabelIdx);
		operationLabelIdx++;
		data << label << " dd " << ((Float*)v)->value << endl;
		return { label };
	}

	case REFERENCE: {
		Reference* id = (Reference*)v;
		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == id->value.value) {
				Pointer p = Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size);
				return { p.ptr, _PTR};
			}
		break;
	}
	case UN_OPERATION: {
		UnaryOperation* uo = (UnaryOperation*)v;
		switch (uo->op)
		{
		case NOT: {
			Register reg = rr.alloc(BIT_SIZE);
			compileInstruction(MOV2, &reg, uo->right, fn);
			compileInstruction(XOR2, &reg, new Bit(true), fn);
			rr.free(reg);
			return { reg.reg,_REGISTER };
		}

		case NEGATIVE: {
			Register reg = rr.alloc(getSize(uo, fn, false));
			compileInstruction(MOV2, &reg, uo->right, fn);
			compileInstruction(NEG1, &reg, nullptr, fn);
			rr.free(reg);
			return { reg.reg,_REGISTER };
		}
		}
		break;
	}


	case MULTI_OPERATION: {
		MultipleOperation* mo = (MultipleOperation*)v;
		AsmSize sz = getSize(mo, fn, true);
		switch (mo->op) {

		case PLUS: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(ADD2, &reg, mo->operands[i], fn);
			for (size_t i = 0; i < mo->invoperands.size(); i++)
				compileInstruction(SUB2, &reg, mo->invoperands[i], fn);
			rr.free(reg);
			return { reg.reg ,_REGISTER };
		}
		case MULTIPLY: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(IMUL2, &reg, mo->operands[i], fn);

			Register Areg = rr.A(sz);

			for (size_t i = 0; i < mo->invoperands.size(); i++)
			{
				if (rr.getRegIdx() != 0) {
					compileInstruction(SUB2, rr.rsp, new Int(sz), fn);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), &Areg, fn, true);
					compileInstruction(MOV2, &Areg, &reg, fn);
				}
				Register diver = rr.alloc(sz);
				if (rr.getRegIdx() == 2) {
					Register diver2 = rr.alloc(sz);
					compileInstruction(MOV2, &diver2, mo->invoperands[i], fn);
					compileInstruction(CDQ0, nullptr, nullptr, fn);
					compileInstruction(IDIV1, &diver2, nullptr, fn);
					rr.free(diver2);
				}
				else {
					compileInstruction(MOV2, &diver, mo->invoperands[i], fn);
					compileInstruction(CDQ0, nullptr, nullptr, fn);
					compileInstruction(IDIV1, &diver, nullptr, fn);
				}
				rr.free(diver);

				if (rr.getRegIdx() != 0) {
					compileInstruction(MOV2, &reg, &Areg, fn);
					compileInstruction(MOV2, &Areg, new Pointer("[rsp]", sz), fn);
					compileInstruction(ADD2, rr.rsp, new Int(sz), fn);
				}
			}
			rr.free(reg);
			return { reg.reg,_REGISTER };
		}		
		case MODULO: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			Register Areg = rr.A(sz);
			Register Dreg = rr.D(sz);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				if (rr.getRegIdx() != 0) {
					compileInstruction(SUB2, rr.rsp, new Int(sz), fn);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), &Areg, fn, true);
					compileInstruction(MOV2, &Areg, &reg, fn);
				}
				Register diver = rr.alloc(sz);
				if (rr.getRegIdx() == 2) {
					Register diver2 = rr.alloc(sz);
					compileInstruction(MOV2, &diver2, mo->operands[i], fn);
					compileInstruction(CDQ0, nullptr, nullptr, fn);
					compileInstruction(IDIV1, &diver2, nullptr, fn);
					rr.free(diver2);
				}
				else {
					compileInstruction(MOV2, &diver, mo->operands[i], fn);
					compileInstruction(CDQ0, nullptr, nullptr, fn);
					compileInstruction(IDIV1, &diver, nullptr, fn);
				}
				compileInstruction(MOV2, &reg, &Dreg, fn);
				rr.free(diver);

				if (rr.getRegIdx() != 0) {
					compileInstruction(MOV2, &reg, &Areg, fn);
					compileInstruction(MOV2, &Areg, new Pointer("[rsp]", sz), fn);
					compileInstruction(ADD2, rr.rsp, new Int(sz), fn);
				}
			}
			rr.free(reg);
			return { reg.reg,_REGISTER };
		}

		case OR: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, &reg, s, fn);
				compileInstruction(TEST2, &reg, &reg, fn);
				compileInstruction(JNZ1, new CompilationToken(".LABOP_S" + to_string(lidx)), nullptr, fn);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx)), nullptr, fn);
			File << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, &reg, new Bit(true), fn);
			File << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return { reg.reg,_REGISTER };
		}
		case AND: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, &reg, s, fn);
				compileInstruction(TEST2, &reg, &reg, fn);
				compileInstruction(JZ1, new CompilationToken(".LABOP_S" + to_string(lidx)), nullptr, fn);
			}

			compileInstruction(JMP1, new CompilationToken(".LABOP_E" + to_string(lidx)), nullptr, fn);
			File << ".LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, &reg, new Bit(false), fn);
			File << ".LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return { reg.reg,_REGISTER };
		}
		case XOR: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, &reg, mo->operands[i], fn);
			}

			rr.free(reg);
			return { reg.reg ,_REGISTER };
		}		
		case BITWISE_AND: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(AND2, &reg, mo->operands[i], fn);
			}

			rr.free(reg);
			return { reg.reg , _REGISTER};
		}		
		case BITWISE_OR: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(OR2, &reg, mo->operands[i], fn);
			}

			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case COMPARISON: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETE1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case NOT_EQUAL: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETNE1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case GREATER_THAN: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETGE1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case GREATER_THAN_EQUAL: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETGE1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case SMALLER_THAN: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETL1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		case SMALLER_THAN_EQUAL: {
			Register reg = rr.alloc(getSize(mo, fn, true));
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++) {
				compileInstruction(CMP2, &reg, mo->operands[i], fn);
				reg = rr.realloc(getSize(mo, fn, false));
				compileInstruction(SETLE1, &reg, nullptr, fn);
			}
			rr.free(reg);
			return { reg.reg , _REGISTER};
		}
		}
		break;
	};
	};
}