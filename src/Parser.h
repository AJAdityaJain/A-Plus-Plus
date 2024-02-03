#pragma once

#include "Lexer.h"

using namespace std;

struct Reference {
	unsigned int off;
	unsigned int size;
};

struct Statement {
	virtual StatementType getType() {
		return NONE_STMT;
	}
	virtual void print() {
		cout << "STMT_FAIL" << endl;
	}

};

struct Value : Statement {
	virtual int getSize() {
		return VOID_SIZE;
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



struct Int : Value
{
	IntToken value;

	StatementType getType()override {
		return INT_STMT;
	}
	Int(IntToken val) : value(val) {}
	void print()override {
		cout << value.value;	
	}
	int getSize()override {
		return INT_SIZE;
	}
};
struct Float :Value
{
	FloatToken value;

	StatementType getType()override {
		return FLOAT_STMT;
	}

	Float(FloatToken val):value(val) {}
	void print()override {
		cout << value.value;
	}
	int getSize()override {
		return FLOAT_SIZE;
	}
};

struct Double : Value
{
	DoubleToken value;

	StatementType getType()override {
		return DOUBLE_STMT;
	}

	Double(DoubleToken val):value(val) {}
	void print()override {
		cout << value.value << "d";
	}
	int getSize()override {
		return DOUBLE_SIZE;
	}
};

struct Bit : Value
{
	BitToken value;

	StatementType getType()override {
		return BIT_STMT;
	}

	Bit(BitToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
	int getSize()override {
		return BIT_SIZE;
	}
};



struct String : Value
{

	StringToken value;

	StatementType getType()override {
		return STRING_STMT;
	}

	String(StringToken val) : value(val) {}
	void print()override {
		cout << value.value;
	}
	int getSize()override {
		return STRING_SIZE;
	}
};



struct Identifier : Value
{

	IdentifierToken value;
	Reference ref;

	StatementType getType()override {
		return ID_STMT;
	}

	Identifier(IdentifierToken val, Reference ref) {
		this->value = val;
		this->ref = ref;
	}
	void print() {
		cout << value.value;
	}
	int getSize()override {
		return ref.size;
	}


};






struct Func : Statement {
	IdentifierToken name;
	
	vector<IdentifierToken> params;
	
	CodeBlock* body;


	vector<Identifier> varsStack;
	vector<int> scopesStack;



	StatementType getType()override {
		return FUNC_DEFINITION;
	}

	~Func () {

		deallocstmt(body);
	}
	Func(IdentifierToken nam , vector<IdentifierToken> param, CodeBlock* val) : name(nam) {
		params = param;
		body = val;
	}
	
	void print() override {
		cout << "Defining func " << name.value;
		cout << " as ";
		body->print();
		cout << endl;

	}
};


struct Definition : Statement {
	IdentifierToken name;
	Value* value;

	StatementType getType()override {
		return DEFINITION;
	}

	~Definition() {

		deallocstmt(value);
	}
	Definition(IdentifierToken nam = IdentifierToken (-1,- 1), Value* val = nullptr) : name(nam) {
		value = val;
	}

	void print() override {
		cout << "Defining " << name.value;
		cout << " as ";
		value->print();
		cout << endl;
	}
};
struct Assignment : Statement {
	
	IdentifierToken name;
	Value* value;


	StatementType getType()override {
		return ASSIGNMENT;
	}

	~Assignment() {
		deallocstmt(value);
	}

	Assignment(IdentifierToken nam, Value* val) {
		name = nam;
		value = val;
	}

	void print()override {
		cout << "Reassigning " << name.value;
		cout << " to ";
		value->print();
		cout << endl;

	}
};

struct WhileStatement : Statement {
	Value* condition;
	Statement* whileBlock;


	StatementType getType()override {
		return WHILE_STMT;
	}

	~WhileStatement() {
		deallocstmt(condition);
		deallocstmt(whileBlock);
	}

	WhileStatement(Value* con, Statement* whileb) {
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
	Value* condition;
	Statement* ifBlock;


	StatementType getType()override {
		return IF_STMT;
	}

	~IfStatement() {
		deallocstmt(condition);
		deallocstmt(ifBlock);
	}

	IfStatement(Value* con, Statement* ifb) {
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



struct MultipleOperation : Value {

	vector<Value*> operands;
	vector<Value*> invoperands;
	//Statement* right;
	MultipleOperatorType op;

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

	void print()override {
		cout << "(";
		for (Statement* operand : operands)
		{
			cout << " " << (op) << " ";
			operand->print();
		}
		cout << ")";
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

	void print()override {
		cout << (op) << " ";
		right->print();
	}

	int getSize()override {
		switch (op) {
		case NEGATIVE:return right->getSize();
		case POSITIVE:return right->getSize();
		case NOT:return BIT_SIZE;
		case BITWISE_NOT:return right->getSize();
		}

		aThrowError(2, -1);
	};
};

Statement* parseStatement(vector<Token*> stack, bool waitForElse = false);

vector<Statement*> parseStatements(vector<Token*> stack);

CodeBlock* parseTree(vector<Token*> tokens);

