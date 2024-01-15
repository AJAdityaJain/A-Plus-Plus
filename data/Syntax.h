#pragma once

#include "Tokens.h"

using namespace std;


struct STATEMENT {virtual void print() {
		cout << "STMT_FAIL" << endl;
	}};
struct VALUED : STATEMENT {};

struct BLOCK : STATEMENT{vector<STATEMENT*> code;void print()override {
		cout << "Start Block" << endl;
 		for (STATEMENT* statement : code) {
			statement->print();
		}
		cout << "End Block" << endl;
	}};



struct INT_VAL : VALUED
{
	IntToken& value;
	INT_VAL(IntToken& val) : value(val) {}
	void print()override {
		cout << value.value;	
	}
};
struct FLOAT_VAL :VALUED
{
	FloatToken& value;
	FLOAT_VAL(FloatToken& val):value(val) {}
	void print()override {
		cout << value.value;
	}

};
struct DOUBLE_VAL : VALUED
{
	DoubleToken& value;
	DOUBLE_VAL(DoubleToken& val):value(val) {}
	void print()override {
		cout << value.value << "D";
	}
};
struct BIT_VAL : VALUED
{
	BitToken& value;
	BIT_VAL(BitToken& val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};
struct STRING_VAL : VALUED
{
	StringToken& value;
	STRING_VAL(StringToken& val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};
struct ID_VAL : VALUED
{
	IdentifierToken& value;
	ID_VAL(IdentifierToken& val) : value(val) {}
	void print() {
		cout << value.value;
	}


};





struct DEFINITION : STATEMENT {
	ID_VAL* name;
	VALUED* value;

	DEFINITION(ID_VAL* nam, VALUED* val) {
		name = nam;
		value = val;
	}
	
	void print() override {
		cout << "Defining " << name->value.value;
		cout << " as ";
		value->print();
		cout << endl;
	}
};
struct ASSIGNMENT : STATEMENT {
	ID_VAL* name;
	VALUED* value;

	ASSIGNMENT(ID_VAL* nam, VALUED* val) {
		name = nam;
		value = val;
	}

	void print()override {
		cout << "Reassigning " << name->value.value;
		cout << " to ";
		value->print();
		cout << endl;

	}
};


STATEMENT* parseStatement(vector<Token*> stack) {
	if (stack.size() == 0) throw invalid_argument("Empty statement");

	if (stack.size() == 1) {
		if(stack[0]->type == ID)		return new ID_VAL(*(IdentifierToken*)stack[0]);
		if(stack[0]->type == INT)		return new INT_VAL(*(IntToken*)stack[0]);
		if (stack[0]->type == FLOAT)	return new FLOAT_VAL(*(FloatToken*)stack[0]);
		if (stack[0]->type == DOUBLE)	return new DOUBLE_VAL(*(DoubleToken*)stack[0]);
		if (stack[0]->type == BIT)		return new BIT_VAL(*(BitToken*)stack[0]);
		if (stack[0]->type == STRING)	return new STRING_VAL(*(StringToken*)stack[0]);
		
	}

	///Variable Definition
	if (stack[0]->type == LET && stack[1]->type == ID && stack[2]->type == ASSIGN) {
		DEFINITION* def = new DEFINITION(
			new ID_VAL(*(IdentifierToken*)stack[1]),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
		);
		return def;
	}
	/// Variable Assignment
	if (stack[0]->type == ID && stack[1]->type == ASSIGN) {
		cout << getToken(stack[2]->type) << endl;
		ASSIGNMENT* def = new ASSIGNMENT(
			new ID_VAL(*(IdentifierToken*)stack[0]),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.end()))
		);
		return def;
	}
	/// If Statement
	if(stack[0]->type == IF && stack[stack.end-1] )

	throw invalid_argument("Invalid Statement");
}

BLOCK parseTree(vector<Token*> tokens) {

	BLOCK program = BLOCK();

	vector<Token*> stack;
	for (Token* token : tokens) {
		if (token->type == LINEEND) {
			program.code.push_back(parseStatement(stack));
			stack.clear();
		}
		else
			stack.push_back(token);
	}

	return program;
}

/*

struct IF : STATEMENT {
	VALUED* condition;
	BLOCK code;

	void print()override {
		cout << "if ";
		condition->print();
		cout << endl;
		code.print();
		cout << endl;

	}
};

struct IF_ELSE : STATEMENT
{
	VALUED* condition;
	BLOCK code;
	BLOCK elseCode;

	void print()override {
		cout << "if ";
		condition->print();
		cout << endl;
		code.print();
		cout << "else" << endl;
		elseCode.print();
		cout << endl;

	}

};

struct WHILE : STATEMENT {
	VALUED* condition;
	BLOCK code;

	void print()override {
		cout << "while ";
		condition->print();
		cout << endl;
		code.print();
		cout << endl;

	}
};
*/