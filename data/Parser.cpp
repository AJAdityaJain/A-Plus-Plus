#include "Parser.h"


void deallocstmt(Statement * statement){
	cout << "Deallocating "<< statement->getType()  <<" at " << statement << endl;
	switch (statement->getType())
		{
		case NONE_STMT: {
			delete statement;
			break;
		}
		case BIT_STMT: {
			delete (Bit*)statement;
			break;
		}
		case INT_STMT: {
			delete (Int*)statement;
			break;
		}
		case FLOAT_STMT: {
			delete (Float*)statement;
			break;
		}
		case DOUBLE_STMT: {
			delete (Double*)statement;
			break;
		}
		case STRING_STMT: {
			delete (String*)statement;
			break;
		}
		case ID_STMT: {
			delete (Identifier*)statement;
			break;
		}
		case CALL: {
			delete (CallingFunc*)statement;
			break;
		}
		case FUNC_DEFINITION: {
			delete (Func*)statement;
			break;
		}
		case DEFINITION: {
			delete (Definition*)statement;
			break;
		}
		case ASSIGNMENT: {
			delete (Assignment*)statement;
			break;
		}
		case WHILE_STMT: {
			delete (WhileStatement*)statement;
			break;
		}
		case IF_STMT: {
			delete (IfStatement*)statement;
			break;
		}
		case ELSE_STMT: {
			delete (ElseStatement*)statement;
			break;
		}
		case BI_OPERATION: {
			delete (BinaryOperation*)statement;
			break;
		}
		case UN_OPERATION: {
			delete (UnaryOperation*)statement;
			break;
		}
		case SCOPE: {
			delete (CodeBlock*)statement;
			break;
		}
		case PARENTHESIS: {
			delete (Parenthesis*)statement;
			break;
		}
		default:
			break;
		}	
}

CodeBlock* parseTree(vector<Token*> tokens) {
	return (CodeBlock*)parseStatement(tokens);
}



Statement* parseStatement(vector<Token*> stack, bool waitForElse) {
	//printf("\x1B[32mTexting\033[0m\t\t\n");	for (Token* t : stack) printToken(t);	printf("\n\x1B[31mTexting\033[0m\t\t\n\n\n");

	size_t size = stack.size();

	if (size == 0)
		return nullstmt;

	TokenType st0 = stack[0]->getType();
	TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new Identifier(*((IdentifierToken*)stack[0]));
		case INT:return new Int(*((IntToken*)stack[0]));
		case FLOAT:	return new Float(*((FloatToken*)stack[0]));
		case DOUBLE:return new Double(*((DoubleToken*)stack[0]));
		case BIT:return new Bit(*((BitToken*)stack[0]));
		case STRING:return new String(*((StringToken*)stack[0]));
		default:printf("\x1B[31m Unexpected identifier \033[0m\t\t\n");
		}

	}

	TokenType st1 = stack[1]->getType();

	///Scope Definition
	if (size >= 2 && st0 == CURLY_OPEN && stb == CURLY_CLOSE) {
		CodeBlock* block = new CodeBlock();

		block->code = parseStatements(vector<Token*>(stack.begin() + 1, stack.end() - 1));
		return block;
	}

	///Func Definition
	if (st0 == FUNC && st1 == ID) {
		vector<IdentifierToken> params;

		int bi = -1;
		int depth = 0;
		for (int i = 2; i < stack.size(); i++) {
			TokenType t = stack[i]->getType();
			if (t == PARENTHESIS_OPEN) depth++;
			if (t == PARENTHESIS_CLOSE) depth--;
			if (depth == 1 && t == ID && bi == -1) {
				params.push_back(*(IdentifierToken*)stack[i]);
			} 

			if (depth == 0 && t == CURLY_OPEN) {
				bi = i;
			}
		}

		CodeBlock* body = (CodeBlock*)parseStatement(vector<Token*>(stack.begin() + bi, stack.end()));
		return new Func(*(IdentifierToken*)stack[1], params, body);
		
	}

	///Variable Definition
	if (size >= 4 && st0 == LET && st1 == ID && stack[2]->getType() == ASSIGN) {
		if (((AssignToken*)stack[2])->value == EQUALS) {
			return new Definition(
					new Identifier(((IdentifierToken*)stack[1])->value),
					parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
				);
		}
	}

	///Calling
	if (size >= 3 && st0 == ID && st1 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE){
		vector<Statement*> params;
		vector<Token*> sub;
		int depth = 0;


		for (int i = 2; i < stack.size() - 1; i++)
		{
			TokenType t = stack[i]->getType();

			if (t == PARENTHESIS_OPEN) depth++;
			if (t == PARENTHESIS_CLOSE) depth--;
			if (depth == 0 && t == COMMA) {
				params.push_back(parseStatement(sub));
				sub.clear();
			}
			else sub.push_back(stack[i]);

		}
		if(sub.size() >0)
			params.push_back(parseStatement(sub));

		return new CallingFunc(*(IdentifierToken*)stack[0],params);
	}


	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		AssignmentType at = ((AssignToken*)stack[1])->value;
		Identifier* id = new Identifier(((IdentifierToken*)stack[0])->value);
		Statement* Value = nullptr;

		switch (at) {
		case PLUS_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), PLUS, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MINUS_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), MINUS, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MULTIPLY_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), MULTIPLY, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case DIVIDE_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), DIVIDE, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MODULO_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), MODULO, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case BITWISE_OR_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), BITWISE_OR, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case BITWISE_AND_EQUAL:
			Value = new BinaryOperation(new Identifier(*id), BITWISE_AND, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		default:
			Value = parseStatement(vector<Token*>(stack.begin() + 2, stack.end()));
			break;
		}
		

		return new Assignment(id, Value);
	}

	///While
	if (st0 == WHILE) {
		if (st1 == PARENTHESIS_OPEN) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
				case PARENTHESIS_OPEN: depth++; break;
				case PARENTHESIS_CLOSE: depth--; break;
				}
				depth = abs(depth);
				if (depth == 0) return new WhileStatement (parseStatement(vector<Token*>(stack.begin() + 2, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

			}
		}
		for (int i = 1; i < stack.size() - 1; i++)
			if (stack[i]->getType() == COLON)
				return new WhileStatement(parseStatement(vector<Token*>(stack.begin() + 1, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

	}

	///If
	if (st0 == IF) {
		if (st1 == PARENTHESIS_OPEN) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
				case PARENTHESIS_OPEN: depth++; break;
				case PARENTHESIS_CLOSE: depth--; break;
				}
				depth = abs(depth);
				if (depth == 0) return new IfStatement(parseStatement(vector<Token*>(stack.begin() + 2, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));


			}
		}
		for (int i = 1; i < stack.size() - 1; i++)
			if (stack[i]->getType() == COLON)
				return new IfStatement(parseStatement(vector<Token*>(stack.begin() + 1, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

	}

	///Else
	if (st0 == ELSE) {
		return new ElseStatement(parseStatement(vector<Token*>(stack.begin() + 1, stack.end())));
	}

	///Parenthesis
	if (size >= 3 && st0 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE) {
		int depth = 0;
		for (int i = 1; i < stack.size() - 1; i++) {
			switch (stack[i]->getType()) {
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break;
			}
			depth = abs(depth);
		}
		if (depth == 0)
			return new Parenthesis(parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1)));
	}


	///Binary Operation 
	if (size >= 3) {
		int i = 0;
		int depth = 0;
		int tokenIdx = 0;
		BinaryOperatorType bot = NONE_BI_OPERATOR;

		for (Token* t : stack) {
			switch (t->getType())
			{
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break;
			case OPERATOR: {
				if (i != 0 && depth == 0) {
					BinaryOperatorType tt = ((OperatorToken*)t)->biValue;
					if (bot == NONE_BI_OPERATOR) {
						bot = tt;
						tokenIdx = i;
					}
					else if (tt < bot) {
						bot = tt;
						tokenIdx = i;
					}

				}
				break;
			}
			}
			i++;
		}

		if (bot != NONE_BI_OPERATOR) {
				return new BinaryOperation(
				parseStatement(vector<Token*>(
					stack.begin(),
					stack.begin() + tokenIdx
				)),
				bot,
				parseStatement(vector<Token*>(
					stack.begin() + tokenIdx + 1,
					stack.end()
				))
			);
		}
	}

	///Unary Operation 
	if (size >= 2 && st0 == OPERATOR) {
		return new UnaryOperation(((OperatorToken*)stack[0])->uValue, parseStatement(vector<Token*>(stack.begin() + 1, stack.end())));
	}

	return nullstmt;
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


