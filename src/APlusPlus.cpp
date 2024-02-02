#include "Compiler.h"


int main(int argc, char* argv[])
{
	cout << argc << ',' << * argv << endl << endl;


	///READ FILE
	cout << "READING" << endl << endl;
	vector<string> programString;
	ifstream AsmFile("C:\\Users\\agnee\\Code\\C++Proj\\APlusPlus\\test.app");
	string tempString;
	while (getline(AsmFile, tempString, ';')) {
		//tempString.erase(tempString.begin(),find_if_not(tempString.begin(), tempString.end(),[](char c) { return isspace(c); }));
		//tempString.erase(find_if_not(tempString.rbegin(), tempString.rend(),[](char c) { return isspace(c); }).base(),tempString.end());
		programString.push_back(tempString+";\n");
	}
	AsmFile.close();

	tempString.clear();
	tempString.shrink_to_fit();

	
	///TOKENIZE
	cout << "TOKENIZING" << endl << endl;
	Lexer lexer = Lexer();
	lexer.tokenize(programString);

	programString.clear();
	programString.shrink_to_fit();



	///PARSE
	cout << "PARSING" << endl << endl;
	vector<Statement*> tree = parseStatements(lexer.tokens);
	lexer.clean();
	
	for(Statement* s : tree) s->print();

	cout << "COMPILING" << endl << endl;
	
	Compiler compiler = Compiler();
	compiler.compile(tree,"C:\\Users\\agnee\\Code\\C++Proj\\APlusPlus");


	cout << "FREEING" << endl << endl;
	for (Statement* s : tree) deallocstmt(s);


	return 0;
}
 