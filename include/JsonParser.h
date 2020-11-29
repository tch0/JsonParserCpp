#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "JsonValue.h"

namespace MyJson
{

enum ParseResult
{
	eOk = 0,
	eExpectValue,					// 只含空白
	eInvalidValue,					// 无效值
	eRootNotSingular,				// 一个有效值后未结束
	eNumberTooBig,					// 浮点溢出
	eMissQuatationMark,				// 字符串缺失引号
	eInvalidStringEscape,			// 无效字符转义
	eInvalidStringChar,				// 无效字符, 0x00 ~ 0x1F
	eInvalidUniCodeHex,				// 无效UniCode转义序列
	eInvalidUniCodeSurrogate,		// 无效UniCode代理项
	eArrayMissCommaOrSquareBracket,	// 无效数组，缺失逗号或者方括号
	eObjectMissKey,					// object缺失key
	eObjectMissColon,				// object缺失冒号
	eObjectMissCommaOrCurlyBracket	// object缺失逗号或大括号
};

enum LFStyle
{
	eCR = 0,	// \r to change line
	eLF,		// \n to change line
	eCRLF,		// \r\n to change line, by default when writing.
	eCRLFAll	// \r\n or seperate \r and \n to chagne line, by default when parsing.
};

class JsonParser
{
public:
	JsonParser();
	~JsonParser();

	int parseJson(JsonValue& value, const std::string& json);
	int parseJson(JsonValue& value, const std::string& json, std::string & errLoc);
	int errorLine();
	int errorColumn();
	void setParseLFStyle(LFStyle style);

	static void encodeUtf8(unsigned int u, std::string& parseStr);

private:
	void parseWhiteSpace();
	int parseLiteral(JsonValue& value, const char* literal, ValueType vType);
	int parseNumber(JsonValue& value);
	bool parseHex4(const char* p, unsigned int & u);
	int parseString(JsonValue& value);
	int parseArray(JsonValue& value);
	int parseObject(JsonValue& value);
	int parseValue(JsonValue& value);

	const char* m_pJson;
	const char* m_pCurLineHead;
	int m_curLine;
	int m_curColumn;
	LFStyle m_CRLFStyle;
};

}
#endif
