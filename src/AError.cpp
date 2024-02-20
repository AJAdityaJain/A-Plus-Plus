#include "AError.h"
void aThrowError(const AXXCode code,const  unsigned int line) {

	printf("\x1B[31m");

	cout << "Error Axx" << code << ":";
	if(line != -1) cout << " @line " << line << ":";
	switch (code)
	{
	case UNKNOWN_VALUE: cout << "Unidentifiable value found in code";break;
	case UNKNOWN_STATEMENT: cout << "Unexpected statement";break;
	case UNKNOWN_OPERATION: cout << "Unknown operation on values";break;
	case UNKNOWN_ESCAPE_SEQUENCE: cout << "Unknown escaped character";break;
	case ILLEGAL_EXPRESSION: cout << "Illegal expression found";break;
	case ILLEGAL_CHARACTER: cout << "Unexpected character found";break;
	case OVERSIZED_VALUE: cout << "Oversized value. Fatal error";break;
	case OVERFLOW_REGISTER: cout << "Now listen to me. Have you heard of 'CODE READABLITY'.\nWHAT THE HELL HAVE YOU WRITTEN.\nTry making more variables in your statements and not cram EVERY SINGLE OPERATION IN ONE LINE!!!\n";break;
	case MISMATCHED_BRACKET: cout << "Mismatched bracket/brace/parenthesis";break;
	case ASSIGNED_WRONG_TYPE: cout << "Variable assigned to a different type from what it was defined with";break;
	case REDUNDANT_IMMUTABLE_ARRAY: cout << "0 element immutable array. Why is it here?";break;
	}
	printf("\n\033[0m\t\t");

	exit(-code);
}