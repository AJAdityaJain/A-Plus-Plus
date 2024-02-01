#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "Enums.h"

using namespace std;


struct Token {
	virtual TokenType getType() {
		return NONE;
	}
	virtual void print(){
		cout << "..." << endl;
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
	MultipleOperatorType biValue;
	UnaryOperatorType uValue;

	TokenType getType()override {
		return OPERATOR;
	}

	OperatorToken(UnaryOperatorType u, MultipleOperatorType bi) {
		uValue = u;
		biValue = bi;
	}
	OperatorToken(MultipleOperatorType value) {
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
	void print() override {
		cout << value << endl;
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
	void print() override {
		if(value)
			cout << "True" << endl;
		else
			cout << "False" << endl;
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
	void print() override {
		cout << value << endl;
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
	void print() override {
		cout << value << endl;
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
	void print() override {
		cout << value << endl;
	}

	DoubleToken(double value){
		this->value = value;
	}
};



struct IdentifierToken : Token {
	unsigned int value;

	TokenType getType()override {
		return ID;
	}

	IdentifierToken(unsigned int value){
		this->value = value;
	}
};


//vector<Token*> tokenize(vector<string> lines);
void tokenize(vector<string> lines, vector<Token*>& tokens);
void printToken(Token* t);
int isNumeric(const std::string& str);