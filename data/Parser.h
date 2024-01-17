#pragma once

#include "Lexer.h"

using namespace std;


struct Statement {
	virtual TokenType getType() {
		return NONE;
	}
	virtual void print() {
		cout << "STMT_FAIL" << endl;
	}

};

struct VALUED : Statement {};







struct CodeBlock : Statement {
	vector<Statement*> code; 

	TokenType getType()override {
		return _SCOPE;
	}
	void print()override {
		cout << "Start Block" << endl;
		for (Statement* statement : code) {
			statement->print();
		}
		cout << "End Block" << endl;
	}
};



struct Int : VALUED
{
	IntToken value;

	TokenType getType()override {
		return INT;
	}
	Int(IntToken val) : value(val) {}
	void print()override {
		cout << value.value;	
	}
};
struct Float :VALUED
{
	FloatToken value;

	TokenType getType()override {
		return FLOAT;
	}

	Float(FloatToken val):value(val) {}
	void print()override {
		cout << value.value;
	}
};

struct Double : VALUED
{
	DoubleToken value;

	TokenType getType()override {
		return DOUBLE;
	}

	Double(DoubleToken val):value(val) {}
	void print()override {
		cout << value.value << "d";
	}
};

struct Bit : VALUED
{
	BitToken value;

	TokenType getType()override {
		return BIT;
	}

	Bit(BitToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};



struct String : VALUED
{

	StringToken value;

	TokenType getType()override {
		return STRING;
	}

	String(StringToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};



struct Identifier : VALUED
{

	StringToken value;

	TokenType getType()override {
		return ID;
	}

	Identifier(StringToken val) : value(val) {}
	void print() {
		cout << value.value;
	}


};





struct Definition : Statement {
	Identifier* name;
	VALUED* value;

	TokenType getType()override {
		return DEFINITION;
	}

	Definition(Identifier* nam, VALUED* val) {
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

struct Assignment : Statement {
	
	Identifier* name;
	VALUED* value;
	AssignmentType type;


	TokenType getType()override {
		return ASSIGNMENT;
	}

	Assignment(Identifier* nam, VALUED* val, AssignmentType type) {
		name = nam;
		this->type = type;
		value = val;
	}

	void print()override {
		cout << "Reassigning " << name->value.value;
		cout << " to ";
		value->print();
		cout << endl;

	}
};

struct Parenthesis : VALUED {
	
	VALUED* value;

	TokenType getType()override {
		return PARENTHESIS;
	}

	Parenthesis(VALUED* val) {
		value = val;
	}

	void print()override {
		cout << "(";
		value->print();
		cout << ")";
	}
};

struct Operation : VALUED {

	VALUED* left;
	VALUED* right;
	OperatorType op;

	TokenType getType()override {
		return OPERATION;
	}

	Operation(VALUED* left, OperatorType op, VALUED* right) {
		this->left = left;
		this->right = right;
		this->op = op;
	}

	void print()override {
		left->print();
		cout << " " << (op) << " ";
		right->print();
	}
};;


Statement* parseStatement(vector<Token*> stack);

vector<Statement*> parseStatements(vector<Token*> stack);

CodeBlock* parseTree(vector<Token*> tokens);

