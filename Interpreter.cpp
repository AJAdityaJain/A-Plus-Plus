#include "Interpreter.h"


int main(int argc, char* argv[])
{
	cout << argc << ',' << * argv << endl << endl;


	///READ FILE
	cout << "READING" << endl << endl;
	vector<string> programString;
	ifstream File("C:\\Users\\agnee\\Code\\C++Proj\\Interpreter\\test.app");
	string tempString;
	while (getline(File, tempString, ';')) {
		tempString.erase(tempString.begin(),find_if_not(tempString.begin(), tempString.end(),[](char c) { return isspace(c); }));
		tempString.erase(find_if_not(tempString.rbegin(), tempString.rend(),[](char c) { return isspace(c); }).base(),tempString.end());
		if(tempString.size() > 0)programString.push_back(tempString+";");
	}

	File.close();


	///TOKENIZE
	cout << "TOKENIZING" << endl << endl;
	vector<Token*> tokens;
	tokenize(programString, tokens);
	programString.clear();

	
	///PARSE
	cout << "PARSING" << endl << endl;
	CodeBlock* tree = parseTree(tokens);
	tokens.clear();
	

	///EXECUTE
	cout << "EXECUTING" << endl << endl;
	Execute(tree);


	return 0;
}
 