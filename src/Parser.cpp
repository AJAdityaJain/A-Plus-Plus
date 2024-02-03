#include "Parser.h"


void deallocstmt(Statement * statement){
	//cout << "Deallocating "<< statement->getType()  <<" at " << statement << endl;
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
		case FUNC_DEFINITION: {
			delete (Func*)statement;
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
		case MULTI_OPERATION: {
			delete (MultipleOperation*)statement;
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
		default:
			break;
		}	
}

CodeBlock* parseTree(vector<Token*> tokens) {
	return (CodeBlock*)parseStatement(tokens);
}



Statement* parseStatement(vector<Token*> stack, bool waitForElse) {

	size_t size = stack.size();

	if (size == 0)aThrowError(2, -1);

	TokenType st0 = stack[0]->getType();
	TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new Identifier(((IdentifierToken*)stack[0])->value, {});
		case INT:return new Int(*((IntToken*)stack[0]));
		case FLOAT:	return new Float(*((FloatToken*)stack[0]));
		case DOUBLE:return new Double(*((DoubleToken*)stack[0]));
		case BIT:return new Bit(*((BitToken*)stack[0]));
		case STRING:return new String(*((StringToken*)stack[0]));
		default:aThrowError(0,stack[0]->ln);
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


	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		AssignmentType at = ((AssignToken*)stack[1])->value;
		IdentifierToken id = *(IdentifierToken*)stack[0];
		Value* val = nullptr;

		switch (at) {
		case PLUS_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), PLUS, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MINUS_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), MINUS, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MULTIPLY_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), MULTIPLY, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case DIVIDE_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), DIVIDE, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case MODULO_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), MODULO, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case BITWISE_OR_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), BITWISE_OR, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		case BITWISE_AND_EQUAL:
			//Value = new BinaryOperation(new Identifier(*id), BITWISE_AND, parseStatement(vector<Token*>(stack.begin() + 2, stack.end())));
			break;
		default:
			val = (Value*) parseStatement(vector<Token*>(stack.begin() + 2, stack.end()));
			break;
		}
		

		return new Assignment(id, val);
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
				if (depth == 0) return new WhileStatement ((Value*)parseStatement(vector<Token*>(stack.begin() + 2, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

			}
		}
		for (int i = 1; i < stack.size() - 1; i++)
			if (stack[i]->getType() == COLON)
				return new WhileStatement((Value*)parseStatement(vector<Token*>(stack.begin() + 1, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

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
				if (depth == 0) return new IfStatement((Value*)parseStatement(vector<Token*>(stack.begin() + 2, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));


			}
		}
		for (int i = 1; i < stack.size() - 1; i++)
			if (stack[i]->getType() == COLON)
				return new IfStatement((Value*)parseStatement(vector<Token*>(stack.begin() + 1, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

	}

	///Else
	if (st0 == ELSE) {
		return new ElseStatement(parseStatement(vector<Token*>(stack.begin() + 1, stack.end())));
	}

	///Parenthesis
	if (size >= 3 && st0 == PARENTHESIS_OPEN) {
		if (stb == PARENTHESIS_CLOSE) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
				case PARENTHESIS_OPEN: depth++; break;
				case PARENTHESIS_CLOSE: depth--; break;
				}
				depth = abs(depth);
			}
			if (depth == 0)
				return parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1));
		}
		else aThrowError(4, stack[0]->ln);
	}


	///Binary Operation 
	if (size >= 3) {
		int i = 0;
		int depth = 0;
		vector<int> tokenIdx;
		tokenIdx.push_back(-1);
		MultipleOperatorType bot = NONE_BI_OPERATOR;
		MultipleOperatorType _bot = NONE_BI_OPERATOR;

		for (Token* t : stack) {
			switch (t->getType())
			{
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break;
			case OPERATOR: {
				if (i != 0 && depth == 0) {
					MultipleOperatorType tt = ((OperatorToken*)t)->biValue;
					if (tt == MINUS) tt = PLUS;
					if (tt == DIVIDE) tt = MULTIPLY;
					if (bot == NONE_BI_OPERATOR || tt < bot) {
						bot = tt;
					}
				}
				break;
			}
			}
			i++;
		}

		if (bot == PLUS) _bot = MINUS;
		if (bot == MULTIPLY) _bot = DIVIDE;
		i = 0;
		for (Token* t : stack) {
			switch (t->getType())
			{
			case PARENTHESIS_OPEN: depth++; break;
			case PARENTHESIS_CLOSE: depth--; break; 
			case OPERATOR: {
				if (i != 0 && depth == 0) {
					MultipleOperatorType a = ((OperatorToken*)t)->biValue;
					if (a == bot) {
						tokenIdx.push_back(i);
					}
					else if (a == _bot) {
						tokenIdx.push_back(-i);
					}
				}
				break;
			}
			}
			i++;
		}

		tokenIdx.push_back(i);

		if (bot != NONE_BI_OPERATOR) {
			vector<Value*> operands;
			vector<Value*> invoperands;


			for (size_t j = 0; j < tokenIdx.size()-1; j++)
			{
				if (tokenIdx[j] > 0 || j == 0) {
					operands.push_back((Value*)parseStatement(vector<Token*>(
						stack.begin() + (tokenIdx[j] + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					)));
				}
				else {
					invoperands.push_back((Value*)parseStatement(vector<Token*>(
						stack.begin() + (abs(tokenIdx[j]) + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					)));
				}
			}

			return new MultipleOperation(bot, operands, invoperands);
		}
	}

	///Unary Operation 
	if (size >= 2 && st0 == OPERATOR) {
		UnaryOperatorType uop = ((OperatorToken*)stack[0])->uValue;
		if (uop == POSITIVE) {
			return parseStatement(vector<Token*>(stack.begin() + 1, stack.end()));
		}
		return new UnaryOperation(uop,(Value*) parseStatement(vector<Token*>(stack.begin() + 1, stack.end())));
	}
	aThrowError(1,stack[0]->ln);
	//return nullstmt;
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


