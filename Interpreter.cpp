﻿#include "Interpreter.h"


int main(int argc, char* argv[])
{
	cout << argc << ',' << * argv << endl << endl;

	vector<string> programString;

	///READ FILE
	cout << "READING" << endl << endl;


	ifstream File("C:\\Users\\agnee\\Code\\C++Proj\\Interpreter\\test.app");
	string tempString;
	while (getline(File, tempString, ';')) {
		tempString.erase(tempString.begin(),find_if_not(tempString.begin(), tempString.end(),[](char c) { return isspace(c); }));
		tempString.erase(find_if_not(tempString.rbegin(), tempString.rend(),[](char c) { return isspace(c); }).base(),tempString.end());
		if(tempString.size() > 0)programString.push_back(tempString+";");
	}

	File.close();
	


	vector<Token*> tokens;
	///TOKENIZE
	cout << "TOKENIZING" << endl << endl;
	tokens = tokenize(programString);

	PROGRAM program = PROGRAM();
	//PARSe
	cout << "PARSING" << endl << endl;
	program = parse(tokens);


	//program.code.code.push_back();

	//program.print();

	return 0;
}
