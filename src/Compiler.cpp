#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, string loc) {
	File = ofstream(loc);

	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.text' code readable executable\n\n\n";

	for (Statement* st : tree) {
		switch (st->getType()) {
		case FUNC_DEFINITION: {
			Func* fn = (Func*)st;
			if (fn->name.value == MAIN) {
				File << "start:" << endl;

				prologue(fn);
				for (Statement* st : fn->body->code)
				{
					compileStatement(st,fn);
				}
				epilogue(fn);

				File << "\n\ninvoke  exit, rax"<<endl;
			}
			break;
		}
		}
	}
	File << "\nsection '.data' data readable writeable\n";
	//DATA
	File << data.str();
	//~DATA

	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \\msvcrt, 'MSVCRT.DLL'\nimport kernel,\\exit,'ExitProcess'\nimport msvcrt,\\printf, 'printf'";
	File.close();
}


void Compiler::compileStatement(Statement* b, Func* fn) {
	switch (b->getType())
	{

	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;
		AsmSize sz = getSize(a->value, fn);

		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == a->name.value && fn->varsStack[i]->size == sz) {
				compileInstruction(MOV2, new Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size), a->value, fn, true);
				return;
			}

		//FUTURE PROOF THIS
		compileInstruction(MOV2, new Pointer("[rsp-"+to_string(sz) + "]", sz), a->value, fn, true);
		compileInstruction(SUB2, rr.rsp, new Int(sz), fn);

		fn->scopesStack.back()++;
		if (fn->varsStack.size() == 0)
			fn->varsStack.push_back(new Variable(sz, sz, a->name));
		else
			fn->varsStack.push_back(new Variable(fn->varsStack.back()->off + sz, sz, a->name));


		return;
	}

	}
	
	std::cout << "compiler.cpp";
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

		}

		return;
	}




	CompilationToken o2 = compileValue(op2, fn);
	if (o1isptr && o2.isPtr && i == MOV2) {
		Reference* op2ptr = (Reference*)op2;
		Register r = rr.alloc(getSize(op,fn));

		File << "mov " << r.reg << ", " << o2.line << endl;

		File << "mov " << o1.line << ", " << r.reg << endl;

		rr.free(r);
		return;
	}
	
	switch (i)
	{
	case MOV2: {
		File << "mov " << o1.line << ", " << o2.line << endl;
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
	case PTR: return { ((Pointer*)v)->ptr ,true };
	case REGISTER: return { ((Register*)v)->reg };
	case INT_STMT: return { to_string(((Int*)v)->value) };
	case BIT_STMT: return{ to_string((((Bit*)v)->value) ? 1 : 0)};
	case FLOAT_STMT: {
		string label = "LABDAT" + to_string(dataLabelIdx);
		operationLabelIdx++;
		data << label << " dd " << ((Float*)v)->value << endl;
		return { label };
	}

	case REFERENCE: {
		Reference* id = (Reference*)v;
		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == id->value.value) {
				Pointer p = Pointer("[rbp - " + to_string(fn->varsStack[i]->off) + "]", fn->varsStack[i]->size);
				return { p.ptr, true };
			}
		break;
	}
	case UN_OPERATION: {
		UnaryOperation* uo = (UnaryOperation*)v;
		switch (uo->op)
		{
		case NOT: {
			Register reg = rr.alloc(BIT_SIZE);
			compileInstruction(MOV2,&reg,uo->right,fn);
			compileInstruction(XOR2,&reg,new Int(1),fn);
			rr.free(reg);
			return { reg.reg };
		}

		case NEGATIVE: {
			Register reg = rr.alloc(getSize(uo, fn));
			compileInstruction(MOV2,&reg,uo->right,fn);
			compileInstruction(NEG1,&reg,nullptr,fn);
			rr.free(reg);
			return { reg.reg  };
		}
		}
		break;
	}


	case MULTI_OPERATION: {
		MultipleOperation* mo = (MultipleOperation*)v;
		AsmSize sz = getSize(mo, fn);
		switch (mo->op) {

		case PLUS: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(ADD2, &reg, mo->operands[i], fn);
			for (size_t i = 0; i < mo->invoperands.size(); i++)
				compileInstruction(SUB2, &reg, mo->invoperands[i], fn);
			rr.free(reg);
			return { reg.reg };
		}

		case MULTIPLY: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
				compileInstruction(IMUL2, &reg, mo->operands[i], fn);

			for (size_t i = 0; i < mo->invoperands.size(); i++)
			{
				Register* Areg;
				if (rr.getRegIdx() != 0) {
					Register A = rr.A(sz);
					Areg = &A;
					compileInstruction(SUB2, rr.rsp, new Int(sz), fn);
					compileInstruction(MOV2, new Pointer("[rsp]", sz), Areg, fn, true);
					compileInstruction(MOV2, Areg, &reg, fn);
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
					compileInstruction(MOV2, &reg, Areg, fn);
					compileInstruction(MOV2, Areg, new Pointer("[rsp]", sz), fn);
					compileInstruction(ADD2, rr.rsp, new Int(sz), fn);
				}
			}
			rr.free(reg);
			return { reg.reg };
		}
		case OR: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, &reg, s, fn);
				compileInstruction(TEST2, &reg, &reg, fn);
				compileInstruction(JNZ1, new CompilationToken("LABOP_S" + to_string(lidx)), nullptr, fn);
			}

			compileInstruction(JMP1, new CompilationToken("LABOP_E" + to_string(lidx)), nullptr, fn);
			File << "LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, &reg, new Int(1), fn);
			File << "LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return { reg.reg };
		}
		case AND: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;
			Register reg = rr.alloc(BIT_SIZE);
			for (Value* s : mo->operands) {
				compileInstruction(MOV2, &reg, s, fn);
				compileInstruction(TEST2, &reg, &reg, fn);
				compileInstruction(JZ1, new CompilationToken("LABOP_S" + to_string(lidx)), nullptr, fn);
			}

			compileInstruction(JMP1, new CompilationToken("LABOP_E" + to_string(lidx)), nullptr, fn);
			File << "LABOP_S" << lidx << ":" << endl;
			compileInstruction(MOV2, &reg, new Int(0), fn);
			File << "LABOP_E" << lidx << ":" << endl;

			rr.free(reg);
			return { reg.reg };
		}

		case XOR: {
			Register reg = rr.alloc(sz);
			compileInstruction(MOV2, &reg, mo->operands[0], fn);
		
			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compileInstruction(XOR2, &reg, mo->operands[i], fn);
			}

			rr.free(reg);
			return { reg.reg };
		}
		}
		break;
	};
	};
}