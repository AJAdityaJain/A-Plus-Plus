#include "Parser.h"

Statement* Parser::parseStatement(vector<Token*> stack, const unsigned int line) { // NOLINT(*-no-recursion)

	const size_t size = stack.size();

	if (size == 0)aThrowError(2, -1);

	const TokenType st0 = stack[0]->getType();
	const TokenType stb = stack.back()->getType();

	if (size == 1) {
		switch (st0) {
		case ID:return new Reference(dynamic_cast<IdentifierToken*>(stack[0])->value);
		case INT:return new Int(dynamic_cast<IntToken*>(stack[0])->value);
		case FLOAT:	return new Float(dynamic_cast<FloatToken*>(stack[0])->value);
		case DOUBLE:return new Double(dynamic_cast<DoubleToken*>(stack[0])->value);
		case BIT:return new Bit(dynamic_cast<BitToken*>(stack[0])->value);
		case STRING:return new String(dynamic_cast<StringToken*>(stack[0])->value);
		default:aThrowError(0,line);
		}

	}

	const TokenType st1 = stack[1]->getType();

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
				checkDepth(t,depth);
				if (depth == 0 && t == COMMA) {
					params.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + p, stack.begin() + i),line)));
					p = i + 1;
				}
			}
			params.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + p, stack.begin() + i),line)));
		}
		return new FuncCall(*dynamic_cast<IdentifierToken*>(stack[0]),params);
	}

	///Func Definition
	if (st0 == FUNC && st1 == ID) {
		//vector<IdentifierToken> params;

		int bi = -1;
		int depth = 0;
		for (int i = 2; i < stack.size(); i++) {
			const TokenType t = stack[i]->getType();
			checkDepth(t,depth);
			//if (depth == 1 && t == ID && bi == -1) {
				//params.push_back(*(IdentifierToken*)stack[i]);
			//} 

			if (t == CURLY_OPEN) {
				bi = i;
				break;
			}
		}

		const auto body = dynamic_cast<CodeBlock*>(parseStatement(vector(stack.begin() + bi, stack.end()),line));
		return new Func(*dynamic_cast<IdentifierToken*>(stack[1]), body);
		
	}

	//Array def
	if(st0 == BRACKET_OPEN && stb == BRACKET_CLOSE){
		auto values = vector<Value*>();
		if (stack.size() > 2) {
			int tstart = 1;
			int depth = 0;
			for(int i = 1; i < stack.size() - 1; i++){
				const TokenType toktype = stack[i]->getType();
				checkDepth(toktype,depth);
				if(depth == 0 && toktype == COMMA){
					values.push_back(dynamic_cast<Value*>(
						parseStatement(vector(stack.begin() + tstart, stack.begin() + i),line))
						);
					tstart = i + 1;
				}
			}
			values.push_back(dynamic_cast<Value*>(
				parseStatement(vector(stack.begin() + tstart, stack.end() - 1),line))
				);
		}
		return new Array(values);
	}

	//Array Access
	if(st0 == ID && st1 == BRACKET_OPEN && stb == BRACKET_CLOSE){
		return new ArrayAccess(
			*dynamic_cast<IdentifierToken*>(stack[0]),
			dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end() - 1),line))
		);
	}

	/// Variable Assignment
	if (size >= 3 && st0 == ID && st1 == ASSIGN) {
		AssignmentType at = dynamic_cast<AssignToken*>(stack[1])->value;
		if (at == DIVIDE_EQUAL) {
			auto invop = vector<Value*>();
			invop.push_back( dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end()),line)));
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
			op.push_back(dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end()),line)));

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
		dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.end()),line))
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
				checkDepth(stack[i]->getType(),depth);
				depth = abs(depth);
				if (depth == 0) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.begin() + i),line));
					whileb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end()),line));
					break;
				}
			}
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
				checkDepth(stack[i]->getType(),depth);
				depth = abs(depth);
				if (depth == 0) {
					con = dynamic_cast<Value*>(parseStatement(vector(stack.begin() + 2, stack.begin() + i),line));
					ifb = new CodeBlock(parseStatement(vector(stack.begin() + i + 1, stack.end()),line));
					break;
				}

			}
		}

		if(con != nullptr && ifb != nullptr)
			return new IfStatement(con, ifb);
	}

	///Else
	if (st0 == ELSE) {
		return new ElseStatement(new CodeBlock(parseStatement(vector(stack.begin() + 1, stack.end()),line)));
	}

	///Parenthesis
	if (size >= 3 && st0 == PARENTHESIS_OPEN && stb == PARENTHESIS_CLOSE) {
			int depth = 0;
			for (int i = 1; i < stack.size() - 1; i++) {
				checkDepth(stack[i]->getType(),depth);

				depth = abs(depth);
			}
			if (depth == 0)
				return parseStatement(vector(stack.begin() + 1, stack.end() - 1),line);
			aThrowError(4,line);
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
				checkDepth(t->getType(),depth);
			if (t->getType() ==  OPERATOR) {
				if (i != 0 && depth == 0) {
					MultipleOperatorType tt = dynamic_cast<OperatorToken*>(t)->biValue;
					if (tt == MINUS) tt = PLUS;
					if (tt == DIVIDE) tt = MULTIPLY;
					if (bot == NONE_BI_OPERATOR || tt < bot) bot = tt;
				}
			}
			i++;
		}

		if (bot == PLUS) _bot = MINUS;
		if (bot == MULTIPLY) _bot = DIVIDE;
		i = 0;
		for (Token* t : stack)
		{
			checkDepth(t->getType(), depth);
			if (t->getType() == OPERATOR)
			{
				MultipleOperatorType mop = dynamic_cast<OperatorToken*>(t)->biValue;
				if (i != 0 && depth == 0) {
					if (mop == bot) {
						tokenIdx.push_back(i);
					}
					else if (mop == _bot) {
						tokenIdx.push_back(-i);
					}
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
					operands.push_back(dynamic_cast<Value*>(parseStatement(vector(
						stack.begin() + (tokenIdx[j] + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					),line)));
				}
				else {
					invoperands.push_back(dynamic_cast<Value*>(parseStatement(vector(
						stack.begin() + (abs(tokenIdx[j]) + 1),
						stack.begin() + abs(tokenIdx[j + 1])
					),line)));
				}
			}

			return new MultipleOperation(bot, operands, invoperands);
		}
	}

	///Unary Operation 
	if (size >= 2 && st0 == OPERATOR) {
		UnaryOperatorType uop = dynamic_cast<OperatorToken*>(stack[0])->uValue;
		if (uop == POSITIVE) {
			return parseStatement(vector(stack.begin() + 1, stack.end()),line);
		}
		return new UnaryOperation(uop,dynamic_cast<Value*>( parseStatement(vector(stack.begin() + 1, stack.end()),line)));
	}
	aThrowError(1,line);
	return nullptr;
}

vector<Statement*> Parser::parse() {
	return parse(tks);
}

vector<Statement*> Parser::parse(const vector<Token*>& stack) { // NOLINT(*-no-recursion)
	auto statements = vector<Statement*>();
	auto subStack = vector<Token*>();

	unsigned int line = -1;
	int depth = 0;
	bool shouldParse = false;
	bool elseplz = false;

	for (auto i : stack) {
		const TokenType sti = i->getType();

		checkDepth(sti, depth);

		if (sti == LINE_END && depth == 0)
		{
			line = dynamic_cast<LineToken*>(i)->line;
			shouldParse = true;
		}
		else {
			subStack.push_back(i);
			if (sti == CURLY_CLOSE && depth == 0)shouldParse = true;
		}


		if (shouldParse) {
			shouldParse = false;
			if (!subStack.empty()) {
				if(Statement* ret = parseStatement(subStack,line); elseplz && ret->getType() == ELSE_STMT)
					dynamic_cast<IfStatement*>(statements.back())->elseBlock =dynamic_cast<ElseStatement*>(ret)->elseBlock;
				else
				{
					statements.push_back(ret);
					elseplz = ret->getType() == IF_STMT;
				}
			}
			subStack.clear();
		}
	}

	return statements;
}


