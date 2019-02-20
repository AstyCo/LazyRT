#ifndef SOURCE_PARSER_HPP
#define SOURCE_PARSER_HPP

#include "tokenizer.hpp"
#include "types/splitted_string.hpp"

class FileNode;
class FileTree;

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
    void skipLine(const TokenVector &tokens, int &offset);
    void skipTemplate(const TokenVector &v, int &offset);
    void skipTemplateReverse(const TokenVector &tokens, int &offset) const;

    void prepare();
    TokenName readUntil(const TokenVector &tokens, int &offset,
                        const TokenNameSet &tokenNames);

    void increment(const TokenVector &tokens, int &offset);
    bool isTopLevelCB() const;
    void setNamespace();
    void dealWithClassDeclaration(const TokenVector &tokens, int offset);

    bool checkOffset(const TokenVector &tokens, int offset);
    bool isClassToken(const TokenVector &tokens, int offset);
    bool isInheritanceToken(const TokenVector &tokens, int offset);

private:
    const FileTree &_fileTree;
    FileNode *_node;

    ScopedName _currentNamespace;
    std::vector< ScopedName > _listUsingNamespace;

    int _openBracketCount;
    int _openCurlyBracketCount;
    int _namespaceDeep;
    SimpleStack< int > _stackNamespaceBrackets;
};

#endif // SOURCE_PARSER_HPP
