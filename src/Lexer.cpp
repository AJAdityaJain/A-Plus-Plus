#include "Lexer.h"

void tokenize(const vector<string>& lines, vector<Token*>& tokens, map<string, unsigned int>& symbols)
{
	string tempString;

	unsigned int idx = 0;
	unsigned int lineIdx = 0;

	char strStart = ' ';

	// 0:normal  1:string  2:literal   3:comment
	int inwhat = 0;

	symbols.insert({ "main", idx++ });
	symbols.insert({ "write", idx++ });
	symbols.insert({ "read", idx++ });
	symbols.insert({ "beep", idx++ });
	symbols.insert({ "len", idx++ });
	symbols.insert({ "size", idx++ });


	for (string line : lines) {
		for (size_t i = 0; i < line.size(); i++)
		{
			const char c = line[i];
			tempString += c;

			switch (inwhat)
			{
			case 0:{
					if (c == '\'' || c == '"' || c == '`') {
						inwhat = 1;
						strStart = c;
						break;
					}
					if(c == '#')
					{
						inwhat = 3;
						break;
					}

					bool cont = false;

					TokenType token = NONE;
					MultipleOperatorType tokenmul = NONE_BI_OPERATOR;
					UnaryOperatorType tokenun = NONE_UN_OPERATOR;

					switch (c)
					{
						case '(': token = PARENTHESIS_OPEN;break;
						case ')': token = PARENTHESIS_CLOSE;break;
						case '@': token = ATR;break;
						case '#': token = HASH;break;
						case '{': token = CURLY_OPEN;break;
						case '}': token = CURLY_CLOSE;break;
						case '[': token = BRACKET_OPEN;break;
						case ']': token = BRACKET_CLOSE;break;
						case ':': token = COLON;break;
						case ',': token = COMMA;break;
						case ';': token = LINE_END; break;
						case '=': token = ASSIGN;break;
						case '!': tokenun = NOT;break;
						case '+': tokenun = POSITIVE; tokenmul = PLUS;break;
						case '-': tokenun = NEGATIVE; tokenmul = MINUS;break;
						case '*': tokenmul = MULTIPLY;break;
						case '/': tokenmul = DIVIDE;break;
						case '%': tokenmul = MODULO;break;
						case '>': tokenmul = GREATER_THAN;break;
						case '<': tokenmul = SMALLER_THAN;break;
						case '&': tokenmul = BITWISE_AND;break;
						case '|': tokenmul = BITWISE_OR;break;
						case '~': tokenun = BITWISE_NOT;break;
						case ' ': case '\r': case '\t': tempString.pop_back();break;
						case '\n': lineIdx++; tempString.pop_back();break;
						default: { inwhat = 2; cont = true; }break;
					}

					if(token == LINE_END)tokens.push_back(new LineToken{ lineIdx  });
					else if(token == ASSIGN)tokens.push_back(new AssignToken{ EQUALS  });
					else if(token != NONE)tokens.push_back(new KeyWordToken{ token});
					else if(tokenmul != NONE_BI_OPERATOR || tokenun != NONE_UN_OPERATOR)tokens.push_back(new OperatorToken{ tokenmul, tokenun  });

					if(i < line.size() -1)
					{
						if (line[i+1] == '=')
						{
							Token* replace = nullptr;
							if(token == ASSIGN)
								replace = new OperatorToken(COMPARISON);
							else if(tokenmul != NONE_BI_OPERATOR)
								switch (tokenmul)
								{
								case PLUS:replace = new AssignToken(PLUS_EQUAL); break;
								case MINUS:replace = new AssignToken(MINUS_EQUAL); break;
								case MULTIPLY:replace = new AssignToken(MULTIPLY_EQUAL); break;
								case DIVIDE:replace = new AssignToken(DIVIDE_EQUAL); break;
								case GREATER_THAN:replace = new OperatorToken(GREATER_THAN_EQUAL); break;
								case SMALLER_THAN:replace = new OperatorToken(SMALLER_THAN_EQUAL); break;
								case BITWISE_AND:replace = new AssignToken(BITWISE_AND_EQUAL); break;
								case BITWISE_OR:replace = new AssignToken(BITWISE_OR_EQUAL); break;
								default:aThrowError(UNKNOWN_OPERATION,lineIdx);
								}
							else if(tokenun != NONE_UN_OPERATOR)
								switch (tokenun)
								{
								case NOT:replace = new OperatorToken(NOT_EQUAL); break;
								default:aThrowError(UNKNOWN_OPERATION,lineIdx);
								}

							if(replace != nullptr)
							{
								tokens.pop_back();
								tokens.push_back(replace);
								i++;
							}
						}
					}

					if (!cont) 	tempString = "";
					break;
				}
			case 1:{
				if(c == '\\')
				{
					if(i < line.size() -1)
					{
						i++;
						if     (line[i] == 'n') tempString.pop_back(),tempString += '\n';
						else if(line[i] == 't') tempString.pop_back(),tempString += '\t';
						else if(line[i] == 'r') tempString.pop_back(),tempString += '\r';
						else if(line[i] == '\\')tempString.pop_back(),tempString += '\\';
						else if(line[i] == '\'')tempString.pop_back(),tempString += '\'';
						else if(line[i] == '"') tempString.pop_back(),tempString += '"';
						else if(line[i] == '`') tempString.pop_back(),tempString += '`';
						else aThrowError(UNKNOWN_ESCAPE_SEQUENCE,lineIdx);
					}
					break;
				}
				if(c == strStart) {
					inwhat = 0;
					strStart = ' ';
					tokens.push_back(new StringToken{
						tempString.substr(1,tempString.length()-2)
						});
					tempString = "";
				}
				break;
			}
			case 2:{
				if (!isalnum(c) && c != '.') {
					inwhat = 0;
					string sub = tempString.substr(0, tempString.size() - 1);
					i--;

					if (sub == "func")			tokens.push_back(new KeyWordToken{ FUNC });
					else if (sub == "if")		tokens.push_back(new KeyWordToken{ IF  });
					else if (sub == "else")		tokens.push_back(new KeyWordToken{ ELSE  });
					else if (sub == "while")	tokens.push_back(new KeyWordToken{ WHILE  });
					else if (sub == "loop")		tokens.push_back(new KeyWordToken{ LOOP  });
					else if (sub == "pool")		tokens.push_back(new KeyWordToken{ POOL  });
					else if (sub == "with")		tokens.push_back(new KeyWordToken{ WITH  });
					else if (sub == "return")	tokens.push_back(new KeyWordToken{ RETURN  });
					else if (sub == "stop")		tokens.push_back(new KeyWordToken{ STOP  });
					else if (sub == "skip")		tokens.push_back(new KeyWordToken{ SKIP  });
					else if (sub == "and")		tokens.push_back(new OperatorToken{ AND  });
					else if (sub == "or")		tokens.push_back(new OperatorToken{ OR  });
					else if (sub == "xor")		tokens.push_back(new OperatorToken{ XOR });
					else if (sub == "true")		tokens.push_back(new BooleanToken{ true  });
					else if (sub == "false")	tokens.push_back(new BooleanToken{ false  });
					else if (sub == "int")		tokens.push_back(new SizeToken{ INT_SIZE  });
					else if (sub == "float")	tokens.push_back(new SizeToken{ FLOAT_SIZE });
					else if (sub == "double")	tokens.push_back(new SizeToken{ DOUBLE_SIZE });
					else if (sub == "short")	tokens.push_back(new SizeToken{ SHORT_SIZE });
					else if (sub == "long")		tokens.push_back(new SizeToken{ LONG_SIZE });
					else if (sub == "string")	tokens.push_back(new SizeToken{ STRPTR_SIZE });
					else {
						switch (isNumeric(sub))
						{
						case ID:
							{
								for (const char subc : sub)
									if (!isalnum(subc))
										aThrowError(ILLEGAL_CHARACTER,lineIdx);

								if (symbols.contains(sub))
									tokens.push_back(new IdentifierToken{ symbols[sub]  });
								else {
									symbols.insert({ sub, idx });
									tokens.push_back(new IdentifierToken{ idx  });
									idx++;
								}
								break;
							}
						case INT:tokens.push_back(new IntToken{ stoi(sub)  });						break;
						case SHORT:tokens.push_back(new ShortToken{ static_cast<short>(stoi(sub))  });	break;
						case LONG:tokens.push_back(new LongToken{ stol(sub)  });					break;
						case FLOAT:tokens.push_back(new FloatToken{ stof(sub)  });					break;
						case DOUBLE:tokens.push_back(new DoubleToken{ stod(sub)  });				break;
						default:aThrowError(ILLEGAL_CHARACTER,lineIdx);											break;
						}
					}
					tempString = "";
				}

				break;
			}
			case 3:
				if(c == '\n')
					inwhat = 0;
				break;
			// ReSharper disable once CppDFAUnreachableCode
			default:break;
			}
		}
	}
}


TokenType isNumeric(const std::string& str) {
	size_t i = 0;
	if (str[i] == '+' || str[i] == '-') {
		++i;
	}

	bool decimalPointFound = false;
	TokenType type = FLOAT;

	while (i < str.length() && (str[i] >= '0' && str[i] <= '9' || (str[i] == '.' && !decimalPointFound))) {
		if (str[i] == '.') {
			decimalPointFound = true;
		}
		++i;
	}
		if (str[i] == 'd' || str[i] == 'D') type = DOUBLE;
		if (str[i] == 's' || str[i] == 'S') type = SHORT;
		if (str[i] == 'l' || str[i] == 'L') type = LONG;
	if(type != FLOAT)
		++i;

	if (i != str.length())return ID;
	if (type == FLOAT)
	{
		if(decimalPointFound)return FLOAT;
		return INT;
	}
	if(str.length() >1){
		return type;
	}
	return ID;
}