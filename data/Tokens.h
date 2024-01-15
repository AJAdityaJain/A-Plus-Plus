#pragma once

#include <string>
#include <vector>
#include <iostream>
//#include <map>

using namespace std;

//enum TokenType {
//	_IDENTIFIER,
//	_OPERATOR,
//	_VALUE,
//	_KEYWORD,
//	_SEPARATOR
//};

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

	ASSIGN,
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
	UNKNOWN

};

struct Token {
	Tokens type;
	
	Token(Tokens type) : type(type) {}
};

struct StringToken : Token {
	string value;
	StringToken(string value) : Token(STRING){
		this->value = value;
	}
};
struct BitToken : Token {
	bool value;
	BitToken(bool value) : Token(BIT){
		this->value = value;
	}
};

struct IntToken : Token {
	int value;
	IntToken(int value) : Token(INT){
		this->value = value;
	}
};
struct FloatToken : Token {
	float value;
	FloatToken(float value) : Token(FLOAT){
		this->value = value;
	}
};

struct DoubleToken : Token {
	double value;
	DoubleToken(double value) : Token(DOUBLE){
		this->value = value;
	}
};

struct IdentifierToken : Token {
	string value;
	IdentifierToken(string value) : Token(ID){
		this->value = value;
	}
};


//vector<Token*> tokenize(vector<string> lines);
void tokenize(vector<string> lines, vector<Token*>& tokens);

int isNumeric(const std::string& str);
const char* getToken(Tokens value);