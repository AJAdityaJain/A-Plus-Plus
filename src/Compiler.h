#pragma once 

#include <fstream>
#include <map>
#include <format>

#include "Parser.h"

//struct CompileTimeVar {
//	unsigned int var;
//	unsigned int size;
//};

struct Compiler {
	ofstream File;
	const char* pusheax = "sub rsp, 4\n\tmov dword[rsp], eax\n";
	const char* popebx = "mov ebx, dword[rsp]\nadd rsp, 4\n";

	const char* push4 = "sub rsp, 4\n\tmov dword[rsp], {0}\n";
	const char* pop4 = "mov {0}, dword[rsp]\nadd rsp, 4\n";

	int reg = -1;
	string* regs4 = new string[]{
		"eax","ecx" ,"edx",
		"esi","edi" ,"r8d",
		"r9d","r10d","r11d"
	};

	void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		File << "push rbp" << endl;
		File << "mov rbp, rsp" << endl;
	}

	void epilogue(Func* fn) {
		for (size_t i = 0; i < fn->scopesStack.back(); i++)
		{
			fn->varsStack.pop_back();
		}
		fn->scopesStack.pop_back();
		File << "mov rsp, rbp" << endl;
		File << "pop rbp;" << endl;
	}

	//template<typename T> 
	//void push4(T reg) {
	//	File << "sub rsp, 4" << endl;
	//	File << "mov dword[rsp], " << reg << endl;
	//}



	void compile(vector<Statement*> tree, string s);

	void compile(Statement* b, Func* fn);
	void compile(string f, Statement* b, Func* fn);

	int getSize(Statement* b) {
		switch (b->getType()) {
		case INT_STMT: return 4;
		case FLOAT_STMT: return 4;
		case STRING_STMT: return 8;
		case DOUBLE_STMT: return 8;
		case BIT_STMT: return 1;
		default: return 0;
		}
	}
};