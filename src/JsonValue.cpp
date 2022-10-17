#include <cassert> // for assert
#include "JsonValue.h"

namespace MyJson
{

const JsonValue JsonValue::sNullValue;

JsonValue::JsonValue() :
	m_valueType(eNull), 
	m_var() {}

JsonValue::JsonValue(ValueType t) :
	m_valueType(t),
	m_var()
	{
		switch (t)
		{
		case eString:
			m_var = StringType();
			break;
		case eArray:
			m_var = ArrayType();
			break;
		case eObject:
			m_var = ObjectType();
			break;
		}
	}

JsonValue::JsonValue(bool b) :
	m_valueType(b ? eTrue : eFalse),
	m_var() {}

JsonValue::JsonValue(double val) : 
	m_valueType(eNumber), 
	m_var(val) {}

JsonValue::JsonValue(int val) :
	m_valueType(eNumber),
	m_var(double(val)) {}

JsonValue::JsonValue(const std::string& str) :
	m_valueType(eString),
	m_var(str){}

JsonValue::JsonValue(const char* str) :
	m_valueType(eString), 
	m_var(str) {}

JsonValue::JsonValue(const char* begin, const char* end) :
	m_valueType(eString), 
	m_var(StringType(begin, end)) {}

JsonValue::JsonValue(const char* str, size_t len) :
	m_valueType(eString),
	m_var(StringType(str, str + len)) {}

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
		case eString:
		case eArray:
		case eObject:
			return m_var == value.m_var;
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
	m_var = value.m_var;
}

void JsonValue::moveFrom(JsonValue& value)
{
	setNull();
	m_valueType = value.type();
	m_var = std::move(value.m_var);
}

void JsonValue::setType(ValueType t) 
{
	m_valueType = t;
	switch (t)
	{
	case eString:
		m_var = StringType();
		break;
	case eArray:
		m_var = ArrayType();
		break;
	case eObject:
		m_var = ObjectType();
		break;
	}
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
	m_var = 0.0;
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
	m_valueType = (b ? eTrue : eFalse);
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
	assert(m_var.index() == 0);
	return std::get<double>(m_var);
}

void JsonValue::setNumber(double val)
{
	assert(m_valueType == eNull || m_valueType == eNumber);
	m_valueType = eNumber;
	m_var = val;
}

// string
bool JsonValue::isString() const
{
	return m_valueType == eString;
}

const std::string& JsonValue::getString() const
{
	assert(m_valueType == eString);
	assert(m_var.index() == 1);
	return std::get<StringType>(m_var);
}

void JsonValue::setString(const char* str, size_t len) // str can include \0
{
	setString(str, str + len);
}

void JsonValue::setString(const char* begin, const char* end)
{
	assert(m_valueType == eNull || m_valueType == eString);
	m_valueType = eString;
	m_var = std::string(begin, end);
}

void JsonValue::setString(const std::string& str)
{
	assert(m_valueType == eNull || m_valueType == eString);
	m_valueType = eString;
	m_var = str;
}

// array
bool JsonValue::isArray() const
{
	return m_valueType == eArray;
}

const JsonValue& JsonValue::get(size_t index) const // get copy of array element
{
	assert(m_valueType == eArray);
	assert(m_var.index() == 2);
	assert(index >= 0 && index < std::get<ArrayType>(m_var).size());
	return std::get<ArrayType>(m_var)[index];
}

void JsonValue::resize(size_t newSize) // resize array
{
	assert(m_valueType == eArray && newSize >= 0);
	assert(m_var.index() == 2);
	std::get<ArrayType>(m_var).resize(newSize);
}

void JsonValue::append(const JsonValue& value) // append a new value to array
{
	append(JsonValue(value));
}

void JsonValue::append(JsonValue&& value) // append a rvalue
{
	assert(m_valueType == eNull || m_valueType == eArray);
	if (m_valueType == eNull)
	{
		m_var = ArrayType{};
	}
	m_valueType = eArray;
	assert(m_var.index() == 2);
	std::get<ArrayType>(m_var).emplace_back(std::move(value));
}

bool JsonValue::insert(size_t index, const JsonValue& value) // insert a value to index
{
	return insert(index, JsonValue(value));
}

bool JsonValue::insert(size_t index, JsonValue&& value) // insert a rvalue to index
{
	assert(m_var.index() == 2);
	if (m_valueType != eArray || !(index >= 0 && index <= std::get<ArrayType>(m_var).size()))
		return false;
	std::get<ArrayType>(m_var).emplace(std::get<ArrayType>(m_var).begin() + index, std::move(value));
	return true;
}

bool JsonValue::removeAt(size_t index, JsonValue& removed) // remove value of index, get removed value
{
	assert(m_var.index() == 2);
	if (m_valueType != eArray || !(index >= 0 && index < std::get<ArrayType>(m_var).size()))
		return false;
	removed = std::move(std::get<ArrayType>(m_var)[index]);
	std::get<ArrayType>(m_var).erase(std::get<ArrayType>(m_var).begin() + index);
	return true;
}

bool JsonValue::removeAt(size_t index) // just remove value of index
{
	assert(m_var.index() == 2);
	if (m_valueType != eArray || !(index >= 0 && index < std::get<ArrayType>(m_var).size()))
		return false;
	std::get<ArrayType>(m_var).erase(std::get<ArrayType>(m_var).begin() + index);
	return true;
}

JsonValue& JsonValue::operator[](size_t index) // get reference of array element
{
	assert(m_valueType == eArray);
	assert(m_var.index() == 2);
	assert(index >= 0 && index < std::get<ArrayType>(m_var).size());
	return std::get<ArrayType>(m_var)[index];
}

const JsonValue& JsonValue::operator[](size_t index) const // get const reference of array element
{
	assert(m_valueType == eArray);
	assert(m_var.index() == 2);
	assert(index >= 0 && index < std::get<ArrayType>(m_var).size());
	return std::get<ArrayType>(m_var)[index];
}

// object
bool JsonValue::isObject() const
{
	return m_valueType == eObject;
}

bool JsonValue::containsKey(const std::string& key) const // if these is a key in object
{
	assert(m_valueType == eObject);
	assert(m_var.index() == 3);
	return std::get<ObjectType>(m_var).find(key) != std::get<ObjectType>(m_var).end();
}

std::vector<std::string> JsonValue::getKeys() const // get all keys
{
	assert(m_valueType == eObject);
	assert(m_var.index() == 3);
	std::vector<std::string> keys;
	for (auto iter = std::get<ObjectType>(m_var).begin(); iter != std::get<ObjectType>(m_var).end(); iter++)
		keys.push_back(iter->first);
	return std::move(keys);
}

const JsonValue& JsonValue::get(const std::string& key) const // get const reference of value of key
{
	assert(m_valueType == eObject);
	assert(m_var.index() == 3);
	auto iter = std::get<ObjectType>(m_var).find(key);
	if (iter == std::get<ObjectType>(m_var).end())
		return sNullValue;
	else
		return iter->second;
}

void JsonValue::appendKey(const std::string& key, const JsonValue& value) // append a value to object for key
{
	assert(m_valueType == eObject || m_valueType == eNull);
	if (m_valueType == eNull)
	{
		m_var = ObjectType();
	}
	m_valueType = eObject;
	assert(m_var.index() == 3);
	std::get<ObjectType>(m_var)[key] = value;
}

bool JsonValue::removeKey(const std::string& key, JsonValue& removed) // erase value of key, get removed vlaue
{
	assert(m_var.index() == 3);
	if (m_valueType != eObject || !containsKey(key))
		return false;
	removed = std::move(std::get<ObjectType>(m_var)[key]);
	std::get<ObjectType>(m_var).erase(key);
	return true;
}

bool JsonValue::removeKey(const std::string& key) // just remove value of key
{
	assert(m_var.index() == 3);
	if (m_valueType != eObject || !containsKey(key))
		return false;
	std::get<ObjectType>(m_var).erase(key);
	return true;
}

JsonValue& JsonValue::operator[](const std::string& key) // get reference of value of key
{
	assert(m_valueType == eObject || m_valueType == eNull);
	if (m_valueType == eNull)
	{
		m_var = ObjectType();
	}
	m_valueType = eObject;
	assert(m_var.index() == 3);
	return std::get<ObjectType>(m_var)[key];
}

// array & object in common
size_t JsonValue::size() const // object & array size
{
	assert(m_valueType == eArray || m_valueType == eObject);
	if (m_valueType == eArray)
	{
		assert(m_var.index() == 2);
		return std::get<ArrayType>(m_var).size();
	}
	else
	{
		assert(m_var.index() == 3);
		return std::get<ObjectType>(m_var).size();
	}
}

void JsonValue::clear() // clear all elements
{
	assert(m_valueType == eArray || m_valueType == eObject);
	assert(m_var.index() == 2 || m_var.index() == 3);
	if (m_var.index() == 2)
	{
		std::get<ArrayType>(m_var).clear();
	}
	else
	{
		std::get<ObjectType>(m_var).clear();
	}
}

bool JsonValue::empty() const // is array or object empty
{
	assert(m_valueType == eArray || m_valueType == eObject);
	if (m_valueType == eArray)
	{
		assert(m_var.index() == 2);
		return std::get<ArrayType>(m_var).empty();
	}
	else
	{
		assert(m_var.index() == 3);
		return std::get<ObjectType>(m_var).empty();
	}
}

}