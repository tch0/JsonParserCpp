# 一个C++ Json解析库

## 参考

- 思路学习自[miloyip/json-tutorial][1]，使用C++实现。
- API设计一定程度上参考了[open-source-parsers/jsoncpp][2]。

## 特性

- 符合标准的C++ JSON解析器和生成器。
- 解析器
    - 仅支持解析不带注释的UTF-8字符串文本。
    - 仅以`double`类型存储JSON number。
    - 完善的解析错误提示：错误类型与错误位置。
- 生成器
    - 仅支持生成UTF-8文本。
    - 格式化：支持不同样式缩进，不同操作系统的不同换行符，是否包含UTF-8BOM头。
- 访问
    - 丰富的接口、友好方便的API设计，上手即用，一看便懂。
- 覆盖全面的测试保证正确性。


## 构建

- 将`./include`与`./src`的文件添加到你的项目直接参与编译，或编译为静态链接库引用。
- Windows下项目文件：`./Project`，环境：Visual Studio 2017。


## API

所有的实现存放在`MyJson`命名空间中。
仅有的三个类：
- `MyJson::JsonValue` ：JSON的value类型。
- `MyJson::JsonParser`：解析器。
- `MyJson::JsonWriter`：生成器。

解析和生成的操作对象都是字符串`std::string`。

### 解析
```C++
using namespace MyJson;
JsonValue value;
JsonParser parser;
std::string errInfo;
ParseResult ret = parser.parseJson(value, "[{}, null]", errInfo);
if (ret != eOk)
{
    int line = parser.errorLine();
    int column = parser.errorColumn();
    std::cout << "line : " << line << ", column" << column << "\n";
}
```

### 生成
```C++
using namespace MyJson;
JsonWriter writer;
writer.setUseSpacesToIndent();
writer.setIndentSpaceCount(4);
writer.setRewriteString();
writer.setAddUtf8Bom(true);
writer.setLFStyle(eCRLF);
std::string json;
JsonValue value;
writer.writeJson(value, json);
```

默认使用Tab来缩进，使用空格缩进时宽度为4字符，不添加UTF-8 BOM头，将value添加到字符串而不是覆盖字符串，使用`CR LF`来换行。

### 访问

`JsonValue`类型接口：
- 使用一个类型参数的构造函数。
- 使用不同类型参数并将value初始化对应类型的构造函数。
- 拷贝构造、移动构造与`operator=`。
- null类型可转化为所有其他类型。
- 类型获取与设置：`setType`, `type`。
- 类型判断：`isNull`,`isBool`,`isNumber`,`isString`,`isArray`,`isObject`。
- Null类型操作：`setNull`，清空所有数据，类型转化为null。
- 以下各类型专属操作使用前应该先确定类型，或者确定为null时使用：
    - Boolean操作：`isTrue`,`isFalse`,`setBool`,`getBool`。
    - Number操作：`getNumber`,`setNumber`。
    - String操作：`getString`,`setString`。
    - Array操作：`get`,`resize`,`append`,`insert`,`removeAt`,`operator[size_t]`。
    - Obejct操作：`isObject`,`containsKey`,`getKeys`，`appendKey`,`removeKey`,`operator[std::string]`。
    - Array&Object共用操作：`size`,`clear`,`empty`。
- `operator==`,`operator!=`。
- Null类型可转化为所有类型，可通过调用设置或者添加值的接口将其转化为其他类型。
- 所有的获取接口都使用引用，如果需要拷贝一个value，通过获取后调用拷贝构造或者`operator=`完成。

## 存在问题
- 在一个Value中存储了number/string/array/object所有类型的信息。构造时需要同时构造所有类型存储结构，时间和空间上都不够友好，但实现足够简单与直接。
- 不能解析与生成注释。
- 未区分整型和浮点。


## TODO or NOT TODO

- 使用`union`存储不同类型的Value，优化时间和空间。
- 支持整型解析与识别，区分整型和浮点。
- 支持解析与生成C/C++风格的注释。
- Benchmark

如果你发现了任何BUG或者想分享你的改进，欢迎[Pull Request][3]或者发起[Issue][4]。

[1]: https://github.com/miloyip/json-tutorial
[2]: https://github.com/open-source-parsers/jsoncpp
[3]: https://github.com/aojueliuyun/JsonParserCpp/pulls
[4]: https://github.com/aojueliuyun/JsonParserCpp/issues