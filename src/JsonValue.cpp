#include <cassert> // for assert
#include "JsonValue.h"

namespace MyJson
{

const JsonValue JsonValue::sNullValue;

JsonValue::JsonValue() :
	m_valueType(eNull), 
	m_val(0.0) {}

JsonValue::JsonValue(ValueType t) :
	m_valueType(t),
	m_val(0.0) {}

JsonValue::JsonValue(bool b) :
	m_valueType(b ? eTrue : eFalse),
	m_val(0.0) {}

JsonValue::JsonValue(double val) : 
	m_valueType(eNumber), 
	m_val(val) {}

JsonValue::JsonValue(int val) :
	m_valueType(eNumber),
	m_val(val) {}

JsonValue::JsonValue(const std::string& str) :
	m_valueType(eString),
	m_str(str), 
	m_val(0.0) {}

JsonValue::JsonValue(const char* str) :
	m_valueType(eString), 
	m_str(str), 
	m_val(0.0) {}

JsonValue::JsonValue(const char* begin, const char* end) :
	m_valueType(eString), 
	m_str(begin, end), 
	m_val(0.0) {}

JsonValue::JsonValue(const char* str, size_t len) :
	m_valueType(eString),
	m_str(str, str + len),
	m_val(0.0) {}

JsonValue::JsonValue(const JsonValue& value)
{
	copyFrom(value);
}

JsonValue::JsonValue(JsonValue&& value)
{
	moveFrom(value);
}

JsonValue::~JsonValue() {}

JsonValue& JsonValue::operator=(const JsonValue& value)
{
	copyFrom(value);
	return *this;
}
JsonValue& JsonValue::operator=(JsonValue&& value)
{
	moveFrom(value);
	return *this;
}

bool JsonValue::operator==(const JsonValue& value) const
{
	if (m_valueType == value.m_valueType)
	{
		switch (m_valueType)
		{
		case eNull:
		case eTrue:
		case eFalse:
			return true;
		case eNumber:
			return m_val == value.m_val;
		case eString:
			return m_str == value.m_str;
		case eArray:
			{
				return m_arr == value.m_arr;
			}
		case eObject:
			{
				return m_obj == value.m_obj;
			}
		}
	}
	return false;
}

bool JsonValue::operator!=(const JsonValue& value) const
{
	return !(*this == value);
}

void JsonValue::copyFrom(const JsonValue& value)
{
	setNull();
	m_valueType = value.type();
	if (value.isNumber())
		m_val = value.m_val;
	else if (value.isString())
		m_str = value.m_str;
	else if (value.isArray())
		m_arr = value.m_arr;
	else if (value.isObject())
		m_obj = value.m_obj;
}

void JsonValue::moveFrom(JsonValue& value)
{
	setNull();
	m_valueType = value.type();
	if (value.isNumber())
		m_val = value.m_val;
	else if (value.isString())
		m_str = std::move(value.m_str);
	else if (value.isArray())
		m_arr = std::move(value.m_arr);
	else if (value.isObject())
		m_obj = std::move(value.m_obj);
}

void JsonValue::setType(ValueType t) 
{
	m_valueType = t;

}
ValueType JsonValue::type() const
{ 
	return m_valueType;
}

// null
bool JsonValue::isNull() const
{
	return m_valueType == eNull;
}
void JsonValue::setNull()
{
	m_valueType = eNull;
	m_val = 0.0;
	m_str.clear();
	m_arr.clear();
	m_obj.clear();
}

// boolean
bool JsonValue::isBool() const
{
	return m_valueType == eTrue || m_valueType == eFalse;
}

bool JsonValue::isTrue() const 
{ 
	return m_valueType == eTrue;
}

bool JsonValue::isFalse() const 
{ 
	return m_valueType == eFalse;
}

void JsonValue::setBool(bool b)
{
	assert(m_valueType == eNull || m_valueType == eTrue || m_valueType == eFalse);
	m_valueType = b ? eTrue : eFalse;
}

bool JsonValue::getBool() const
{
	assert(m_valueType == eTrue || m_valueType == eFalse);
	return m_valueType == eTrue;
}

// number
bool JsonValue::isNumber() const
{
	return m_valueType == eNumber;
}

double JsonValue::getNumber() const
{
	assert(m_valueType == eNumber);
	return m_val;
}

void JsonValue::setNumber(double val)
{
	assert(m_valueType == eNull || m_valueType == eNumber);
	m_valueType = eNumber;
	m_val = val;
}

// string
bool JsonValue::isString() const
{
	return m_valueType == eString;
}

const std::string& JsonValue::getString() const
{
	assert(m_valueType == eString);
	return m_str;
}

void JsonValue::setString(const char* str, size_t len) // str can include \0
{
	setString(str, str + len);
}

void JsonValue::setString(const char* begin, const char* end)
{
	assert(m_valueType == eNull || m_valueType == eString);
	m_valueType = eString;
	m_str = std::string(begin, end);
}

void JsonValue::setString(const std::string& str)
{
	assert(m_valueType == eNull || m_valueType == eString);
	m_valueType = eString;
	m_str = str;
}

// array
bool JsonValue::isArray() const
{
	return m_valueType == eArray;
}

const JsonValue& JsonValue::get(size_t index) const // get copy of array element
{
	assert(m_valueType == eArray);
	assert(index >= 0 && index < m_arr.size());
	return m_arr[index];
}

void JsonValue::resize(size_t newSize) // resize array
{
	assert(m_valueType == eArray && newSize >= 0);
	m_arr.resize(newSize);
}

void JsonValue::append(const JsonValue& value) // append a new value to array
{
	append(JsonValue(value));
}

void JsonValue::append(JsonValue&& value) // append a rvalue
{
	assert(m_valueType == eNull || m_valueType == eArray);
	m_valueType = eArray;
	m_arr.emplace_back(std::move(value));
}

bool JsonValue::insert(size_t index, const JsonValue& value) // insert a value to index
{
	return insert(index, JsonValue(value));
}

bool JsonValue::insert(size_t index, JsonValue&& value) // insert a rvalue to index
{
	if (m_valueType != eArray || !(index >= 0 && index <= m_arr.size()))
		return false;
	m_arr.emplace(m_arr.begin() + index, std::move(value));
	return true;
}

bool JsonValue::removeAt(size_t index, JsonValue& removed) // remove value of index, get removed value
{
	if (m_valueType != eArray || !(index >= 0 && index < m_arr.size()))
		return false;
	removed = std::move(m_arr[index]);
	m_arr.erase(m_arr.begin() + index);
	return true;
}

bool JsonValue::removeAt(size_t index) // just remove value of index
{
	if (m_valueType != eArray || !(index >= 0 && index < m_arr.size()))
		return false;
	m_arr.erase(m_arr.begin() + index);
	return true;
}

JsonValue& JsonValue::operator[](size_t index) // get reference of array element
{
	assert(m_valueType == eArray);
	assert(index >= 0 && index < m_arr.size());
	return m_arr[index];
}

const JsonValue& JsonValue::operator[](size_t index) const // get const reference of array element
{
	assert(m_valueType == eArray);
	assert(index >= 0 && index < m_arr.size());
	return m_arr[index];
}

// object
bool JsonValue::isObject() const
{
	return m_valueType == eObject;
}

bool JsonValue::containsKey(const std::string& key) const // if these is a key in object
{
	assert(m_valueType == eObject);
	return m_obj.find(key) != m_obj.end();
}

std::vector<std::string> JsonValue::getKeys() const // get all keys
{
	assert(m_valueType == eObject);
	std::vector<std::string> keys;
	for (auto iter = m_obj.begin(); iter != m_obj.end(); iter++)
		keys.push_back(iter->first);
	return std::move(keys);
}

const JsonValue& JsonValue::get(const std::string& key) const // get const reference of value of key
{
	assert(m_valueType == eObject);
	auto iter = m_obj.find(key);
	if (iter == m_obj.end())
		return sNullValue;
	else
		return iter->second;
}

void JsonValue::appendKey(const std::string& key, const JsonValue& value) // append a value to object for key
{
	assert(m_valueType == eObject || m_valueType == eNull);
	m_valueType = eObject;
	m_obj[key] = value;
}

bool JsonValue::removeKey(const std::string& key, JsonValue& removed) // erase value of key, get removed vlaue
{
	if (m_valueType != eObject || !containsKey(key))
		return false;
	removed = std::move(m_obj[key]);
	m_obj.erase(key);
	return true;
}

bool JsonValue::removeKey(const std::string& key) // just remove value of key
{
	if (m_valueType != eObject || !containsKey(key))
		return false;
	m_obj.erase(key);
	return true;
}

JsonValue& JsonValue::operator[](const std::string& key) // get reference of value of key
{
	assert(m_valueType == eObject || m_valueType == eNull);
	m_valueType = eObject;
	return m_obj[key];
}

// array & object in common
size_t JsonValue::size() const // object & array size
{
	assert(m_valueType == eArray || m_valueType == eObject);
	if (m_valueType == eArray)
		return m_arr.size();
	else
		return m_obj.size();
}

void JsonValue::clear() // clear all elements
{
	assert(m_valueType == eArray || m_valueType == eObject);
	m_arr.clear();
	m_obj.clear();
}

bool JsonValue::empty() const // is array or object empty
{
	assert(m_valueType == eArray || m_valueType == eObject);
	return m_valueType == eArray ? m_arr.empty() : m_obj.empty();
}

}