#include "Lexer.h"

void tokenize(vector<string> lines, vector<Token*>& tokens )
{
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
				else if (c == ',')tokens.push_back(new KeyWordToken{ COMMA });

				else if (c == '=')tokens.push_back(new AssignToken{ EQUALS });

				else if (c == '+')tokens.push_back(new OperatorToken{ PLUS });
				else if (c == '-')tokens.push_back(new OperatorToken{ MINUS });
				else if (c == '*')tokens.push_back(new OperatorToken{ MULTIPLY });
				else if (c == '/')tokens.push_back(new OperatorToken{ DIVIDE });
				else if (c == '%')tokens.push_back(new OperatorToken{ MODULO });


				else if (c == ' ' || c == '\n' || c == '\r' || c== '\t')tempString.pop_back();
				else {inLiteral = true;cont = true;}

				if (!cont) 	tempString = "";

			}
			else if (!inStr && inLiteral) {
				if (!isalnum(c) && c != '.') {
					inLiteral = false;
					string sub = tempString.substr(0, tempString.size() - 1);
					i--;

						 if(sub.compare("let") == 0)		tokens.push_back(new KeyWordToken{ LET });
					else if (sub.compare("if") == 0)		tokens.push_back(new KeyWordToken{ IF });
					else if (sub.compare("else") == 0)		tokens.push_back(new KeyWordToken{ ELSE });
					else if (sub.compare("while") == 0)		tokens.push_back(new KeyWordToken{ WHILE });
					else if (sub.compare("do") == 0)	tokens.push_back(new KeyWordToken{ DO });
					else if (sub.compare("and") == 0)		tokens.push_back(new KeyWordToken{ AND });
					else if (sub.compare("or") == 0)		tokens.push_back(new KeyWordToken{ OR});
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
							else tokens.push_back(new IdentifierToken{ sub });
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

void printToken(TokenType t) {
	switch (t)
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
		cout << ";";
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
	case ELSE:
		cout << "else";
		break;
	case WHILE:
		cout << "while";
		break;
	case DO:
		cout << "do";
		break;
	case AND:
		cout << "&";
		break;
	case OR:
		cout << "or";
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
	case ASSIGN:
		cout << " ?= ";
		break;
	case COMMA:
		cout << " , ";
		break;
	case OPERATOR:
		cout << " op ";
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