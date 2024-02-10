#pragma once

#include <string>
#include <utility>
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
	explicit Token(const unsigned int ln) :ln(ln) {};
};


struct AssignToken : Token {
	AssignmentType value;

	TokenType getType()override {
		return ASSIGN;
	}

	AssignToken(const AssignmentType value,const unsigned int ln) :Token(ln) {
		this->value = value;
	}
};

struct OperatorToken : Token {
	MultipleOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(UnaryOperatorType u, MultipleOperatorType bi, const unsigned int ln) :Token(ln) {
		uValue = u;
		biValue = bi;
	}
	OperatorToken(MultipleOperatorType value, const unsigned int ln) :Token(ln) {
		biValue = value;
		uValue = NONE_UN_OPERATOR;
	}
	OperatorToken(UnaryOperatorType value, const unsigned int ln) :Token(ln) {
		uValue = value;
		biValue = NONE_BI_OPERATOR;
	}
};


struct KeyWordToken : Token {
	TokenType value;

	TokenType getType()override {
		return value;
	}

	KeyWordToken(TokenType value, const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct StringToken : Token {
	string value;

	TokenType getType()override {
		return STRING;
	}
	StringToken(string value,const unsigned int ln):Token(ln){
		this->value = std::move(value);
	}
};
struct BitToken : Token {
	bool value;

	TokenType getType()override {
		return BIT;
	}
	BitToken(bool value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct IntToken : Token {
	int value;

	TokenType getType()override {
		return INT;
	}

	IntToken(int value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};
struct FloatToken : Token {
	float value;

	TokenType getType()override {
		return FLOAT;
	}

	FloatToken(float value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct DoubleToken : Token {
	double value;

	TokenType getType()override {
		return DOUBLE;
	}

	DoubleToken(double value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};



struct IdentifierToken : Token {
	unsigned int value;

	TokenType getType()override {
		return ID;
	}

	explicit IdentifierToken(unsigned int value=-1,const unsigned int ln=-1):Token(ln){
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