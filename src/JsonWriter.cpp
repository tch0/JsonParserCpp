#include "JsonWriter.h"

namespace MyJson
{

JsonWriter::JsonWriter(bool bStyle /*= true*/, bool bComment /*= false*/) 
{
	m_bWithStyle = bStyle;
	m_bUseSpace = false;
	m_spaceCount = 4;
	m_curIndentLevel = 0;
	m_LFStyle = eCRLF;
	m_bAddUtf8Bom = false;
	m_bAddMode = true;
	m_bComment = bComment;
	m_commentStyle = eCommentBeforeValue;
}

JsonWriter::~JsonWriter() {}

void JsonWriter::setWithStyle(bool bStyle)
{
	m_bWithStyle = bStyle;
}

void JsonWriter::setUseTabToIndent()
{
	m_bUseSpace = false;
}

void JsonWriter::setUseSpacesToIndent()
{
	m_bUseSpace = true;
}

void JsonWriter::setIndentSpaceCount(int count)
{
	m_spaceCount = (count > 8 || count < 2) ? 4 : count;
}

void JsonWriter::setLFStyle(LFStyle style)
{
	m_LFStyle = (style < eCR || style > eCRLF) ? eCRLF : style;
}

void JsonWriter::setAddUtf8Bom(bool bBom)
{
	m_bAddUtf8Bom = bBom;
}

void JsonWriter::setAddToString()
{
	m_bAddMode = true;
}

void JsonWriter::setRewriteString()
{
	m_bAddMode = false;
}

void JsonWriter::setWriteComment()
{
	m_bComment = true;
}

void JsonWriter::setCommentStyle(WriterCommentStyle style)
{
	m_commentStyle = (style < eCommentBeforeValue || style > eCommentAfterOnSameLine) ? eCommentBeforeValue : style;
}

void JsonWriter::writeJson(const JsonValue& value, std::string& json)
{
	if (!m_bAddMode) // ReWrite
	{
		json.clear();
	}
	if (m_bAddUtf8Bom)
	{
		std::string utf8Bom;
		JsonParser::encodeUtf8(0xFEFF, utf8Bom);
		json += utf8Bom; // UTF-8 BOM, 0xEFBBBF
	}
	writeJsonRaw(value, json);
}

void JsonWriter::writeJsonRaw(const JsonValue& value, std::string& json)
{
	switch (value.type())
	{
	case eNull:
		json += "null";
		break;
	case eTrue:
		json += "true";
		break;
	case eFalse:
		json += "false";
		break;
	case eNumber:
		json += std::to_string(value.getNumber()); // avoid precision lost
		break;
	case eString:
		{
			using namespace std::string_literals;
			static const std::string hex = "0123456789ABCDEF"s;
			json += "\"";
			const std::string& str = value.getString();
			for (int i = 0; i < str.length(); i++)
			{
				unsigned char ch = str[i];
				switch (ch)
				{
				case '\"': json += "\\\""; break;
				case '\\': json += "\\\\"; break;
				case '\b': json += "\\b"; break;
				case '\f': json += "\\f"; break;
				case '\n': json += "\\n"; break;
				case '\r': json += "\\r"; break;
				case '\t': json += "\\t"; break;
				default:
					if (ch < 0x20)
					{
						json += "\\u00";
						json += hex[ch >> 4];
						json += hex[ch & 0x0F];
					}
					else
						json += ch;
				}
			}
			json += "\"";
		}
		break;
	case eArray:
		if (value.empty())
		{
			json += "[]";
			break;
		}
		json += "[";
		writeLineFeed(json);
		for (int i = 0; i < value.size(); i++)
		{
			if (i == 0)
				writeIndentationBegin(json);
			else
				writeIndentation(json);
			writeJsonRaw(value[i], json);
			if (i < value.size() - 1)
			{
				json += ",";
			}
			writeLineFeed(json);
		}
		writeIndentationEnd(json);
		json += "]";
		break;
	case eObject:
		if (value.empty())
		{
			json += "{}";
			break;
		}
		json += "{";
		writeLineFeed(json);
		std::vector<std::string> keys = value.getKeys();
		for (int i = 0; i < keys.size(); i++)
		{
			if (i == 0)
				writeIndentationBegin(json);
			else
				writeIndentation(json);
			json += ("\"" + keys[i] + "\"");
			json += ": ";
			writeJsonRaw(value.get(keys[i]), json);
			if (i < keys.size() - 1)
			{
				json += ",";
			}
			writeLineFeed(json);
		}
		writeIndentationEnd(json);
		json += "}";
		break;
	}
}

void JsonWriter::writeOneIndentation(std::string& json)
{
	if (m_bUseSpace)
	{
		json += std::string(m_spaceCount, ' ');
	}
	else
	{
		json += "\t";
	}
}

void JsonWriter::writeIndentation(std::string& json)
{
	if (m_bWithStyle)
	{
		for (int i = 0; i < m_curIndentLevel; i++)
		{
			writeOneIndentation(json);
		}
	}
}

void JsonWriter::writeIndentationBegin(std::string& json)
{
	m_curIndentLevel++;
	writeIndentation(json);
}

void JsonWriter::writeIndentationEnd(std::string& json)
{
	m_curIndentLevel--;
	writeIndentation(json);
}

void JsonWriter::writeSpace(std::string& json)
{
	if (m_bWithStyle)
	{
		json += " ";
	}
}

void JsonWriter::writeLineFeed(std::string& json)
{
	if (m_bWithStyle)
	{
		switch (m_LFStyle)
		{
		case eLF:   json += "\n";   break;
		case eCR:   json += "\r";   break;
		case eCRLF: json += "\r\n"; break;
		}
	}
}

void JsonWriter::writeComment(const JsonValue& value, std::string& json)
{
	// TODO, after implementation of parsing comment
}

}