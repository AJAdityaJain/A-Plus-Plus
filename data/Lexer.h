#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;


enum Tokens {
	LET,
	IF,
	ELSE,
	WHILE,
	CONTINUE,
	RETURN,
	BREAK,

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


	INT,
	FLOAT,
	DOUBLE,
	BIT,
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
	virtual Tokens getType() {
		return UNKNOWN;
	}
	Token() {}
};
struct KeyWordToken : Token {
	Tokens value;

	Tokens getType()override {
		return value;
	}

	KeyWordToken(Tokens value) {
		this->value = value;
	}
};

struct StringToken : Token {
	string value;

	Tokens getType()override {
		return STRING;
	}

	StringToken(string value){
		this->value = value;
	}
};
struct BitToken : Token {
	bool value;

	Tokens getType()override {
		return BIT;
	}

	BitToken(bool value){
		this->value = value;
	}
};

struct IntToken : Token {
	int value;

	Tokens getType()override {
		return INT;
	}

	IntToken(int value){
		this->value = value;
	}
};
struct FloatToken : Token {
	float value;

	Tokens getType()override {
		return FLOAT;
	}

	FloatToken(float value){
		this->value = value;
	}
};

struct DoubleToken : Token {
	double value;

	Tokens getType()override {
		return DOUBLE;
	}

	DoubleToken(double value){
		this->value = value;
	}
};

struct IdentifierToken : Token {
	string value;

	Tokens getType()override {
		return ID;
	}

	IdentifierToken(string value){
		this->value = value;
	}
};


//vector<Token*> tokenize(vector<string> lines);
void tokenize(vector<string> lines, vector<Token*>& tokens);

int isNumeric(const std::string& str);
const char* getToken(Tokens value);