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



struct Int : Statement
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
struct Float :Statement
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

struct Double : Statement
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

struct Bit : Statement
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



struct String : Statement
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



struct Identifier : Statement
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

struct CallingFunc : Statement
{
	IdentifierToken name;
	vector<Statement*> params;

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
	CallingFunc(IdentifierToken val, vector<Statement*> params) : name(val) {
		this->params = params;
	}

	void print() {
		cout << name.value << "(";
		for (Statement* v : params) {
			v->print();
			cout << " ";
		}
		cout << ")" ;
	}
};





struct Definition : Statement {
	Identifier* name;
	Statement* value;

	StatementType getType()override {
		return DEFINITION;
	}

	~Definition() {
		deallocstmt(name);
		deallocstmt(value);
	}
	Definition(Identifier* nam, Statement* val) {
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
	Statement* value;


	StatementType getType()override {
		return ASSIGNMENT;
	}

	~Assignment() {
		deallocstmt(name);
		deallocstmt(value);
	}

	Assignment(Identifier* nam, Statement* val) {
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
	Statement* condition;
	Statement* whileBlock;


	StatementType getType()override {
		return WHILE_STMT;
	}

	~WhileStatement() {
		deallocstmt(condition);
		deallocstmt(whileBlock);
	}

	WhileStatement(Statement* con, Statement* whileb) {
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
	Statement* condition;
	Statement* ifBlock;


	StatementType getType()override {
		return IF_STMT;
	}

	~IfStatement() {
		deallocstmt(condition);
		deallocstmt(ifBlock);
	}

	IfStatement(Statement* con, Statement* ifb) {
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


struct Parenthesis : Statement {
	
	Statement* value;

	StatementType getType()override {
		return PARENTHESIS;
	}

	~Parenthesis() {
		deallocstmt(value);
	}

	Parenthesis(Statement* val) {
		value = val;
	}

	void print()override {
		cout << "(";
		value->print();
		cout << ")";
	}
};

struct BinaryOperation : Statement {

	Statement* left;
	Statement* right;
	BinaryOperatorType op;

	StatementType getType()override {
		return BI_OPERATION;
	}

	~BinaryOperation() {
		deallocstmt(left);
		deallocstmt(right);
	}

	BinaryOperation(Statement* left, BinaryOperatorType op, Statement* right) {
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

struct UnaryOperation : Statement {

	UnaryOperatorType op;
	Statement* right;

	StatementType getType()override {
		return UN_OPERATION;
	}

	~UnaryOperation() {
		deallocstmt(right);
	}

	UnaryOperation(UnaryOperatorType op, Statement* right) {
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

