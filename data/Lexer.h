#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;


enum TokenType {
	NONE,

	BIT,
	INT,
	FLOAT,
	DOUBLE,
	STRING,

	ID,

	LET,
	IF,
	ELSE,
	WHILE,
	DO,
	RETURN,

	BRACKET_OPEN,
	BRACKET_CLOSE,
	PARENTHESIS_OPEN,
	PARENTHESIS_CLOSE,
	CURLY_OPEN,
	CURLY_CLOSE,
	
	LINE_END,
	COMMA,
	ASSIGN,
	OPERATOR,

};

enum StatementType {
	NONE_STMT,
	BIT_STMT,
	INT_STMT,
	FLOAT_STMT,
	DOUBLE_STMT,
	STRING_STMT,
	ID_STMT,

	DEFINITION,
	ASSIGNMENT,
	IF_ELSE,
	BI_OPERATION,
	UN_OPERATION,
	SCOPE,
	PARENTHESIS,
};

enum AssignmentType {
	NONE_ASSIGN,
	EQUALS,
	PLUS_EQUAL,
	MINUS_EQUAL,
	MULTIPLY_EQUAL,
	DIVIDE_EQUAL,

	MODULO_EQUAL,
	BITWISE_OR_EQUAL,
	BITWISE_AND_EQUAL,

};

enum UnaryOperatorType {
	NONE_UN_OPERATOR,
	NOT,
	BITWISE_NOT,
	POSITIVE,
	NEGATIVE,
};

enum BinaryOperatorType {
	NONE_BI_OPERATOR,

	OR,
	AND,
	BITWISE_OR,
	XOR,
	BITWISE_AND,

	COMPARISON,
	NOT_EQUAL,

	GREATER_THAN,
	SMALLER_THAN,
	GREATER_THAN_EQUAL,
	SMALLER_THAN_EQUAL,

	PLUS,
	MINUS,

	MULTIPLY,
	DIVIDE,
	MODULO,
};

struct Token {
	virtual TokenType getType() {
		return NONE;
	}
	Token() {}
};
static Token* null = new Token();


struct AssignToken : Token {
	AssignmentType value;

	TokenType getType()override {
		return ASSIGN;
	}

	AssignToken(AssignmentType value) {
		this->value = value;
	}
};

struct OperatorToken : Token {
	BinaryOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(UnaryOperatorType u, BinaryOperatorType bi) {
		uValue = u;
		biValue = bi;
	}
	OperatorToken(BinaryOperatorType value) {
		biValue = value;
		uValue = NONE_UN_OPERATOR;
	}
	OperatorToken(UnaryOperatorType value) {
		uValue = value;
		biValue = NONE_BI_OPERATOR;
	}
};


struct KeyWordToken : Token {
	TokenType value;

	TokenType getType()override {
		return value;
	}

	KeyWordToken(TokenType value) {
		this->value = value;
	}
};

struct StringToken : Token {
	string value;

	TokenType getType()override {
		return STRING;
	}

	StringToken(string value){
		this->value = value;
	}
};
struct BitToken : Token {
	bool value;

	TokenType getType()override {
		return BIT;
	}

	BitToken(bool value){
		this->value = value;
	}
};

struct IntToken : Token {
	int value;

	TokenType getType()override {
		return INT;
	}

	IntToken(int value){
		this->value = value;
	}
};
struct FloatToken : Token {
	float value;

	TokenType getType()override {
		return FLOAT;
	}

	FloatToken(float value){
		this->value = value;
	}
};

struct DoubleToken : Token {
	double value;

	TokenType getType()override {
		return DOUBLE;
	}

	DoubleToken(double value){
		this->value = value;
	}
};

struct IdentifierToken : Token {
	string value;

	TokenType getType()override {
		return ID;
	}

	IdentifierToken(string value){
		this->value = value;
	}
};


//vector<Token*> tokenize(vector<string> lines);
void tokenize(vector<string> lines, vector<Token*>& tokens);
void printToken(Token* t);
int isNumeric(const std::string& str);