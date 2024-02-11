// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>

#include "AError.h"
#include "Enums.h"


struct Token {
	virtual TokenType getType() {
		return NONE;
	}
	virtual ~Token() = default;
};

struct AssignToken final : Token {
	AssignmentType value;

	TokenType getType()override {
		return ASSIGN;
	}

	explicit AssignToken(const AssignmentType value) {
		this->value = value;
	}
};
struct OperatorToken final : Token {
	MultipleOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	explicit OperatorToken(const MultipleOperatorType bi = NONE_BI_OPERATOR,const UnaryOperatorType u = NONE_UN_OPERATOR) {
		uValue = u;
		biValue = bi;
	}
};

struct LineToken final: Token {
	unsigned int line;

	TokenType getType()override {
		return LINE_END;
	}

	explicit LineToken(const unsigned int line){
		this->line = line;
	}
};
struct KeyWordToken final: Token {
	TokenType value;

	TokenType getType()override {
		return value;
	}

	explicit KeyWordToken(const TokenType value){
		this->value = value;
	}
};
struct StringToken final: Token {
	string value;

	TokenType getType()override {
		return STRING;
	}
	explicit StringToken(string value){
		this->value = std::move(value);
	}
};
struct BitToken final : Token {
	bool value;

	TokenType getType()override {
		return BIT;
	}

	explicit BitToken(const bool value){
		this->value = value;
	}
};
struct IntToken final: Token {
	int value;

	TokenType getType()override {
		return INT;
	}

	explicit IntToken(const int value){
		this->value = value;
	}
};
struct FloatToken final: Token {
	float value;

	TokenType getType()override {
		return FLOAT;
	}

	explicit FloatToken(const float value){
		this->value = value;
	}
};
struct DoubleToken final: Token {
	double value;

	TokenType getType()override {
		return DOUBLE;
	}

	explicit DoubleToken(const double value){
		this->value = value;
	}
};
struct IdentifierToken final: Token {
	unsigned int value;

	TokenType getType()override {
		return ID;
	}

	explicit IdentifierToken(const unsigned int value=-1){
		this->value = value;
	}
};


struct Lexer {
	vector<Token*> tokens;

	~Lexer() {
		for (const Token* t: tokens) {
			delete t;
		}
		tokens.clear();
		tokens.shrink_to_fit();
	}

	void tokenize(const vector<string>& lines);
	static int isNumeric(const std::string& str);
};