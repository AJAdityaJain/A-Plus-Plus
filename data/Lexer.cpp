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
				else if (c == '=')tokens.push_back(new KeyWordToken{ EQUALS });
				else if (c == '+')tokens.push_back(new KeyWordToken{ PLUS });
				else if (c == '-')tokens.push_back(new KeyWordToken{ MINUS });
				else if (c == '*')tokens.push_back(new KeyWordToken{ MULTIPLY });
				else if (c == '/')tokens.push_back(new KeyWordToken{ DIVIDE });
				else if (c == '%')tokens.push_back(new KeyWordToken{ MODULO });
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
					else if (sub.compare("continue") == 0)	tokens.push_back(new KeyWordToken{ CONTINUE });
					else if (sub.compare("break") == 0)		tokens.push_back(new KeyWordToken{ BREAK });
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
							if (t)tokens.push_back(new KeyWordToken{ UNKNOWN });
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

const char* getToken(Tokens value) {
	switch (value) {
	case LET: return "LET";
	case IF: return "IF";
	case ELSE: return "ELSE";

	case WHILE: return "WHILE";
	case CONTINUE: return "CONTINUE";
	case RETURN: return "RETURN";
	case BREAK: return "BREAK";

	case BRACKET_OPEN: return "[";
	case BRACKET_CLOSE: return "]";
	case PARENTHESIS_OPEN: return "(";
	case PARENTHESIS_CLOSE: return ")";
	case CURLY_OPEN: return "}\n";
	case CURLY_CLOSE: return "\n{";
	case LINE_END: return "END\n";
	case COMMA: return "COMMA";

	case EQUALS: return "EQUALS";
	case PLUS: return "PLUS";
	case MINUS: return "MINUS";
	case MULTIPLY: return "MULTIPLY";
	case DIVIDE: return "DIVIDE";
	case MODULO: return "MODULO";

	case INT: return "INT";
	case FLOAT: return "FLOAT";
	case DOUBLE: return "DOUBLE";
	case BIT: return "BIT";
	case STRING: return "STRING";

	case ID: return "ID";
	case UNKNOWN: return "UNKNOWN";
	}

	return "Tokens.cpp->getToken() Bugged";
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
	else if (isDouble) return 3;
	else return 1;
}