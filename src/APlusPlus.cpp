#include <chrono>
#include <filesystem>
#include "Compiler.h"

string replaceFileExtension(const string& path) {
	const size_t dot = path.find_last_of('.');
	return path.substr(0,dot)+".exe";
}


int main(const int argc, char* argv[])
{
	string input;
	string output;
	string midput;
	bool delMid = false;

	switch(argc)
	{
	case 0:	exit(0);
	case 1:{
			short applogo[] = {124,32,176,219,219,219,219,219,187,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,32,124,10,124,32,219,219,201,205,205,219,219,187,176,176,219,219,187,176,176,176,176,219,219,187,176,176,32,124,10,124,32,219,219,219,219,219,219,219,186,219,219,219,219,219,219,187,219,219,219,219,219,219,187,32,124,10,124,32,219,219,201,205,205,219,219,186,200,205,219,219,201,205,188,200,205,219,219,201,205,188,32,124,10,124,32,219,219,186,176,176,219,219,186,176,176,200,205,188,176,176,176,176,200,205,188,176,176,32,124,10,124,32,200,205,188,176,176,200,205,188,176,176,176,176,176,176,176,176,176,176,176,176,176,176,32,124,10,};
			cout <<"+------------------------+"<<endl;
			for (short c : applogo)
				cout << static_cast<char>(c);
			cout <<"+------------------------+"<<endl;
			cout << "A++ Copyright(c) "+format("{:%Y}",chrono::system_clock::now())+" Aditya Jain"<<endl;
			cout << "Use -h or -help" << endl;
			exit(0);
		}
	default:{
			for (int i = 1; i < argc; i++)
			{
				if(argv[i][0] == '-')
				{
					if(strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-version") == 0)
					{
						cout << "A++ compiler v0.3.2" << endl;
						exit(0);
					}
					if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
					{
						cout << "A++ Compiler by Aditya Jain" << endl;
						cout << "Options:" << endl;
						cout << "\tappx  -v, -version                                   : Version information" << endl;
						cout << "\tappx  -h, -help                                      : Display this information" << endl;
						cout << "\tappx <InputFile.app>                                 : Compiles app files into exe" << endl;
						cout << "\tappx <InputFile.app> <OutputFile.exe>                : Compiles app files into exe at that location" << endl;
						cout << "\tappx <InputFile.app> <OutputFile.exe> <AsmFile.asm>  : Compiles app files into exe at that location also outputs raw assembly" << endl;
						exit(0);
					}
					cout << "Invalid Option use -h for all options" << endl;
					exit(0);
				}
				if(input.empty())
					input = argv[i];
				else if(output.empty())
					output = argv[i];
				else if(midput.empty())
					midput = argv[i];
			}
			break;
		}
	}

	if(midput.empty())
	{
		filesystem::path appx = argv[0];
		srand ( time(nullptr) ); // NOLINT(*-msc51-cpp)
		midput = appx.parent_path().parent_path().string() +  "\\TEMP\\"+ to_string(rand()) +".asm"; // NOLINT(*-msc50-cpp)
		delMid = true;
	}
	if(output.empty())
		output = replaceFileExtension(input);

	{
		vector<string> programString;
		ifstream AppFile(input);
		string tempString;
		while (getline(AppFile, tempString, ';')) {
			programString.push_back(tempString+";\n");
		}
		AppFile.close();
		tempString.clear();
		tempString.shrink_to_fit();
		auto lexer = Lexer();
		lexer.tokenize(programString);
		programString.clear();
		programString.shrink_to_fit();
		auto parser = Parser(lexer.tokens);
		const vector<Statement*> tree = parser.parse();
		lexer.clean();
		auto compiler = Compiler();
		compiler.compile(tree, midput);
		for (const Statement* s : tree) delete s;
	}

	system(("fasm "+ midput + " " + output).c_str());
	if(delMid)
		system(("del " + midput).c_str());
	return 0;
}
