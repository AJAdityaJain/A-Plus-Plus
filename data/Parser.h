#pragma once

#include "Lexer.h"

using namespace std;


struct Statement {
	virtual StatementType getType() {
		return NONE_STMT;
	}
	virtual void print() {
		cout << "STMT_FAIL" << endl;
	}

};

struct VALUED : Statement {};







struct CodeBlock : Statement {
	vector<Statement*> code; 

	StatementType getType()override {
		return SCOPE;
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

	StatementType getType()override {
		return INT_STMT;
	}
	Int(IntToken val) : value(val) {}
	void print()override {
		cout << value.value;	
	}
};
struct Float :VALUED
{
	FloatToken value;

	StatementType getType()override {
		return FLOAT_STMT;
	}

	Float(FloatToken val):value(val) {}
	void print()override {
		cout << value.value;
	}
};

struct Double : VALUED
{
	DoubleToken value;

	StatementType getType()override {
		return DOUBLE_STMT;
	}

	Double(DoubleToken val):value(val) {}
	void print()override {
		cout << value.value << "d";
	}
};

struct Bit : VALUED
{
	BitToken value;

	StatementType getType()override {
		return BIT_STMT;
	}

	Bit(BitToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};



struct String : VALUED
{

	StringToken value;

	StatementType getType()override {
		return STRING_STMT;
	}

	String(StringToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
};



struct Identifier : VALUED
{

	StringToken value;

	StatementType getType()override {
		return ID_STMT;
	}

	Identifier(StringToken val) : value(val) {}
	void print() {
		cout << value.value;
	}


};





struct Definition : Statement {
	Identifier* name;
	VALUED* value;

	StatementType getType()override {
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


	StatementType getType()override {
		return ASSIGNMENT;
	}

	Assignment(Identifier* nam, VALUED* val) {
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

struct IfElseStatement: Statement {
	VALUED* condition;
	Statement* ifBlock;
	Statement* elseBlock;


	StatementType getType()override {
		return IF_ELSE;
	}

	IfElseStatement(VALUED* con, Statement* ifb) {
		condition = con;
		ifBlock = ifb;
		elseBlock = nullptr;
	}

	IfElseStatement(VALUED* con, Statement* ifb, Statement* elseb) {
		condition = con;
		ifBlock = ifb;
		elseBlock = elseb;
	}

	void print()override {
		cout << "if";
		condition->print();
		cout << " do ";
		ifBlock->print();
		cout << endl;
		if (elseBlock != nullptr) {
			cout << "else ";
			elseBlock->print();
			cout << endl;
		}

	}
};


struct Parenthesis : VALUED {
	
	VALUED* value;

	StatementType getType()override {
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

struct BinaryOperation : VALUED {

	VALUED* left;
	VALUED* right;
	BinaryOperatorType op;

	StatementType getType()override {
		return BI_OPERATION;
	}

	BinaryOperation(VALUED* left, BinaryOperatorType op, VALUED* right) {
		this->left = left;
		this->right = right;
		this->op = op;
	}

	void print()override {
		left->print();
		cout << " " << (op) << " ";
		right->print();
	}
};

struct UnaryOperation : VALUED {

	UnaryOperatorType op;
	VALUED* right;

	StatementType getType()override {
		return UN_OPERATION;
	}

	UnaryOperation(UnaryOperatorType op, VALUED* right) {
		this->right = right;
		this->op = op;
	}

	void print()override {
		cout << (op) << " ";
		right->print();
	}
};


Statement* parseStatement(vector<Token*> stack);

vector<Statement*> parseStatements(vector<Token*> stack);

CodeBlock* parseTree(vector<Token*> tokens);

