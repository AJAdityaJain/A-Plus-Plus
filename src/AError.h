#pragma once

#include <iostream>

using namespace std;
enum AXXCode
{
    UNKNOWN_VALUE,
    UNKNOWN_STATEMENT,
    UNKNOWN_OPERATION,
    UNKNOWN_ESCAPE_SEQUENCE,

    ILLEGAL_EXPRESSION,
    ILLEGAL_CHARACTER,

    OVERSIZED_VALUE,
    OVERFLOW_REGISTER,

    MISMATCHED_BRACKET,
    ASSIGNED_WRONG_TYPE,
    REDUNDANT_IMMUTABLE_ARRAY,
};
void aThrowError(AXXCode code, unsigned int line);