#pragma once 

#include <fstream>
#include <format>
#include <sstream>

#include "Parser.h"

static  ofstream File;



struct Register : Value {
	const char* reg;
	bool isVol;
	AsmSize size;
	Register(const char* reg, AsmSize size, bool isVol = true) {
		this->reg = reg;
		this->size = size;
		this->isVol = isVol;
	};
	StatementType getType()override {
		return REGISTER;
	};

};


struct Pointer : Value {
	string ptr;
	AsmSize size;
	Pointer(string ptr,AsmSize size) {
		string prefix = "";
		switch (size.sz)
		{
		case 1:
			prefix = "byte ";
			break;
		case 2:
			prefix = "word ";
			break;
		case 4:
			prefix = "dword ";
			break;
		case 8:
			prefix = "qword ";
			break;
		case 16: {
			if(size.prec == 1)
				prefix = "dword ";
			else
				prefix = "qword ";
			break;
		}
		default:
			break;
		}
		this->ptr = prefix + ptr;
		this->size = size;
	}
	StatementType getType()override {
		return PTR;
	}
};
struct CompilationToken:Value {
	string line;
	CompilationTokenType type;
	StatementType getType()override {
		return _TOKEN;
	}
	CompilationToken(string line, CompilationTokenType type) {
		this->line = line;
		this->type = type;
	}
	CompilationToken(string line) {
		this->line = line;
		this->type = _NONE;
	}

};


struct RegisterRegister {

	Register* regs1[14] = {

		new Register("al",BIT_SIZE,false),
		new Register("cl",BIT_SIZE,false),
		new Register("dl",BIT_SIZE,false),
		new Register("r8b",BIT_SIZE,false),
		new Register("r9b",BIT_SIZE,false),
		new Register("r10b",BIT_SIZE,false),
		new Register("r11b",BIT_SIZE,false),
		new Register("bl",BIT_SIZE),
		new Register("sil",BIT_SIZE),
		new Register("dil",BIT_SIZE),
		new Register("r12b",BIT_SIZE),
		new Register("r13b",BIT_SIZE),
		new Register("r14b",BIT_SIZE),
		new Register("r15b",BIT_SIZE)
	};

	Register* regs2[14] = {

		new Register("ax",SHORT_SIZE,false),
		new Register("cx",SHORT_SIZE,false),
		new Register("dx",SHORT_SIZE,false),
		new Register("r8w",SHORT_SIZE,false),
		new Register("r9w",SHORT_SIZE,false),
		new Register("r10w",SHORT_SIZE,false),
		new Register("r11w",SHORT_SIZE,false),
		new Register("bx",SHORT_SIZE),
		new Register("si",SHORT_SIZE),
		new Register("di",SHORT_SIZE),
		new Register("r12w",SHORT_SIZE),
		new Register("r13w",SHORT_SIZE),
		new Register("r14w",SHORT_SIZE),
		new Register("r15w",SHORT_SIZE)
	};

	Register* regs4[14] = {
		new Register("eax",INT_SIZE,false),
		new Register("ecx",INT_SIZE,false),
		new Register("edx",INT_SIZE,false),
		new Register("r8d",INT_SIZE,false),
		new Register("r9d",INT_SIZE,false),
		new Register("r10d",INT_SIZE,false),
		new Register("r11d",INT_SIZE,false),
		new Register("ebx",INT_SIZE),
		new Register("esi",INT_SIZE),
		new Register("edi",INT_SIZE),
		new Register("r12d",INT_SIZE),
		new Register("r13d",INT_SIZE),
		new Register("r14d",INT_SIZE),
		new Register("r15d",INT_SIZE)
	};

	Register* regs8[14] = {

		new Register("rax",LONG_SIZE,false),
		new Register("rcx",LONG_SIZE,false),
		new Register("rdx",LONG_SIZE,false),
		new Register("r8" ,LONG_SIZE,false),
		new Register("r9" ,LONG_SIZE,false),
		new Register("r10",LONG_SIZE,false),
		new Register("r11",LONG_SIZE,false),
		new Register("rbx",LONG_SIZE),
		new Register("rsi",LONG_SIZE),
		new Register("rdi",LONG_SIZE),
		new Register("r12",LONG_SIZE),
		new Register("r13",LONG_SIZE),
		new Register("r14",LONG_SIZE),
		new Register("r15",LONG_SIZE)
	};

	Register* regsXMM[16] = {

		new Register("xmm0" ,DOUBLE_SIZE,false),
		new Register("xmm1" ,DOUBLE_SIZE,false),
		new Register("xmm2" ,DOUBLE_SIZE,false),
		new Register("xmm3" ,DOUBLE_SIZE,false),
		new Register("xmm4" ,DOUBLE_SIZE,false),
		new Register("xmm5" ,DOUBLE_SIZE,false),
		new Register("xmm6" ,DOUBLE_SIZE,true),
		new Register("xmm7" ,DOUBLE_SIZE,true),
		new Register("xmm8" ,DOUBLE_SIZE,true),
		new Register("xmm9" ,DOUBLE_SIZE,true),
		new Register("xmm10",DOUBLE_SIZE,true),
		new Register("xmm11",DOUBLE_SIZE,true),
		new Register("xmm12",DOUBLE_SIZE,true),
		new Register("xmm13",DOUBLE_SIZE,true),
		new Register("xmm14",DOUBLE_SIZE,true),
		new Register("xmm15",DOUBLE_SIZE,true)
	};

	Register* rsp = new Register("rsp", PTR_SIZE);
	vector<int> rspOff = vector<int>();

	int reg = -1;
	int regxmm = -1;

	//ONLY FOR GENERAL REGS
	int getRegIdx() const {
		return reg;
	}
	Register* A(AsmSize sz) const {
		switch (sz.sz) {
		case 1: return regs1[0];
		case 2: return regs2[0];
		case 4: return regs4[0];
		case 8: return regs8[0];
		}
		aThrowError(5, -1);
		return nullptr;
	}
	Register* D(AsmSize sz) const {
		switch (sz.sz) {
		case 1: return regs1[2];
		case 2: return regs2[2];
		case 4: return regs4[2];
		case 8: return regs8[2];
		}
		aThrowError(5, -1);
		return nullptr;
	}



	Register* alloc(AsmSize sz) {
		if (sz.prec == 0) {
			reg++;
			if (reg >= 14) aThrowError(6, -1);
		}
		else {
			regxmm++;
			if (reg >= 16) aThrowError(6, -1);
		}
		return realloc(sz);
	}
	Register* realloc(AsmSize sz) const {
		if (sz.prec != 0) {
			Register* r = regsXMM[regxmm];
			if (r->isVol) {
				File << "sub rsp, 8" << endl;
				File << "movsd QWORD[rsp], " << regsXMM[regxmm]->reg << endl;
			}
			r->size = sz;
			return r;
		}
		switch (sz.sz) {
		case 1: {
			Register* r = regs1[reg];
			if (r->isVol) {
				File << "push " << regs8[reg]->reg << endl;
			}
			return r;
		}
		case 2: {
			Register* r = regs2[reg];
			if (r->isVol) {
				File << "push " << regs8[reg]->reg << endl;
			}
			return r;

		};
		case 4: {
			Register* r = regs4[reg];
			if (r->isVol) {
				File << "push " << regs8[reg]->reg << endl;
			}
			return r;
		}
		case 8: {
			Register* r = regs8[reg];
			if (r->isVol) {
				File << "push " << regs8[reg]->reg << endl;
			}
			return r;
		}
		}

		aThrowError(5, -1);
		return nullptr;
	}
	void free(Register* r) {
		bool voliti = (r->isVol);
		if (r->size.prec != 0) {
			if (voliti) {
				File << "movsd " << r->reg << ", QWORD[rsp]" << endl;
				File << "add rsp, 8" << endl;
			}
			regxmm--;
		}
		else {
			if (voliti) 
				File << "pop " << regs8[reg]->reg << endl;
			reg--;
		}
	}

};

struct Compiler {

	
	unsigned int operationLabelIdx = 0;
	unsigned int dataLabelIdx = 0;
	RegisterRegister rr;

	stringstream data;

	Compiler() {
		addToData("intfmt db '%d',10");
		addToData("doublefmt db '%f',10");
		rr = RegisterRegister();
	}

	void addToData(string s) {
		data << "section '.data' data readable writeable\n";
		data << s << endl;
	}

	void functionPrologue()const {
		File << "push rbp" << endl;
		File << "mov rbp, rsp" << endl << endl;

	}

	void functionEpilogue()const {
		File << endl << "mov rsp, rbp" << endl;
		File << "pop rbp;" << endl;
	}

	void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		rr.rspOff.push_back(0);
	}

	void epilogue(Func* fn) {
		for (size_t i = 0; i < fn->scopesStack.back(); i++)
		{
			fn->varsStack.pop_back();
		}
		fn->scopesStack.pop_back();
		File << "add rsp,"<< rr.rspOff.back() << endl;
		rr.rspOff.pop_back();
		//File << "pop rbx" << endl;
	}



	void compile(vector<Statement*> tree, string base);

	void compileStatement(Statement* b, Func* fn);

	void compileInstruction(INSTRUCTION i, Value* op,
		Value* op2, Func* fn, AsmSize sz);

	Register* cast(Value* v, AsmSize from, AsmSize to, Func* fn);


	CompilationToken compileValue(Value* v, Func* fn);

	AsmSize getSize(Value* v, Func* fn, bool inp);
};