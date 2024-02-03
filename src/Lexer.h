#pragma once

#include <string>
#include <vector>
#include <map>

#include "AError.h"
#include "Enums.h"

///using namespace std;


struct Token {
	unsigned int ln;
	virtual TokenType getType() {
		return NONE;
	}
	Token(unsigned int ln) :ln(ln) {};
};


struct AssignToken : Token {
	AssignmentType value;

	TokenType getType()override {
		return ASSIGN;
	}

	AssignToken(AssignmentType value, unsigned int ln) :Token(ln) {
		this->value = value;
	}
};

struct OperatorToken : Token {
	MultipleOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(UnaryOperatorType u, MultipleOperatorType bi, unsigned int ln) :Token(ln) {
		uValue = u;
		biValue = bi;
	}
	OperatorToken(MultipleOperatorType value, unsigned int ln) :Token(ln) {
		biValue = value;
		uValue = NONE_UN_OPERATOR;
	}
	OperatorToken(UnaryOperatorType value, unsigned int ln) :Token(ln) {
		uValue = value;
		biValue = NONE_BI_OPERATOR;
	}
};


struct KeyWordToken : Token {
	TokenType value;

	TokenType getType()override {
		return value;
	}

	KeyWordToken(TokenType value, unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct StringToken : Token {
	string value;

	TokenType getType()override {
		return STRING;
	}
	StringToken(string value,unsigned int ln):Token(ln){
		this->value = value;
	}
};
struct BitToken : Token {
	bool value;

	TokenType getType()override {
		return BIT;
	}
	BitToken(bool value,unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct IntToken : Token {
	int value;

	TokenType getType()override {
		return INT;
	}

	IntToken(int value,unsigned int ln):Token(ln){
		this->value = value;
	}
};
struct FloatToken : Token {
	float value;

	TokenType getType()override {
		return FLOAT;
	}

	FloatToken(float value,unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct DoubleToken : Token {
	double value;

	TokenType getType()override {
		return DOUBLE;
	}

	DoubleToken(double value,unsigned int ln):Token(ln){
		this->value = value;
	}
};



struct IdentifierToken : Token {
	unsigned int value;

	TokenType getType()override {
		return ID;
	}

	IdentifierToken(unsigned int value=-1,unsigned int ln=-1):Token(ln){
		this->value = value;
	}
};


struct Lexer {
	vector<Token*> tokens;

	~Lexer() {
		clean();
	}

	void clean() {
		for (Token* t: tokens) {
			delete t;
		}
		tokens.clear();
		tokens.shrink_to_fit();
	}

	void tokenize(vector<string> lines);
	int isNumeric(const std::string& str);
};