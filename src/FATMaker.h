//
// Created by Aditya Jain on 9/3/24.
//

#pragma once
#include "Parser.h"
#include <fstream>

void createFatFile(const char* filename, const vector<Statement*>& abstractTree);

void addBytes(Statement* statement, ofstream& bytes);
