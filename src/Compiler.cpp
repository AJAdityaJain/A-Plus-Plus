#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, string base) {
	rr = RegisterRegister();
	File = ofstream(base+ "\\test.asm");
	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.data' data readable writeable\n";
	//DATA
	File << "version db '0.3.1'";
	//~DATA
	File << "\nsection '.text' code readable executable\n\n\n";

	for (Statement* st : tree) {
		switch (st->getType()) {
		case FUNC_DEFINITION: {
			Func* fn = (Func*)st;
			if (fn->name.value == MAIN) {
				File << "start:" << endl;

				prologue(fn);
				for (Statement* st : fn->body->code)
				{
					compile(st,fn);
				}
				epilogue(fn);

				File << "\n\ninvoke  exit, rax"<<endl;
			}
			break;
		}
		}
	}

	File << "\n\n\n\n\nsection '.idata' import data readable writeable\nlibrary kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'\nimport kernel,\exit,'ExitProcess'\nimport msvcrt,\printf, 'printf'";
	File.close();
}















void Compiler::compile(Statement* b, Func* fn) {
	switch (b->getType())
	{

	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;
		unsigned int sz = getSize(a->value,fn);

		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name.value == a->name.value && fn->varsStack[i]->size == sz) {

				compile("mov [rbp - " + to_string(fn->varsStack[i]->off) + "], {0}\n", a->value, fn,true);

				return;
			}

 
		if (sz == 1)
			compile(push1, a->value, fn, true);
		if (sz == 4)
			compile(push4, a->value, fn, true);

		fn->scopesStack.back()++;
		if (fn->varsStack.size() == 0)
			fn->varsStack.push_back(new Variable( sz, sz,a->name));
		else
			fn->varsStack.push_back(new Variable(fn->varsStack.back()->off + sz, sz,a->name ));


		return;
	}

	}
	
	std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA compiler.cpp";
	return;

}










void Compiler::compile(string f, Statement* b, Func* fn, bool hasptr) {
	switch (b->getType())
	{
	case NONE_STMT: {
		break;
	}
	case INT_STMT: {
		File << vformat(f, make_format_args((((Int*)b)->value).value));
		break;
	}
	case FLOAT_STMT: {
		break;
	}
	case DOUBLE_STMT: {
		break;
	}
	case STRING_STMT: {
		break;
	}
	case BIT_STMT: {
		break;
	}
	case REFERENCE: {
		Reference* id = (Reference*)b;
		for (int i = 0; i < fn->varsStack.size(); i++)
			if (fn->varsStack[i]->name .value == id->value.value) {
				if (fn->varsStack[i]->size == 4) {
					if (hasptr) {
						string reg = rr.alloc(4);

						File << "mov " + reg + ", dword[rbp - " + to_string(fn->varsStack[i]->off) + "]\n";
						File << vformat(f, make_format_args(reg));

						rr.free();
					}
					else
						File << vformat(f, make_format_args("dword[rbp - " + to_string(fn->varsStack[i]->off) + "]\n"));
				}
			}
		break;
	}
	case MULTI_OPERATION: {
		MultipleOperation* mo = (MultipleOperation*)b;
		switch (mo->op) {
		case PLUS: {


			string reg = rr.alloc(4);
			compile("mov " + reg + ", {0}\n", mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compile("add " + reg + ", {0}\n", mo->operands[i], fn);
			}

			for (size_t i = 0; i < mo->invoperands.size(); i++)
			{
				compile("sub " + reg + ", {0}\n", mo->invoperands[i], fn);
			}

			File << vformat(f, make_format_args(reg));
			rr.free();

			break;
		}
		case MULTIPLY: {

			string reg = rr.alloc(4);
			compile("mov " + reg + ", {0}\n", mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compile("IMUL " + reg + ", {0}\n", mo->operands[i], fn);
			}

			for (size_t i = 0; i < mo->invoperands.size(); i++)
			{
				File << pushebx;
				compile("mov ebx, {0}\n", mo->invoperands[i], fn);

				File << pushedx;
				File << "mov edx, 0" << endl;

				File << pusheax;
				File << "mov eax, " << reg << endl;

				File << "idiv ebx" << endl;

				File << "mov " << reg << ", eax" << endl;

				if (rr.getRegIdx() != 0) File << popeax;
				else File << pop;
				if (rr.getRegIdx() != 2) File << popedx;
				else File << pop;
				File << popebx;

			}

			File << vformat(f, make_format_args(reg));
			rr.free();

			break;
		}
		case OR: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;

			string reg = rr.alloc(1);

			for (Statement* s : mo->operands)
				compile(
					"mov "+reg+",{0}\ntest " + reg + ", " + reg + "\njnz LABOP_S" + to_string(lidx) + "\n", 
					s, 
					fn
				);


			File << "jmp LABOP_E" << lidx << endl << "LABOP_S" << lidx << ":" << endl << "\tmov " << reg << ",1" << endl << "LABOP_E" << lidx << ":" << endl;

			File << vformat(f, make_format_args(reg));

			rr.free();

			break;
		}
		case AND: {
			unsigned int lidx = operationLabelIdx;
			operationLabelIdx++;

			string reg = rr.alloc(1);

			for (Statement* s : mo->operands)
				compile(
					"mov " + reg + ",{0}\ntest " + reg + ", " + reg + "\njz LABOP_S" + to_string(lidx) + "\n",
					s,
					fn
				);


			File << "jmp LABOP_E" << lidx << endl << "LABOP_S" << lidx << ":" << endl << "\tmov " << reg << ",0" << endl << "LABOP_E" << lidx << ":" << endl;

			File << vformat(f, make_format_args(reg));

			rr.free();

			break;
		}
		case XOR: {

			string reg = rr.alloc(1);

			compile("mov " + reg + ", {0}\n", mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compile("xor " + reg + ", {0}\n", mo->operands[i], fn);
			}


			File << vformat(f, make_format_args(reg));
			rr.free();

			break;
		}
		}

		break;
	}
	case UN_OPERATION: {
		UnaryOperation* uo = (UnaryOperation*)b;

		switch (uo->op)
		{
		case NOT: {
			string reg = rr.alloc(1);
			compile("mov " + reg + ",{0}\n", uo->right, fn);
			File << "xor " << reg << ", 1" << endl;
			File << vformat(f, make_format_args(reg));
			break;
		}
		case NEGATIVE: {
			string reg = rr.alloc(4);
			compile("mov " + reg + ",{0}\n", uo->right, fn);
			File << "neg " << reg << endl;
			File << vformat(f, make_format_args(reg));
			break;
		}
		}
		rr.free();
		break;
	}
	default: {
		std::cout << "CC compiler.cpp";
	}
	}

}