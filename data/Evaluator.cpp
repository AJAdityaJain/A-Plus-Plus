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

Token* Execute(STATEMENT* line) {
	
	switch (line->getType()) {
	case SCOPE: {
		StartScope();

		for (STATEMENT* st : ((BLOCK*)line)->code) {
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

	case ID: {
		for (Variable v : varsStack) 		
			if (((ID_VAL*)line)->value.value.compare(v.name) == 0) 
				return v.value;

		break;
	}

	case BIT: {
		return &((BIT_VAL*)line)->value;
		break;

	}
	case INT: {
		return &((INT_VAL*)line)->value;
		break;

	}
	case FLOAT: {
		return &((FLOAT_VAL*)line)->value;
		break;

	}
	case DOUBLE: {
		return &((DOUBLE_VAL*)line)->value;
		break;

	}
	case STRING: {
		return &((STRING_VAL*)line)->value;
		break;

	}

	case DEFINE: {
		DEFINITION* def = (DEFINITION*)line;
		Token* value = Execute(def->value);
		varsStack.push_back(Variable(def->name->value.value, value));
		scopesStack.back()++;
		return null;
		break;
	}

	case ASSIGN: {
		ASSIGNMENT* def = (ASSIGNMENT*)line;
		Token* value = Execute(def->value);
		for (Variable& v : varsStack)
			if (def->name->value.value.compare(v.name) == 0) {
				v.value = value;
				return null;
			}
		break;
	}

	case OPERATE: {
		OPERATION* op = (OPERATION*)line;
		return Operate(Execute(op->left), Execute(op->right), op->op);
		break;
	}

	case PRECEDER:{
		return Execute(((PARENTHESIS*)line)->value);
		break;
	}

	default:
		cout << "ERROR : Unknown statement type" << line->getType() << endl;
		break;
	}

	return null;
}


Token* Operate(Token* left, Token* right, TokenType op) {
	TokenType lt = left->getType();
	TokenType rt = right->getType();
	if (lt > rt) {
		Token* tmp = left;
		left = right;
		right = tmp;
		lt = left->getType();
		rt = right->getType();
	}
	if (lt <= rt) {
		if (lt == INT && rt == INT) {
			if (op == MODULO) 
				return new IntToken(((IntToken*)left)->value % ((IntToken*)right)->value);
			return Operatee<IntToken>((IntToken*)left, (IntToken*)right, op);
		}
		else if (lt == FLOAT && rt == FLOAT) {
			return Operatee<FloatToken>((FloatToken*)left, (FloatToken*)right, op);
		}
		else if (lt == DOUBLE && rt == DOUBLE) {
			return Operatee<DoubleToken>((DoubleToken*)left, (DoubleToken*)right, op);
		}
		else if (lt == INT && rt == FLOAT) {
			return Operatee<FloatToken>(new FloatToken(((IntToken*)left)->value), (FloatToken*)right, op);
		}
		else if (lt == FLOAT && rt == DOUBLE) {
			return Operatee<DoubleToken>(new DoubleToken(((FloatToken*)left)->value), (DoubleToken*)right, op);
		}
		else if (lt == INT && rt == DOUBLE) {
			return Operatee<DoubleToken>(new DoubleToken(((IntToken*)left)->value), (DoubleToken*)right, op);
		}
	}
	else {
		cout << "ERROR : Faulty operands" << endl;
		return null;
	}
}
