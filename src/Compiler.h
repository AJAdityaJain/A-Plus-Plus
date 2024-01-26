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

	//map<CompileTimeVar,unsigned int> varmap;

	void prologue(Func* fn){
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

	template<typename T> 
	void push4(T reg) {
		File << "sub rsp, 4" << endl;
		File << "mov dword[rsp], " << reg << endl;
	}


	void pop4(string reg) {
		File << "mov " << reg << ", dword[rsp]" << endl;
		File << "add rsp, 4" << endl;
	}

	void compile(vector<Statement*> tree, string s);

	int compile(Statement* b, Func* fn);

};
