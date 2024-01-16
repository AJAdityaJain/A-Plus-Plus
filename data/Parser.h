#pragma once

#include "Lexer.h"

using namespace std;


struct STATEMENT {
	virtual Tokens getType() {
		return UNKNOWN;
	}
	virtual void print() {
		cout << "STMT_FAIL" << endl;
	}

};
struct VALUED : STATEMENT {};







struct BLOCK : STATEMENT {
	vector<STATEMENT*> code; 

	Tokens getType()override {
		return SCOPE;
	}
	void print()override {
		cout << "Start Block" << endl;
		for (STATEMENT* statement : code) {
			statement->print();
		}
		cout << "End Block" << endl;
	}
};



struct INT_VAL : VALUED
{
	IntToken value;

	Tokens getType()override {
		return INT;
	}
	INT_VAL(IntToken val) : value(val) {}
	void print()override {
		cout << value.value;	
	}
};
struct FLOAT_VAL :VALUED
{
	FloatToken value;

	Tokens getType()override {
		return FLOAT;
	}

	FLOAT_VAL(FloatToken val):value(val) {}
	void print()override {
		cout << value.value;
	}
};
struct DOUBLE_VAL : VALUED
{
	DoubleToken value;

	Tokens getType()override {
		return DOUBLE;
	}

	DOUBLE_VAL(DoubleToken val):value(val) {}
	void print()override {
		cout << value.value << "d";
	}
};
struct BIT_VAL : VALUED
{
	BitToken value;

	Tokens getType()override {
		return BIT;
	}

	BIT_VAL(BitToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};
struct STRING_VAL : VALUED
{

	StringToken value;

	Tokens getType()override {
		return STRING;
	}

	STRING_VAL(StringToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};
struct ID_VAL : VALUED
{

	StringToken value;

	Tokens getType()override {
		return ID;
	}

	ID_VAL(StringToken val) : value(val) {}
	void print() {
		cout << value.value;
	}


};





struct DEFINITION : STATEMENT {
	ID_VAL* name;
	VALUED* value;

	Tokens getType()override {
		return DEFINE;
	}

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

	Tokens getType()override {
		return ASSIGN;
	}

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

	Tokens getType()override {
		return PRECEDER;
	}

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

	Tokens getType()override {
		return OPERATE;
	}

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

