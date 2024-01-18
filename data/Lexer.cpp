#include "Lexer.h"

void tokenize(vector<string> lines, vector<Token*>& tokens )
{
	map<string, unsigned int> idMap;
	unsigned int idx = 0;

	tokens.push_back(new KeyWordToken{ CURLY_OPEN});
	string tempString;
	bool cont = false;

	char strStart = ' ';
	bool inStr = false;
	bool inLiteral = false;

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
				tokens.push_back(new StringToken{ tempString });
				tempString = "";
				continue;
			}


			if (!inStr && !inLiteral) {
				if (c == '(')tokens.push_back(new KeyWordToken{ PARENTHESIS_OPEN });
				else if (c == ')')tokens.push_back(new KeyWordToken{ PARENTHESIS_CLOSE });
				else if (c == '{')tokens.push_back(new KeyWordToken{ CURLY_OPEN });
				else if (c == '}')tokens.push_back(new KeyWordToken{ CURLY_CLOSE });
				else if (c == '[')tokens.push_back(new KeyWordToken{ BRACKET_OPEN });
				else if (c == ']')tokens.push_back(new KeyWordToken{ BRACKET_CLOSE });
				else if (c == ';')tokens.push_back(new KeyWordToken{ LINE_END });
				else if (c == ':')tokens.push_back(new KeyWordToken{ COLON});
				else if (c == ',')tokens.push_back(new KeyWordToken{ COMMA });

				else if (c == '=')tokens.push_back(new AssignToken{ EQUALS });
				else if (c == '!')tokens.push_back(new OperatorToken{ NOT });
				else if (c == '+')tokens.push_back(new OperatorToken{ POSITIVE, PLUS });
				else if (c == '-')tokens.push_back(new OperatorToken{ NEGATIVE, MINUS });
				else if (c == '*')tokens.push_back(new OperatorToken{ MULTIPLY });
				else if (c == '/')tokens.push_back(new OperatorToken{ DIVIDE });
				else if (c == '%')tokens.push_back(new OperatorToken{ MODULO });
				else if (c == '>')tokens.push_back(new OperatorToken{ GREATER_THAN });
				else if (c == '<')tokens.push_back(new OperatorToken{ SMALLER_THAN });

				else if (c == '&')tokens.push_back(new OperatorToken{ BITWISE_AND });
				else if (c == '|')tokens.push_back(new OperatorToken{ BITWISE_OR });
				else if (c == '~')tokens.push_back(new OperatorToken{ BITWISE_NOT });


				else if (c == ' ' || c == '\n' || c == '\r' || c == '\t')tempString.pop_back();
				else { inLiteral = true; cont = true; }

				if (!inLiteral && tokens.size() > 1 && i < line.size()-1) {
					Token* b = tokens.back();
					if (line[i+1] == '=') {
						if (b->getType() == ASSIGN) {
							tokens.pop_back();
							tokens.push_back(new OperatorToken(COMPARISON));
							i++;
						}
						if (b->getType() == OPERATOR) {
							Token* replace = null;
							switch (((OperatorToken*)b)->uValue) {
							case NOT:
								replace = new OperatorToken(NOT_EQUAL);
								break;
							}
							switch (((OperatorToken*)b)->biValue) {
							case PLUS:
								replace = new AssignToken(PLUS_EQUAL);
								break;
							case MINUS:
								replace = new AssignToken(MINUS_EQUAL);
								break;
							case MULTIPLY:
								replace = new AssignToken(MULTIPLY_EQUAL);
								break;
							case DIVIDE:
								replace = new AssignToken(DIVIDE_EQUAL);
								break;
							case GREATER_THAN:
								replace = new OperatorToken(GREATER_THAN_EQUAL);
								break;
							case SMALLER_THAN:
								replace = new OperatorToken(SMALLER_THAN_EQUAL);
								break;
							case BITWISE_AND:
								replace = new AssignToken(BITWISE_AND_EQUAL);
								break;
							case BITWISE_OR:
								replace = new AssignToken(BITWISE_OR_EQUAL);
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

						 if(sub.compare("let") == 0)		tokens.push_back(new KeyWordToken{ LET });
						 else if (sub.compare("if") == 0)		tokens.push_back(new KeyWordToken{ IF });
						 else if (sub.compare("else") == 0) tokens.push_back(new KeyWordToken{ ELSE });
					else if (sub.compare("while") == 0)		tokens.push_back(new KeyWordToken{ WHILE });
					else if (sub.compare("and") == 0)		tokens.push_back(new OperatorToken{ AND });
					else if (sub.compare("or") == 0)		tokens.push_back(new OperatorToken{ OR });
					else if (sub.compare("xor") == 0)		tokens.push_back(new OperatorToken{ XOR});
					else if (sub.compare("return") == 0)	tokens.push_back(new KeyWordToken{ RETURN });
					else if (sub.compare("true") == 0)		tokens.push_back(new BitToken{ true });
					else if (sub.compare("false") == 0)		tokens.push_back(new BitToken{ false });

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
							if (t)tokens.push_back(new KeyWordToken{ NONE });
							else { 
								if (idMap.count(sub) > 0) {
									tokens.push_back(new IdentifierToken{ idMap[sub] });
								}
								else {
									idMap.insert({ sub, idx });
									tokens.push_back(new IdentifierToken{ idx });
									idx++;
								}
							}
						}
						else if (typ == 1)tokens.push_back(new IntToken{ stoi(sub) });
						else if (typ == 2)tokens.push_back(new FloatToken{ stof(sub) });
						else if (typ == 3)tokens.push_back(new DoubleToken{ stod(sub) });

					}

					tempString = "";
				}
			}

		}

	}

	tokens.push_back(new KeyWordToken{ CURLY_CLOSE});

}

void printToken(Token* t) {
	switch (t->getType())
	{
	case BIT:
		cout << "BIT";
		break;
	case INT:
		cout << "INT";
		break;
	case FLOAT:
		cout << "FLOAT";
		break;
	case DOUBLE:
		cout << "DOUBLE";
		break;
	case STRING:
		cout << "STR";
		break;
	case ID:
		cout << "ID";
		break;
	case LINE_END:
		cout << "end";
		break;
	case NONE:
		cout << "???";
		break;
	case LET:
		cout << "LET";
		break;
	case IF:
		cout << "if";
		break;
	case COLON:
		cout << " : ";
		break;
	case ELSE:
		cout << "else";
		break;
	case WHILE:
		cout << "while";
		break;
	case RETURN:
		cout << "ret";
		break;
	case BRACKET_OPEN:
		cout << "[";
		break;
	case BRACKET_CLOSE:
		cout << "]";
		break;
	case PARENTHESIS_OPEN:
		cout << "(";
		break;
	case PARENTHESIS_CLOSE:
		cout << ")";
		break;
	case CURLY_OPEN:
		cout << "{";
		break;
	case CURLY_CLOSE:
		cout << "}";
		break;
	case ASSIGN: {
		AssignToken at = *(AssignToken*)t;
		cout << at.value;
	}
		break;
	case COMMA:
		cout << " , ";
		break;
	case OPERATOR: {
		cout << " op ";

	}
		break;
	default:
		cout << "BUGGED LEXER";
		break;
	}

	cout << " ";
}
int isNumeric(const std::string& str) {
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