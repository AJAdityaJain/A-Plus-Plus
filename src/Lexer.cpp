#include "Lexer.h"

void Lexer::tokenize(const vector<string>& lines)
{
	map<string, unsigned int> idMap;
	string tempString;

	unsigned int idx = 0;
	unsigned int lineIdx = 0;

	char strStart = ' ';

	// 0:normal  1:string  2:literal   3:comment
	int inwhat = 0;

	idMap.insert({ "main", idx++ });
	idMap.insert({ "write", idx++ });
	idMap.insert({ "read", idx++ });

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
								default:aThrowError(7,lineIdx);
								}
							else if(tokenun != NONE_UN_OPERATOR)
								switch (tokenun)
								{
								case NOT:replace = new OperatorToken(NOT_EQUAL); break;
								default:aThrowError(7,lineIdx);
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
						else aThrowError(2,lineIdx);
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
					else if (sub == "return")	tokens.push_back(new KeyWordToken{ RETURN  });
					else if (sub == "and")		tokens.push_back(new OperatorToken{ AND  });
					else if (sub == "or")		tokens.push_back(new OperatorToken{ OR  });
					else if (sub == "xor")		tokens.push_back(new OperatorToken{ XOR });
					else if (sub == "true")		tokens.push_back(new BitToken{ true  });
					else if (sub == "false")	tokens.push_back(new BitToken{ false  });
					else {
						switch (isNumeric(sub))
						{
						case 0:
							{
								for (const char subc : sub)
									if (!isalnum(subc))
										aThrowError(3,lineIdx);

								if (idMap.contains(sub))
									tokens.push_back(new IdentifierToken{ idMap[sub]  });
								else {
									idMap.insert({ sub, idx });
									tokens.push_back(new IdentifierToken{ idx  });
									idx++;
								}
								break;
							}
						case 1:tokens.push_back(new IntToken{ stoi(sub)  });	break;
						case 2:tokens.push_back(new FloatToken{ stof(sub)  });	break;
						case 3:tokens.push_back(new DoubleToken{ stod(sub)  });break;
						default:aThrowError(3,lineIdx);						break;
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


int Lexer::isNumeric(const std::string& str) {
	if (str.empty()) {
		return false;  // Empty string is not numeric
	}

	size_t i = 0;

	// Check for an optional sign
	if (str[i] == '+' || str[i] == '-') {
		++i;
	}

	bool decimalPointFound = false;
	bool isDouble = false;

	// Check for digits and at most one decimal point
	while (i < str.length() && (str[i] >= '0' && str[i] <= '9' || (str[i] == '.' && !decimalPointFound))) {
		if (str[i] == '.') {
			decimalPointFound = true;
		}
		++i;
	}
	if (str[i] == 'd' || str[i] == 'D') {
		isDouble = true;
		++i;
	}

	if (i != str.length())return 0;
	if (decimalPointFound && !isDouble) return 2;
	if (isDouble)
	{
		if(str.length() >1)	return 3;
		return 0;
	}
	return 1;
}