#include <assert.h>	// for assert
#include <stdlib.h>
#include <string.h>
#include <ctype.h>	// for isdigit
#include <errno.h>	// for errno, a macro that define a lvalue
#include <math.h>	// for HUGE_VAL in C
#include "JsonParser.h"

namespace MyJson
{

using namespace std::string_literals;

JsonParser::JsonParser() :
	m_pJson(NULL),
	m_pCurLineHead(NULL),
	m_curLine(0), 
	m_curColumn(0),
	m_CRLFStyle(eCRLFAll) {}

JsonParser::~JsonParser() {}

// white space
/*
ws = *(%x20 / %x09 / %x0A / %x0D)
' ' / '\t' / '\r' / '\n'
*/
void JsonParser::parseWhiteSpace()
{
	const char* p = m_pJson;
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	{
		if (m_CRLFStyle == eCR) // \r
		{
			if (*p == '\r')
			{
				p++;
				m_curLine++;
				m_pCurLineHead = p;
				continue;
			}
		}
		else if (m_CRLFStyle == eLF) // \n
		{
			if (*p == '\n')
			{
				p++;
				m_curLine++;
				m_pCurLineHead = p;
				continue;
			}
		}
		else if (m_CRLFStyle == eCRLF) // \r\n
		{
			if (*p == '\r' && *(p+1) == '\n')
			{
				p += 2;
				m_curLine++;
				m_pCurLineHead = p;
				continue;
			}
		}
		else if (m_CRLFStyle == eCRLFAll) // \r\n \r \n
		{
			if (*p == '\r')
			{
				p++;
				m_curLine++;
				if (*p == '\n')
					p++;
				m_pCurLineHead = p;
				continue;
			}
			else if (*p == '\n')
			{
				p++;
				m_curLine++;
				m_pCurLineHead = p;
				continue;
			}
		}
		p++;
	}
	m_pJson = p;
}

// string literal: null / true / false
/*
need setType outside parseLiteral
literal should not be "" or NULL
*/
int JsonParser::parseLiteral(JsonValue& value, const char* literal, ValueType vType)
{
	assert(*m_pJson == literal[0]);
	m_pJson++;
	literal++;
	int i = 0;
	for (i = 0; literal[i]; i++) // literal[i] != '\0'
	{
		if (m_pJson[i] != literal[i])
			return eInvalidValue;
	}
	m_pJson += i;
	value.setType(vType);
	return eOk;
}

// number
/*
number = ["-"] int [frac] [exp]
int = "0" / digit1-9*digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit

注意：
四部分组成：负号、整数、小数、指数
整数部分正号不合法
只有整数部分是必需的
整数部分0开始的话，只能是单个0
由1~9开始

需要考虑如何存储解析后结果 ---------> 简单起见，采用double，需要更精确的话应该分整型和浮点来存储
需要考虑分整型和浮点存储并且格式校验的话：number解析是整个parser中最难的部分。
指数部分未禁止前导0，整数部分禁止前导0的原因是更久的JavaScript版本允许整数前导0来表示八进制，这种表示方式来自C语言。

实现选择：
1.naive解析，可能会有精度问题，参考https://github.com/google/double-conversion，可以考虑实现高精度大整数
2.使用标准库strtod做转换 + 自己做校验。

数字判断：isdigit
*/
int JsonParser::parseNumber(JsonValue& value)
{
	value.setType(eNull);
	// validate number
	const char* p = m_pJson;
	// -
	if (*p == '-')
		p++;
	// int = 0 / digit1-9 *digit
	if (*p == '0')
		p++;
	else
	{
		if (!isdigit(*p)) // 1 - 9
			return eInvalidValue;
		for (p++; isdigit(*p); p++);
	}
	// frac = .1*digit
	if (*p == '.')
	{
		p++;
		if (!isdigit(*p)) // at least one digit
			return eInvalidValue;
		for (p++; isdigit(*p); p++);
	}
	// exp = e/E [-/+] 1*digit
	if (*p == 'e' || *p == 'E')
	{
		p++;
		if (*p == '+' || *p == '-')
			p++;
		if (!isdigit(*p))
			return eInvalidValue;
		for (p++; isdigit(*p); p++);
	}

	// strtod做转换，考虑溢出时也有正负号
	errno = 0;
	double ret = strtod(m_pJson, NULL);
	if (errno == ERANGE && (ret == HUGE_VAL || ret == -HUGE_VAL))
	{
		return eNumberTooBig;
	}
	m_pJson = p;
	value.setType(eNumber);
	value.setNumber(ret);
	return eOk;
}

bool JsonParser::parseHex4(const char* p, unsigned int & u)
{
	u = 0;
	for (int i = 0; i < 4; i++)
	{
		char ch = *p++;
		u <<= 4;
		if (isdigit(ch))
			u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			u |= ch - 'A' + 10;
		else if (ch >= 'a' && ch <= 'f')
			u |= ch - 'a' + 10;
		else
			return false;
	}
	return true;
}

void JsonParser::encodeUtf8(unsigned int u, std::string& parseStr)
{
	if (u <= 0x7F)
	{
		parseStr += (char)(u & 0xFF);
	}
	else if (u <= 0x7FF)
	{
		parseStr += (char)(0xC0 | ((u >> 6) & 0xFF));
		parseStr += (char)(0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF)
	{
		parseStr += (char)(0xE0 | ((u >> 12) & 0xFF));
		parseStr += (char)(0x80 | ((u >> 6) & 0x3F));
		parseStr += (char)(0x80 | (u & 0x3F));
	}
	else
	{
		assert(u <= 0x10FFFF);
		parseStr += (char)(0xF0 | ((u >> 18) & 0xFF));
		parseStr += (char)(0x80 | ((u >> 12) & 0x3F));
		parseStr += (char)(0x80 | ((u >> 6) & 0x3F));
		parseStr += (char)(0x80 | (u & 0x3F));
	}
}

// string
/*
string = quotation-mark *char quotation-mark
char = unescaped /
   escape (
	   %x22 /          ; "    quotation mark  U+0022
	   %x5C /          ; \    reverse solidus U+005C
	   %x2F /          ; /    solidus         U+002F
	   %x62 /          ; b    backspace       U+0008
	   %x66 /          ; f    form feed       U+000C
	   %x6E /          ; n    line feed       U+000A
	   %x72 /          ; r    carriage return U+000D
	   %x74 /          ; t    tab             U+0009
	   %x75 4HEXDIG )  ; uXXXX                U+XXXX
escape = %x5C          ; \
quotation-mark = %x22  ; "
unescaped = %x20-21 / %x23-5B / %x5D-10FFFF

转义字符：\" \\ \/ \b \f \n \r \t
转义序列：\uXXXX	UTF-16编码	其中可能包含正确的\0字符

UniCode字符集:
UniCode码点范围: U+000000 ~ U+10FFFF
基本多文种平面: U+0000 ~ U+FFFF 用两个字节即可表示, 或者叫第零平面，共2^16个码点
其中保留了2048个代理码点: 1024个高代理码点U+D800 ~ U+DBFF，1024个低代理码点U+DC00 ~ U+DFFF
然后用一个高代理码点和一个低代理码点组成代理对 (H, L)
计算得到真实码点:
	CodePoint = 0x10000 + (H - 0xD800) * 0x400 + (L - 0xDC00)
代理对表示范围: 0x010000 ~ 0x10FFFF 共 2^20个码点, 16个平面

UTF-8编码: 编码单元8位1个字节，每个码点编码成1到4个字节，按照码点范围，将码点二进制分拆成1到多个字节即可实现
码点对应编码:
码点范围			码点位数		字节1		字节2		字节3		字节4
U+0000 ~ U+007F		7		0xxxxxxx										(与ASCII兼容)
U+0080 ~ U+07FF		11		110xxxxx	10xxxxxx
U+0800 ~ U+FFFF		16		1110xxxx	10xxxxxx	10xxxxxx
U+10000 ~ U+10FFFF	21		11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
*/
int JsonParser::parseString(JsonValue& value)
{
	assert(*m_pJson == '\"');
	m_pJson++;
	std::string parseStr;
	value.setType(eNull);
	const char* p = m_pJson; // point the character after "
	while (true)
	{
		char ch = *p++;
		switch (ch)
		{
		case '\"': // end
			value.setString(parseStr);
			m_pJson = p;
			return eOk;
		case '\\': // escape characters
			ch = *p++;
			switch (ch)
			{
			case '\"':	parseStr += '\"'; break;
			case '\\':	parseStr += '\\'; break;
			case '/':	parseStr += '/';  break;
			case 'b':	parseStr += '\b'; break;
			case 'f':	parseStr += '\f'; break;
			case 'n':	parseStr += '\n'; break;
			case 'r':	parseStr += '\r'; break;
			case 't':	parseStr += '\t'; break;
			case 'u': // unicode escapes
				{
					unsigned int u = 0, u2 = 0;
					if (!parseHex4(p, u))
					{
						return eInvalidUniCodeHex;
					}
					p += 4;
					if (u >= 0xD800 && u <= 0xDBFF) // surrogate pair
					{
						if (*p == '\\' && *(p + 1) == 'u' && parseHex4(p + 2, u2) && u2 >= 0xDC00 && u2 <= 0xDFFF)
						{
							p += 6;
							u = 0x10000 + ((u - 0xD800) << 10) + (u2 - 0xDC00);
						}
						else
						{
							return eInvalidUniCodeSurrogate;
						}
					}
					encodeUtf8(u, parseStr);
					break;
				}
			default:
				return eInvalidStringEscape;
			}
			break;
		case '\0':
			return eMissQuatationMark;
		default:
			if ((unsigned char)ch < 0x20)
			{
				return eInvalidStringChar;
			}
			parseStr += ch;
			break;
		}
	}
}

// array
/*
array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
%x5B = [
%x2C = ,
%x5D = ]

不接受最后一个元素后的额外逗号
*/
int JsonParser::parseArray(JsonValue& value)
{
	assert(*m_pJson == '[');
	m_pJson++;
	value.setType(eNull);
	parseWhiteSpace();
	if (*m_pJson == ']') // empty array
	{
		m_pJson++;
		value.setType(eArray);
		return eOk;
	}
	while (true)
	{
		parseWhiteSpace();
		JsonValue tmpValue;
		int ret = parseValue(tmpValue);
		if (ret != eOk)
		{
			value.setNull();
			return ret;
		}
		parseWhiteSpace();

		value.append(tmpValue);
		if (*m_pJson == ']')
		{
			m_pJson++;
			return eOk;
		}
		else if (*m_pJson == ',')
		{
			m_pJson++;
		}
		else
		{
			m_pJson++;
			value.setNull();
			return eArrayMissCommaOrSquareBracket;
		}
	}
}

// object
/*
member = string ws %x3A ws value
object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D

%x3A = :
%x7B = {
%x7D = }
%x2C = ,
*/
int JsonParser::parseObject(JsonValue& value)
{
	assert(*m_pJson == '{');
	m_pJson++;
	value.setType(eNull);
	parseWhiteSpace();
	if (*m_pJson == '}')
	{
		m_pJson++;
		value.setType(eObject);
		return eOk;
	}

	int ret = eOk;
	while (true)
	{
		parseWhiteSpace();
		JsonValue tmpValue, keyValue;
		if (*m_pJson != '\"')
		{
			ret = eObjectMissKey;
			break;
		}
		ret = parseString(keyValue);
		if (ret != eOk || !keyValue.isString())
		{
			break;
		}

		parseWhiteSpace();
		if (*m_pJson != ':')
		{
			ret = eObjectMissColon;
			break;
		}
		m_pJson++;

		parseWhiteSpace();
		ret = parseValue(tmpValue);
		if (ret != eOk)
		{
			break;
		}
		value.appendKey(keyValue.getString(), tmpValue);
		parseWhiteSpace();
		if (*m_pJson == '}') // success
		{
			m_pJson++;
			return eOk;
		}
		else if (*m_pJson == ',')
		{
			m_pJson++;
		}
		else
		{
			ret = eObjectMissCommaOrCurlyBracket;
			break;
		}
	}
	value.setNull();
	return ret;
}

// value
/*
value = null / false / true / number / array / object
*/
int JsonParser::parseValue(JsonValue& value)
{
	switch (*m_pJson)
	{
	case 'n':	return parseLiteral(value, "null", eNull);
	case 't':	return parseLiteral(value, "true", eTrue);
	case 'f':	return parseLiteral(value, "false", eFalse);
	case '\"':	return parseString(value);
	case '\0':	return eExpectValue;
	case '[':	return parseArray(value);
	case '{':	return parseObject(value);
	default:	return parseNumber(value);
	}
}


int JsonParser::parseJson(JsonValue& value, const std::string& json)
{
	m_curLine = 1;
	m_curColumn = 1;
	m_pJson = json.c_str();
	value.setNull();
	parseWhiteSpace();
	int ret = parseValue(value);
	if (ret == eOk)
	{
		parseWhiteSpace();
		if (*m_pJson != '\0')
		{
			value.setNull();
			ret = eRootNotSingular;
		}
	}
	m_pJson = NULL;
	return ret;
}

int JsonParser::parseJson(JsonValue& value, const std::string& json, std::string& errInfo)
{
	m_curLine = 1;
	m_curColumn = 1;
	m_pJson = json.c_str();
	m_pCurLineHead = m_pJson;
	value.setNull();
	parseWhiteSpace();
	ParseResult ret = (ParseResult)parseValue(value);
	if (ret == eOk)
	{
		parseWhiteSpace();
		if (*m_pJson != '\0')
		{
			value.setNull();
			ret = eRootNotSingular;
		}
	}

	std::string errorStr;
	switch (ret)
	{
	case eExpectValue:
		errorStr = "No value";
		break;
	case eInvalidValue:
		errorStr = "Invalid value";
		break;
	case eRootNotSingular:
		errorStr = "Not noly a value";
		break;
	case eNumberTooBig:
		errorStr = "Number is too big";
		break;
	case eMissQuatationMark:
		errorStr = "Miss qutation mark(\")";
		break;
	case eInvalidStringEscape:
		errorStr = "Invalid string escape";
		break;
	case eInvalidStringChar:
		errorStr = "Invalid char of string";
		break;
	case eInvalidUniCodeHex:
		errorStr = "Invalid unicode escape sequence";
		break;
	case eInvalidUniCodeSurrogate:
		errorStr = "Invalid unicode surrogate pair";
		break;
	case eArrayMissCommaOrSquareBracket:
		errorStr = "Array miss comma(,) or square bracket([)";
		break;
	case eObjectMissKey:
		errorStr = "Object miss key";
		break;
	case eObjectMissColon:
		errorStr = "Object miss colon(;)";
		break;
	case eObjectMissCommaOrCurlyBracket:
		errorStr = "Object miss comma(,) or curly bracket(})";
		break;
	default:
		break;
	}
	
	if (ret != eOk)
	{
		m_curColumn = int(m_pJson - m_pCurLineHead + 1);
		errInfo += "line "s + std::to_string(m_curLine);
		errInfo += ", column "s + std::to_string(m_curColumn);
		errInfo += ": "s + errorStr + ".";
	}
	else
	{
		errInfo.clear();
		m_curLine = 0;
		m_curColumn = 0;
	}

	m_pJson = NULL;
	return ret;
}

int JsonParser::errorLine()
{
	return m_curLine;
}

int JsonParser::errorColumn()
{
	return m_curColumn;
}

void JsonParser::setParseLFStyle(LFStyle style)
{
	m_CRLFStyle = (style < eCR && style > eCRLFAll) ? eCRLFAll : style;
}

}