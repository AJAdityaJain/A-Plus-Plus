#include "Evaluator.h"

void StartScope() {
	scopesStack.push_back(0);
}

void EndScope() {
	for (int i = 0; i < scopesStack.back(); i++) {
		varsStack.pop_back();
	}
	scopesStack.pop_back();
}

bool waitForElse = false;

Token* Execute(Statement* line) {
	
	switch (line->getType()) {
	case SCOPE: {
		StartScope();

		for (Statement* st : ((CodeBlock*)line)->code) {
			Execute(st);
		}

		for (int i = 0; i < scopesStack.back(); i++) {
			TokenType t = varsStack[i].value->getType();

			cout << varsStack[i].name << " = ";
			switch (t) {
			case BIT: {
				cout << ((BitToken*)varsStack[i].value)->value;
				break;
			}
			case INT: {
				cout << ((IntToken*)varsStack[i].value)->value;
				break;
			}
			case FLOAT: {
				cout << ((FloatToken*)varsStack[i].value)->value;
				break;
			}
			case DOUBLE: {
				cout << ((DoubleToken*)varsStack[i].value)->value;
				break;
			}
			case STRING: {
				cout << ((StringToken*)varsStack[i].value)->value;
				break;
			}

			}
			cout << endl;
		}

		EndScope();
		return null;
		break;
	}

	case ID: {for (Variable v : varsStack) if (((Identifier*)line)->value.value.compare(v.name) == 0) return v.value;break;}
	case BIT: return &((Bit*)line)->value;
	case INT: return &((Int*)line)->value;
	case FLOAT: return &((Float*)line)->value;
	case DOUBLE: return &((Double*)line)->value;
	case STRING: return &((String*)line)->value;

	case DEFINITION: {
		Definition* def = (Definition*)line;
		Token* value = Execute(def->value);
		varsStack.push_back(Variable(def->name->value.value, value));
		scopesStack.back()++;
		return null;
		break;
	}

	case ASSIGNMENT: {
		Assignment* def = (Assignment*)line;
		Token* value = Execute(def->value);
		for (Variable& v : varsStack)
			if (def->name->value.value.compare(v.name) == 0) {
				v.value = value;
				return null;
			}
		break;
	}

	case ELSE_STMT: {
		if (waitForElse) {
			waitForElse = false;
			Execute(((ElseStatement*)line)->elseBlock);
		}
		return null;
		break;
	}
				   
	case IF_STMT: {
		IfStatement* ifst = (IfStatement*)line;
		Token* cond = Execute(ifst->condition);
	
		if (cond->getType() == BIT) {
			if (((BitToken*)cond)->value) {
				Execute(ifst->ifBlock);
			}
			else {
				waitForElse = true;
			}
		}
		else {
			cout << "ERROR : Condition is not a boolean" << endl;
		}
		return null;
		break;
	}

	case UN_OPERATION: {
		UnaryOperation* oper = (UnaryOperation*)line;
		Token* right = Execute(oper->right);
		switch (right->getType()) {
		case BIT:
			return Operate_Unary<BitToken>((BitToken*)right, oper->op);
		case INT:
			return Operate_Unary<IntToken>((IntToken*)right, oper->op);
		case FLOAT:
			return Operate_Unary_Dec<FloatToken>((FloatToken*)right, oper->op);
		case DOUBLE:
			return Operate_Unary_Dec<DoubleToken>((DoubleToken*)right, oper->op);
		}
		break;
	}

	case BI_OPERATION: {
		BinaryOperation* oper = (BinaryOperation*)line;
		Token* left = Execute(oper->left);
		Token* right = Execute(oper->right);

		TokenType lt = left->getType();
		TokenType rt = right->getType();
		bool flip = false;
		if (lt > rt) {
			Token* tmp = left;
			left = right;
			right = tmp;
			lt = left->getType();
			rt = right->getType();
			flip = true;
		}
		if (lt <= rt) {
			if (lt == BIT && rt == BIT) {
				return Operate_Binary((BitToken*)left, (BitToken*)right, oper->op);
			}
			if (lt == INT && rt == INT) {
				return Operate_Binary<IntToken>((IntToken*)left, (IntToken*)right, oper->op);
			}
			else if (lt == FLOAT && rt == FLOAT) {
				return Operate_Binary_Dec<FloatToken>((FloatToken*)left, (FloatToken*)right, oper->op);
			}
			else if (lt == DOUBLE && rt == DOUBLE) {
				return Operate_Binary_Dec<DoubleToken>((DoubleToken*)left, (DoubleToken*)right, oper->op);
			}
			else if (lt == INT && rt == FLOAT) {
				if (flip)
					return Operate_Binary_Dec<FloatToken>((FloatToken*)right, new FloatToken(((IntToken*)left)->value), oper->op);
				return Operate_Binary_Dec<FloatToken>(new FloatToken(((IntToken*)left)->value), (FloatToken*)right, oper->op);
			}
			else if (lt == FLOAT && rt == DOUBLE) {
				if (flip)
					return Operate_Binary_Dec<DoubleToken>((DoubleToken*)right, new DoubleToken(((FloatToken*)left)->value), oper->op);
				return Operate_Binary_Dec<DoubleToken>(new DoubleToken(((FloatToken*)left)->value), (DoubleToken*)right, oper->op);
			}
			else if (lt == INT && rt == DOUBLE) {
				if (flip)
					return Operate_Binary_Dec<DoubleToken>((DoubleToken*)right, new DoubleToken(((IntToken*)left)->value), oper->op);
				return Operate_Binary_Dec<DoubleToken>(new DoubleToken(((IntToken*)left)->value), (DoubleToken*)right, oper->op);
			}
		}
		else {
			cout << "ERROR : Faulty operands" << endl;
			return null;
		}		break;
	}

	case PARENTHESIS: Execute(((Parenthesis*)line)->value);

	default: {
		cout << "ERROR : Unknown statement type" << line->getType() << endl;
		break;
	}
	}

	waitForElse = false;
	return null;
}


template<typename T>
Token* Operate_Binary(T* left, T* right, BinaryOperatorType op) {
	switch (op)
	{
	case MODULO:
		return new T(left->value % right->value);
	case OR:
		return new T(left->value || right->value);
	case AND:
		return new T(left->value && right->value);
	case XOR:
		return new T(left->value ^ right->value);
	case BITWISE_OR:
		return new T(left->value | right->value);
	case BITWISE_AND:
		return new T(left->value & right->value);
	default:
		return Operate_Binary_Dec<T>(left, right, op);
	}
}

template<typename T>
Token* Operate_Binary_Dec(T* left, T* right, BinaryOperatorType op) {
	switch (op)
	{
	case PLUS:
		return new T(left->value + right->value);
	case MINUS:
		return new T(left->value - right->value);
	case MULTIPLY:
		return new T(left->value * right->value);
	case DIVIDE:
		return new T(left->value / right->value);
	case COMPARISON:
		return new BitToken(left->value == right->value);
	case NOT_EQUAL:
		return new BitToken(left->value != right->value);
	case GREATER_THAN:
		return new BitToken(left->value > right->value);
	case GREATER_THAN_EQUAL:
		return new BitToken(left->value >= right->value);
	case SMALLER_THAN:
		return new BitToken(left->value < right->value);
	case SMALLER_THAN_EQUAL:
		return new BitToken(left->value <= right->value);
	}
	cout << "Error: Unknown operator" << endl;
	return null;
}


template<typename T>
Token* Operate_Unary(T* right, UnaryOperatorType op) {
	switch (op)
	{
	case NOT: {
		return new T(!(right->value));
	}
	case BITWISE_NOT: {
		return new T(~(right->value));
	}
	default:
		return Operate_Unary_Dec(right, op);
	}
	cout << "Error: Unknown operator" << endl;
	return null;
}

template<typename T>
Token* Operate_Unary_Dec(T* right, UnaryOperatorType op) {
	switch (op)
	{
	case POSITIVE: {
		return right;
	}
	case NEGATIVE: {
		return new T(-right->value);
	}
	}
	cout << "Error: Unknown operator" << endl;
	return null;
}