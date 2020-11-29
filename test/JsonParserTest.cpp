#include <iostream>
#include <fstream>
#include <string>
#include "JsonParser.h"
#include "JsonWriter.h"

using namespace std::string_literals;
using namespace MyJson;

// ========================================= A Simple Test Framework ==================================================================
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

// test failed, print: file, line, expect, actual
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		test_count ++;\
		if (equality)\
			test_pass++;\
		else {\
			fprintf(stderr, "%s:%d : expect: " format " actual: " format"\n", __FILE__, __LINE__, expect, actual);\
			main_ret = 1;\
		}\
	} while(0)


#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), expect, actual, "%lf")
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), expect.c_str(), actual.c_str(), "%s")
#define EXPECT_EQ_BOOL(expect, actual) EXPECT_EQ_INT(expect, actual)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE(((expect) == (actual)), (size_t)expect, (size_t)actual, "%zu")

// test prase result and value type
#define TEST_RET_TYPE(json, ret, t)\
	do{\
		JsonValue value;\
		JsonParser parser;\
		EXPECT_EQ_INT(ret, parser.parseJson(value, json));\
		EXPECT_EQ_INT(t, value.type());\
	} while(0)

#define TEST_ERROR(ret, json) TEST_RET_TYPE(json, ret, eNull)

#define TEST_NUMBER(val, json)\
	do {\
		JsonValue value; \
		JsonParser parser;\
		EXPECT_EQ_INT(eOk, parser.parseJson(value, json)); \
		EXPECT_EQ_INT(eNumber, value.type()); \
		EXPECT_EQ_DOUBLE(val, value.getNumber());\
} while (0)

#define TEST_STRING(val, json)\
	do{\
		JsonValue value;\
		JsonParser parser;\
		EXPECT_EQ_INT(eOk, parser.parseJson(value, json));\
		EXPECT_EQ_INT(eString, value.type());\
		EXPECT_EQ_STRING(val, value.getString());\
	} while (0)

// ========================================= A Simple Test Framework ==================================================================

// parse
static void test_parse_null()
{
	TEST_RET_TYPE("null", eOk, eNull);
}

static void test_parse_true()
{
	TEST_RET_TYPE("false", eOk, eFalse);
}

static void test_parse_false()
{
	TEST_RET_TYPE("true", eOk, eTrue);
}

static void test_parse_number()
{
	TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
	TEST_NUMBER(1E010, "1E010");
	// corner case, strtod can handle all of this, all we need to do is to validate number.
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
	TEST_NUMBER(1e-309, "1e-309"); // too small
	TEST_NUMBER(1e-1000, "1e-1000"); // too small to 0
}

static void test_parse_string()
{
	TEST_STRING(""s, "\"\"");	// ""
	TEST_STRING("hello,world!"s, "\"hello,world!\""); // "hello,world!"
	TEST_STRING("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM`~!@#$%^&*()_+-=[]{};:',<.>/?"s, "\"qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM`~!@#$%^&*()_+-=[]{};:',<.>/?\""); // all usual characters
	TEST_STRING("hello\nworld"s, "\"hello\\nworld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t"s, "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\""); // escape characters
	// unicode escapes
	TEST_STRING("hello\0world"s, "\"hello\\u0000world\"");
	TEST_STRING("\x24"s, "\"\\u0024\"");		// Dollar Sign: $
	TEST_STRING("\xC2\xA2"s, "\"\\u00A2\"");	// Cent Sign: U+00A2
	TEST_STRING("\xE2\x82\xAC"s, "\"\\u20AC\"");	// Euro Sign: U+20AC
	// unicode surrogates
	TEST_STRING("\xF0\x9D\x84\x9E"s, "\"\\uD834\\uDD1E\"");  // G clef sign U+1D11E
	TEST_STRING("\xF0\x9D\x84\x9E"s, "\"\\ud834\\udd1e\"");  // G clef sign U+1D11E
}

static void test_parse_array()
{
	// test return value and type
	TEST_RET_TYPE("[]", eOk, eArray);
	TEST_RET_TYPE("[100]", eOk, eArray);
	TEST_RET_TYPE("[1.0e-100, null, true, false, [1.59E100, \"hello\"], \"world\"]", eOk, eArray);
	TEST_RET_TYPE("[[], [[], [], [ [ [ [] ] ] ] ]", eOk, eArray);

	// test the element of array
	JsonValue value;
	JsonParser parser;
	int result = parser.parseJson(value, "[ 1.0e-100 , null , true, false, [1.59E100,\"hello\"], \"world\"]");
	EXPECT_EQ_INT(eOk, result);
	EXPECT_EQ_SIZE_T(value.size(), 6);
	EXPECT_EQ_DOUBLE(1.0e-100, value[0].getNumber());
	EXPECT_EQ_INT(eNull, value[1].type());
	EXPECT_EQ_BOOL(true, value[2].getBool());
	EXPECT_EQ_BOOL(false, value[3].getBool());
	EXPECT_EQ_INT(eArray, value[4].type());
	EXPECT_EQ_SIZE_T(2, value[4].size());
	EXPECT_EQ_DOUBLE(1.59e100, value[4][0].getNumber());
	EXPECT_EQ_STRING("hello"s, value[4][1].getString());
	EXPECT_EQ_STRING("world"s, value[5].getString());
}

static void test_parse_object()
{
	// the return value and type
	TEST_RET_TYPE("{}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":1}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":[]}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":null}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":true}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":false}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":[]}", eOk, eObject);
	TEST_RET_TYPE("{\"a\":[{}, [], true], \"b\" : \"value\"}", eOk, eObject);

	// test the element of object
	JsonValue value;
	JsonParser parser;
	int ret = parser.parseJson(value, "{\"a\" : null, \"b\": false, \"c\" : {\"d\" : []}}");
	EXPECT_EQ_INT(eOk, ret);
	EXPECT_EQ_INT(eObject, value.type());
	EXPECT_EQ_SIZE_T(3, value.size());
	EXPECT_EQ_BOOL(true, value["a"s].isNull());
	EXPECT_EQ_INT(eFalse, value.get("b"s).type());
	EXPECT_EQ_INT(eObject, value["c"s].type());
	EXPECT_EQ_INT(eArray, value["c"s]["d"s].type());
	EXPECT_EQ_BOOL(true, value["c"s]["d"s].empty());
}


// parse error
static void test_parse_expect_value()
{
	TEST_ERROR(eExpectValue, " ");
	TEST_ERROR(eExpectValue, "");
	TEST_ERROR(eExpectValue, "    ");
	TEST_ERROR(eExpectValue, "\n \r \t ");
}

static void test_parse_invalid_value()
{
	TEST_ERROR(eInvalidValue, "nul");
	TEST_ERROR(eInvalidValue, "?");

	// number
	TEST_ERROR(eInvalidValue, "+0");
	TEST_ERROR(eInvalidValue, "+1");
	TEST_ERROR(eInvalidValue, ".123");
	TEST_ERROR(eInvalidValue, "1.");
	TEST_ERROR(eInvalidValue, "1.e9");
	TEST_ERROR(eInvalidValue, ".1e9");
	TEST_ERROR(eInvalidValue, "INF");
	TEST_ERROR(eInvalidValue, "inf");
	TEST_ERROR(eInvalidValue, "NAN");
	TEST_ERROR(eInvalidValue, "nan");

	// array
	TEST_ERROR(eInvalidValue, "[1, ]");
	TEST_ERROR(eInvalidValue, "[,");
	TEST_ERROR(eInvalidValue, "[,]");

	// object
	TEST_ERROR(eInvalidValue, "{\"hello\" : nice }");
	TEST_ERROR(eInvalidValue, "{\"100\" : [,}");
}

static void test_parse_root_not_singular()
{
	TEST_ERROR(eRootNotSingular, "null sad");
	TEST_ERROR(eRootNotSingular, "true asdf");
	TEST_ERROR(eRootNotSingular, "-1.123e100t");
	
	// number
	TEST_ERROR(eRootNotSingular, "0123");	// after 0, should be '.'/'e'/'E' or nothing
	TEST_ERROR(eRootNotSingular, "0x0");
	TEST_ERROR(eRootNotSingular, "0x1234");
}

static void test_parse_number_too_big()
{
	// number
	TEST_ERROR(eNumberTooBig, "1e309"); // too big
	TEST_ERROR(eNumberTooBig, "-1e309");
	TEST_ERROR(eNumberTooBig, "1e1000");
}

static void test_parse_miss_quotation_mark()
{
	// string
	TEST_ERROR(eMissQuatationMark, "\"");
	TEST_ERROR(eMissQuatationMark, "\"hello");
}

static void test_parse_invalid_string_escape()
{
	// string
	TEST_ERROR(eInvalidStringEscape, "\"\\v\""); // "\v"
	TEST_ERROR(eInvalidStringEscape, "\"\\'\""); // "\'"
	TEST_ERROR(eInvalidStringEscape, "\"\\0\""); // "\0"
	TEST_ERROR(eInvalidStringEscape, "\"\\x12\""); // "\x13"
}

static void test_parse_invalid_string_char()
{
	// string
	// invalid char in Json: 0x00 ~ 0x1F
	TEST_ERROR(eInvalidStringChar, "\"\x01\"");
	TEST_ERROR(eInvalidStringChar, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex()
{
	// string
	TEST_ERROR(eInvalidUniCodeHex, "\"\\u\"");
	TEST_ERROR(eInvalidUniCodeHex, "\"\\u0\"");
	TEST_ERROR(eInvalidUniCodeHex, "\"\\u01\"");
	TEST_ERROR(eInvalidUniCodeHex, "\"\\u$012\"");
	TEST_ERROR(eInvalidUniCodeHex, "\"\\u5FFj\"");
	TEST_ERROR(eInvalidUniCodeHex, "\"hello\\u7Fcworld\"");
}

static void test_parse_invalid_unicode_surrogate()
{
	// string
	TEST_ERROR(eInvalidUniCodeSurrogate, "\"\\uD800\"");
	TEST_ERROR(eInvalidUniCodeSurrogate, "\"\\uDBFF\"");
	TEST_ERROR(eInvalidUniCodeSurrogate, "\"\\uD800\\\\\"");
	TEST_ERROR(eInvalidUniCodeSurrogate, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(eInvalidUniCodeSurrogate, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket()
{
	TEST_ERROR(eArrayMissCommaOrSquareBracket, "[100 123]");
	TEST_ERROR(eArrayMissCommaOrSquareBracket, "[100, 123");
	TEST_ERROR(eArrayMissCommaOrSquareBracket, "[100}");
	TEST_ERROR(eArrayMissCommaOrSquareBracket, "[[]");
}

static void test_parse_object_miss_key()
{
	TEST_ERROR(eObjectMissKey, "{null : 0}");
	TEST_ERROR(eObjectMissKey, "{true : 0}");
	TEST_ERROR(eObjectMissKey, "{false : 0}");
	TEST_ERROR(eObjectMissKey, "{1.0 : 0}");
	TEST_ERROR(eObjectMissKey, "{[] : 0}");
	TEST_ERROR(eObjectMissKey, "{{} : 0}");
	TEST_ERROR(eObjectMissKey, "{\"a\" : 1, }");
	TEST_ERROR(eObjectMissKey, "{\"hello\" : 100, 1.0 : 20}");
}

static void test_parse_object_miss_colon()
{
	TEST_ERROR(eObjectMissColon, "{\"adsf\"}");
	TEST_ERROR(eObjectMissColon, "{\"adsf\" : true, \"test\" 200}");
}

static void test_parse_object_miss_comma_or_curly_bracket()
{
	TEST_ERROR(eObjectMissCommaOrCurlyBracket, "{\"a\" : 1");
	TEST_ERROR(eObjectMissCommaOrCurlyBracket, "{\"a\" : 1)");
	TEST_ERROR(eObjectMissCommaOrCurlyBracket, "{\"a\" : []");
	TEST_ERROR(eObjectMissCommaOrCurlyBracket, "{\"a\" : {}");
}

// access
static void test_access_null()
{
	JsonValue value;
	EXPECT_EQ_INT(value.type(), eNull);
	EXPECT_EQ_BOOL(value.isNull(), true);
	EXPECT_EQ_BOOL(value.isNull(), true);
}

static void test_access_boolean()
{
	JsonValue value;
	value.setBool(true);
	EXPECT_EQ_INT(value.type(), eTrue);
	EXPECT_EQ_BOOL(value.getBool(), true);
	EXPECT_EQ_BOOL(value.isBool(), true);
	EXPECT_EQ_BOOL(value.isTrue(), true);
	value.setBool(false);
	EXPECT_EQ_INT(value.type(), eFalse);
	EXPECT_EQ_BOOL(value.getBool(), false);
	EXPECT_EQ_BOOL(value.isBool(), true);
	EXPECT_EQ_BOOL(value.isFalse(), true);
}

static void test_access_number()
{
	JsonValue value;
	value.setNumber(1e-100);
	EXPECT_EQ_INT(value.type(), eNumber);
	EXPECT_EQ_DOUBLE(value.getNumber(), 1e-100);
	EXPECT_EQ_BOOL(value.isNumber(), true);
}

static void test_access_string()
{
	
	JsonValue value;
	value.setString("", size_t(0));
	EXPECT_EQ_STRING(value.getString(), ""s);
	value.setString("\0\0hello", 7);
	EXPECT_EQ_STRING(value.getString(), "\0\0hello"s);
}

static void test_access_array()
{
	JsonValue value;
	JsonValue tmpValue;
	tmpValue.setNumber(100);
	value.append(tmpValue);
	tmpValue.setNull();
	tmpValue.setString("hello");
	value.append(tmpValue);
	tmpValue.setNull();
	tmpValue.append(JsonValue::sNullValue);
	value.append(tmpValue);
	tmpValue.setNull();
	tmpValue.setBool(true);
	value[2][0] = tmpValue;
	EXPECT_EQ_BOOL(true, value.isArray());
	EXPECT_EQ_SIZE_T(3, value.size());
	EXPECT_EQ_DOUBLE(100.0, value[0].getNumber());
	EXPECT_EQ_STRING("hello"s, value[1].getString());
	EXPECT_EQ_BOOL(true, value[2].get(0).getBool());
}

static void test_access_object()
{
	JsonValue value;
	value.appendKey("a", 1.0);
	value["b"] = 2.0;
	value["c"] = "nice"s;
	value["d"] = JsonValue::sNullValue;
	value["e"] = true;

	EXPECT_EQ_BOOL(true, value.containsKey("a") && value.containsKey("e"));
	EXPECT_EQ_INT(eObject, value.type());
	EXPECT_EQ_SIZE_T(5, value.size());
	
	value["d"] = "hello";
	EXPECT_EQ_STRING("hello"s, value["d"].getString());
	
	JsonValue tmp = value.get("d");
	tmp.setString("world");
	EXPECT_EQ_STRING("hello"s, value["d"].getString());

	value.clear();
	EXPECT_EQ_SIZE_T(0, value.size());
	EXPECT_EQ_BOOL(true, value.empty());
}

static void test_parse()
{
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_object();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_miss_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_miss_comma_or_square_bracket();
	test_parse_object_miss_key();
	test_parse_object_miss_colon();
	test_parse_object_miss_comma_or_curly_bracket();
}

static void test_access()
{
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
	test_access_array();
	test_access_object();
}

static void test_write()
{
	std::string json = "{\"a\" : null, \"b\": false, \"c\" : {\"d\" : [100.0, \"hello\", 1234212799.45]}}";
	JsonValue value;
	JsonParser parser;
	parser.parseJson(value, json);
	std::string outputJson;
	JsonWriter writer;
	writer.writeJson(value, outputJson);

	JsonValue tmpValue;
	EXPECT_EQ_INT(eOk, parser.parseJson(tmpValue, outputJson));
	EXPECT_EQ_BOOL(true, tmpValue == value);
}

static void test_value_api()
{
	// array
	{
		JsonValue value;
		value.append(JsonValue());
		value.append(true);
		value.append(false);
		value.append(1);
		value.append(3.1415926);
		value.append("hello");
		value.append(JsonValue(eArray));
		value.append(JsonValue(eObject));
		EXPECT_EQ_BOOL(true, value.isArray());
		EXPECT_EQ_BOOL(false, value.empty());
		EXPECT_EQ_SIZE_T(8, value.size());
		EXPECT_EQ_INT(eNull, value.get(0).type());
		EXPECT_EQ_INT(eTrue, value[1].type());
		EXPECT_EQ_DOUBLE(3.1415926, value[4].getNumber());
		EXPECT_EQ_STRING("hello"s, value.get(5).getString());
		EXPECT_EQ_INT(eArray, value[6].type());
		EXPECT_EQ_SIZE_T(0, value[6].size());
		EXPECT_EQ_INT(eObject, value[7].type());

		value.resize(5);
		EXPECT_EQ_SIZE_T(5, value.size());
		JsonValue removed;
		value.removeAt(4, removed);
		EXPECT_EQ_DOUBLE(3.1415926, removed.getNumber());
		value.removeAt(0);
		EXPECT_EQ_BOOL(true, value[0].isTrue());
		value[1] = JsonValue(eObject);
		EXPECT_EQ_BOOL(true, value[1].isObject());
		
		value.clear();
		EXPECT_EQ_BOOL(true, value.empty());
	}
	
	// object
	{
		JsonValue value(eObject);
		EXPECT_EQ_BOOL(true, value.isObject());
		EXPECT_EQ_INT(eObject, value.type());
		EXPECT_EQ_BOOL(true, value.empty());
		EXPECT_EQ_SIZE_T(0, value.size());
		EXPECT_EQ_BOOL(false, value.containsKey("hello"));

		value["hello"].append(100);
		value["world"]["owner"] = "me";
		EXPECT_EQ_SIZE_T(2, value.size());
		EXPECT_EQ_INT(eArray, value.get("hello").type());
		EXPECT_EQ_INT(eObject, value.get("world").type());
		EXPECT_EQ_DOUBLE(100.0, value.get("hello")[0].getNumber());
		EXPECT_EQ_STRING("me"s, value["world"].get("owner").getString());
		EXPECT_EQ_INT(eNull, value.get("yes").type());

		auto keys = value.getKeys();
		EXPECT_EQ_SIZE_T(2, keys.size());
		EXPECT_EQ_STRING("hello"s, keys[0]);
		EXPECT_EQ_STRING("world"s, keys[1]);
		
		value.appendKey("yes", "no");
		EXPECT_EQ_STRING("no"s, value.get("yes").getString());
		JsonValue removed;
		value.removeKey("yes", removed);
		EXPECT_EQ_SIZE_T(2, value.size());
		EXPECT_EQ_STRING("no"s, removed.getString());
		value.removeKey("hello");
		EXPECT_EQ_SIZE_T(1, value.size());

		value.clear();
		EXPECT_EQ_SIZE_T(0, value.size());
	}
}
 
static void test_parser_api()
{
	// error infomation
	{
		std::string json = "{ \n \r\n \r \"hello\"[,]}";
		JsonValue value;
		JsonParser parser;
		std::string errInfo;
		EXPECT_EQ_INT(eObjectMissColon, parser.parseJson(value, json, errInfo));
		EXPECT_EQ_INT(4, parser.errorLine());
		EXPECT_EQ_INT(9, parser.errorColumn());

		parser.setParseLFStyle(eCR);
		EXPECT_EQ_INT(eObjectMissColon, parser.parseJson(value, json, errInfo));
		EXPECT_EQ_INT(3, parser.errorLine());
		EXPECT_EQ_INT(9, parser.errorColumn());
		
		parser.setParseLFStyle(eLF);
		EXPECT_EQ_INT(eObjectMissColon, parser.parseJson(value, json, errInfo));
		EXPECT_EQ_INT(3, parser.errorLine());
		EXPECT_EQ_INT(11, parser.errorColumn());

		parser.setParseLFStyle(eCRLF);
		EXPECT_EQ_INT(eObjectMissColon, parser.parseJson(value, json, errInfo));
		EXPECT_EQ_INT(2, parser.errorLine());
		EXPECT_EQ_INT(11, parser.errorColumn());

		json = "{}";
		EXPECT_EQ_INT(eOk, parser.parseJson(value, json, errInfo));
		EXPECT_EQ_INT(0, parser.errorLine());
		EXPECT_EQ_INT(0, parser.errorColumn());
	}

	// support parsing comment, not implement yet
	{

	}
}

static void test_writer_api()
{
	// style
	{
		JsonWriter writer(false);

	}

	// add or rewrite, utf-bom
	{
		std::string json = "hello";
		JsonValue value(eObject);
		JsonWriter writer;
		writer.setAddToString();
		writer.setAddUtf8Bom(true);
		std::string expect = "hello"s + char(0xEF) + char(0xBB) + char(0xBF) + "{}";
		writer.writeJson(value, json);
		EXPECT_EQ_STRING(expect, json);

		writer.setRewriteString();
		writer.setAddUtf8Bom(false);
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("{}"s, json);
	}

	// line feed style
	{
		JsonValue value;
		value.append("hello");
		std::string json;

		JsonWriter writer;
		writer.setRewriteString();
		writer.setLFStyle(eLF);
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("[\n\t\"hello\"\n]"s, json);
		writer.setLFStyle(eCR);
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("[\r\t\"hello\"\r]"s, json);
		writer.setLFStyle(eCRLF);
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("[\r\n\t\"hello\"\r\n]"s, json);
	}

	// indentation
	{
		JsonWriter writer;
		writer.setRewriteString();
		writer.setUseSpacesToIndent();
		writer.setIndentSpaceCount(2);
		
		std::string json;
		JsonValue value;
		value.append(JsonValue(eObject));
		writer.writeJson(value, json);   
		EXPECT_EQ_STRING("[\r\n  {}\r\n]"s, json);

		writer.setIndentSpaceCount(6);
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("[\r\n      {}\r\n]"s, json);

		writer.setUseTabToIndent();
		writer.writeJson(value, json);
		EXPECT_EQ_STRING("[\r\n\t{}\r\n]"s, json);
	}

	// write comment, not implement yet
	{
		
	}
}

static void test_api()
{
	test_value_api();
	test_parser_api();
	test_writer_api();
}

int main()
{
	test_parse();
	test_access();
	test_write();
	test_api();
	printf("%d/%d %3.2f%% passed", test_pass, test_count, test_pass * 100.0 / test_count);
	return 0;
}