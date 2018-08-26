#ifndef SOURCE_PARSER_HPP
#define SOURCE_PARSER_HPP

#include "types/scoped_name.hpp"

class FileNode;
class FileTree;

class SourceParser
{
public:
    explicit SourceParser(const FileTree &ftree);

    void parseFile(FileNode *node);
    void parseFileOld(FileNode *node);

    const char *skipTemplate(const char *p) const;
    int skipTemplateR(const char *line, int len) const;
    const char *skipLine(const char *p) const;
    const char *skipSpaces(const char *line) const;
    const char *skipSpacesAndComments(const char *line) const;
    int skipSpacesAndCommentsR(const char *line, int len) const;


    const char *parseName(const char *p, ScopedName &name) const;
    const char *parseWord(const char *p, int &wordLength) const;

    int parseNameR(const char *p, int len, ScopedName &name) const;
    int parseWordR(const char *p, int len) const;

    const char *readUntil(const char *p, const char *substr) const;
    void analyzeLine(const char *line, FileNode *node);
private:
    const FileTree &_fileTree;

private:
    // States
    enum SpecialState
    {
        NoSpecialState,         //
        HashSign,               // #
        IncludeState,           // #include
        NotIncludeMacroState,   // #smth (not #include)
        StructState,            // struct
        ClassState,             // class
        Quotes,                 // "
        OpenBracket,            // (
        MultiComments,          // /*
        SingleComments          // //
    };
    static std::string stateToString(SpecialState state);

    SpecialState _state;

    ScopedName _currentNamespace;
    std::list<ScopedName> _listUsingNamespace;

    ScopedName _funcName;
    mutable int _line;
    FileNode *_currentFile;
};

#endif // SOURCE_PARSER_HPP
