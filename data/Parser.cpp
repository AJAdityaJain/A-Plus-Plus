#include "Parser.h"


CodeBlock* parseTree(vector<Token*> tokens) {
	return (CodeBlock*)parseStatement(tokens);
}



Statement* parseStatement(vector<Token*> stack, bool waitForElse) {
	printf("\x1B[32mTexting\033[0m\t\t\n");	for (Token* t : stack) printToken(t);	printf("\n\x1B[31mTexting\033[0m\t\t\n\n\n");

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
		default:cout << "ERROR : Unable to identify datatype";
		}

	}

	TokenType st1 = stack[1]->getType();

	///Scope Definition
	if (size >= 2 && st0 == CURLY_OPEN && stb == CURLY_CLOSE) {
		CodeBlock* block = new CodeBlock();

		block->code = parseStatements(vector<Token*>(stack.begin() + 1, stack.end() - 1));
		return block;
	}

	///Variable Definition
	if (size >= 4 && st0 == LET && st1 == ID && stack[2]->getType() == ASSIGN) {
		if (((AssignToken*)stack[2])->value == EQUALS) {
			Definition* def = new Definition(
				new Identifier(((IdentifierToken*)stack[1])->value),
				(VALUED*)parseStatement(vector<Token*>(stack.begin() + 3, stack.end()))
			);
			return def;
		}
	}

	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		AssignmentType at = ((AssignToken*)stack[1])->value;
		Identifier* id = new Identifier(((IdentifierToken*)stack[0])->value);
		VALUED* Value = (VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.end()));

		switch (at) {
		case PLUS_EQUAL:
			Value = new BinaryOperation(id, PLUS, Value);
			break;
		case MINUS_EQUAL:
			Value = new BinaryOperation(id, MINUS, Value);
			break;
		case MULTIPLY_EQUAL:
			Value = new BinaryOperation(id, MULTIPLY, Value);
			break;
		case DIVIDE_EQUAL:
			Value = new BinaryOperation(id, DIVIDE, Value);
			break;
		case MODULO_EQUAL:
			Value = new BinaryOperation(id, MODULO, Value);
			break;
		case BITWISE_OR_EQUAL:
			Value = new BinaryOperation(id, BITWISE_OR, Value);
			break;
		case BITWISE_AND_EQUAL:
			Value = new BinaryOperation(id, BITWISE_AND, Value);
			break;
		}

		return new Assignment(id, Value);
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
				if (depth == 0) return new IfStatement((VALUED*)parseStatement(vector<Token*>(stack.begin() + 2, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));

				
			}
		}
		for (int i = 1; i < stack.size() - 1; i++) 
			if (stack[i]->getType() == COLON) 
				return new IfStatement((VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.begin() + i)), parseStatement(vector<Token*>(stack.begin() + i + 1, stack.end())));		

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
			return new Parenthesis((VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.end() - 1)));
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
				(VALUED*)parseStatement(vector<Token*>(
					stack.begin(),
					stack.begin() + tokenIdx
				)),
				bot,
				(VALUED*)parseStatement(vector<Token*>(
					stack.begin() + tokenIdx + 1,
					stack.end()
				))
			);
		}
	}

	///Unary Operation 
	if (size >= 2 && st0 == OPERATOR) {
		return new UnaryOperation(((OperatorToken*)stack[0])->uValue, (VALUED*)parseStatement(vector<Token*>(stack.begin() + 1, stack.end())));
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


