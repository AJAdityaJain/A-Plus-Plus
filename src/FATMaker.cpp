//
// Created by Aditya Jain on 9/3/24.
//

#include "FATMaker.h"

constexpr unsigned char DELIM = 0xff;
union {
    long a;
    uint8_t bytes[8];
} longintshortbyte;
union {
    float a;
    uint8_t bytes[4];
} floatbyte;
union {
    double a;
    uint8_t bytes[8];
} doublebyte;

vector<Statement*> readFatFile(const char* filename)
{
    auto vec = vector<uint8_t>();
    {
        ifstream file(filename, ios::binary);
        char byte;
        while (file.get(byte)) {
            vec.push_back(static_cast<uint8_t>(byte));
        }
        file.close();
        reverse(vec.begin(),vec.end());
    }

    auto abstractTree = vector<Statement*>();
    while (!vec.empty())
        abstractTree.push_back(readBytes(vec));
    return abstractTree;
}

void createFatFile(const char* filename, const vector<Statement*>& abstractTree)
{
    auto vec = vector<uint8_t>();
    for (const auto stmt : abstractTree)
        addBytes(stmt, vec);

    auto bytes = ofstream(filename, ios::binary);
    if(bytes.is_open())
        for (const auto b : vec)
            bytes << static_cast<char>(b);
    bytes.close();
}

void addBytes(Statement* statement, vector<uint8_t>& bytes) // NOLINT(*-no-recursion)
{
    const auto type = static_cast<uint8_t>(statement->getType());
    bytes.push_back(type);
    switch (type)
    {
    case BOOL_STMT:
        {
            if (dynamic_cast<Boolean*>(statement)->value)
                bytes.push_back(1);
            else
                bytes.push_back(0);
            break;
        }
    case SHORT_STMT:
        {
            longintshortbyte.a = dynamic_cast<Short*>(statement)->value;
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            break;
        }
    case INT_STMT:
        {
            longintshortbyte.a = dynamic_cast<Int*>(statement)->value;
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            break;
        }
    case FLOAT_STMT:
        {
            floatbyte.a = dynamic_cast<Float*>(statement)->value;
            bytes.push_back(floatbyte.bytes[3]);
            bytes.push_back(floatbyte.bytes[2]);
            bytes.push_back(floatbyte.bytes[1]);
            bytes.push_back(floatbyte.bytes[0]);
            break;
        }
    case DOUBLE_STMT:
        {
            doublebyte.a = dynamic_cast<Double*>(statement)->value;
            bytes.push_back(doublebyte.bytes[7]);
            bytes.push_back(doublebyte.bytes[6]);
            bytes.push_back(doublebyte.bytes[5]);
            bytes.push_back(doublebyte.bytes[4]);
            bytes.push_back(doublebyte.bytes[3]);
            bytes.push_back(doublebyte.bytes[2]);
            bytes.push_back(doublebyte.bytes[1]);
            bytes.push_back(doublebyte.bytes[0]);
            break;
        }
    case LONG_STMT:
        {
            longintshortbyte.a = dynamic_cast<Long*>(statement)->value;
            bytes.push_back(longintshortbyte.bytes[7]);
            bytes.push_back(longintshortbyte.bytes[6]);
            bytes.push_back(longintshortbyte.bytes[5]);
            bytes.push_back(longintshortbyte.bytes[4]);
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            break;
        }
    case STRING_STMT:
        {
            const string& str = dynamic_cast<String*>(statement)->value;
            longintshortbyte.a = static_cast<int>(str.size());
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            for (const auto c : str)
            {
                bytes.push_back(c);
            }
            break;
        }
    case REFERENCE:
        {
            longintshortbyte.a = static_cast<long>(dynamic_cast<Reference*>(statement)->value);
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);

            break;
        }
    case SIZE:
        {
            const auto [sz, prec] = dynamic_cast<Size*>(statement)->value;
            bytes.push_back(sz);
            bytes.push_back(prec);
            break;
        }
    case FUNC_CALL:
        {
            const auto _call = dynamic_cast<FuncCall*>(statement);
            auto i = static_cast<int>(bytes.size());
            bytes.push_back(0);
            bytes.push_back(0);
            longintshortbyte.a = static_cast<long>(_call->name.value);
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            for(const auto v : _call->params)
            {
                addBytes(v, bytes);
            }
            longintshortbyte.a = static_cast<int>(bytes.size()-i-6);
            bytes[i  ] = longintshortbyte.bytes[1];
            bytes[i+1] = longintshortbyte.bytes[0];
            break;
        }
    case FUNC_DEFINITION:
        {
            const auto _func = dynamic_cast<Func*>(statement);
            longintshortbyte.a = static_cast<long>(_func->name.value);
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            longintshortbyte.a = _func->params;
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            bytes.push_back(_func->returns.sz);
            bytes.push_back(_func->returns.prec);
            addBytes(_func->body, bytes);
            break;
        }
    case ASSIGNMENT:
        {
            const auto _assign = dynamic_cast<Assignment*>(statement);
            longintshortbyte.a = static_cast<long>(_assign->name.value);
            bytes.push_back(longintshortbyte.bytes[3]);
            bytes.push_back(longintshortbyte.bytes[2]);
            bytes.push_back(longintshortbyte.bytes[1]);
            bytes.push_back(longintshortbyte.bytes[0]);
            bytes.push_back(_assign->type);
            bytes.push_back(_assign->isconst?1:0);
            addBytes(_assign->value, bytes);
            break;
        }
    case WHILE_STMT:
        {
            const auto _while = dynamic_cast<WhileStatement*>(statement);
            addBytes(_while->condition, bytes);
            addBytes(_while->whileBlock, bytes);
            break;
        }
    case IF_STMT:
        {
            const auto _if = dynamic_cast<IfStatement*>(statement);
            addBytes(_if->condition,bytes);
            addBytes(_if->ifBlock,bytes);
            if(_if->elseBlock != nullptr)
            {
                bytes.push_back(DELIM);
                addBytes(_if->elseBlock,bytes);
            }
            break;
        }
    case INTERUPT:
        {
            const auto _interupt = dynamic_cast<Interupt*>(statement);
            bytes.push_back(_interupt->type);
            if(_interupt->value != nullptr)
            {
                bytes.push_back(DELIM);
                addBytes(_interupt->value,bytes);
            }
            break;
        }
    case SCOPE:
        {
            const auto _scope = dynamic_cast<CodeBlock*>(statement);
            int i = static_cast<int>(bytes.size());
            bytes.push_back(0);
            bytes.push_back(0);
            bytes.push_back(0);
            bytes.push_back(0);
            for(const auto v : _scope->code)
            {
                addBytes(v,bytes);
            }
            longintshortbyte.a = static_cast<int>(bytes.size()-i-4);
            bytes[i  ] = longintshortbyte.bytes[3];
            bytes[i+1] = longintshortbyte.bytes[2];
            bytes[i+2] = longintshortbyte.bytes[1];
            bytes[i+3] = longintshortbyte.bytes[0];
            break;
        }
    case MULTI_OPERATION:
        {
            const auto _multi = dynamic_cast<MultipleOperation*>(statement);
            const int i = static_cast<int>(bytes.size());
            int is = 5;
            bytes.push_back(0);
            bytes.push_back(0);
            bytes.push_back(0);
            bytes.push_back(0);
            if(!_multi->invoperands.empty())
            {
                bytes.push_back(DELIM);
                bytes.push_back(0);
                bytes.push_back(0);
                bytes.push_back(0);
                bytes.push_back(0);
                is += 5;
            }
            bytes.push_back(_multi->op);
            for (const auto op : _multi->operands)
                addBytes(op, bytes);
            longintshortbyte.a = static_cast<int>(bytes.size() - i - is);
            is += longintshortbyte.a;
            bytes[i] = longintshortbyte.bytes[3];
            bytes[i + 1] = longintshortbyte.bytes[2];
            bytes[i + 2] = longintshortbyte.bytes[1];
            bytes[i + 3] = longintshortbyte.bytes[0];

            for (const auto op : _multi->invoperands)
                addBytes(op, bytes);

            longintshortbyte.a = static_cast<int>(bytes.size() - i - is);
            if(!_multi->invoperands.empty())
            {
                bytes[i+5] = longintshortbyte.bytes[3];
                bytes[i+6] = longintshortbyte.bytes[2];
                bytes[i+7] = longintshortbyte.bytes[1];
                bytes[i+8] = longintshortbyte.bytes[0];
            }

            break;
        }
    case UN_OPERATION:
        {
            const auto _un = dynamic_cast<UnaryOperation*>(statement);
            bytes.push_back(_un->op);
            addBytes(_un->right, bytes);
            break;
        }
    default:
        {
            aThrowError(UNABLE_TO_COMPILE_FAT, -1);
            break;
        }
    }
}

Statement* readBytes(vector<uint8_t>& bytes)
{
    const auto type = static_cast<StatementType>(bytes.back());
    bytes.pop_back();
    switch (type)
    {
    case BOOL_STMT:
        {
            uint8_t b = bytes.back();bytes.pop_back();
            return new Boolean(b == 1);
        }
    case SHORT_STMT:
        {
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Short(static_cast<short>(longintshortbyte.a));
        }
    case INT_STMT:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Int(static_cast<int>(longintshortbyte.a));
        }
    case FLOAT_STMT:
        {
            floatbyte.bytes[3] = bytes.back();bytes.pop_back();
            floatbyte.bytes[2] = bytes.back();bytes.pop_back();
            floatbyte.bytes[1] = bytes.back();bytes.pop_back();
            floatbyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Float(floatbyte.a);
        }
    case DOUBLE_STMT:
        {
            doublebyte.bytes[7] = bytes.back();bytes.pop_back();
            doublebyte.bytes[6] = bytes.back();bytes.pop_back();
            doublebyte.bytes[5] = bytes.back();bytes.pop_back();
            doublebyte.bytes[4] = bytes.back();bytes.pop_back();
            doublebyte.bytes[3] = bytes.back();bytes.pop_back();
            doublebyte.bytes[2] = bytes.back();bytes.pop_back();
            doublebyte.bytes[1] = bytes.back();bytes.pop_back();
            doublebyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Double(doublebyte.a);
        }
    case LONG_STMT:{
            longintshortbyte.bytes[7] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[6] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[5] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[4] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Long(longintshortbyte.a);
    }
    case STRING_STMT:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            char *string = new char[longintshortbyte.a];
            for (int i = 0; i < longintshortbyte.a; i++)
            {
                string[i] = static_cast<char>(bytes.back());
                bytes.pop_back();
            }
            return new String(string);
        }
    case SIZE:
        {
            AsmSize sz = {0,0};
            sz.sz = static_cast<int8_t>(bytes.back());
            bytes.pop_back();
            sz.prec = static_cast<int8_t>(bytes.back());
            bytes.pop_back();
            return new Size(sz);
        }
    case REFERENCE:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            return new Reference(static_cast<unsigned int>(longintshortbyte.a));
        }
    case UN_OPERATION:
        {
            const auto op = static_cast<UnaryOperatorType>(bytes.back());
            bytes.pop_back();
            const auto right = dynamic_cast<Value*>(readBytes(bytes));
            return new UnaryOperation(op, right);
        }
    case FUNC_CALL:
        {
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            auto i = static_cast<unsigned long long>(longintshortbyte.a);

            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            const auto f = IdentifierToken(longintshortbyte.a);
            auto params = vector<Value*>();
            i = bytes.size() - i;
            while(bytes.size() != i)
                params.push_back(dynamic_cast<Value*>(readBytes(bytes)));
            return new FuncCall(f, params);
        }
    case FUNC_DEFINITION:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            const auto name = IdentifierToken(longintshortbyte.a);
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            const auto params = static_cast<int>(longintshortbyte.a);
            const auto sz = static_cast<int8_t>(bytes.back());
            bytes.pop_back();
            const auto prec = static_cast<int8_t>(bytes.back());
            bytes.pop_back();
            const auto returns = AsmSize(sz, prec);
            const auto body = dynamic_cast<CodeBlock*>(readBytes(bytes));
            return new Func(name, body, returns, params);
        }
    case ASSIGNMENT:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            const auto name = IdentifierToken(longintshortbyte.a);
            const auto atype = static_cast<AssignmentType>(bytes.back());bytes.pop_back();
            const auto isconst = bytes.back() == 1;bytes.pop_back();
            const auto value = dynamic_cast<Value*>(readBytes(bytes));
            return new Assignment(name, value, atype, isconst);
        }
    case WHILE_STMT:
        {
            return new WhileStatement(dynamic_cast<Value*>(readBytes(bytes)),dynamic_cast<CodeBlock*>(readBytes(bytes)));
        }
    case IF_STMT:
        {
            const auto condition = dynamic_cast<Value*>(readBytes(bytes));
            const auto ifBlock = dynamic_cast<CodeBlock*>(readBytes(bytes));
            const auto ifs = new IfStatement(condition,ifBlock,nullptr);
            if(!bytes.empty() && bytes.back() == DELIM)
            {
                bytes.pop_back();
                ifs->elseBlock = dynamic_cast<CodeBlock*>(readBytes(bytes));
            }
            return ifs;
        }
    case INTERUPT:
        {
            const auto t = static_cast<TokenType>(bytes.back());bytes.pop_back();
            if (!bytes.empty() &&bytes.back() == DELIM)
            {
                bytes.pop_back();
                return new Interupt(t, dynamic_cast<Value*>(readBytes(bytes)));
            }
            return new Interupt(t);
        }
    case MULTI_OPERATION:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            auto is = longintshortbyte.a;
            auto is2 = 0;
            if(bytes.back() == DELIM)
            {
                bytes.pop_back();
                longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
                longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
                longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
                longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
                is2 = longintshortbyte.a;
            }
            const auto op = static_cast<MultipleOperatorType>(bytes.back());bytes.pop_back();
            auto operands = vector<Value*>();
            auto invoperands = vector<Value*>();
            is2 = static_cast<long> (bytes.size()) - is2 -is;
            is = static_cast<long> (bytes.size()) - is;
            while (bytes.size() != is)
                operands.push_back(dynamic_cast<Value*>(readBytes(bytes)));
            while (bytes.size() != is2)
                invoperands.push_back(dynamic_cast<Value*>(readBytes(bytes)));
            return new MultipleOperation(op, operands, invoperands);
        }
    case SCOPE:
        {
            longintshortbyte.bytes[3] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[2] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[1] = bytes.back();bytes.pop_back();
            longintshortbyte.bytes[0] = bytes.back();bytes.pop_back();
            const auto i = bytes.size() - longintshortbyte.a;
            vector<Statement*> stmts;
            while (bytes.size() != i)
                stmts.push_back(readBytes(bytes));

            return new CodeBlock(stmts);

        }
    default:break;
    }
    aThrowError(ILLEGAL_EXPRESSION, -12);
    return nullptr;
}