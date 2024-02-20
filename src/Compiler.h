#pragma once 

#include <fstream>
#include "DebugLogger.h"


struct Register final: Value {
	const char* reg;
	bool isPreserved;
	AsmSize size{};
	Register(const char* reg, const AsmSize size, const bool isPreserved = true) {
		this->reg = reg;
		this->size = size;
		this->isPreserved = isPreserved;
	}
	StatementType getType()override {
		return REGISTER;
	}

};


struct Pointer final: Value {
	string ptr;
	AsmSize size{};
	Pointer(const string& ptr,const AsmSize size) {
		string prefix;
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

struct CompilationToken final:Value {
	string line;
	CompilationTokenType type;
	StatementType getType()override {
		return COMPILETIME_TOKEN;
	}
	explicit CompilationToken(const string& line,const  CompilationTokenType type=COMPILETIME_NONE) {
		this->line = line;
		this->type = type;
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
		new Register("xmm6" ,DOUBLE_SIZE),
		new Register("xmm7" ,DOUBLE_SIZE),
		new Register("xmm8" ,DOUBLE_SIZE),
		new Register("xmm9" ,DOUBLE_SIZE),
		new Register("xmm10",DOUBLE_SIZE),
		new Register("xmm11",DOUBLE_SIZE),
		new Register("xmm12",DOUBLE_SIZE),
		new Register("xmm13",DOUBLE_SIZE),
		new Register("xmm14",DOUBLE_SIZE),
		new Register("xmm15",DOUBLE_SIZE)
	};

	Register* rsp = new Register("rsp", PTR_SIZE);
	Register* rbp = new Register("rbp", PTR_SIZE);
	vector<int> rspOff = vector<int>();

	vector<Register*> saves = vector<Register*>();
	vector<int> scratches = vector<int>();

	vector<int> regIdx = vector<int>();
	vector<int> xmmIdx = vector<int>();


	[[nodiscard]] Register* A(const AsmSize sz) const {
		switch (sz.sz) {
		case 1: return regs1[0];
		case 2: return regs2[0];
		case 4: return regs4[0];
		case 8: return regs8[0];
		default:aThrowError(5, -1);
		}
		return nullptr;
	}
	[[nodiscard]] Register* D(const AsmSize sz) const {
		switch (sz.sz)
		{
		case 1: return regs1[2];
		case 2: return regs2[2];
		case 4: return regs4[2];
		case 8: return regs8[2];
		default:		aThrowError(5, -1);
		}
		return nullptr;
	}


	Register* alloc(const AsmSize sz) {
		Register* rptr;
		if (sz.prec == 0) {
			regIdx.back()++;
			if (regIdx.back() >= 14) aThrowError(6, -1);
			rptr = regs8[regIdx.back()];
		}
		else {
			xmmIdx.back()++;
			if (xmmIdx.back() >= 16) aThrowError(6, -1);
			rptr = regsXMM[xmmIdx.back()];
		}

		if (rptr != nullptr) if (rptr->isPreserved) {
			bool found = false;
			for (const auto & save : saves)
				if (save == rptr) {
					found = true;
					break;
				}
			if (!found) {
				saves.push_back(rptr);
			}

		}

		return realloc(sz);
	}
	void free(const Register* r) {
		if (r->size.prec != 0)
			xmmIdx.back()--;
		else
			regIdx.back()--;
	}
	[[nodiscard]] Register* realloc(const AsmSize sz) const {
		if (sz.prec != 0) {
			Register* r = regsXMM[xmmIdx.back()];
			r->size = sz;
			return r;
		}
		switch (sz.sz) {
		case 1: return regs1[regIdx.back()];
		case 2: return regs2[regIdx.back()];
		case 4: return regs4[regIdx.back()];
		case 8: return regs8[regIdx.back()];
			default:aThrowError(5, -1);
		}


		return nullptr;
	}
	
};

struct Compiler {

	
	unsigned int operationLabelIdx = 0;
	unsigned int dataLabelIdx = 0;

	RegisterRegister rr;
	ofstream File;
	stringstream data;

	Compiler() {
		addToData("intfmt db '%d'");
		addToData("doublefmt db '%f'");
		addToData("scanfmt db '%[^',10,']s'");
		addToData("charfmt db '?'");
		rr = RegisterRegister();
	}

	void addToData(const string& s) {
		data << s << ",0" << endl;
	}
	void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		rr.rspOff.push_back(0);
		rr.regIdx.push_back(-1);
		rr.xmmIdx.push_back(-1);
	}
	void epilogue(Func* fn) {
		for (int i = 0; i < fn->scopesStack.back(); i++)
		{
			fn->varsStack.pop_back();
		}
		fn->fbody << "add rsp," << rr.rspOff.back() << endl;

		rr.regIdx.pop_back();
		rr.xmmIdx.pop_back();
		rr.rspOff.pop_back();
		fn->scopesStack.pop_back();

		fn->varsStack.shrink_to_fit();
		fn->scopesStack.shrink_to_fit();
	}


	void savePreserved() {
		for (const Register* rptr : rr.saves)
		{
			if (rptr->size.prec == 0) {
				File << "push " << rptr->reg << endl;
			}
			else {

				File << "sub rsp, 8" << endl;
				File << "movsd QWORD[rsp], " << rptr->reg << endl;
			}
		}
	}
	void restorePreserved() {
		for (const Register* rptr : rr.saves)
		{
			if (rptr->size.prec == 0) {
				File << "pop " << rptr->reg << endl;
			}
			else {
				File << "movsd " << rptr->reg << ", QWORD[rsp]" << endl;
				File << "add rsp, 8" << endl;
			}
		}

		rr.saves.clear();
		rr.saves.shrink_to_fit();
	}
	void saveScratch(Func* fn) {
		for (int i = 0; i <= rr.regIdx.back(); i++) {
			if (!rr.regs8[i]->isPreserved)
				fn->fbody << "push " << rr.regs8[i]->reg << endl;
			else
				break;
		}
		for (int i = 0; i <= rr.xmmIdx.back(); i++) {
			if (!rr.regsXMM[i]->isPreserved) {

				fn->fbody << "sub rsp, 8" << endl;
				fn->fbody << "movsd QWORD[rsp], " << rr.regsXMM[i]->reg << endl;
			}
			else
				break;
		}

		rr.scratches.push_back(rr.regIdx.back());
		rr.scratches.push_back(rr.xmmIdx.back());
		rr.regIdx.back() = -1;
		rr.xmmIdx.back() = -1;
	}
	void restoreScratch(Func* fn) {
		rr.xmmIdx.back() = rr.scratches.back();
		rr.scratches.pop_back();
		rr.regIdx.back() = rr.scratches.back();
		rr.scratches.pop_back();
		for (int i = rr.xmmIdx.back(); i >= 0 ; i--)
			if (!rr.regsXMM[i]->isPreserved) {
				fn->fbody << "movsd " << rr.regsXMM[i]->reg << ", QWORD[rsp]" << endl;
				fn->fbody << "add rsp, 8" << endl;
			}
			else
				break;

		for (int i = rr.regIdx.back(); i >= 0 ; i--)
			if (!rr.regs8[i]->isPreserved)
				fn->fbody << "pop " << rr.regs8[i]->reg << endl;
			else
				break;
	}











	void compile(const vector<Statement*>& tree, const string& loc);

	void compileStatement(Statement* b, Func* fn);

	void compileInstruction(INSTRUCTION i, Value* op, Value* op2, Func* fn, AsmSize sz);

	Register* cast(Value* v, AsmSize from, AsmSize to, Func* fn);


	CompilationToken compileValue(Value* v, Func* fn);

	static AsmSize getSize(Value* v, Func* fn, bool inp);
};