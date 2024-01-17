#include "Parser.h"


BLOCK* parseTree(vector<Token*> tokens) {

	BLOCK* program = (BLOCK*)parseStatement(tokens);
	
	return program;
}



STATEMENT* parseStatement(vector<Token*> stack) {
	size_t size = stack.size();

	if (size == 0) throw invalid_argument("Empty statement");

	TokenType st0 = stack[0]->getType();
	TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new ID_VAL(((IdentifierToken*)stack[0])->value);
		case INT:return new INT_VAL(((IntToken*)stack[0])->value);
		case FLOAT:	return new FLOAT_VAL(((FloatToken*)stack[0])->value);
		case DOUBLE:return new DOUBLE_VAL(((DoubleToken*)stack[0])->value);
		case BIT:return new BIT_VAL(((BitToken*)stack[0])->value);
		case STRING:return new STRING_VAL(((StringToken*)stack[0])->value);
		}

	}
	
	TokenType st1 = stack[1]->getType();

	///Scope Definition
	if(size >= 2 && st0 == CURLY_OPEN && stb == CURLY_CLOSE) {
		BLOCK* block = new BLOCK();

		block->code = parseStatements(vector<Token*>(stack.begin()+1,stack.end()-1));
		return block;
	}
	///Variable Definition
	if (size >= 4 && st0 == LET && st1 == ID && stack[2]->getType() == EQUALS) {
		DEFINITION* def = new DEFINITION(
			new ID_VAL(((IdentifierToken*)stack[1])->value),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
		);
		return def;
	}
	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == EQUALS) {
		ASSIGNMENT* def = new ASSIGNMENT(
			new ID_VAL(((IdentifierToken*)stack[0])->value),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.end()))
		);
		return def;
	}
	///Parenthesiss
	if (size >= 3 && st0 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE) {
		int depth = 0;
		for(int i = 1; i < stack.size()-1; i++){
			switch (stack[i]->getType()) {
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break;
			}
			depth = abs(depth);
		}
		if(depth == 0)
			return new PARENTHESIS((VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1)));
	}
	///Operation 
	if (size >= 3) {
		//printf("\x1B[32mTexting\033[0m\t\t\n");	for (Token* t : stack) { cout << getToken(t->getType()) << " "; }	printf("\n\x1B[31mTexting\033[0m\t\t\n\n\n");

		VALUED* LHS = nullptr;
		VALUED* RHS = nullptr;
		TokenType op = UNKNOWN;

		int depth = 0;
		vector<Token*> subStack = vector<Token*>();
		
		int i = 0;
		for (Token* t : stack) {
			subStack.push_back(t);
			TokenType tt = t->getType();

			switch (tt)
			{
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break;
			}
			
			if (depth==0&&(tt == PLUS || tt == MINUS || tt == MULTIPLY || tt == DIVIDE || tt == MODULO)) {
				subStack.clear();


				op = tt;
				RHS = (VALUED*)parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end()));

				if (LHS != nullptr ) 
					return new OPERATION(LHS, op, RHS);
			}	
			else if (depth == 0 && op == UNKNOWN) {
				LHS = (VALUED*)parseStatement(subStack);
				subStack.clear();
			}
			i++;
		}
	}


	throw invalid_argument("Invalid Statement");
}

vector<STATEMENT*> parseStatements(vector<Token*> stack) {
	vector <STATEMENT*> statements = vector<STATEMENT*>();
	vector<Token*> subStack = vector<Token*>();

	int depth = 0;
	bool shouldParse = false;

	for (int i = 0; i < stack.size(); i++) {
		TokenType sti = stack[i]->getType();

		if(sti == CURLY_OPEN) depth++;
		if (sti == CURLY_CLOSE) depth--;

		if (sti == LINE_END && depth == 0)shouldParse = true;
		
		else {
			subStack.push_back(stack[i]);
			if (sti == CURLY_CLOSE && depth == 0)shouldParse = true;
		}


		if (shouldParse) {
			shouldParse = false;
			if(subStack.size() > 0)statements.push_back(parseStatement(subStack));
			subStack.clear();
		}
	}

	return statements;
}


