#include "Syntax.h"


BLOCK* parseTree(vector<Token*> tokens) {

	BLOCK* program = (BLOCK*)parseStatement(tokens);
	
	return program;
}



STATEMENT* parseStatement(vector<Token*> stack) {
	//printf("\x1B[32mTexting\033[0m\t\t\n");	for(Token* t : stack) {	cout << getToken(t->type) << " ";}	printf("\n\x1B[31mTexting\033[0m\t\t\n\n\n"); 
	size_t size = stack.size();

	if (size == 0) throw invalid_argument("Empty statement");
	if (size == 1) {
		if (stack[0]->type == ID)		return new ID_VAL(*(IdentifierToken*)stack[0]);
		if (stack[0]->type == INT)		return new INT_VAL(*(IntToken*)stack[0]);
		if (stack[0]->type == FLOAT)	return new FLOAT_VAL(*(FloatToken*)stack[0]);
		if (stack[0]->type == DOUBLE)	return new DOUBLE_VAL(*(DoubleToken*)stack[0]);
		if (stack[0]->type == BIT)		return new BIT_VAL(*(BitToken*)stack[0]);
		if (stack[0]->type == STRING)	return new STRING_VAL(*(StringToken*)stack[0]);

	}
	

	///Scope Definition
	if(size >= 2 && stack[0]->type == CURLY_OPEN && stack.back()->type == CURLY_CLOSE) {
		BLOCK* block = new BLOCK();

		block->code = parseStatements(vector<Token*>(stack.begin()+1,stack.end()-1));
		return block;
	}
	///Variable Definition
	if (size >= 4 && stack[0]->type == LET && stack[1]->type == ID && stack[2]->type == ASSIGN) {
		DEFINITION* def = new DEFINITION(
			new ID_VAL(*(IdentifierToken*)stack[1]),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
		);
		return def;
	}
	/// Variable Assignment
	if (size >= 3 && stack[0]->type == ID && stack[1]->type == ASSIGN) {
		ASSIGNMENT* def = new ASSIGNMENT(
			new ID_VAL(*(IdentifierToken*)stack[0]),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.end()))
		);
		return def;
	}

	///Parenthesiss
	if (size >= 3 && stack[0]->type == PARENTHESIS_OPEN && stack.back()->type == PARENTHESIS_CLOSE) {
		int depth = 0;
		for(int i = 1; i < stack.size()-1; i++){
			if (stack[i]->type == PARENTHESIS_OPEN) depth++;
			if (stack[i]->type == PARENTHESIS_CLOSE) depth--;
			depth = abs(depth);
		}
		if(depth == 0)
			return new PARENTHESIS((VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1)));
	}

	///Operation 
	if (size >= 3) {
		VALUED* LHS = nullptr;
		VALUED* RHS = nullptr;
		Tokens op = UNKNOWN;

		int depth = 0;
		vector<Token*> subStack = vector<Token*>();
		
		for (Token* t : stack) {
			subStack.push_back(t);
			if (t->type == PARENTHESIS_OPEN) depth++;
			if (t->type == PARENTHESIS_CLOSE) depth--;
			
			if (depth==0&&(t->type == PLUS || t->type == MINUS || t->type == MULTIPLY || t->type == DIVIDE || t->type == MODULO)) {
				op = t->type;
				subStack.clear();
			}	
			else if (depth == 0 && op == UNKNOWN) {
				LHS = (VALUED*)parseStatement(subStack);
				subStack.clear();
			}
			else if (depth == 0 && op != UNKNOWN) {
				RHS = (VALUED*)parseStatement(subStack);
				subStack.clear();
			}
		}


		if(LHS != nullptr && RHS != nullptr && op != UNKNOWN)
			return new OPERATION(LHS,op,RHS);
	}


	throw invalid_argument("Invalid Statement");
}

vector<STATEMENT*> parseStatements(vector<Token*> stack) {
	vector <STATEMENT*> statements = vector<STATEMENT*>();
	vector<Token*> subStack = vector<Token*>();

	int depth = 0;
	bool shouldParse = false;

	for (int i = 0; i < stack.size(); i++) {

		if(stack[i]->type == CURLY_OPEN) depth++;
		if (stack[i]->type == CURLY_CLOSE) depth--;

		if (stack[i]->type == LINE_END && depth == 0)shouldParse = true;
		
		else {
			subStack.push_back(stack[i]);
			if (stack[i]->type == CURLY_CLOSE && depth == 0)shouldParse = true;
		}


		if (shouldParse) {
			shouldParse = false;

			if(subStack.size() > 0)statements.push_back(parseStatement(subStack));
			subStack.clear();
		}


		//else {
		// 
		//	for (Token* t : subStack) {
		//		cout << "\x1B[35m" << getToken(t->type) << "\033[0m ";
		//	}
		//	cout << endl;
		//}
	}

	return statements;
}


