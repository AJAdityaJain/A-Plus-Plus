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
struct BooleanToken final : Token {
	bool value;

	TokenType getType()override {
		return BOOL;
	}

	explicit BooleanToken(const bool value){
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
struct ShortToken final: Token {
	short value;

	TokenType getType()override {
		return SHORT;
	}

	explicit ShortToken(const short value){
		this->value = value;
	}
};
struct LongToken final: Token {
	long value;

	TokenType getType()override {
		return LONG;
	}

	explicit LongToken(const long value){
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
struct SizeToken final: Token {
	AsmSize value{};

	TokenType getType()override {
		return SIZE_T;
	}

	explicit SizeToken(const AsmSize value){
		this->value = value;
	}
};

	void tokenize(const vector<string>& lines, vector<Token*>& tokens);
	static TokenType isNumeric(const std::string& str);