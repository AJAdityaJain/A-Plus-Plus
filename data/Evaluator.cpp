#include "Evaluator.h"

void StartScope() {
	scopesStack.push_back(0);
}

void EndScope() {
	for (int i = 0; i < scopesStack.back(); i++) {
		varsStack.pop_back();
	}
	scopesStack.pop_back();

	varsStack.shrink_to_fit();
	scopesStack.shrink_to_fit();
}

bool waitForElse = false;

Token* Execute(Statement* line) {
	
	switch (line->getType()) {
	case SCOPE: {
		StartScope();

		for (Statement* st : ((CodeBlock*)line)->code) {
			Token* t = Execute(st);
			if (t != null) delete t;
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

	case ID_STMT: {
		for (Variable v : varsStack)
			if (((Identifier*)line)->value.value == v.name)
				switch (v.value->getType()) {
				case BIT:
					return new BitToken(*(BitToken*)v.value);
				case INT:
					return new IntToken(*(IntToken*)v.value);
				case FLOAT:
					return new FloatToken(*(FloatToken*)v.value);
				case DOUBLE:
					return new DoubleToken(*(DoubleToken*)v.value);
				case STRING:
					return new StringToken(*(StringToken*)v.value);
				}
		break;
	}
	case BIT_STMT: return new BitToken(((Bit*)line)->value);
	case INT_STMT: return new IntToken(((Int*)line)->value);
	case FLOAT_STMT: return new FloatToken(((Float*)line)->value);
	case DOUBLE_STMT: return new DoubleToken(((Double*)line)->value);
	case STRING_STMT: return new StringToken(((String*)line)->value);

	case CALL: {


	}
	case DEFINITION: {
		Definition* def = (Definition*)line;

		varsStack.push_back(Variable(def->name->value.value, Execute(def->value)));
		scopesStack.back()++;

		return null;
		break;
	}

	case ASSIGNMENT: {
		Assignment* def = (Assignment*)line;
		
		Token* value = Execute(def->value);
		for (Variable& v : varsStack)
			if (def->name->value.value == v.name) {
				delete v.value;
				v.value = value;
				return null;
			}
		break;
	}

	case ELSE_STMT: {
		if (waitForElse) {
			waitForElse = false;
			Token*t = Execute(((ElseStatement*)line)->elseBlock);
			if (t != null)
				delete t;
		}

		return null;
		break;
	}

	case WHILE_STMT: {
		WhileStatement* whilest = (WhileStatement*)line;
		Token* cond = Execute(whilest->condition);
		
		if (cond->getType() == BIT) {
			while (((BitToken*)cond)->value) {
				Token* t = Execute(whilest->whileBlock);
				if (t != null) delete t;
				delete cond;
				cond = Execute(whilest->condition);
			}
			waitForElse = true;
		}
		else {
			printf("\x1B[31m Not a boolean condition \033[0m\t\t\n");
		}
		delete cond;
		return null;
		break;
	}
				   
	case IF_STMT: {
		IfStatement* ifst = (IfStatement*)line;
		Token* cond = Execute(ifst->condition);
	
		if (cond->getType() == BIT) {
			if (((BitToken*)cond)->value) {
				Token *t =Execute(ifst->ifBlock);
				if (t != null) delete t;
			}
			else {
				waitForElse = true;
			}
		}
		else {
			printf("\x1B[31m Not a boolean condition \033[0m\t\t\n");
		}
		delete cond;
		return null;
		break;
	}

	case UN_OPERATION: {
		UnaryOperation* oper = (UnaryOperation*)line;
		Token* right = Execute(oper->right);
		Token* res = null;

		switch (right->getType()) {
		case BIT: {
			res = Operate_Unary<BitToken>((BitToken*)right, oper->op);
			break;
		}
		case INT:
			res = Operate_Unary<IntToken>((IntToken*)right, oper->op);
			break;
		case FLOAT:
			res = Operate_Unary_Dec<FloatToken>((FloatToken*)right, oper->op);
			break;
		case DOUBLE:
			res = Operate_Unary_Dec<DoubleToken>((DoubleToken*)right, oper->op);
			break;
		}

		if(right != null)
			delete right;

		return res;
		break;
	}

	case BI_OPERATION: {
		BinaryOperation* oper = (BinaryOperation*)line;
		Token* left = Execute(oper->left);
		Token* right = Execute(oper->right);
		Token* res = null;

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
				res = Operate_Binary((BitToken*)left, (BitToken*)right, oper->op);
			}
			if (lt == INT && rt == INT) {
				res = Operate_Binary<IntToken>((IntToken*)left, (IntToken*)right, oper->op);
			}
			else if (lt == FLOAT && rt == FLOAT) {
				res = Operate_Binary_Dec<FloatToken>((FloatToken*)left, (FloatToken*)right, oper->op);
			}
			else if (lt == DOUBLE && rt == DOUBLE) {
				res = Operate_Binary_Dec<DoubleToken>((DoubleToken*)left, (DoubleToken*)right, oper->op);
			}
			else if (lt == INT && rt == FLOAT) {
				if (flip)
					res = Operate_Binary_Dec<FloatToken>((FloatToken*)right, new FloatToken(((IntToken*)left)->value), oper->op);
				else
					res = Operate_Binary_Dec<FloatToken>(new FloatToken(((IntToken*)left)->value), (FloatToken*)right, oper->op);
			}
			else if (lt == FLOAT && rt == DOUBLE) {
				if (flip)
					res = Operate_Binary_Dec<DoubleToken>((DoubleToken*)right, new DoubleToken(((FloatToken*)left)->value), oper->op);
				else
					res = Operate_Binary_Dec<DoubleToken>(new DoubleToken(((FloatToken*)left)->value), (DoubleToken*)right, oper->op);
			}
			else if (lt == INT && rt == DOUBLE) {
				if (flip)
					res = Operate_Binary_Dec<DoubleToken>((DoubleToken*)right, new DoubleToken(((IntToken*)left)->value), oper->op);
				else
					res = Operate_Binary_Dec<DoubleToken>(new DoubleToken(((IntToken*)left)->value), (DoubleToken*)right, oper->op);
			}
		}
		else {
			printf("\x1B[31m Faulty Operands \033[0m\t\t\n");
		}


		if (left != null)
			delete left;
		if (right != null)
			delete right;

		return res;
		break;
	}

	case PARENTHESIS: {
		return Execute(((Parenthesis*)line)->value);
		break;
	}
	}

	printf("\x1B[31m Unexpected statement \033[0m\t\t\n");
	deallocstmt(line);
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
	printf("\x1B[31m Unknown Binary Operator \033[0m\t\t\n");
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
	printf("\x1B[31m Unknown Unary Operator \033[0m\t\t\n");

	return null;
}