#include "Parser.h"

Statement* Parser::parseStatement(vector<Token*> stack, bool waitForElse) { // NOLINT(*-no-recursion)

	size_t size = stack.size();

	if (size == 0)aThrowError(2, -1);

	TokenType st0 = stack[0]->getType();
	TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new Reference(dynamic_cast<IdentifierToken*>(stack[0])->value);
		case INT:return new Int(dynamic_cast<IntToken*>(stack[0])->value);
		case FLOAT:	return new Float(dynamic_cast<FloatToken*>(stack[0])->value);
		case DOUBLE:return new Double(dynamic_cast<DoubleToken*>(stack[0])->value);
		case BIT:return new Bit(dynamic_cast<BitToken*>(stack[0])->value);
		case STRING:return new String(dynamic_cast<StringToken*>(stack[0])->value);
		default:aThrowError(0,stack[0]->ln);
		}

	}

	TokenType st1 = stack[1]->getType();

	///Scope Definition
	if (size >= 2 && st0 == CURLY_OPEN && stb == CURLY_CLOSE) {
		auto* block = new CodeBlock(parse(vector(stack.begin() + 1, stack.end() - 1)));
		return block;
	}
	///Func Call
	if (st0 == ID && st1 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE) {
		auto params = vector<Value*>();
		if (stack.size() > 3) {
			int depth = 0;
			int p = 2;
			int i = 2;
			for (; i < stack.size() - 1; i++) {
				TokenType t = stack[i]->getType();
				if (t == PARENTHESIS_OPEN) depth++;
				if (t == PARENTHESIS_CLOSE) depth--;
				if (depth == 0 && t == COMMA) {
					params.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + p, stack.begin() + i))));
					p = i + 1;
				}
			}
			params.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + p, stack.begin() + i))));
		}
		return new FuncCall(*dynamic_cast<IdentifierToken*>(stack[0]),params);
	}

	///Func Definition
	if (st0 == FUNC && st1 == ID) {
		//vector<IdentifierToken> params;

		int bi = -1;
		int depth = 0;
		for (int i = 2; i < stack.size(); i++) {
			TokenType t = stack[i]->getType();
			if (t == PARENTHESIS_OPEN) depth++;
			if (t == PARENTHESIS_CLOSE) depth--;
			//if (depth == 1 && t == ID && bi == -1) {
				//params.push_back(*(IdentifierToken*)stack[i]);
			//} 

			if (t == CURLY_OPEN) {
				bi = i;
				break;
			}
		}

		auto body = dynamic_cast<CodeBlock*>(parseStatement(vector(stack.begin() + bi, stack.end())));
		return new Func(*dynamic_cast<IdentifierToken*>(stack[1]), body);
		
	}


	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		AssignmentType at = dynamic_cast<AssignToken*>(stack[1])->value;
		if (at == DIVIDE_EQUAL) {
			auto invop = vector<Value*>();
			invop.push_back( dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end()))));
			auto op = vector<Value*>();
			op.push_back(new Reference(dynamic_cast<IdentifierToken*>(stack[0])->value));

			return new Assignment(
				*dynamic_cast<IdentifierToken*>(stack[0]),
				new MultipleOperation(
					MULTIPLY, 
					op,
					invop
					),
				EQUALS
			);
		}
		if (at == MODULO_EQUAL) {
			auto op = vector<Value*>();
			op.push_back(new Reference(dynamic_cast<IdentifierToken*>(stack[0])->value));
			op.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end()))));

			return new Assignment(
				*dynamic_cast<IdentifierToken*>(stack[0]),
				new MultipleOperation(
					MODULO,
					op,
					vector<Value*>()
					),
				EQUALS
			);

		}

		return new Assignment(
		*dynamic_cast<IdentifierToken*>(stack[0]),
		dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end())))
		, at
		);
	}

	///While
	if (st0 == WHILE) {
		Value* con = nullptr;
		CodeBlock* whileb = nullptr;

		if (st1 == PARENTHESIS_OPEN) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
					case PARENTHESIS_OPEN: depth++; break;
					case PARENTHESIS_CLOSE: depth--; break;
					default:break;
				}
				depth = abs(depth);
				if (depth == 0) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.begin() + i)));
					whileb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end())));
					break;
				}
			}
		}
		else 
			for (int i = 1; i < stack.size() - 1; i++)
				if (stack[i]->getType() == COLON) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 1, stack.begin() + i)));
					whileb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end())));
					break;
				}

		if(con != nullptr && whileb != nullptr)
			return new WhileStatement(con, whileb);
	}

	///If
	if (st0 == IF) {
		Value* con = nullptr;
		CodeBlock* ifb = nullptr;

		if (st1 == PARENTHESIS_OPEN) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
					case PARENTHESIS_OPEN: depth++; break;
					case PARENTHESIS_CLOSE: depth--; break;
					default:break;
				}
				depth = abs(depth);
				if (depth == 0) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.begin() + i)));
					ifb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end())));
					break;
				}

			}
		}
		else 
			for (int i = 1; i < stack.size() - 1; i++)
				if (stack[i]->getType() == COLON) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 1, stack.begin() + i)));
					ifb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end())));
					break;
				}
		
		if(con != nullptr && ifb != nullptr)
			return new IfStatement(con, ifb);
	}

	///Else
	if (st0 == ELSE) {
		return new ElseStatement(new CodeBlock(parseStatement(vector(stack.begin() + 1, stack.end()))));
	}

	///Parenthesis
	if (size >= 3 && st0 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				switch (stack[i]->getType()) {
					case PARENTHESIS_OPEN: depth++; break;
					case PARENTHESIS_CLOSE: depth--; break;
					default:break;
				}
				depth = abs(depth);
			}
			if (depth == 0)
				return parseStatement(vector(stack.begin() + 1, stack.end() - 1));
			aThrowError(4, stack[0]->ln);
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
					MultipleOperatorType tt = dynamic_cast<OperatorToken*>(t)->biValue;
					if (tt == MINUS) tt = PLUS;
					if (tt == DIVIDE) tt = MULTIPLY;
					if (bot == NONE_BI_OPERATOR || tt < bot) bot = tt;
				}
				break;
			}
				default:break;
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
				MultipleOperatorType mop = dynamic_cast<OperatorToken*>(t)->biValue;
				if (i != 0 && depth == 0) {
					if (mop == bot) {
						tokenIdx.push_back(i);
					}
					else if (mop == _bot) {
						tokenIdx.push_back(-i);
					}
				}
				break;
			}
				default:break;
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
					operands.push_back(dynamic_cast<Value*>(parseStatement(vector(
						stack.begin() + (tokenIdx[j] + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					))));
				}
				else {
					invoperands.push_back(dynamic_cast<Value*>(parseStatement(vector(
						stack.begin() + (abs(tokenIdx[j]) + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					))));
				}
			}

			return new MultipleOperation(bot, operands, invoperands);
		}
	}

	///Unary Operation 
	if (size >= 2 && st0 == OPERATOR) {
		UnaryOperatorType uop = dynamic_cast<OperatorToken*>(stack[0])->uValue;
		if (uop == POSITIVE) {
			return parseStatement(vector(stack.begin() + 1, stack.end()));
		}
		return new UnaryOperation(uop,dynamic_cast<Value*>( parseStatement(vector(stack.begin() + 1, stack.end()))));
	}
	aThrowError(1,stack[0]->ln);
	return nullptr;
}

vector<Statement*> Parser::parse() {
	return parse(tks);
}

vector<Statement*> Parser::parse(const vector<Token*>& stack) { // NOLINT(*-no-recursion)
	auto statements = vector<Statement*>();
	auto subStack = vector<Token*>();

	int depth = 0;
	bool shouldParse = false;

	for (auto i : stack) {
		const TokenType sti = i->getType();

		if (sti == CURLY_OPEN) depth++;
		if (sti == CURLY_CLOSE) depth--;

		if (sti == LINE_END && depth == 0)shouldParse = true;
		else {
			subStack.push_back(i);
			if (sti == CURLY_CLOSE && depth == 0)shouldParse = true;
		}


		if (shouldParse) {
			shouldParse = false;
			if (!subStack.empty()) {
				statements.push_back(parseStatement(subStack));
			}
			subStack.clear();
		}
	}

	return statements;
}


