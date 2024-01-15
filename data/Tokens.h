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


	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MODULO,

	INT,
	FLOAT,
	DOUBLE,
	BIT,
	CHAR,
	STRING,

	VAL_INT,
	VAL_FLOAT,
	VAL_DOUBLE,
	VAL_BIT,
	VAL_CHAR,
	VAL_STRING,

	IDENTIFIER,

};

struct Token {
	Tokens type;
};

struct TokenValue : Token {
	string value;
};


vector<Token*> tokenize(vector<string> lines);

bool isNumeric(const std::string& str);
//const char* getToken(TokenType value);