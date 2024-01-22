#pragma once

#include "Parser.h"


static vector<Func*> funcStack;


void ExecuteTree(vector<Statement*> tree);
Token* Execute(Statement* line, FuncInstance* parentFunc);


template<typename T>
Token* Operate_Unary(T* right, UnaryOperatorType op);
template<typename T>
Token* Operate_Unary_Dec(T* right, UnaryOperatorType op);


template<typename T>
Token* Operate_Binary(T* left, T* right, BinaryOperatorType op);
template<typename T>
Token* Operate_Binary_Dec(T* left, T* right, BinaryOperatorType op);

