#pragma once 

#include <fstream>
#include <format>
#include <sstream>

#include "Parser.h"

static  ofstream File;

struct Register : Value{
	string reg;
	AsmSize size;
	int prec;
	bool isVol;

	//Register& operator & ();
	Register(string reg, AsmSize size, bool isVol= true, int prec = 0) {
		this->reg = reg;
		this->size = size;
		this->prec = prec;
		this->isVol = isVol;
	}
	StatementType getType()override {
		return REGISTER;
	}
	//string getmov() {
	//	if(size == FLOAT_SIZE) return "movss";
	//		
	//	return "mov";
	//}
};

struct Pointer : Value {
	string ptr;
	AsmSize size;
	Pointer(string ptr,AsmSize size) {
		string prefix = "";
		switch (size)
		{
		case BIT_SIZE:
			prefix = "byte";
			break;
		case SHORT_SIZE:
			prefix = "word";
			break;
		case INT_SIZE:
			prefix = "dword";
			break;
		case LONG_SIZE:
			prefix = "qword";
			break;
		case FLOAT_SIZE:
			prefix = "dword";
			break;
		//case DOUBLE_SIZE:
		//	break;
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

	Register regs1[14] = {

		Register("al",BIT_SIZE,false), Register("cl",BIT_SIZE,false),
		Register("dl",BIT_SIZE,false), Register("r8b",BIT_SIZE,false) ,
		Register("r9b" ,BIT_SIZE,false),Register("r10b",BIT_SIZE,false),
		Register("r11b",BIT_SIZE,false),Register("bl",BIT_SIZE),
		Register("sil",BIT_SIZE), Register("dil",BIT_SIZE),
		Register("r12b",BIT_SIZE), Register("r13b",BIT_SIZE),
		Register("r14b",BIT_SIZE), Register("r15b",BIT_SIZE) 
	};

	Register regs2[14] = {

		Register("ax",SHORT_SIZE,false), Register("cx",SHORT_SIZE,false),
		Register("dx",SHORT_SIZE,false), Register("r8w",SHORT_SIZE,false) ,
		Register("r9w" ,SHORT_SIZE,false),Register("r10w",SHORT_SIZE,false),
		Register("r11w",SHORT_SIZE,false),Register("bx",SHORT_SIZE),
		Register("si",SHORT_SIZE), Register("di",SHORT_SIZE),
		Register("r12w",SHORT_SIZE), Register("r13w",SHORT_SIZE),
		Register("r14w",SHORT_SIZE), Register("r15w",SHORT_SIZE)
	};

	Register regs4[14]=  {
		Register("eax",INT_SIZE,false), Register("ecx",INT_SIZE,false),
		Register("edx",INT_SIZE,false), Register("r8d",INT_SIZE,false) ,
		Register("r9d" ,INT_SIZE,false),Register("r10d",INT_SIZE,false),
		Register("r11d",INT_SIZE,false),Register("ebx",INT_SIZE),
		Register("esi",INT_SIZE), Register("edi",INT_SIZE),
		Register("r12d",INT_SIZE), Register("r13d",INT_SIZE),
		Register("r14d",INT_SIZE), Register("r15d",INT_SIZE)
	};

	Register regs8[14] = {

		Register("rax",LONG_SIZE,false), Register("rcx",LONG_SIZE,false),
		Register("rdx",LONG_SIZE,false), Register("r8",LONG_SIZE,false) ,
		Register("r9" ,LONG_SIZE,false),Register("r10",LONG_SIZE,false),
		Register("r11",LONG_SIZE,false),Register("rbx",LONG_SIZE),
		Register("rsi",LONG_SIZE), Register("rdi",LONG_SIZE),
		Register("r12",LONG_SIZE), Register("r13",LONG_SIZE),
		Register("r14",LONG_SIZE), Register("r15",LONG_SIZE)
	};

	Register* rsp = new Register("rsp",PTR_SIZE);
	vector<int> rspOff = vector<int>();

	int reg = -1;

	int getRegIdx() const {
		return reg;
	}
	void emptyReg() const {

		File << "xor " << regs8[reg].reg << "," << regs8[reg].reg << endl;
	}

	Register alloc(AsmSize sz) {
		reg++;
		if (reg > 13) aThrowError(6, -1);
		emptyReg();
		return realloc(sz);
	}

	Register realloc(AsmSize sz) const {

		switch (sz) {
		case BIT_SIZE: {
			Register r = regs1[reg];
			if (r.isVol) {
				File << "push " << regs8[reg].reg << endl;
			}
			return r;
		}
		case SHORT_SIZE: {
			Register r = regs2[reg];
			if (r.isVol) {
				File << "push " << regs8[reg].reg << endl;
			}
			return r;

		};
		case INT_SIZE: {
			Register r = regs4[reg];
			if (r.isVol) {
				File << "push " << regs8[reg].reg << endl;
			}
			return r;
		}
		case PTR_SIZE: {
			Register r = regs8[reg];
			if (r.isVol) {
				File << "push " << regs8[reg].reg << endl;
			}
			return r;
		}
		}

		aThrowError(5, -1);
		//return nullptr;
	}


	Register A(AsmSize sz) const {
		switch (sz) {
		case BIT_SIZE: return regs1[0];
		case SHORT_SIZE: return regs2[0];
		case INT_SIZE: return regs4[0];
		case PTR_SIZE: return regs8[0];
		}
		aThrowError(5, -1);
	}	
	Register D(AsmSize sz) const {
		switch (sz) {
		case BIT_SIZE: return regs1[2];
		case SHORT_SIZE: return regs2[2];
		case INT_SIZE: return regs4[2];
		case PTR_SIZE: return regs8[2];
		}
		aThrowError(5, -1);
	}


	void free(Register r) {
		if (r.isVol) {
			File << "pop " << regs8[reg].reg << endl;
		}
		reg--;
	}

};

struct Compiler {

	
	unsigned int operationLabelIdx = 0;
	unsigned int dataLabelIdx = 0;
	RegisterRegister rr;

	stringstream data;

	Compiler() {
		data << "ns db '%d',10\n";
		rr = RegisterRegister();
	}

	void functionPrologue() {
		File << "push rbp" << endl;
		File << "mov rbp, rsp" << endl << endl;

	}

	void functionEpilogue() {
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



	void compile(vector<Statement*> tree, char* base);

	void compileStatement(Statement* b, Func* fn);

	void compileInstruction(INSTRUCTION i, Value* op,
		Value* op2, Func* fn, bool o1isptr = false);

	CompilationToken compileValue(Value* v, Func* fn);

	AsmSize getSize(Value* v, Func* fn,bool inp) {
		switch (v->getType())
		{
		case REGISTER: {
			return ((Register*)v)->size;
		}
		case PTR: {
			return ((Pointer*)v)->size;
		}
		case REFERENCE: {
			for (size_t i = 0; i < fn->varsStack.size(); i++)
			{
				if (fn->varsStack[i]->name.value == ((Reference*)v)->value.value) {
					return fn->varsStack[i]->size;
				}
			}
			break;
		}

		case MULTI_OPERATION: {
			MultipleOperation* mop = (MultipleOperation*)v;
			if (mop->op == OR || mop->op == AND) return BIT_SIZE;
			if(!inp)
				if (
					mop->op == COMPARISON || 
					mop->op == NOT_EQUAL || 
					mop->op == GREATER_THAN || 
					mop->op == SMALLER_THAN || 
					mop->op == GREATER_THAN_EQUAL || 
					mop->op == SMALLER_THAN_EQUAL
					) 
					return BIT_SIZE;

			AsmSize sz = VOID_SIZE;

			for (size_t i = 0; i < mop->operands.size(); i++) {
				AsmSize osz = getSize(mop->operands[i], fn, inp);
				if (osz > sz)
					sz = osz;
			}

			for (size_t i = 0; i < mop->invoperands.size(); i++) {
				AsmSize osz = getSize(mop->invoperands[i], fn,inp);
				if (osz > sz)
					sz = osz;
			}
			mop->size = sz;
			return sz;

		}
		case UN_OPERATION: {
			UnaryOperation* uop = (UnaryOperation*)v;
			switch (uop->op) {
			case NEGATIVE:return getSize(uop->right,fn, inp);
			case POSITIVE:return getSize(uop->right, fn, inp);
			case NOT:return BIT_SIZE;
			case BITWISE_NOT:return getSize(uop->right, fn,inp);
			}

			break;
		}
		case INT_STMT: {
			return INT_SIZE;
		}
		case FLOAT_STMT: {
			return FLOAT_SIZE;
		}
		case DOUBLE_STMT: {
			return DOUBLE_SIZE;
		}
		case STRING_STMT: {
			return PTR_SIZE;
		}
		case BIT_STMT: {
			return BIT_SIZE;
		}
		}

		aThrowError(2, -1);
		return VOID_SIZE;
	}
};