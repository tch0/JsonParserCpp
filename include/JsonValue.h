#ifndef _JSON_VALUE_H_
#define _JSON_VALUE_H_

#include <string>
#include <vector>
#include <map>

namespace MyJson
{

enum ValueType
{
	eNull = 0,
	eTrue,
	eFalse,
	eNumber,
	eString,
	eArray,
	eObject
};

class JsonValue
{
public:
	JsonValue();
	JsonValue(ValueType t);
	JsonValue(bool b);
	JsonValue(double val);
	JsonValue(int val);
	JsonValue(const std::string& str);
	JsonValue(const char* str);
	JsonValue(const char* begin, const char* end);
	JsonValue(const char* str, size_t len);
	JsonValue(std::nullptr_t p) = delete;
	JsonValue(void* p) = delete;
	JsonValue(const JsonValue& value);
	JsonValue(JsonValue&& value);

	~JsonValue();

	JsonValue& operator=(const JsonValue& value);
	JsonValue& operator=(JsonValue&& value);

	bool operator==(const JsonValue& value) const;
	bool operator!=(const JsonValue& value) const;

private:
	void copyFrom(const JsonValue& value);
	void moveFrom(JsonValue& value);

public:
	static const JsonValue sNullValue;

	void setType(ValueType t);
	ValueType type() const;

	// null
	bool isNull() const;
	void setNull();

	// boolean
	bool isBool() const;
	bool isTrue() const;
	bool isFalse() const;
	void setBool(bool b);
	bool getBool() const;

	// number
	bool isNumber() const;
	double getNumber() const;
	void setNumber(double val);

	// string
	bool isString() const;
	const std::string& getString() const;
	void setString(const char* str, size_t len);
	void setString(const char* begin, const char* end);
	void setString(const std::string& str);

	// array
	bool isArray() const;
	const JsonValue& get(size_t index) const;
	void resize(size_t newSize);
	void append(const JsonValue& value);
	void append(JsonValue&& value);
	bool insert(size_t index, const JsonValue& value);
	bool insert(size_t index, JsonValue&& value);
	bool removeAt(size_t index, JsonValue& removed);
	bool removeAt(size_t index);
	JsonValue& operator[](size_t index);
	const JsonValue& operator[](size_t index) const;

	// object
	bool isObject() const;
	bool containsKey(const std::string& key) const;
	std::vector<std::string> getKeys() const;
	const JsonValue& get(const std::string& key) const;
	void appendKey(const std::string& key, const JsonValue& value);
	bool removeKey(const std::string& key, JsonValue& removed);
	bool removeKey(const std::string& key);
	JsonValue& operator[](const std::string& key);

	// array & object in common
	size_t size() const;
	void clear();
	bool empty() const;

private:
	ValueType m_valueType;
	double m_val;
	std::string m_str;
	std::vector<JsonValue> m_arr;
	std::map<std::string, JsonValue> m_obj;
};

}
#endif
