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

struct PARENTHESIS : VALUED {
	VALUED* value;

	PARENTHESIS(VALUED* val) {
		value = val;
	}

	void print()override {
		cout << "(";
		value->print();
		cout << ")";
	}
};

struct OPERATION : VALUED {
	VALUED* left;
	VALUED* right;
	Tokens op;

	OPERATION(VALUED* left, Tokens op, VALUED* right) {
		this->left = left;
		this->right = right;
		this->op = op;
	}

	void print()override {
		left->print();
		cout << " " << getToken(op) << " ";
		right->print();
	}
};;


STATEMENT* parseStatement(vector<Token*> stack);

vector<STATEMENT*> parseStatements(vector<Token*> stack);

BLOCK* parseTree(vector<Token*> tokens);

