#pragma once

#include "Parser.h"
#include <map>


struct Variable {
	string& name;
	Token* value;
};

static vector<Variable> varsStack ;
static vector<int> scopesStack;



Token* Execute(Statement* line);

Token* Operate(Token* left, Token* right, OperatorType op);
	
template<typename T>
T* Operatee(T* left, T* right, OperatorType op) {
	switch (op)
	{
	case PLUS:
		return new T(left->value + right->value);
	case MINUS:
		return new T(left->value - right->value);
	case MULTIPLY:
		return new T(left->value * right->value);
	case DIVIDE:
		return new T(left->value / right->value);
	}
	cout << "Error: Unknown operator" << endl;
}

void StartScope();
void EndScope();

/*
ii i
if f
id d
ff f
fd d
dd d

*/