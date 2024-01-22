#include "APlusPlus.h"


int main(int argc, char* argv[])
{
	cout << argc << ',' << * argv << endl << endl;


	///READ FILE
	cout << "READING" << endl << endl;
	vector<string> programString;
	ifstream File("C:\\Users\\agnee\\Code\\C++Proj\\APlusPlus\\test.app");
	string tempString;
	while (getline(File, tempString, ';')) {
		tempString.erase(tempString.begin(),find_if_not(tempString.begin(), tempString.end(),[](char c) { return isspace(c); }));
		tempString.erase(find_if_not(tempString.rbegin(), tempString.rend(),[](char c) { return isspace(c); }).base(),tempString.end());
		if(tempString.size() > 0)programString.push_back(tempString+";");
	}
	File.close();

	tempString.clear();
	tempString.shrink_to_fit();


	///TOKENIZE
	cout << "TOKENIZING" << endl << endl;
	vector<Token*> tokens;
	tokenize(programString, tokens);
	programString.clear();
	programString.shrink_to_fit();



	///PARSE
	cout << "PARSING" << endl << endl;
	vector<Statement*> tree = parseStatements(tokens);
	for (Token* t : tokens) delete t;
	tokens.clear();
	tokens.shrink_to_fit();
	
	//for(Statement* s : tree) s->print();
	///EXECUTE
	cout << "EXECUTING" << endl << endl;
	ExecuteTree(tree);
	
	cout << "FREEING" << endl << endl;
	for (Statement* s : tree) deallocstmt(s);

	cin >> tempString;

	return 0;
}
 