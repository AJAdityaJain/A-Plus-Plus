#pragma once

#include "Parser.h"
#include <map>


struct Variable {
	string& name;
	Token* value;
};

static vector<Variable> varsStack ;
static vector<int> scopesStack;


Token* ExecuteBlock(BLOCK* block);

Token* ExecuteLine(STATEMENT* line);



void StartScope();
void EndScope();