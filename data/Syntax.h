#include "Tokens.h"

using namespace std;


struct STATEMENT {
	virtual void print() {
		cout << "STMT_FAIL" << endl;
	}
};
struct VALUED : STATEMENT {};



struct BLOCK : STATEMENT{
	vector<STATEMENT*> code;
	void print()override {
		cout << "{" << endl;
		for (STATEMENT* statement : code) {
			statement->print();
		}
		cout << "}" << endl;
	}
};
struct PROGRAM : STATEMENT{ 
	BLOCK code;
	void print() override {
		code.print();
	}
};
struct IDENTIFIER : VALUED{
	string name;

	IDENTIFIER(string name = "NULL") {
		this->name = name;
	}
	void print()override {
		cout << name;
	}
};



enum DATATYPES {
	_INTEGER,
	_FLOAT,
	_DOUBLE,
	_BIT
};

enum STATETYPE{
	_CONTINUE,
	_DEFINITION,
	_ASSIGNMENT,
	_IF,
	_IF_ELSE,
	_WHILE,
	_BLOCK	
};


struct INTEGER : VALUED
{
	INTEGER(int value) {
		this->value = value;
	}
	int value = 0; void print()override {
		cout << value;	
	}
};

struct FLOAT :VALUED
{
	float value = 0;
	FLOAT(float value) {
		this->value = value;
	}
	void print()override {
		cout << value;
	}

};

struct DOUBLE : VALUED
{
	double value = 0;
	DOUBLE(double value) {
		this->value = value;
	}
	void print()override {
		cout << value;
	}
};

struct BIT : VALUED
{bool value = true;
BIT(bool value) {
	this->value = value;
}
void print()override {
	cout << value;
}
};





struct DEFINITION : STATEMENT {
	IDENTIFIER name;
	VALUED* value;

	DEFINITION(IDENTIFIER name, VALUED *value) {
		this->name = name;
		this->value = value;
	}

	void print() override {
		name.print();
		cout << " = ";
		value->print();
		cout << endl;
	}
};

struct ASSIGNMENT : STATEMENT {
	IDENTIFIER name;
	VALUED *value;
	void print()override {
		name.print();
		cout << " = ";
		value->print();
		cout << endl;

	}
};

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


STATEMENT* parse(Token* tokens);