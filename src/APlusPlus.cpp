#include "Compiler.h"

string replaceFileExtension(string path) {
	size_t dot = path.find_last_of('.');
	return path.substr(0,dot)+".asm";
}


int main(int argc, char* argv[])
{
	if (argc < 2) {
		cout << "No path provided";
		exit(0);
	}


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
	Parser parser = Parser(lexer.tokens);
	vector<Statement*> tree = parser.parse();
	lexer.clean();	

	///COMPILE
	Compiler compiler = Compiler();
	string outAsm = replaceFileExtension(argv[1]);
	compiler.compile(tree, outAsm);

	///FREE
	for (Statement* s : tree) deallocstmt(s);

	system(("fasm " + outAsm).c_str());

	return 0;
}
 