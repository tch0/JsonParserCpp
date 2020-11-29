#ifndef _JSON_WRITER_H_
#define _JSON_WRITER_H_

#include "JsonValue.h"
#include "JsonParser.h"

namespace MyJson
{

enum WriterCommentStyle
{
	eCommentBeforeValue, // by default
	eCommentAfterValue,
	eCommentAfterOnSameLine
};

class JsonWriter
{
public:
	JsonWriter(bool bStyle = true, bool bComment = false);
	~JsonWriter();
	
	void setWithStyle(bool bStyle);
	void setUseTabToIndent(); // use Tab by deafult
	void setUseSpacesToIndent();
	void setIndentSpaceCount(int count); // 4 by default
	void setLFStyle(LFStyle style); // CRLF by default
	void setAddUtf8Bom(bool bBom); // do not add Utf-8 BOM by default
	void setAddToString(); // add to string, default
	void setRewriteString(); // rewrite input string
	void setWriteComment(); // do not write comment by default, not implement yet
	void setCommentStyle(WriterCommentStyle style); // before by default

	void writeJson(const JsonValue& value, std::string& json);

private:
	void writeJsonRaw(const JsonValue& value, std::string& json);
	void writeOneIndentation(std::string& json);
	void writeIndentation(std::string& json);
	void writeIndentationBegin(std::string& json);
	void writeIndentationEnd(std::string& json);
	void writeSpace(std::string& json);
	void writeLineFeed(std::string& json);
	void writeComment(const JsonValue& value, std::string& json);

	bool m_bWithStyle;
	bool m_bUseSpace;
	int m_spaceCount;
	LFStyle m_LFStyle;
	bool m_bAddUtf8Bom;
	bool m_bAddMode; // true -> Add, false -> rewrite
	int m_curIndentLevel;
	bool m_bComment;
	WriterCommentStyle m_commentStyle;
};

}
#endif
