#pragma once

#include "Lexer.h"

using namespace std;

struct Variable {
	unsigned int off;
	AsmSize size;
	IdentifierToken name;

	Variable(unsigned int off, AsmSize size, IdentifierToken name) {
		this->off = off;
		this->size = size;
		this->name = name;
	}
};

struct Statement {
	virtual StatementType getType() {
		return NONE_STMT;
	}
};

struct Value : Statement {};

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

	CodeBlock(vector<Statement*> code) {
		this->code = code;
	}
	CodeBlock(Statement* line) {
		if (line->getType() == SCOPE) {
			this->code = ((CodeBlock*)line)->code;
			delete line;
		}
		else {
			this->code = vector<Statement*>();
			this->code.push_back(line);
		}

	}

	StatementType getType()override {
		return SCOPE;
	}
};



struct Int : Value
{
	int value;

	StatementType getType()override {
		return INT_STMT;
	}
	Int(int val) : value(val) {}

};
struct Float :Value
{
	float value;

	StatementType getType()override {
		return FLOAT_STMT;
	}

	Float(float val):value(val) {}
};

struct Double : Value
{
	DoubleToken value;

	StatementType getType()override {
		return DOUBLE_STMT;
	}

	Double(DoubleToken val):value(val) {}
};

struct Bit : Value
{
	bool value;

	StatementType getType()override {
		return BIT_STMT;
	}

	Bit(bool val) : value(val) {}
};



struct String : Value
{

	string value;

	StatementType getType()override {
		return STRING_STMT;
	}

	String(string val) : value(val) {}
};



struct Reference : Value
{

	IdentifierToken value;

	StatementType getType()override {
		return REFERENCE;
	}

	Reference(IdentifierToken val) {
		this->value = val;
	}

};



struct FuncCall : Statement {
	IdentifierToken name;
	vector<Value*> params;

	FuncCall(IdentifierToken name, vector<Value*> params) {
		this->name = name;
		this->params = params;
	}
	~FuncCall() {
		for (Value* v : params)
		{
			delete v;
		}
		params.clear();
		params.shrink_to_fit();
	}

	StatementType getType()override {
		return FUNC_CALL;
	}
};

struct Func : Statement {
	IdentifierToken name;	
	CodeBlock* body;


	vector<Variable*> varsStack;
	vector<int> scopesStack;



	StatementType getType()override {
		return FUNC_DEFINITION;
	}

	~Func () {

		deallocstmt(body);
	}
	Func(IdentifierToken nam , CodeBlock* val) : name(nam) {
		body = val;
	}	
};


struct Assignment : Statement {
	
	AssignmentType type;
	IdentifierToken name;
	Value* value;


	StatementType getType()override {
		return ASSIGNMENT;
	}

	~Assignment() {
		deallocstmt(value);
	}

	Assignment(IdentifierToken name, Value* value, AssignmentType type) {
		this -> name = name;
		this->value = value;
		this->type = type;
	}

};

struct WhileStatement : Statement {
	Value* condition;
	CodeBlock* whileBlock;


	StatementType getType()override {
		return WHILE_STMT;
	}

	~WhileStatement() {
		deallocstmt(condition);
		deallocstmt(whileBlock);
	}

	WhileStatement(Value* con, CodeBlock* whileb) {
		condition = con;
		whileBlock = whileb;
	}

};

struct IfStatement : Statement {
	Value* condition;
	CodeBlock* ifBlock;


	StatementType getType()override {
		return IF_STMT;
	}

	~IfStatement() {
		deallocstmt(condition);
		deallocstmt(ifBlock);
	}

	IfStatement(Value* con, CodeBlock* ifb) {
		condition = con;
		ifBlock = ifb;
	}

};

struct ElseStatement : Statement {
	CodeBlock* elseBlock;

	StatementType getType()override {
		return ELSE_STMT;
	}

	~ElseStatement() {
		deallocstmt(elseBlock);
	}

	ElseStatement(CodeBlock* elseb) {
		elseBlock = elseb;
	}

};



struct MultipleOperation : Value {

	vector<Value*> operands;
	vector<Value*> invoperands;
	//Statement* right;
	MultipleOperatorType op;
	AsmSize size = VOID_SIZE;

	StatementType getType()override {
		return MULTI_OPERATION;
	}

	~MultipleOperation() {
		for (Statement* operand : operands)
		{
			deallocstmt(operand);

		}
		operands.clear();
		operands.shrink_to_fit();

		for (Statement* operand : invoperands)
		{
			deallocstmt(operand);

		}
		invoperands.clear();
		invoperands.shrink_to_fit();
	}

	MultipleOperation(MultipleOperatorType op, vector<Value*> operands, vector<Value*> invoperands) {
		this->operands = operands;
		this->invoperands = invoperands;
		this->op = op;
	}

};

struct UnaryOperation : Value {

	UnaryOperatorType op;
	Value* right;

	StatementType getType()override {
		return UN_OPERATION;
	}

	~UnaryOperation() {
		deallocstmt(right);
	}

	UnaryOperation(UnaryOperatorType op, Value* right) {
		this->right = right;
		this->op = op;
	}

};


Statement* parseStatement(vector<Token*> stack, bool waitForElse = false);

vector<Statement*> parseStatements(vector<Token*> stack);

CodeBlock* parseTree(vector<Token*> tokens);

