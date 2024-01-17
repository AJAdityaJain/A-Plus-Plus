#include "Parser.h"


CodeBlock* parseTree(vector<Token*> tokens) {
	return (CodeBlock*)parseStatement(tokens);
}



Statement* parseStatement(vector<Token*> stack) {
	size_t size = stack.size();

	if (size == 0) throw invalid_argument("Empty statement");

	TokenType st0 = stack[0]->getType();
	TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new Identifier(((IdentifierToken*)stack[0])->value);
		case INT:return new Int(((IntToken*)stack[0])->value);
		case FLOAT:	return new Float(((FloatToken*)stack[0])->value);
		case DOUBLE:return new Double(((DoubleToken*)stack[0])->value);
		case BIT:return new Bit(((BitToken*)stack[0])->value);
		case STRING:return new String(((StringToken*)stack[0])->value);
		}

	}
	
	TokenType st1 = stack[1]->getType();

	///Scope Definition
	if(size >= 2 && st0 == CURLY_OPEN && stb == CURLY_CLOSE) {
		CodeBlock* block = new CodeBlock();

		block->code = parseStatements(vector<Token*>(stack.begin()+1,stack.end()-1));
		return block;
	}
	///Variable Definition
	if (size >= 4 && st0 == LET && st1 == ID && stack[2]->getType() == ASSIGN) {
		Definition* def = new Definition(
			new Identifier(((IdentifierToken*)stack[1])->value),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
		);
		return def;
	}
	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		Assignment* def = new Assignment(
			new Identifier(((IdentifierToken*)stack[0])->value),
			(VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.end())),
			((AssignToken*)st1)->value
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
			return new Parenthesis((VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1)));
	}
	///Operation 
	if (size >= 3) {
		//printf("\x1B[32mTexting\033[0m\t\t\n");	for (Token* t : stack) { cout << getToken(t->getType()) << " "; }	printf("\n\x1B[31mTexting\033[0m\t\t\n\n\n");

		VALUED* LHS = nullptr;
		VALUED* RHS = nullptr;
		OperatorType op = (NONE_OPERATOR);

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
			
			if (depth==0&&(tt == OPERATOR)) {
				subStack.clear();


				op = ((OperatorToken*)t)->value;
				RHS = (VALUED*)parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end()));

				if (LHS != nullptr ) 
					return new Operation(LHS, op, RHS);
			}	
			else if (depth == 0 && op == NONE_OPERATOR) {
				LHS = (VALUED*)parseStatement(subStack);
				subStack.clear();

			}
			i++;
		}
	}


	throw invalid_argument("Invalid Statement");
}

vector<Statement*> parseStatements(vector<Token*> stack) {
	vector <Statement*> statements = vector<Statement*>();
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


