#ifndef SOURCE_PARSER_HPP
#define SOURCE_PARSER_HPP

#include "tokenizer.hpp"
#include <types/splitted_string.hpp>

class FileNode;
class FileTree;
class IncludeDirective;

template < typename T >
class SimpleStack
{
    std::vector< T > stack;

public:
    void push(const T &value) { stack.push_back(value); }

    bool isOnTop(const T &value) const
    {
        if (stack.empty())
            return false;
        return stack.back() == value;
    }
    void pop() { stack.pop_back(); }
    void clear() { stack.clear(); }
};

template < typename T >
class SparceStack
{
    int _deep = 0;
    SimpleStack< T > _stack;

public:
    void push(const T &value)
    {
        ++_deep;
        _stack.push(value);
    }

    bool pop(int totalDeep)
    {
        if (_stack.isOnTop(totalDeep)) {
            --_deep;
            _stack.pop();
            return true;
        }
        return false;
    }
    void clear()
    {
        _deep = 0;
        _stack.clear();
    }

    int deep() const { return _deep; }
};

class SourceParser
{
    using TokenVector = std::vector< Token >;
    using TokenNameSet = std::unordered_set< TokenName, EnumClassHash >;

public:
    explicit SourceParser(const FileTree &ftree);

    void parseFile(FileNode *node);

private:
    bool parseScopedName(const TokenVector &v, int offset, int end,
                         SplittedPath &name);

    int getIdentifierStart(const TokenVector &v, int offset) const;
    void skipOperatorOverloadingReverse(const TokenVector &v,
                                        int &offset) const;
    void skipLine(const TokenVector &tokens, int &offset) const;
    void skipUntilEndif(const TokenVector &tokens, int &offset) const;
    bool skipTemplate(const TokenVector &v, int &offset) const;
    bool skipTemplateReverse(const TokenVector &tokens, int &offset) const;

    void prepare();
    TokenName readUntil(const TokenVector &tokens, int &offset,
                        const TokenNameSet &tokenNames);

    void increment(const TokenVector &tokens, int &offset,
                   bool checkRange = false);
    void increment_pp(int &offset, int n = 1) const { offset += n; }
    void decrement_pp(int &offset, int n = 1) const { offset -= n; }

    bool isTopLevelCB() const;
    void setNamespace();
    void dealWithClassDeclaration(const TokenVector &tokens, int offset);
    void parseIncludeFilename(const TokenVector &tokens, int &offset,
                              IncludeDirective &dir);
    void readPath(const TokenVector &tokens, int &offset, SplittedPath &path);

    bool checkOffset(const TokenVector &tokens, int offset) const;
    void assertOnBadRange(const TokenVector &tokens, int offset) const;
    bool isClassToken(const TokenVector &tokens, int offset) const;
    bool isInheritanceToken(const TokenVector &tokens, int offset) const;

private:
    const FileTree &_fileTree;
    FileNode *_node;

    ScopedName _currentNamespace;
    std::vector< ScopedName > _listUsingNamespace;

    int _openBracketCount;
    int _openCurlyBracketCount;
    SparceStack< int > _stackNamespaceBrackets;
    SparceStack< int > _stackExternConstruction;
};

#endif // SOURCE_PARSER_HPP
