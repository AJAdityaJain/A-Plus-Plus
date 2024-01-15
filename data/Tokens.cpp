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
				tokens.push_back(new TokenValue{ VAL_STRING, tempString });
				tempString = "";
			}


			if (!inStr) {

				if (tempString.compare("let") == 0)
					tokens.push_back(new Token{ LET});
				else if (tempString.compare("if") == 0)
					tokens.push_back(new Token{ IF});
				else if (tempString.compare("else") == 0)
					tokens.push_back(new Token{ ELSE});
				else if (tempString.compare("while") == 0)
					tokens.push_back(new Token{ WHILE});
				else if (tempString.compare("continue") == 0)
					tokens.push_back(new Token{ CONTINUE});
				else if (tempString.compare("break") == 0)
					tokens.push_back(new Token{ BREAK});
				else if (tempString.compare("return") == 0)
					tokens.push_back(new Token{ RETURN});

				else if (tempString.compare("(") == 0)
					tokens.push_back(new Token{ PARENTHESIS_OPEN});
				else if (tempString.compare(")") == 0)
					tokens.push_back(new Token{ PARENTHESIS_CLOSE});
				else if (tempString.compare("{") == 0)
					tokens.push_back(new Token{ CURLY_OPEN});
				else if (tempString.compare("}") == 0)
					tokens.push_back(new Token{ CURLY_CLOSE});
				else if (tempString.compare("[") == 0)
					tokens.push_back(new Token{ BRACKET_OPEN});
				else if (tempString.compare("]") == 0)
					tokens.push_back(new Token{ BRACKET_CLOSE});
				else if (tempString.compare(";") == 0)
					tokens.push_back(new Token{ _SEPARATOR, tempString });
				else if (tempString.compare(",") == 0)
					tokens.push_back(new Token{ _SEPARATOR, tempString });

				else if (tempString.compare("=") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });
				else if (tempString.compare("+") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });
				else if (tempString.compare("-") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });
				else if (tempString.compare("*") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });
				else if (tempString.compare("/") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });
				else if (tempString.compare("%") == 0)
					tokens.push_back(new Token{ _OPERATOR, tempString });

				else if (tempString.compare("true") == 0)
					tokens.push_back(new Token{ _VALUE, tempString });
				else if (tempString.compare("false") == 0)
				//	tokens.push_back(new Token{ _VALUE, tempString });
				//else if (tempString.compare("int") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });
				//else if (tempString.compare("float") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });
				//else if (tempString.compare("double") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });
				//else if (tempString.compare("bit") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });
				//else if (tempString.compare("char") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });
				//else if (tempString.compare("string") == 0)
				//	tokens.push_back(new Token{ _KEYWORD, tempString });


				else if (!isalnum(c)) {
					if (tempString.size() > 1) {
						string sub = tempString.substr(0, tempString.size() - 1);
						if (isNumeric(sub))
							tokens.push_back(new Token{ _VALUE, sub });
						else
							tokens.push_back(new Token{ _IDENTIFIER, sub });

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

//const char* getToken(new TokenType value) {
//	switch (value) {
//	case _IDENTIFIER: return "IDENTIFIER";
//	case _OPERATOR: return "OPERATOR";
//		case _VALUE: return "VALUE";
//	case _KEYWORD: return "KEYWORD";
//
//		case _SEPARATOR: return "SEPARATOR";
//	}
//}


bool isNumeric(const std::string& str) {
	if (str.empty()) {
		return false;  // Empty string is not numeric
	}

	size_t i = 0;

	// Check for an optional sign
	if (str[i] == '+' || str[i] == '-') {
		++i;
	}

	bool decimalPointFound = false;

	// Check for digits and at most one decimal point
	while (i < str.length() && (str[i] >= '0' && str[i] <= '9' || (str[i] == '.' && !decimalPointFound))) {
		if (str[i] == '.') {
			decimalPointFound = true;
		}
		++i;
	}

	return i == str.length();  // Check if we reached the end of the string
}
