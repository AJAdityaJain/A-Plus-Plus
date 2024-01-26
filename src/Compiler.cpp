#include "Compiler.h"

void Compiler::compile(vector<Statement*> tree, string base) {
	File = ofstream(base+ "\\test.asm");
	File << "section .text\n\textern GetStdHandle\n\textern WriteFile\n\textern ExitProcess\n\tglobal main\n";

	for (Statement* st : tree) {
		switch (st->getType()) {
		case FUNC_DEFINITION: {
			Func* fn = (Func*)st;
			if (fn->name.value == MAIN) {
				File << "main:" << endl;

				prologue(fn);
				for (Statement* st : fn->body->code)
				{
					compile(st,fn);
				}
				epilogue(fn);

				File << "mov rcx,2" << endl;
				File << "call  ExitProcess"<<endl;
			}
			break;
		}
		}
	}

	File.close();
}


int Compiler::compile(Statement* b, Func* fn) {
	switch (b->getType())
	{
	case NONE_STMT: {
		break;
	}
	case INT_STMT: {
		File << format("mov eax, {0}\n", (((Int*)b)->value).value);
		return 4;
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
				if(v.size == 4)
					File << format("mov eax, dword[rbp - {0}]\n", v.off);
				return v.size;
			}

	
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
		unsigned int sz = compile(d->value,fn);

		fn->scopesStack.back()++;
		if (fn->varsStack.size() == 0)
			fn->varsStack.push_back(Variable({ d->name.value,sz,sz }));
		else
			fn->varsStack.push_back(Variable({ d->name.value,fn->varsStack.back().off + sz,sz }));

		push4("eax");

		break;
	}
	case ASSIGNMENT: {
		Assignment* a = (Assignment*)b;
		compile(a->value, fn);
		for (Variable v : fn->varsStack)
			if (v.id == a->name->value.value)
				File << format("mov [rbp - {0}], eax\n", v.off);

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
	case BI_OPERATION: {
		break;
	}
	case UN_OPERATION: {
		break;
	}
	case SCOPE: {
		break;
	}
	case PARENTHESIS: {
		break;
	}
	default: {
		cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA compiler.cpp";
	}
	}

	return 0;
}