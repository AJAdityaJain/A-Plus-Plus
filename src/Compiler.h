#pragma once

#include "DebugLogger.h"


inline Register* regs1[14] = {

		new Register("al",BOOL_SIZE,false),
		new Register("cl",BOOL_SIZE,false),
		new Register("dl",BOOL_SIZE,false),
		new Register("r8b",BOOL_SIZE,false),
		new Register("r9b",BOOL_SIZE,false),
		new Register("r10b",BOOL_SIZE,false),
		new Register("r11b",BOOL_SIZE,false),
		new Register("bl",BOOL_SIZE),
		new Register("sil",BOOL_SIZE),
		new Register("dil",BOOL_SIZE),
		new Register("r12b",BOOL_SIZE),
		new Register("r13b",BOOL_SIZE),
		new Register("r14b",BOOL_SIZE),
		new Register("r15b",BOOL_SIZE)
};

inline Register* regs2[14] = {

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

inline Register* regs4[14] = {
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

inline Register* regs8[14] = {

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

inline Register* regsXMM[16] = {

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

inline auto rsp = new Register("rsp", LONG_SIZE);
inline auto rbp = new Register("rbp", LONG_SIZE);

inline auto rspOff = vector<int>();
inline auto heapDels = vector<int>();

inline auto heaped = vector<Value*>();
inline auto saves = vector<Register*>();

inline vector<Variable> globalRefs;

inline ofstream File;
inline stringstream datarw;

inline unsigned int operationLabelIdx = 0;
inline unsigned int dataLabelIdx = 0;
inline int regIdx = -1;
inline int xmmIdx = -1;

AsmSize getSize(Value* v, Func* fn, bool inp);

Register* cast(Value* v, AsmSize from, AsmSize to, Func* fn);

CompilationToken compileValue(Value* v, Func* fn);

void compile(const vector<Statement*>& tree,int stack, const string& loc, const string& fasmdir);

void compileStatement(Statement* b, Func* fn);

void compileInstruction(INSTRUCTION i, Value* op, Value* op2, Func* fn, AsmSize sz);



inline [[nodiscard]] Register* A(const AsmSize sz) {
		switch (sz.sz) {
		case 1: return regs1[0];
		case 2: return regs2[0];
		case 4: return regs4[0];
		case 8: return regs8[0];
		default:aThrowError(OVERSIZED_VALUE, -1);
		}
		return nullptr;
	}
inline [[nodiscard]] Register* D(const AsmSize sz) {
		switch (sz.sz)
		{
		case 1: return regs1[2];
		case 2: return regs2[2];
		case 4: return regs4[2];
		case 8: return regs8[2];
		default:aThrowError(OVERSIZED_VALUE, -1);
		}
		return nullptr;
	}
inline[[nodiscard]] Register* realloc(const AsmSize sz)  {
	if (sz.prec > 0) {
		Register* r = regsXMM[xmmIdx];
		r->size = sz;
		return r;
	}
	switch (sz.sz) {
	case 1: return regs1[regIdx];
	case 2: return regs2[regIdx];
	case 4: return regs4[regIdx];
	case 8: return regs8[regIdx];
	default:aThrowError(OVERSIZED_VALUE, -1);
	}


	return nullptr;
}

inline Register* alloc(const AsmSize sz) {
		Register* rptr;
		if (sz.prec <= 0) {
			regIdx++;
			if (regIdx >= 14) aThrowError(OVERFLOW_REGISTER,-1);
			rptr = regs8[regIdx];
		}
		else {
			xmmIdx++;
			if (xmmIdx >= 16) aThrowError(OVERFLOW_REGISTER, -1);
			rptr = regsXMM[xmmIdx];
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
inline void free(const Register* r) {
		if (r->size.prec > 0)
			xmmIdx--;
		else
			regIdx--;
	}
static void addToData(const string& s) {
	datarw << s << ",0" << endl;
}

static void saveScratch(Func* fn) {
	for (int i = 0; i <= regIdx; i++) {
		if (!regs8[i]->isPreserved)
			fn->fbody << "push " << regs8[i]->reg << endl;
		else
			break;
	}
	for (int i = 0; i <= xmmIdx; i++) {
		if (!regsXMM[i]->isPreserved) {

			fn->fbody << "sub rsp, 8" << endl;
			fn->fbody << "movsd QWORD[rsp], " << regsXMM[i]->reg << endl;
		}
		else
			break;
	}

}
static void restoreScratch(Func* fn) {
	for (int i = xmmIdx; i >= 0 ; i--)
		if (!regsXMM[i]->isPreserved) {
			fn->fbody << "movsd " << regsXMM[i]->reg << ", QWORD[rsp]" << endl;
			fn->fbody << "add rsp, 8" << endl;
		}
		else
			break;

	for (int i = regIdx; i >= 0 ; i--)
		if (!regs8[i]->isPreserved)
			fn->fbody << "pop " << regs8[i]->reg << endl;
		else
			break;
}

static void prologue(Func* fn) {
		fn->scopesStack.push_back(0);
		rspOff.push_back(0);
		heapDels.push_back(0);
	}
inline void epiloguefree(Func* fn, const unsigned int exclude) {
		for (int i = 0; i < heapDels.back(); i++)
			if(heaped[i]->heapaddr != -1 && heaped[i]->heapaddr != exclude){
				auto [sz, prec] = getSize(heaped[i],fn, false);
				if(sz == STRPTR_SIZE.sz && prec == STRPTR_SIZE.prec)
				{
					saveScratch(fn);
					compileInstruction(MOV2, regs8[1], new Pointer("[rbp - " + to_string(heaped[i]->heapaddr) + "]", STRPTR_SIZE), fn, STRPTR_SIZE);
					fn->fbody << "call delstr" << endl;
					restoreScratch(fn);
				}
			}
		fn->fbody << "add rsp," << rspOff.back() << endl;}
static void epilogue(Func* fn) {
		for (int i = 0; i < heapDels.back(); i++)
				heaped.pop_back();
		heapDels.pop_back();

		for (int i = 0; i < fn->scopesStack.back(); i++)
			fn->varsStack.pop_back();
		fn->scopesStack.pop_back();

		rspOff.pop_back();
	}

static void savePreserved() {
		for (const Register* rptr : saves)
		{
			if (rptr->size.prec <= 0) {
				File << "push " << rptr->reg << endl;
			}
			else {

				File << "sub rsp, 8" << endl;
				File << "movsd QWORD[rsp], " << rptr->reg << endl;
			}
		}
	}
static void restorePreserved() {
		for (const Register* rptr : saves)
		{
			if (rptr->size.prec <= 0) {
				File << "pop " << rptr->reg << endl;
			}
			else {
				File << "movsd " << rptr->reg << ", QWORD[rsp]" << endl;
				File << "add rsp, 8" << endl;
			}
		}

		saves.clear();
		saves.shrink_to_fit();
	}