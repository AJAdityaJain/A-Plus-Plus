#pragma once

#include "Lexer.h"
#include <sstream>
#include <utility>

using namespace std;

constexpr static int ALIGN = 16;
struct AsmSize {
	int sz;
	int prec;
};
constexpr AsmSize VOID_SIZE = { 0 ,0 };
constexpr AsmSize BIT_SIZE = { 1,0 };
constexpr AsmSize SHORT_SIZE = { 2,0 };
constexpr AsmSize INT_SIZE = { 4,0 };
constexpr AsmSize LONG_SIZE = { 8,0 };
constexpr AsmSize PTR_SIZE = { 8,0 };
constexpr AsmSize FLOAT_SIZE = { 4,1 };
constexpr AsmSize DOUBLE_SIZE = { 8,2 };
struct Variable {
	void* val;
	unsigned int off;
	int share;
	AsmSize size{};
	IdentifierToken name;

	Variable(void* val, const unsigned int off,const AsmSize size, IdentifierToken name,const int share= ALIGN) {
		this->val = val;
		this->off = off;
		this->size = size;
		this->name = std::move(name);

		this->share = max(share-size.sz,0);
	}
};

struct Statement {
	virtual ~Statement() = default;
	virtual StatementType getType() {
		return NONE_STMT;
	}
};

struct Value : Statement {};





struct CodeBlock final : Statement {
	vector<Statement*> code; 

	~CodeBlock() override {
		code.clear();
		code.shrink_to_fit();
	}

	explicit CodeBlock(const vector<Statement*>& code) {
		this->code = code;
	}
	explicit CodeBlock(Statement* line) {
		if (line->getType() == SCOPE) {
			this->code = dynamic_cast<CodeBlock*>(line)->code;
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



struct Int final: Value
{
	int value;

	StatementType getType()override {
		return INT_STMT;
	}
	explicit Int(const int val) : value(val) {}

};
struct Float final:Value
{
	float value;
	unsigned int label = -1;

	StatementType getType()override {
		return FLOAT_STMT;
	}

	explicit Float(const float val):value(val) {}
};
struct Double final: Value
{
	double value;
	unsigned int label = -1;

	StatementType getType()override {
		return DOUBLE_STMT;
	}

	explicit Double(const double val):value(val) {}
};
struct Bit final: Value
{
	bool value;

	StatementType getType()override {
		return BIT_STMT;
	}

	explicit Bit(const bool val) : value(val) {}
};
struct String final : Value
{

	string value;
	unsigned int label = -1;
	StatementType getType()override {
		return STRING_STMT;
	}

	explicit String(string val) : value(std::move(val)) {}
};

struct Array final: Value
{
	vector<Value*> values;

	StatementType getType()override {
		return ARRAY;
	}

	~Array() override {
		for (const Statement* v : values)
		{
			delete v;
		}
		values.clear();
		values.shrink_to_fit();
	}

	explicit Array(const vector<Value*>& val) {
		this->values = val;
	}
};
struct ArrayAccess final:Value
{
	IdentifierToken name;
	Value* index;

	StatementType getType()override {
		return ARRAY_ACCESS;
	}

	~ArrayAccess() override {
		delete index;
	}

	explicit ArrayAccess(const IdentifierToken& name, Value* index) {
		this->name = name;
		this->index = index;
	}
};

struct Reference final: Value
{

	unsigned int value;

	StatementType getType()override {
		return REFERENCE;
	}

	explicit Reference(const unsigned int val) {
		this->value = val;

	}

};
struct FuncCall final: Statement {
	IdentifierToken name;
	vector<Value*> params;

	FuncCall(const IdentifierToken& name,const  vector<Value*>& params) {
		this->name = name;
		this->params = params;
	}
	~FuncCall() override {
		for (const Value* v : params)
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
struct Func final: Statement {
	IdentifierToken name;	
	CodeBlock* body;


	vector<Variable*> varsStack;
	vector<int> scopesStack;
	stringstream fbody;


	StatementType getType()override {
		return FUNC_DEFINITION;
	}

	~Func () override {
		delete body;
	}
	Func(IdentifierToken nam , CodeBlock* val) : name(std::move(nam)) {
		body = val;
	}	
};


struct Assignment final: Statement {
	
	AssignmentType type;
	IdentifierToken name;
	Value* value;


	StatementType getType()override {
		return ASSIGNMENT;
	}

	~Assignment() override {
		delete value;
	}

	Assignment(const IdentifierToken& name, Value* value,const AssignmentType type) {
		this -> name = name;
		this->value = value;
		this->type = type;
	}

};
struct WhileStatement final: Statement {
	Value* condition;
	CodeBlock* whileBlock;


	StatementType getType()override {
		return WHILE_STMT;
	}

	~WhileStatement() override {
		delete condition;
		delete whileBlock;
	}

	WhileStatement(Value* con, CodeBlock* whileb) {
		condition = con;
		whileBlock = whileb;
	}

};
struct IfStatement final: Statement {
	Value* condition;
	CodeBlock* ifBlock;
    CodeBlock* elseBlock = nullptr;

	StatementType getType()override {
		return IF_STMT;
	}

	~IfStatement() override {
		delete condition;
		delete ifBlock;
		delete elseBlock;
	}

	IfStatement(Value* con, CodeBlock* ifb) {
		condition = con;
		ifBlock = ifb;
	}

};
struct ElseStatement final: Statement {
	CodeBlock* elseBlock;

	StatementType getType()override {
		return ELSE_STMT;
	}

	~ElseStatement() override {
		delete elseBlock;
	}

	explicit ElseStatement(CodeBlock* elseb) {
		elseBlock = elseb;
	}

};


struct MultipleOperation final: Value {

	vector<Value*> operands;
	vector<Value*> invoperands;
	//Statement* right;
	MultipleOperatorType op;
	AsmSize size = VOID_SIZE;

	StatementType getType()override {
		return MULTI_OPERATION;
	}

	~MultipleOperation() override {
		for (const Statement* operand : operands)
		{
			delete operand;

		}
		operands.clear();
		operands.shrink_to_fit();

		for (const Statement* operand : invoperands)
		{
			delete operand;

		}
		invoperands.clear();
		invoperands.shrink_to_fit();
	}

	MultipleOperation(const MultipleOperatorType op,const vector<Value*>& operands,const vector<Value*>& invoperands) {
		this->operands = operands;
		this->invoperands = invoperands;
		this->op = op;
	}

};
struct UnaryOperation final: Value {

	UnaryOperatorType op;
	Value* right;

	StatementType getType()override {
		return UN_OPERATION;
	}

	~UnaryOperation() override {
		delete right;
	}

	UnaryOperation(const UnaryOperatorType op, Value* right) {
		this->right = right;
		this->op = op;
	}

};


struct Parser {

	vector<Token*> tks;

	explicit Parser(const vector<Token*>& tks) {
		this->tks = tks;
	}

	Statement* parseStatement(vector<Token*> stack, unsigned int line);

	vector<Statement*> parse();
	vector<Statement*> parse(const vector<Token*>& stack);

	static void checkDepth(const TokenType tt, int&depth)
	{
		if(tt == BRACKET_OPEN || tt == CURLY_OPEN || tt == PARENTHESIS_OPEN) depth++;
		else if(tt == BRACKET_CLOSE || tt == CURLY_CLOSE || tt == PARENTHESIS_CLOSE) depth--;
	}
};


