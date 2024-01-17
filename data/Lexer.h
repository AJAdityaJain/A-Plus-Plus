#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;


enum TokenType {
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
	LINE_END,
	COMMA,

	EQUALS,
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MODULO,


	BIT,
	INT,
	FLOAT,
	DOUBLE,
	STRING,

	ID,
	UNKNOWN,

	DEFINE,
	ASSIGN,
	OPERATE,
	SCOPE,
	PRECEDER,

};


struct Token {
	virtual TokenType getType() {
		return UNKNOWN;
	}
	Token() {}
};
static Token* null = new Token();

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