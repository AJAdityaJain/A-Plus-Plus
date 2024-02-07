#include "Compiler.h"

string replaceFileExtension(string path) {
	size_t dot = path.find_last_of('.');
	return path.substr(0,dot)+".asm";
}


int main(int argc, char* argv[])
{
	if (argc < 2)
		return -1;


	///READ FILE
	vector<string> programString;
	ifstream AsmFile(argv[1]);
	string tempString;
	while (getline(AsmFile, tempString, ';')) {
		programString.push_back(tempString+";\n");
	}
	AsmFile.close();

	tempString.clear();
	tempString.shrink_to_fit();

	///TOKENIZE
	Lexer lexer = Lexer();
	lexer.tokenize(programString);
	programString.clear();
	programString.shrink_to_fit();

	///PARSE
	vector<Statement*> tree = parseStatements(lexer.tokens);
	lexer.clean();	
	//for(Statement* s : tree) s->print();

	///COMPILE
	Compiler compiler = Compiler();
	compiler.compile(tree, replaceFileExtension(argv[1]));

	///FREE
	for (Statement* s : tree) deallocstmt(s);

	return 0;
}
 