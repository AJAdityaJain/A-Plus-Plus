#include "Lexer.h"

void Lexer::tokenize(vector<string> lines)
{
	map<string, unsigned int> idMap;
	string tempString;

	unsigned int idx = 0;
	unsigned int lineIdx = 0;

	bool cont = false;

	char strStart = ' ';
	bool inStr = false;

	bool inLiteral = false;

	idMap.insert({ "main", idx++ });
	idMap.insert({ "write", idx++ });

	for (string line : lines) {
		for (int i = 0; i < line.size(); i++) {
			char c = line[i];
			cont = false;
			tempString += c;

			if (!inStr && (c == '\'' || c == '"' || c == '`')) {
				inStr = true;
				strStart = c;
			}


			else if (inStr && c == strStart) {
				inStr = false;
				strStart = ' ';
				tokens.push_back(new StringToken{ 
					tempString.substr(1,tempString.length()-2),lineIdx
					});
				tempString = "";
				continue;
			}


			if (!inStr && !inLiteral) {
				if (c == '(')tokens.push_back(new KeyWordToken{ PARENTHESIS_OPEN ,lineIdx });
				else if (c == ')')tokens.push_back(new KeyWordToken{ PARENTHESIS_CLOSE ,lineIdx });
				else if (c == '{')tokens.push_back(new KeyWordToken{ CURLY_OPEN ,lineIdx });
				else if (c == '}')tokens.push_back(new KeyWordToken{ CURLY_CLOSE ,lineIdx });
				else if (c == '[')tokens.push_back(new KeyWordToken{ BRACKET_OPEN ,lineIdx });
				else if (c == ']')tokens.push_back(new KeyWordToken{ BRACKET_CLOSE ,lineIdx });
				else if (c == ';')tokens.push_back(new KeyWordToken{ LINE_END ,lineIdx });
				else if (c == ':')tokens.push_back(new KeyWordToken{ COLON,lineIdx });
				else if (c == ',')tokens.push_back(new KeyWordToken{ COMMA ,lineIdx });

				else if (c == '=')tokens.push_back(new AssignToken{ EQUALS ,lineIdx });
				else if (c == '!')tokens.push_back(new OperatorToken{ NOT ,lineIdx });
				else if (c == '+')tokens.push_back(new OperatorToken{ POSITIVE, PLUS ,lineIdx });
				else if (c == '-')tokens.push_back(new OperatorToken{ NEGATIVE, MINUS ,lineIdx });
				else if (c == '*')tokens.push_back(new OperatorToken{ MULTIPLY ,lineIdx });
				else if (c == '/')tokens.push_back(new OperatorToken{ DIVIDE ,lineIdx });
				else if (c == '%')tokens.push_back(new OperatorToken{ MODULO ,lineIdx });
				else if (c == '>')tokens.push_back(new OperatorToken{ GREATER_THAN ,lineIdx });
				else if (c == '<')tokens.push_back(new OperatorToken{ SMALLER_THAN ,lineIdx });

				else if (c == '&')tokens.push_back(new OperatorToken{ BITWISE_AND ,lineIdx });
				else if (c == '|')tokens.push_back(new OperatorToken{ BITWISE_OR ,lineIdx });
				else if (c == '~')tokens.push_back(new OperatorToken{ BITWISE_NOT ,lineIdx });


				else if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
					if (c == '\n') {
						lineIdx++;

					}
					tempString.pop_back();
				}
				else { inLiteral = true; cont = true; }

				if (!inLiteral && tokens.size() > 1 && i < line.size()-1) {
					Token* b = tokens.back();
					if (line[i+1] == '=') {
						if (b->getType() == ASSIGN) {
							tokens.pop_back();
							tokens.push_back(new OperatorToken(COMPARISON, lineIdx));
							i++;
						}
						if (b->getType() == OPERATOR) {
							Token* replace;
							switch (((OperatorToken*)b)->uValue) {
							case NOT:
								replace = new OperatorToken(NOT_EQUAL, lineIdx);
								break;
							}
							switch (((OperatorToken*)b)->biValue) {
							case PLUS:
								replace = new AssignToken(PLUS_EQUAL, lineIdx);
								break;
							case MINUS:
								replace = new AssignToken(MINUS_EQUAL, lineIdx);
								break;
							case MULTIPLY:
								replace = new AssignToken(MULTIPLY_EQUAL, lineIdx);
								break;
							case DIVIDE:
								replace = new AssignToken(DIVIDE_EQUAL, lineIdx);
								break;
							case GREATER_THAN:
								replace = new OperatorToken(GREATER_THAN_EQUAL, lineIdx);
								break;
							case SMALLER_THAN:
								replace = new OperatorToken(SMALLER_THAN_EQUAL, lineIdx);
								break;
							case BITWISE_AND:
								replace = new AssignToken(BITWISE_AND_EQUAL, lineIdx);
								break;
							case BITWISE_OR:
								replace = new AssignToken(BITWISE_OR_EQUAL, lineIdx);
								break;
							}
							tokens.pop_back();
							tokens.push_back(replace);
							i++;
						}
					}
				}
				if (!cont) 	tempString = "";
			}
			else if (!inStr && inLiteral) {
				if (!isalnum(c) && c != '.') {
					inLiteral = false;
					string sub = tempString.substr(0, tempString.size() - 1);
					i--;

						 if (sub.compare("func") == 0)		tokens.push_back(new KeyWordToken{ FUNC,lineIdx });
						 else if (sub.compare("if") == 0)		tokens.push_back(new KeyWordToken{ IF ,lineIdx });
						 else if (sub.compare("else") == 0)		tokens.push_back(new KeyWordToken{ ELSE ,lineIdx });
						 else if (sub.compare("while") == 0)		tokens.push_back(new KeyWordToken{ WHILE ,lineIdx });
						 else if (sub.compare("return") == 0)	tokens.push_back(new KeyWordToken{ RETURN ,lineIdx });
						 else if (sub.compare("and") == 0)		tokens.push_back(new OperatorToken{ AND ,lineIdx });
						 else if (sub.compare("or") == 0)		tokens.push_back(new OperatorToken{ OR ,lineIdx });
						 else if (sub.compare("xor") == 0)		tokens.push_back(new OperatorToken{ XOR,lineIdx });
						 else if (sub.compare("true") == 0)		tokens.push_back(new BitToken{ true ,lineIdx });
						 else if (sub.compare("false") == 0)		tokens.push_back(new BitToken{ false ,lineIdx });

					else {
						int typ = isNumeric(sub);
						if (typ == 0) {
							bool t = !isalpha(sub[0]);
							if (!t)
								for (char c : sub)
									if (!isalnum(c)) {
										t = true;
										break;
									}
							if (t)aThrowError(3, lineIdx);
							else { 
								if (idMap.count(sub) > 0) {
									tokens.push_back(new IdentifierToken{ idMap[sub] ,lineIdx });
								}
								else {
									idMap.insert({ sub, idx });
									tokens.push_back(new IdentifierToken{ idx ,lineIdx });
									idx++;
								}
							}
						}
						else if (typ == 1)tokens.push_back(new IntToken{ stoi(sub) ,lineIdx });
						else if (typ == 2)tokens.push_back(new FloatToken{ stof(sub) ,lineIdx });
						else if (typ == 3)tokens.push_back(new DoubleToken{ stod(sub) ,lineIdx });

					}
					tempString = "";
				}
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
	bool isUnknown = false;

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
	else if (decimalPointFound && !isDouble) return 2;
	else if (isDouble)
		if(str.length() >1)	return 3;
		else return 0;
	else return 1;
}