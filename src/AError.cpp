#include "AError.h"
void aThrowError(char code, unsigned int line) {

	printf("\x1B[31m");

	cout << "Error Axx" << (int)code << ":";
	switch (code)
	{
	case 0: cout << "Unexpected token found in line " << line; break;
	case 1: cout << "Unidentifiable statement found in line " << line; break;
	case 2: cout << "Fatal error. Statement with unrecognizable information"; break;
	case 3: cout << "Unknown literal found in line " << line; break;
	case 4: cout << "No closure bracket found in the expression at line " << line; break;
	case 5: cout << "Fatal error. Variable width exceeds stack size"; break;
	case 6: cout << "Now listen to me. Have you heard of 'CODE READABLITY'.\nWHAT THE HELL HAVE YOU WRITTEN.\nI aint spending a singular second tryna understand that.\nTry making more variables in your statements and not cram EVERY SINGLE OPERATION IN ONE LINE!!!\n"; break;
	default:break;
	}
	printf("\033[0m\t\t");

	exit(-code);
	throw invalid_argument("Check terminal for more information");
};