//
// Created by Aditya Jain on 13/2/24.
//

#pragma once
#include "FATMaker.h"

inline const char* getAssign(const AssignmentType type)
{
    switch (type)
    {
    case EQUALS:return "=";
    case PLUS_EQUAL:return "+=";
    case MINUS_EQUAL:return "-=";
    case MULTIPLY_EQUAL:return "*=";
    case DIVIDE_EQUAL:return "/=";
    case MODULO_EQUAL:return "%=";
    case BITWISE_OR_EQUAL:return "|=";
    case BITWISE_AND_EQUAL:return "&=";
    default:return "?";
    }
}

inline const char* getOperator(const UnaryOperatorType type)
{
    switch (type)
    {
    case NOT:return "!";
    case BITWISE_NOT:return "~";
    case POSITIVE: return "+";
    case NEGATIVE: return "-";
    default:return "?";
    }
}

inline const char* getOperator(const MultipleOperatorType type)
{
    switch (type)
    {
    case OR: return "or";
    case AND: return "and";
    case BITWISE_OR: return "|";
    case XOR: return "xor";
    case BITWISE_AND: return "&";
    case COMPARISON: return "==";
    case NOT_EQUAL: return "!=";
    case GREATER_THAN: return ">";
    case SMALLER_THAN: return "<";
    case GREATER_THAN_EQUAL: return ">=";
    case SMALLER_THAN_EQUAL: return "<=";
    case MINUS: return "-";
    case PLUS: return "+";
    case DIVIDE: return "/";
    case MULTIPLY: return "*";
    case MODULO: return "%";
    default: return " ? ";
    }
}

inline void printToken(Token* token)
{
    if(token == nullptr)
    {
        cout << "null";return;
    }

    switch (token->getType())
    {
    case BOOL:cout << "{ BOOL : " << dynamic_cast<BooleanToken*>(token)->value << "b }" << endl;          break;
    case INT:cout << "{ INT : " << dynamic_cast<IntToken*>(token)->value << " }" << endl;           break;
    case SHORT:cout << "{ SHORT : " << dynamic_cast<ShortToken*>(token)->value << ":shrt }" << endl;break;
    case LONG:cout << "{ LONG : " << dynamic_cast<LongToken*>(token)->value << ":long }" << endl;   break;
    case FLOAT:cout << "{ FLOAT : " << dynamic_cast<FloatToken*>(token)->value << ":f }" << endl;   break;
    case DOUBLE:cout << "{ DOUBLE : " << dynamic_cast<DoubleToken*>(token)->value << ":d }" << endl;break;
    case STRING:cout << "{ STRING : `" << dynamic_cast<StringToken*>(token)->value << "` }" << endl;break;
    case ID:cout << "{ ID : v" << dynamic_cast<IdentifierToken*>(token)->value << " }" << endl;     break;
    case FUNC:cout << "{ FUNC }" << endl;                                                           break;
    case IF:cout << "{ IF }" << endl;                                                               break;
    case ELSE:cout << "{ ELSE }" << endl;                                                           break;
    case WHILE:cout << "{ WHILE }" << endl;                                                         break;
    case RETURN:cout << "{ RETURN }" << endl;                                                       break;
    case STOP:cout << "{ STOP }" << endl;                                                       break;
    case SKIP:cout << "{ SKIP }" << endl;                                                       break;
    case BRACKET_OPEN:cout << "{ [ }" << endl;                                                      break;
    case BRACKET_CLOSE:cout << "{ ] }" << endl;                                                     break;
    case PARENTHESIS_OPEN:cout << "{ ( }" << endl;                                                  break;
    case PARENTHESIS_CLOSE:cout << "{ ) }" << endl;                                                 break;
    case CURLY_OPEN:cout << "{ { }" << endl;                                                        break;
    case CURLY_CLOSE:cout << "{ } }" << endl;                                                       break;
    case LINE_END:cout << "{ ; }" << endl;                                                          break;
    case COMMA:cout << "{ , }" << endl;                                                             break;
    case COLON:cout << "{ : }" << endl;                                                             break;
    case ASSIGN:cout << "{ " << getAssign(dynamic_cast<AssignToken*>(token)->value) << " }" << endl;break;
    case OPERATOR:cout << "{ " << getOperator(dynamic_cast<OperatorToken*>(token)->biValue) << " " << getOperator(dynamic_cast<OperatorToken*>(token)->uValue) << " }" << endl;break;
    default:cout << "{ NULL }" << endl;                                                              break;
    }

}

inline void printTokens(const vector<Token*>& tokens)
{
    for (const auto t: tokens)
        printToken(t);
}