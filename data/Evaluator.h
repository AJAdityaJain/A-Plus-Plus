#pragma once

#include "Parser.h"


struct Variable {
	unsigned int name;
	Token* value;
};

static vector<Variable> varsStack ;
static vector<int> scopesStack;



Token* Execute(Statement* line);


template<typename T>
Token* Operate_Unary(T* right, UnaryOperatorType op);
template<typename T>
Token* Operate_Unary_Dec(T* right, UnaryOperatorType op);


template<typename T>
Token* Operate_Binary(T* left, T* right, BinaryOperatorType op);
template<typename T>
Token* Operate_Binary_Dec(T* left, T* right, BinaryOperatorType op);

void StartScope();
void EndScope();