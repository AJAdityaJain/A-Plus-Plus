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
	LINE_END,

	LET,
	IF,
	ELSE,
	WHILE,
	DO,
	AND,
	OR,
	RETURN,

	BRACKET_OPEN,
	BRACKET_CLOSE,
	PARENTHESIS_OPEN,
	PARENTHESIS_CLOSE,
	CURLY_OPEN,
	CURLY_CLOSE,
	
	COMMA,
	ASSIGN,
	OPERATOR,



	DEFINITION,
	ASSIGNMENT,
	OPERATION,
	_SCOPE,
	PARENTHESIS,
};


void printToken(TokenType t);

enum AssignmentType {
	NONE_ASSIGN,
	EQUALS,
	PLUS_EQUAL,
	MINUS_EQUAL,
	MULTIPLY_EQUAL,
	DIVIDE_EQUAL
};

enum OperatorType {
	NONE_OPERATOR,
	NOT,
	EEQUAL,
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MODULO,
	NOT_EQUAL,
	GREATER_THAN,
	SMALLER_THAN,
	GREATER_THAN_EQUAL,
	SMALLER_THAN_EQUAL,
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
	OperatorType value;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(OperatorType value) {
		this->value = value;
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

int isNumeric(const std::string& str);
const char* getToken(TokenType value);