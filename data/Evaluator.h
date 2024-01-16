#pragma once

#include "Parser.h"
#include <map>


struct Variable {
	string& name;
	Token* value;
};

static vector<Variable> varsStack ;
static vector<int> scopesStack;



Token* Execute(STATEMENT* line);



void StartScope();
void EndScope();