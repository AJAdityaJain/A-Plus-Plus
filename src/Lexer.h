// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>

#include "AError.h"
#include "Enums.h"


struct Token {
	unsigned int ln;
	virtual TokenType getType() {
		return NONE;
	}
	virtual ~Token() = default;
	explicit Token(const unsigned int ln) :ln(ln) {}
};


struct AssignToken final : Token {
	AssignmentType value;

	TokenType getType()override {
		return ASSIGN;
	}

	AssignToken(const AssignmentType value,const unsigned int ln) :Token(ln) {
		this->value = value;
	}
};

struct OperatorToken final : Token {
	MultipleOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(const UnaryOperatorType u, const MultipleOperatorType bi, const unsigned int ln) :Token(ln) {
		uValue = u;
		biValue = bi;
	}
	OperatorToken(const MultipleOperatorType value, const unsigned int ln) :Token(ln) {
		biValue = value;
		uValue = NONE_UN_OPERATOR;
	}
	OperatorToken(const UnaryOperatorType value, const unsigned int ln) :Token(ln) {
		uValue = value;
		biValue = NONE_BI_OPERATOR;
	}
};


struct KeyWordToken final: Token {
	TokenType value;

	TokenType getType()override {
		return value;
	}

	KeyWordToken(const TokenType value, const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct StringToken final: Token {
	string value;

	TokenType getType()override {
		return STRING;
	}
	StringToken(string value,const unsigned int ln):Token(ln){
		this->value = std::move(value);
	}
};
struct BitToken final : Token {
	bool value;

	TokenType getType()override {
		return BIT;
	}
	BitToken(const bool value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct IntToken final: Token {
	int value;

	TokenType getType()override {
		return INT;
	}

	IntToken(const int value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};
struct FloatToken final: Token {
	float value;

	TokenType getType()override {
		return FLOAT;
	}

	FloatToken(const float value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};

struct DoubleToken final: Token {
	double value;

	TokenType getType()override {
		return DOUBLE;
	}

	DoubleToken(const double value,const unsigned int ln):Token(ln){
		this->value = value;
	}
};



struct IdentifierToken final: Token {
	unsigned int value;

	TokenType getType()override {
		return ID;
	}

	explicit IdentifierToken(const unsigned int value=-1,const unsigned int ln=-1):Token(ln){
		this->value = value;
	}
};


struct Lexer {
	vector<Token*> tokens;

	~Lexer() {
		clean();
	}

	void clean() {
		for (const Token* t: tokens) {
			delete t;
		}
		tokens.clear();
		tokens.shrink_to_fit();
	}

	void tokenize(const vector<string>& lines);
	static int isNumeric(const std::string& str);
};