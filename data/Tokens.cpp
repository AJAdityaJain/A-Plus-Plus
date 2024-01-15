#include "Tokens.h"

vector<Token*> tokenize(vector<string> lines)
{
	vector<Token*> tokens;
	string tempString;
	bool cont = false;

	char strStart = ' ';
	bool inStr = false;

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
			}


			if (!inStr) {
				if (tempString.compare("let") == 0) tokens.push_back(new Token{ LET });
				else if (tempString.compare("if") == 0)
					tokens.push_back(new Token{ IF });
				else if (tempString.compare("else") == 0)
					tokens.push_back(new Token{ ELSE });
				else if (tempString.compare("while") == 0)
					tokens.push_back(new Token{ WHILE });
				else if (tempString.compare("continue") == 0)
					tokens.push_back(new Token{ CONTINUE });
				else if (tempString.compare("break") == 0)
					tokens.push_back(new Token{ BREAK });
				else if (tempString.compare("return") == 0)
					tokens.push_back(new Token{ RETURN });

				else if (tempString.compare("(") == 0)
					tokens.push_back(new Token{ PARENTHESIS_OPEN });
				else if (tempString.compare(")") == 0)
					tokens.push_back(new Token{ PARENTHESIS_CLOSE });
				else if (tempString.compare("{") == 0)
					tokens.push_back(new Token{ CURLY_OPEN });
				else if (tempString.compare("}") == 0)
					tokens.push_back(new Token{ CURLY_CLOSE });
				else if (tempString.compare("[") == 0)
					tokens.push_back(new Token{ BRACKET_OPEN });
				else if (tempString.compare("]") == 0)
					tokens.push_back(new Token{ BRACKET_CLOSE });
				else if (tempString.compare(";") == 0)
					tokens.push_back(new Token{ LINEEND });
				else if (tempString.compare(",") == 0)
					tokens.push_back(new Token{ COMMA });

				else if (tempString.compare("=") == 0)
					tokens.push_back(new Token{ ASSIGN });
				else if (tempString.compare("+") == 0)
					tokens.push_back(new Token{ PLUS });
				else if (tempString.compare("-") == 0)
					tokens.push_back(new Token{ MINUS });
				else if (tempString.compare("*") == 0)
					tokens.push_back(new Token{ MULTIPLY });
				else if (tempString.compare("/") == 0)
					tokens.push_back(new Token{ DIVIDE });
				else if (tempString.compare("%") == 0)
					tokens.push_back(new Token{ MODULO });

				else if (tempString.compare("true") == 0)
					tokens.push_back(new BitToken{ true });
				else if (tempString.compare("false") == 0)
					tokens.push_back(new BitToken{ false });

				else if (!isalnum(c)) {
					if (tempString.size() > 1) {
						string sub = tempString.substr(0, tempString.size() - 1);
						int typ = isNumeric(sub);
						if (typ == 0)
							tokens.push_back(new IdentifierToken{ sub });
						else if (typ == 1)
							tokens.push_back(new IntToken{ stoi(sub) });
						else if (typ == 2)
							tokens.push_back(new FloatToken{ stof(sub) });
						else if (typ == 3)
							tokens.push_back(new DoubleToken{ stod(sub) });
						i--;
					}
				}
				else
					cont = true;


				if (!cont)
					tempString = "";

			}
		}
	}

	return tokens;
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

	case BRACKET_OPEN: return "BRACKET_OPEN";
	case BRACKET_CLOSE: return "BRACKET_CLOSE";
	case PARENTHESIS_OPEN: return "PARENTHESIS_OPEN";
	case PARENTHESIS_CLOSE: return "PARENTHESIS_CLOSE";
	case CURLY_OPEN: return "CURLY_OPEN";
	case CURLY_CLOSE: return "CURLY_CLOSE";
	case LINEEND: return "LINEEND";
	case COMMA: return "COMMA";

	case ASSIGN: return "ASSIGN";
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
	}
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

	if (i != str.length()) return 0;
	else if (decimalPointFound) return 2;
	else if (isDouble) return 3;
	else return 1;
}
