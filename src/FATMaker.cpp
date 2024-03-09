//
// Created by Aditya Jain on 9/3/24.
//

#include "FATMaker.h"

constexpr unsigned char DELIM = 0xff;
constexpr unsigned char SEP = 0xfe;

union {
    float a;
    unsigned char bytes[4];
} floatbyte;

union {
    double a;
    unsigned char bytes[8];
} doublebyte;

void createFatFile(const char* filename, const vector<Statement*>& abstractTree)
{
    auto bytes = ofstream(filename);
    for (const auto stmt : abstractTree)
    {
        addBytes(stmt, bytes);
    }
    bytes.close();
}


void addBytes(Statement* statement, ofstream& bytes) // NOLINT(*-no-recursion)
{
    const char type = statement->getType();
    bytes << type;
    switch (type)
    {
    case NONE_STMT:break;
    case BOOL_STMT:
        {
            if (dynamic_cast<Boolean*>(statement)->value)
                bytes << 1;
            else
                bytes << 0;
            break;
        }
    case SHORT_STMT:
        {
            const auto _short = dynamic_cast<Short*>(statement)->value;
            bytes <<  static_cast<char>((_short >> (8)) & 0xff);
            bytes <<  static_cast<char>((_short >> (0)) & 0xff);
            break;
        }
    case INT_STMT:
        {
            const auto _int = dynamic_cast<Int*>(statement)->value;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            break;
        }
    case FLOAT_STMT:
        {
            floatbyte.a = dynamic_cast<Float*>(statement)->value;
            bytes << floatbyte.bytes[3];
            bytes << floatbyte.bytes[2];
            bytes << floatbyte.bytes[1];
            bytes << floatbyte.bytes[0];
            break;
        }
    case DOUBLE_STMT:
        {
            doublebyte.a = dynamic_cast<Double*>(statement)->value;
            bytes << doublebyte.bytes[7];
            bytes << doublebyte.bytes[6];
            bytes << doublebyte.bytes[5];
            bytes << doublebyte.bytes[4];
            bytes << doublebyte.bytes[3];
            bytes << doublebyte.bytes[2];
            bytes << doublebyte.bytes[1];
            bytes << doublebyte.bytes[0];
            break;
        }
    case LONG_STMT:
        {
            const long _long = dynamic_cast<Long*>(statement)->value;
            bytes <<  static_cast<char>((_long >> (56)) & 0xff);
            bytes <<  static_cast<char>((_long >> (48)) & 0xff);
            bytes <<  static_cast<char>((_long >> (40)) & 0xff);
            bytes <<  static_cast<char>((_long >> (32)) & 0xff);
            bytes <<  static_cast<char>((_long >> (24)) & 0xff);
            bytes <<  static_cast<char>((_long >> (16)) & 0xff);
            bytes <<  static_cast<char>((_long >> (8)) & 0xff);
            bytes <<  static_cast<char>((_long >> (0)) & 0xff);
            break;
        }
    case STRING_STMT:
        bytes << dynamic_cast<String*>(statement)->value;
        bytes << DELIM;
        break;
    case ARRAY:
    case ARRAY_ACCESS:
        bytes << "??";
        break;
    case REFERENCE:
        {
            const auto _int = dynamic_cast<Reference*>(statement)->value;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
        }
        break;
    case StatementType::SIZE:
        {
            const auto [sz, prec] = dynamic_cast<Size*>(statement)->value;
            auto _int = sz;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            _int = prec;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            break;
        }
    case FUNC_CALL:
        {
            const auto _call = dynamic_cast<FuncCall*>(statement);
            const auto _int = _call->name.value;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            for(const auto v : _call->params)
            {
                addBytes(v, bytes);
                bytes << SEP;
            }
            bytes << DELIM;
            break;
        }
    case FUNC_DEFINITION:
        {
            const auto _func = dynamic_cast<Func*>(statement);

            auto _int = _func->name.value;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            _int = _func->params;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);

            const auto sz = new Size(_func->returns);
            addBytes(sz,bytes);
            delete sz;

            addBytes(_func->body,bytes);
            bytes << DELIM;
            break;
        }
    case ASSIGNMENT:
        {
            const auto _assign = dynamic_cast<Assignment*>(statement);
            const auto _int = _assign->name.value;
            bytes <<  static_cast<char>((_int >> (24)) & 0xff);
            bytes <<  static_cast<char>((_int >> (16)) & 0xff);
            bytes <<  static_cast<char>((_int >> (8)) & 0xff);
            bytes <<  static_cast<char>((_int >> (0)) & 0xff);
            const char atype = _assign->type;
            bytes << atype;
            addBytes(_assign->value,bytes);
            bytes << DELIM;
            break;
        }
    case WHILE_STMT:
        {
            const auto _while = dynamic_cast<WhileStatement*>(statement);
            addBytes(_while->condition,bytes);
            bytes << SEP;
            addBytes(_while->whileBlock,bytes);
            bytes << DELIM;
            break;
        }
    case IF_STMT:
        {
            const auto _if = dynamic_cast<IfStatement*>(statement);
            addBytes(_if->condition,bytes);
            bytes << SEP;
            addBytes(_if->ifBlock,bytes);
            if(_if->elseBlock != nullptr)
            {
                bytes << SEP;
                addBytes(_if->elseBlock,bytes);
            }
            bytes << DELIM;
            break;
        }
    case INTERUPT:
        {
            const auto _interupt = dynamic_cast<Interupt*>(statement);
            const char itype = _interupt->type;
            bytes << itype;
            if(_interupt->value != nullptr)
                addBytes(_interupt->value,bytes);
            bytes << DELIM;
            break;
        }
    case MULTI_OPERATION:
        {
            const auto _multi = dynamic_cast<MultipleOperation*>(statement);
            const char mtype = _multi->op;
            bytes << mtype;
            const auto sz = new Size(_multi->size);
            addBytes(sz,bytes);
            delete sz;
            for(const auto v : _multi->operands)
            {
                addBytes(v,bytes);
                bytes << SEP;
            }
            bytes << DELIM;
            for(const auto v : _multi->invoperands)
            {
                addBytes(v,bytes);
                bytes << SEP;
            }
            bytes << DELIM;
            break;
        }
    case UN_OPERATION:
        {
            const auto _un = dynamic_cast<UnaryOperation*>(statement);
            const char utype = _un->op;
            bytes << utype;
            addBytes(_un->right,bytes);
            bytes << DELIM;
            break;
        }
    case SCOPE:
        {
            const auto _scope = dynamic_cast<CodeBlock*>(statement);
            for(const auto v : _scope->code)
            {
                addBytes(v,bytes);
                bytes << SEP;
            }
            bytes << DELIM;
            break;
        }
    default:
        {
            bytes << "?ERROR?";
            break;
        }
    }
}
