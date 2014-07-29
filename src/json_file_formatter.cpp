#include "json_file_formatter.h"
#include <sstream>

JSONTokenType GenericJSONToken::getType() {
	return mType;
}

std::string GenericJSONToken::toString() {
	return mToken;
}

JSONToken* GenericJSONToken::makeNumberToken(float value) {
	std::ostringstream toString;
	toString << value;
	return new GenericJSONToken(TOK_VALUE, toString.str());
}

JSONToken* GenericJSONToken::makeNumberToken(int value) {
	std::ostringstream toString;
	toString << value;
	return new GenericJSONToken(TOK_VALUE, "" + toString.str());
}

JSONToken* GenericJSONToken::makeBindingToken(std::string bindingName) {
	return new GenericJSONToken(TOK_BINDING, "\"" + bindingName + "\"");
}

JSONToken* GenericJSONToken::makeStringToken(std::string value) {
	return new GenericJSONToken(TOK_VALUE, "\"" + value + "\"");
}

JSONToken* GenericJSONToken::makeBooleanToken(bool value) {
	std::string valueString = "false";
	if (value) {
		valueString = "true";
	}
	return new GenericJSONToken(TOK_VALUE, valueString);
}

JSONToken* GenericJSONToken::makeNullToken() {
	return new GenericJSONToken(TOK_VALUE, "null");
}

JSONToken* GenericJSONToken::makeAssignmentToken() {
	return new GenericJSONToken(TOK_ASSIGNMENT, ":");
}

JSONToken* GenericJSONToken::makeObjectStartToken() {
	return new GenericJSONToken(TOK_START_BLOCK, "{");
}

JSONToken* GenericJSONToken::makeObjectEndToken() {
	return new GenericJSONToken(TOK_END_BLOCK, "}");
}

JSONToken* GenericJSONToken::makeArrayStartToken() {
	return new GenericJSONToken(TOK_START_BLOCK, "[");
}

JSONToken* GenericJSONToken::makeArrayEndToken() {
	return new GenericJSONToken(TOK_END_BLOCK, "]");
}

JSONToken* GenericJSONToken::makeCommaToken() {
	return new GenericJSONToken(TOK_COMMA, ",");
}

JSONToken* GenericJSONToken::makeSpaceToken(int spaces) {
	return new GenericJSONToken(TOK_STYLE, makeRepeatedString(" ", spaces));
}

JSONToken* GenericJSONToken::makeNewlineToken(int newlines) {
	return new GenericJSONToken(TOK_STYLE, makeRepeatedString("\n", newlines));
}

JSONToken* GenericJSONToken::makeTabToken(int tabs) {
	return new GenericJSONToken(TOK_STYLE, makeRepeatedString("\t", tabs));
}

std::string GenericJSONToken::makeRepeatedString(std::string base, int repetitions) {
	std::string returnString = "";
	for (int counter = 0; counter < repetitions; counter++) {
		returnString += base;
	}
	return returnString;
}

GenericJSONToken::GenericJSONToken(JSONTokenType type, std::string token) {
	mType = type;
	mToken = token;
}

std::string JSONValue::toString() {
	std::vector<JSONToken*> allRepresentativeTokens;
	exportTokensToVector(allRepresentativeTokens);
	std::string stringVal = tokenVectorToString(allRepresentativeTokens);
	destroyTokenVector(allRepresentativeTokens);
	return stringVal;
}

std::string JSONValue::tokenVectorToString(std::vector<JSONToken*>& vector) {
	std::string totalString = "";
	for (int index = 0; index < vector.size(); index++) {
		totalString += vector[index]->toString();
	}
	return totalString;
}

void JSONValue::destroyTokenVector(std::vector<JSONToken*>& vector) {
	for (int index = 0; index < vector.size(); index++) {
		delete vector[index];
	}
}

void JSONValue::exportTokensToVector(std::vector<JSONToken*>& tokenPool) {
	tokenPool.push_back(makeToken());
}

void JSONContainerValue::exportTokensToVector(std::vector<JSONToken*>& tokenPool) {
	tokenPool.push_back(makeBlockOpenToken());
	for (int index = 0; index < getNumContentValues(); index++) {
		exportContentToken(tokenPool, index);
	}
	tokenPool.push_back(makeBlockCloseToken());
}

void JSONContainerValue::exportContentToken(std::vector<JSONToken*>& tokenPool, int index) {
	getContentValue(index)->exportTokensToVector(tokenPool);
	if (index < getNumContentValues() - 1) {
		tokenPool.push_back(GenericJSONToken::makeCommaToken());
	}
}

void JSONContainerValue::destroyContent() {
	for (int index = getNumContentValues() - 1; index >= 0; index--) {
		delete getContentValue(index);
	}
}

void JSONObjectValue::setValue(std::string binding, JSONValue* value) {
	int bindingIndex = getBindingIndex(binding);
	if (bindingIndex >= 0) {
		delete mValues[bindingIndex];
		mValues[bindingIndex] = value;
	} else {
		mBindings.push_back(binding);
		mValues.push_back(value);
	}
}

JSONValue* JSONObjectValue::getContentValue(std::string binding) {
	int targetIndex = getBindingIndex(binding);
	if (targetIndex >= 0) {
		return getContentValue(targetIndex);
	} else {
		return nullptr;
	}
}

int JSONObjectValue::getBindingIndex(std::string binding) {
	for (int index = 0; index < getNumContentValues(); index++) {
		if (getBinding(index) == binding) {
			return index;
		}
	}
	return -1;
}

void JSONObjectValue::exportContentToken(std::vector<JSONToken*>& tokenPool, int index) {
	tokenPool.push_back(GenericJSONToken::makeBindingToken(mBindings[index]));
	tokenPool.push_back(GenericJSONToken::makeAssignmentToken());
	super::exportContentToken(tokenPool, index);
}


void JSONFormatter::exportJSON(OUTSTREAM* output, JSONObjectValue& mainObject) {
	std::vector<JSONToken*> outputTokens;
	makeTokens(&mainObject, outputTokens);
	stylize(outputTokens);
	exportToStream(output, outputTokens);
	destroyTokens(outputTokens);
}

void JSONFormatter::makeTokens(JSONObjectValue* mainObject, std::vector<JSONToken*>& tokenVector) {
	((JSONValue*)mainObject)->exportTokensToVector(tokenVector);
}

void JSONFormatter::destroyTokens(std::vector<JSONToken*>& tokenVector) {
	JSONValue::destroyTokenVector(tokenVector);
}

void JSONFormatter::stylize(std::vector<JSONToken*>& tokenVector) {
	std::vector<JSONToken*> stylizedVector;
	int insertedStyleTokens = 0;
	int indentLevel = 0;
	bool startingNewLine = false;
	for (std::vector<JSONToken*>::iterator vectorIterator = tokenVector.begin(); vectorIterator != tokenVector.end(); vectorIterator++) {
		JSONToken* currentToken = *vectorIterator;
		insertedStyleTokens = 0;
		if (currentToken->getType() == TOK_END_BLOCK) {
			indentLevel -= 1;
			startingNewLine = true;
			stylizedVector.push_back(GenericJSONToken::makeNewlineToken(1));
		}
		if (startingNewLine) {
			stylizedVector.push_back(GenericJSONToken::makeTabToken(indentLevel));
			startingNewLine = false;
		}
		stylizedVector.push_back(currentToken);
		switch (currentToken->getType()) {
		case TOK_START_BLOCK:
			indentLevel += 1;
			startingNewLine = true;
			break;
		case TOK_COMMA:
			startingNewLine = true;
			break;
		case TOK_ASSIGNMENT:
			stylizedVector.push_back(GenericJSONToken::makeSpaceToken(1));
			break;
		}
		if (startingNewLine) {
			stylizedVector.push_back(GenericJSONToken::makeNewlineToken(1));
			insertedStyleTokens += 1;
		}
	}
	tokenVector = stylizedVector;
}

void JSONFormatter::exportToStream(OUTSTREAM* output, std::vector<JSONToken*>& tokenVector) {
	for (int tokenIndex = 0; tokenIndex < tokenVector.size(); tokenIndex++) {
		std::string outString = tokenVector[tokenIndex]->toString();
		writeToStream(output, outString.c_str(), outString.size());
	}
}