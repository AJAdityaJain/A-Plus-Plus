#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, string base) {
	File = ofstream(base+ "\\test.asm");
	File << "format PE64 console\nentry start\n\ninclude 'WIN64A.inc'\n\nsection '.data' data readable writeable\n";

	//DATA
	File << "filler db 4";

	File << "\nsection '.text' code readable executable\n\n\n";

	//NASM//"section .text\n\textern GetStdHandle\n\textern WriteFile\n\textern ExitProcess\n\tglobal main\n";

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

				File << "\n\ninvoke  exit, 420"<<endl;
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
	case NONE_STMT: {
		break;
	}
	case CALL: {
		break;
	}
	case FUNC_DEFINITION: {
		break;
	}
	case DEFINITION: {
		Definition* d = (Definition*)b;
		unsigned int sz = getSize(d->value);// compile("mov eax,{0}\n", d->value, fn);

		if (sz == 4)
			compile(push4, d->value, fn);

		fn->scopesStack.back()++;
		if (fn->varsStack.size() == 0)
			fn->varsStack.push_back(Variable({ d->name.value,sz,sz }));
		else
			fn->varsStack.push_back(Variable({ d->name.value,fn->varsStack.back().off + sz,sz }));
	
		break;
	}
	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;

		for (Variable v : fn->varsStack)
			if (v.id == a->name->value.value) {
				compile(format("mov [rbp - {1}], {0}\n", "{0}", v.off), a->value, fn);
				break;
			}
		break;
	}
	case WHILE_STMT: {
		break;
	}
	case IF_STMT: {
		break;
	}
	case ELSE_STMT: {
		break;
	}
	case SCOPE: {
		break;
	}
	default: {
		cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA compiler.cpp";
	}
	}

}



void Compiler::compile(string f, Statement* b, Func* fn) {
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
	case ID_STMT: {
		for (Variable v : fn->varsStack)
			if (v.id == ((Identifier*)b)->value.value) {
				if (v.size == 4) {
					File << format("mov eax, dword[rbp - {0}]\n", v.off);
					File << vformat(f, make_format_args("eax"));
				}
			}
		break;
	}
	case MULTI_OPERATION: {
		MultipleOperation* mo = (MultipleOperation*)b;
		switch (mo->op) {
		case PLUS: {

			reg++;
			compile(format("mov {1}, {0}\n","{0}", regs4[reg]), mo->operands[0], fn);

			for (size_t i = 1; i < mo->operands.size(); i++)
			{
				compile(format("add {1}, {0}\n", "{0}", regs4[reg]), mo->operands[i], fn);
			}
			
			File << vformat(f, make_format_args(regs4[reg]));
			reg--;

			break;
		}
		default: {
			cout << "GG compiler.cpp";
			break;
		}
		}

		break;
	}
	case UN_OPERATION: {
		break;
	}
	default: {
		cout << "CC compiler.cpp";
	}
	}

}