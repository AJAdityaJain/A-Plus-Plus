#pragma once 

#include <fstream>
#include <map>
#include <format>

#include "Parser.h"

//struct CompileTimeVar {
//	unsigned int var;
//	unsigned int size;
//};


struct RegisterRegister {

	string* regs1 = new string[]{
		"al", "bl",
		"cl", "dl",
		"sil", "dil",
		"r8b" , "r9b" ,
		"r10b", "r11b",
		"r12b", "r13b",
		"r14b", "r15b"
	};

	string* regs2 = new string[]{
		"ax", "bx",
		"cx", "dx",
		"si", "di",
		"r8w" , "r9w" ,
		"r10w", "r11w",
		"r12w", "r13w",
		"r14w", "r15w"
	};

	string* regs4 = new string[]{
		"eax", "ebx",
		"ecx", "edx",
		"esi", "edi",
		"r8d" , "r9d" ,
		"r10d", "r11d",
		"r12d", "r13d",
		"r14d", "r15d"
	};

	string* regs8 = new string[]{
		"rax", "rbx",
		"rcx", "rdx",
		"rsi", "rdi",
		"r8" , "r9" ,
		"r10", "r11",
		"r12", "r13",
		"r14", "r15"
	};

	int reg = -1;

	int getRegIdx() {
		return reg;
	}

	string* alloc(int sz) {
		reg++;
		if (reg > regs8->size()) aThrowError(6,-1);
		switch (sz ) {
		case 1:return &regs1[reg];
		case 2:return &regs2[reg];
		case 4:return &regs4[reg];
		case 8:return &regs8[reg];
		}

		aThrowError(5,-1);
	}

	void free() {
		reg--;
	}

};

struct Compiler {
	ofstream File;
	const char* pushedx = "sub rsp, 4\n\tmov dword[rsp], edx\n";
	const char* pusheax = "sub rsp, 4\n\tmov dword[rsp], eax\n";
	const char* pushebx = "sub rsp, 4\n\tmov dword[rsp], ebx\n";

	const char* pop = "add rsp, 4\n";
	const char* popedx = "mov edx, dword[rsp]\nadd rsp, 4\n";
	const char* popeax = "mov eax, dword[rsp]\nadd rsp, 4\n";
	const char* popebx = "mov ebx, dword[rsp]\nadd rsp, 4\n";


	const char* push4 = "sub rsp, 4\n\tmov dword[rsp], {0}\n";
	const char* pop4 = "mov {0}, dword[rsp]\nadd rsp, 4\n";
	RegisterRegister rr;
	
	void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		File << "push rbx" << endl;
		File << "push rsi" << endl;
		File << "push rdi" << endl;
		File << "push r12" << endl;
		File << "push r13" << endl;
		File << "push r14" << endl;
		File << "push r15" << endl;
		File << "push rbp" << endl;
		File << "mov rbp, rsp" << endl << endl;
	}

	void epilogue(Func* fn) {
		for (size_t i = 0; i < fn->scopesStack.back(); i++)
		{
			fn->varsStack.pop_back();
		}
		fn->scopesStack.pop_back();
		File << endl << "mov rsp, rbp" << endl;
		File << "pop rbp;" << endl;
		File << "pop r15" << endl;
		File << "pop r14" << endl;
		File << "pop r13" << endl;
		File << "pop r12" << endl;
		File << "pop rdi" << endl;
		File << "pop rsi" << endl;
		File << "pop rbx" << endl;
	}



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