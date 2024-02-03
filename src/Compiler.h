#pragma once 

#include <fstream>
#include <map>
#include <format>

#include "Parser.h"

static  ofstream File;


struct RegisterRegister {

	string* regs1 = new string[]{
		"al", "cl",
		"dl", "r8b" ,
		"r9b" ,"r10b",
		"r11b","bl",
		"sil", "dil",
		"r12b", "r13b",
		"r14b", "r15b" 
	};

	string* regs2 = new string[]{
		"ax", "cx",
		"dx", "r8w" ,
		"r9w" ,"r10w",
		"r11w","bx",
		"si", "di",
		"r12w", "r13w",
		"r14w", "r15w"
	};

	string* regs4 = new string[]{
		"eax", "ecx",
		"edx", "r8d" ,
		"r9d" ,"r10d",
		"r11d","ebx",
		"esi", "edi",
		"r12d", "r13d",
		"r14d", "r15d"
	};

	string* regs8 = new string[]{
		"rax", "rcx",
		"rdx","r8" ,
		"r9" ,"r10", 
		"r11","rbx",
		"rsi", "rdi",
		"r12", "r13",
		"r14", "r15"
	};

	int reg = -1;

	int getRegIdx() const {
		return reg;
	}
	void emptyReg() const {

		File << "xor " << regs8[reg] << "," << regs8[reg] << endl;
	}

	string alloc(int sz) {
		reg++;
		if (reg >= 8) {
			if (reg > regs8->size()) aThrowError(6, -1);
			File << "push " << regs8[reg] << endl;

		}
		emptyReg();
		switch (sz ) {
		case 1:return regs1[reg];
		case 2:return regs2[reg];
		case 4:return regs4[reg];
		case 8:return regs8[reg];
		}

		aThrowError(5,-1);
	}

	void free() {
		if (reg >= 8) File << "pop " << regs8[reg] << endl;
		reg--;
	}

};

struct Compiler {
	const char* pushedx = "sub rsp, 4\n\tmov dword[rsp], edx\n";
	const char* pusheax = "sub rsp, 4\n\tmov dword[rsp], eax\n";
	const char* pushebx = "sub rsp, 4\n\tmov dword[rsp], ebx\n";

	const char* pop = "add rsp, 4\n";
	const char* popedx = "mov edx, dword[rsp]\nadd rsp, 4\n";
	const char* popeax = "mov eax, dword[rsp]\nadd rsp, 4\n";
	const char* popebx = "mov ebx, dword[rsp]\nadd rsp, 4\n";


	const char* push1 = "sub rsp, 1\n\tmov byte[rsp], {0}\n";
	const char* push4 = "sub rsp, 4\n\tmov dword[rsp], {0}\n";
	const char* pop4 = "mov {0}, dword[rsp]\nadd rsp, 4\n";
	RegisterRegister rr;
	unsigned int operationLabelIdx = 0;
	
	void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		File << "push rbx" << endl;
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
		File << "pop rbx" << endl;
	}



	void compile(vector<Statement*> tree, string s);

	void compile(Statement* b, Func* fn);
	void compile(string f, Statement* b, Func* fn);

};