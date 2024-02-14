//
// Created by Aditya Jain on 13/2/24.
//

#pragma once
#include "Parser.h"

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
    switch (token->getType())
    {
    case BIT:cout << "{ BIT : " << dynamic_cast<BitToken*>(token)->value << " }" << endl;           break;
    case INT:cout << "{ INT : " << dynamic_cast<IntToken*>(token)->value << " }" << endl;           break;
    case FLOAT:cout << "{ FLOAT : " << dynamic_cast<FloatToken*>(token)->value << " }" << endl;     break;
    case DOUBLE:cout << "{ DOUBLE : " << dynamic_cast<DoubleToken*>(token)->value << " }" << endl;  break;
    case STRING:cout << "{ STRING : `" << dynamic_cast<StringToken*>(token)->value << "` }" << endl;  break;
    case ID:cout << "{ ID : v" << dynamic_cast<IdentifierToken*>(token)->value << " }" << endl;     break;
    case FUNC:cout << "{ FUNC }" << endl;                                                           break;
    case IF:cout << "{ IF }" << endl;                                                               break;
    case ELSE:cout << "{ ELSE }" << endl;                                                           break;
    case WHILE:cout << "{ WHILE }" << endl;                                                         break;
    case RETURN:cout << "{ RETURN }" << endl;                                                       break;
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

inline void printStatement(Statement* stmt, const int depth = 0) // NOLINT(*-no-recursion)
{
    switch (stmt->getType())
    {
        case BIT_STMT:cout << dynamic_cast<Bit*>(stmt)->value;break;
    case INT_STMT:cout << dynamic_cast<Int*>(stmt)->value;break;
    case FLOAT_STMT:cout << dynamic_cast<Float*>(stmt)->value;break;
    case DOUBLE_STMT:cout << dynamic_cast<Double*>(stmt)->value;break;
    case STRING_STMT:cout << "`" << dynamic_cast<String*>(stmt)->value << "`";break;
    case ARRAY:
        {
            const auto a = dynamic_cast<Array*>(stmt);
            cout << "[ ";
            for (const auto el : a->values)
            {
                printStatement(el);
                cout << " ";
            }
            cout << "]";
            break;
        }
    case REFERENCE:cout << "v" << dynamic_cast<Reference*>(stmt)->value;break;
    case FUNC_CALL:
        {
            const auto fc = dynamic_cast<FuncCall*>(stmt);
            cout << "{ v" << fc->name.value << " (";
            printStatement(fc->params[0]);
            for (int i = 1; i < fc->params.size(); i++)
            {
                cout << ", ";
                printStatement(fc->params[i]);
            }
            cout << ") }";
            break;
        }
    case FUNC_DEFINITION:
        {
            const auto fd = dynamic_cast<Func*>(stmt);
            cout << "{ "<< endl << "DEFINE v" << fd->name.value << "(";
            // printStatement(fd->params[0]);
            // for (int i = 1; i < fd->params.size(); i++)
            // {
            // cout << ", ";
            // printStatement(fd->params[i]);
            // }
            cout << ")"<< endl;
            printStatement(fd->body,depth+1);
            cout << "}";
            break;
        }
    case ASSIGNMENT:
        {
            const auto a = dynamic_cast<Assignment*>(stmt);
            cout << "{ v"<<a->name.value << " " << getAssign(a->type)<< " ";
            printStatement(a->value);
            cout << " }";
            break;

        }
    case WHILE_STMT:
        {
            const auto w = dynamic_cast<WhileStatement*>(stmt);
            cout << "WHILE ";
            printStatement(w->condition);
            cout << endl;
            printStatement(w->whileBlock,depth);
            break;

        }
    case IF_STMT:
        {
            const auto ife = dynamic_cast<IfStatement*>(stmt);
            cout << "IF ";
            printStatement(ife->condition);
            cout << endl;
            printStatement(ife->ifBlock,depth+1);
            if(ife->elseBlock != nullptr)
            {
                cout << "ELSE" << endl;
                printStatement(ife->elseBlock,depth);
            }
            break;
        }
    case ELSE_STMT:cout << "{ ELSE ERROR }";break;
    case MULTI_OPERATION:
        {
            const auto mo = dynamic_cast<MultipleOperation*>(stmt);
            cout << "{ ";
            printStatement(mo->operands[0]);
            for(int j = 1; j < mo->operands.size(); j++)
            {
                cout << " " << getOperator(mo->op) << " ";
                printStatement(mo->operands[j]);
            }
            for (const auto invers : mo->invoperands)
            {
                cout << " opposite " << getOperator(mo->op) << " ";
                printStatement(invers);
            }
            cout << " }";
            break;

        }
    case UN_OPERATION:
        {
            const auto u = dynamic_cast<UnaryOperation*>(stmt);
            cout << "{ " << getOperator(u->op) << " ";
            printStatement(u->right);
            cout << " }";
            break;
        }
    case SCOPE:
        {
            for (const auto s = dynamic_cast<CodeBlock*>(stmt); const auto &l : s->code)
            {
                for (int j = 0; j < depth; j++)
                    cout << "\t";
                printStatement(l,depth+1);
                cout << endl;
            }
            break;
        }
    default: cout << "{ ? }";break;
    }
}

inline void printTokens(const vector<Token*>& tokens)
{
    for (const auto t: tokens)
        printToken(t);
}

inline void printTree(const vector<Statement*>& tree)
{
    for (const auto t: tree)
    {
        printStatement(t);
        cout << endl;
    }
}