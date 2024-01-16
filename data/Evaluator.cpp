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

Token* ExecuteBlock(BLOCK* block){
	StartScope();

	for (STATEMENT* st : block->code) {
		cout << "Executing line : ";
		st->print();
		cout << endl;
		ExecuteLine(st);
	}

	for (int i = 0; i < scopesStack.back(); i++) {
		cout << varsStack[i].name << " = " << ((IntToken*)varsStack[i].value)->value<< endl;;
	}

	EndScope();
	return new Token();
}

Token* ExecuteLine(STATEMENT* line) {
	switch (line->getType()) {
	case INT:
		return &((INT_VAL*)line)->value;
		break;
	case DEFINE: {
		DEFINITION* def = (DEFINITION*)line;
		Token* value = ExecuteLine(def->value);
		varsStack.push_back(Variable(def->name->value.value, value));
		scopesStack.back()++;
		break;

	}

	case OPERATE: {
		OPERATION* op = (OPERATION*)line;
		IntToken lhs = *(IntToken*)ExecuteLine(op->left);
		IntToken rhs = *(IntToken*)ExecuteLine(op->right);

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
		return ExecuteLine(((PARENTHESIS*)line)->value);
		break;
	}

	default:
		cout << "ERROR : Unknown statement type" << line->getType() << endl;
		break;
	}

	return new Token();
}