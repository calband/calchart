#ifndef _JSON_FILE_FORMATTER_H_
#define _JSON_FILE_FORMATTER_H_

#include <string>
#include <vector>

#include <wx\stream.h>

#define OUTSTREAM wxOutputStream
#define writeToStream(stream, bytes, size) stream->Write(bytes, size)

enum JSONValueType {
	VAL_FIRST = 0,
	VAL_STRING = VAL_FIRST,
	VAL_NUMBER,
	VAL_NULL,
	VAL_BOOLEAN,
	VAL_OBJECT,
	VAL_ARRAY,
	VAL_LAST
};

enum JSONTokenType {
	TOK_FIRST = 0,
	TOK_VALUE = TOK_FIRST,
	TOK_BINDING,
	TOK_START_BLOCK,
	TOK_END_BLOCK,
	TOK_ASSIGNMENT,
	TOK_COMMA,
	TOK_STYLE,
	TOK_LAST
};

class JSONFormatter;
class JSONContainerValue;

class JSONToken {
public:
	virtual JSONTokenType getType() = 0;
	virtual std::string toString() = 0;
};

class GenericJSONToken : public JSONToken {
public:
	virtual JSONTokenType getType();
	virtual std::string toString();

	static JSONToken* makeNumberToken(float value);
	static JSONToken* makeNumberToken(int value);
	static JSONToken* makeBindingToken(std::string bindingName);
	static JSONToken* makeStringToken(std::string value);
	static JSONToken* makeBooleanToken(bool value);
	static JSONToken* makeNullToken();
	static JSONToken* makeAssignmentToken();
	static JSONToken* makeObjectStartToken();
	static JSONToken* makeObjectEndToken();
	static JSONToken* makeArrayStartToken();
	static JSONToken* makeArrayEndToken();
	static JSONToken* makeCommaToken();
	static JSONToken* makeSpaceToken(int spaces);
	static JSONToken* makeNewlineToken(int newlines);
	static JSONToken* makeTabToken(int tabs);
protected:
	static std::string makeRepeatedString(std::string base, int repetitions);
private:
	JSONTokenType mType;
	std::string mToken;

	GenericJSONToken(JSONTokenType type, std::string token);
};

class JSONValue {
public:
	virtual JSONValueType getType() = 0;
	virtual std::string toString();
protected:
	static std::string tokenVectorToString(std::vector<JSONToken*>& vector);
	static void destroyTokenVector(std::vector<JSONToken*>& vector);
	virtual void exportTokensToVector(std::vector<JSONToken*>& tokenPool);
	virtual JSONToken* makeToken() = 0;

	friend JSONFormatter;
	friend JSONContainerValue;
};

class JSONStringValue : public JSONValue {
public:
	JSONStringValue(std::string value) : mVal(value) {};

	std::string getValue() { return mVal; };
	void setValue(std::string newVal) { mVal = newVal; };

	virtual JSONValueType getType() { return VAL_STRING; };
protected:
	virtual JSONToken* makeToken() { return GenericJSONToken::makeStringToken(mVal); };
private:
	std::string mVal;
};

class JSONNumberValue : public JSONValue {
public:
	virtual JSONValueType getType() { return VAL_NUMBER; };
};

class JSONFloatValue : public JSONNumberValue {
public:
	JSONFloatValue(float value) : mVal(value) {};

	float getValue() { return mVal; };
	void setValue(float newVal) { mVal = newVal; };
protected:
	virtual JSONToken* makeToken() { return GenericJSONToken::makeNumberToken(mVal); };
private:
	float mVal;
};

class JSONIntValue : public JSONNumberValue {
public:
	JSONIntValue(int value) : mVal(value) {};

	int getValue() { return mVal; }
	void setValue(int newVal) { mVal = newVal; }

protected:
	virtual JSONToken* makeToken() { return GenericJSONToken::makeNumberToken(mVal); };
private:
	int mVal;
};

class JSONNullValue : public JSONValue {
public:
	virtual JSONValueType getType() { return VAL_NULL; };
protected:
	virtual JSONToken* makeToken() { return GenericJSONToken::makeNullToken(); };
};

class JSONBooleanValue : public JSONValue {
public:
	JSONBooleanValue(bool value) : mVal(value) {};

	bool getValue() { return mVal; };
	void setValue(bool newVal) { mVal = newVal; };

	virtual JSONValueType getType() { return VAL_BOOLEAN; };
protected:
	virtual JSONToken* makeToken() { return GenericJSONToken::makeBooleanToken(mVal); };
private:
	bool mVal;
};

class JSONContainerValue : public JSONValue {
public:
	virtual JSONValue* getContentValue(int index) = 0;
	virtual int getNumContentValues() = 0;
protected:
	virtual void exportTokensToVector(std::vector<JSONToken*>& tokenPool);
	virtual void exportContentToken(std::vector<JSONToken*>& tokenPool, int index);

	virtual JSONToken* makeBlockOpenToken() = 0;
	virtual JSONToken* makeBlockCloseToken() = 0;

	virtual JSONToken* makeToken() { return nullptr; };

	void destroyContent();
};

class JSONObjectValue : public JSONContainerValue {
private:
	using super = JSONContainerValue;
public:
	~JSONObjectValue() { destroyContent(); };

	void setValue(std::string binding, JSONValue* value);

	virtual JSONValueType getType() { return VAL_OBJECT; };

	virtual JSONValue* getContentValue(int index) { return mValues[index]; };
	virtual int getNumContentValues() { return mValues.size(); };

	virtual std::string getBinding(int index) { return mBindings[index]; };
	virtual JSONValue* getContentValue(std::string binding);
protected:
	virtual int getBindingIndex(std::string binding);

	virtual JSONToken* makeBlockOpenToken() { return GenericJSONToken::makeObjectStartToken(); };
	virtual JSONToken* makeBlockCloseToken() { return GenericJSONToken::makeObjectEndToken(); };

	virtual void exportContentToken(std::vector<JSONToken*>& tokenPool, int index);

private:
	std::vector<std::string> mBindings;
	std::vector<JSONValue*> mValues;
};

class JSONArrayValue : public JSONContainerValue {
public:
	~JSONArrayValue() { destroyContent(); };

	virtual void addValue(JSONValue* value) { mValues.push_back(value); };

	virtual JSONValueType getType() { return VAL_ARRAY; };


	virtual JSONValue* getContentValue(int index) { return mValues[index]; };
	virtual int getNumContentValues() { return mValues.size(); };
protected:
	virtual JSONToken* makeBlockOpenToken() { return GenericJSONToken::makeArrayStartToken(); };
	virtual JSONToken* makeBlockCloseToken() { return GenericJSONToken::makeArrayEndToken(); };

private:
	std::vector<JSONValue*> mValues;
};

class JSONFormatter {
public:
	static void exportJSON(OUTSTREAM* output, JSONObjectValue& mainObject);
protected:
	static void makeTokens(JSONObjectValue* mainObject, std::vector<JSONToken*>& tokenVector);
	static void destroyTokens(std::vector<JSONToken*>& tokenVector);
	static void stylize(std::vector<JSONToken*>& tokenVector);
	static void exportToStream(OUTSTREAM* output, std::vector<JSONToken*>& tokenVector);
};

#endif