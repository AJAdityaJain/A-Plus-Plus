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

void deallocstmt(Statement* statement);





struct CodeBlock : Statement {
	vector<Statement*> code; 

	~CodeBlock() {
		for (Statement* statement : code) {
			deallocstmt(statement);
		}
		code.clear();
		code.shrink_to_fit();
	}
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

	IdentifierToken value;

	StatementType getType()override {
		return ID_STMT;
	}

	Identifier(IdentifierToken val) : value(val) {}
	void print() {
		cout << value.value;
	}


};

struct CallingFunc : VALUED
{
	IdentifierToken name;
	vector<VALUED*> params;

	StatementType getType()override {
		return CALL;
	}

	~CallingFunc(){
		for (Statement* p : params)
			deallocstmt(p);
		params.clear();
		params.shrink_to_fit();
	}

	CallingFunc(IdentifierToken val) : name(val) {}
	CallingFunc(IdentifierToken val, vector<VALUED*> params) : name(val) {
		this->params = params;
	}

	void print() {
		cout << name.value << "(";
		for (VALUED* v : params) {
			v->print();
			cout << " ";
		}
		cout << ")" ;
	}
};





struct Definition : Statement {
	Identifier* name;
	VALUED* value;

	StatementType getType()override {
		return DEFINITION;
	}

	~Definition() {
		deallocstmt(name);
		deallocstmt(value);
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

	~Assignment() {
		deallocstmt(name);
		deallocstmt(value);
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

struct WhileStatement : Statement {
	VALUED* condition;
	Statement* whileBlock;


	StatementType getType()override {
		return WHILE_STMT;
	}

	~WhileStatement() {
		deallocstmt(condition);
		deallocstmt(whileBlock);
	}

	WhileStatement(VALUED* con, Statement* whileb) {
		condition = con;
		whileBlock = whileb;
	}

	void print()override {
		cout << "while ";
		//condition->print();
		cout << " :";
		whileBlock->print();
		cout << endl;
	}
};

struct IfStatement : Statement {
	VALUED* condition;
	Statement* ifBlock;


	StatementType getType()override {
		return IF_STMT;
	}

	~IfStatement() {
		deallocstmt(condition);
		deallocstmt(ifBlock);
	}

	IfStatement(VALUED* con, Statement* ifb) {
		condition = con;
		ifBlock = ifb;
	}

	void print()override {
		cout << "if";
		condition->print();
		cout << " do ";
		ifBlock->print();
		cout << endl;
	}
};

struct ElseStatement : Statement {
	Statement* elseBlock;

	StatementType getType()override {
		return ELSE_STMT;
	}

	~ElseStatement() {
		deallocstmt(elseBlock);
	}

	ElseStatement(Statement* elseb) {
		elseBlock = elseb;
	}

	void print()override {
		cout << "else ";
		elseBlock->print();
		cout << endl;
	}
};


struct Parenthesis : VALUED {
	
	VALUED* value;

	StatementType getType()override {
		return PARENTHESIS;
	}

	~Parenthesis() {
		deallocstmt(value);
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

	~BinaryOperation() {
		deallocstmt(left);
		deallocstmt(right);
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

	~UnaryOperation() {
		deallocstmt(right);
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


Statement* parseStatement(vector<Token*> stack, bool waitForElse = false);

vector<Statement*> parseStatements(vector<Token*> stack);

CodeBlock* parseTree(vector<Token*> tokens);

