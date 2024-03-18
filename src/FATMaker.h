//
// Created by Aditya Jain on 9/3/24.
//

#pragma once
#include "Parser.h"
#include <fstream>

vector<Statement*> readFatFile(const char* filename);
void createFatFile(const char* filename, const vector<Statement*>& abstractTree);
Statement* readBytes(vector<uint8_t>& bytes);
void addBytes(Statement* statement, vector<uint8_t>& bytes);
