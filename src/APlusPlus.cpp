#include "Linker.h"


int main(const int argc, char* argv[])
{
	switch(argc)
	{
	case 0:	exit(0);
	case 1:{
			short applogo[] = {124,32,176,219,219,219,219,219,187,176,176,176,176,176,176,176,176,176,176,176,176,176,176,176,32,124,10,124,32,219,219,201,205,205,219,219,187,176,176,219,219,187,176,176,176,176,219,219,187,176,176,32,124,10,124,32,219,219,219,219,219,219,219,186,219,219,219,219,219,219,187,219,219,219,219,219,219,187,32,124,10,124,32,219,219,201,205,205,219,219,186,200,205,219,219,201,205,188,200,205,219,219,201,205,188,32,124,10,124,32,219,219,186,176,176,219,219,186,176,176,200,205,188,176,176,176,176,200,205,188,176,176,32,124,10,124,32,200,205,188,176,176,200,205,188,176,176,176,176,176,176,176,176,176,176,176,176,176,176,32,124,10,};
			cout <<"+------------------------+"<<endl;
			for (const short c : applogo)
				cout << static_cast<char>(c);
			cout <<"+------------------------+"<<endl;
			cout << "A++ Copyright(c) "+format("{:%Y}",chrono::system_clock::now())+" Aditya Jain"<<endl;
			cout << "Use -h or -help" << endl;
			exit(0);
		}
	default:{
			bool config = false;
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
						cout << "\tappx  -c, -config                                    : Attach file for Config" << endl;
						cout << "\tappx <InputFile.app>                                 : Compiles app files into exe" << endl;
						cout << "\tappx <InputFile.app> <OutputFile.exe>                : Compiles app files into exe at that location" << endl;
						cout << "\tappx <InputFile.app> <OutputFile.exe> <AsmFile.asm>  : Compiles app files into exe at that location also outputs raw assembly" << endl;
						exit(0);
					}
					if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"-config") == 0)
						config = true;
					else{
						cout << "Invalid Option use -h for all options" << endl;
						exit(0);
					}
				}
				else if(config)
				{
					linkerConfig(argv[i]);
					break;
				}
				else
				{
					if(files.empty())
						files.emplace_back(argv[i]);
					else if(output.empty())
						output = argv[i];
					else if(midput.empty())
						midput = argv[i];
				}
			}
			break;
		}
	}

	pipeline();

	return 0;
}
