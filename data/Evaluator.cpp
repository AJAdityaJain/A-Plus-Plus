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
			cout << "Executing line : ";
			st->print();
			cout << endl;
			Execute(st);
		}

		for (int i = 0; i < scopesStack.back(); i++) {
			cout << varsStack[i].name << " = " << ((IntToken*)varsStack[i].value)->value << endl;;
		}

		EndScope();
		return new Token();
		break;
	}

	case ID: {
		for (Variable v : varsStack) 		
			if (((ID_VAL*)line)->value.value.compare(v.name) == 0) 
				return v.value;

		break;
	}

	case INT: {
		return &((INT_VAL*)line)->value;
		break;

	}

	case DEFINE: {
		DEFINITION* def = (DEFINITION*)line;
		Token* value = Execute(def->value);
		varsStack.push_back(Variable(def->name->value.value, value));
		scopesStack.back()++;
		return new Token();
		break;
	}

	case ASSIGN: {
		ASSIGNMENT* def = (ASSIGNMENT*)line;
		Token* value = Execute(def->value);
		for (Variable& v : varsStack)
			if (def->name->value.value.compare(v.name) == 0) {
				v.value = value;
				return new Token();
			}
		break;
	}

	case OPERATE: {
		OPERATION* op = (OPERATION*)line;
		IntToken lhs = *(IntToken*)Execute(op->left);
		IntToken rhs = *(IntToken*)Execute(op->right);

		switch (op->op) {
		case PLUS:
			return new IntToken(lhs.value + rhs.value);
			break;

		case MINUS:
			return new IntToken(lhs.value - rhs.value);
			break;

		case MULTIPLY:
			return new IntToken(lhs.value * rhs.value);
			break;

		case DIVIDE:
			return new IntToken(lhs.value / rhs.value);
			break;
		default:
			cout << "ERROR : Unknown Operator type" << endl;
			cout << line->getType() << endl;
			break;

		}
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

	return new Token();
}

