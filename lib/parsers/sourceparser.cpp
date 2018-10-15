#include "parsers/sourceparser.hpp"

#include "types/file_tree.hpp"

#include <set>
#include <map>

#define INCLUDE_TOKEN "include"

#define CLASS_TOKEN "class"
#define STRUCT_TOKEN "struct"
#define UNION_TOKEN "union"
#define NAMESPACE_TOKEN "namespace"
#define TYPEDEF_TOKEN "typedef"
#define USING_TOKEN "using"

#define CONST_TOKEN "const"
#define OPERATOR_TOKEN "operator"
#define TEMPLATE_TOKEN "template"


#define CMP_TOKEN(TOKEN_NAME) (!strncmp(p, TOKEN_NAME, sizeof(TOKEN_NAME) - 1))
#define CHECK_TOKEN(TOKEN_NAME, STATE) \
    if (CMP_TOKEN(TOKEN_NAME)) {\
        if (!is_identifier_ch(*(p-1)) && !is_identifier_ch(*(p + sizeof(TOKEN_NAME) - 1))) {\
            p += sizeof(TOKEN_NAME) - 2;\
            _state = STATE;\
        }\
    }

#define M_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define M_MAX(a, b) (((a) > (b)) ? (a) : (b))

static bool is_identifier_ch(const char c)
{
    return isalpha(c) || c == '_' || isdigit(c);
}

const char *SourceParser::skipSpaces(const char *line) const
{
    MY_ASSERT(line);
    for (;*line;++line) {
        if (!isspace(*line))
            return line;
    }
    return line;
}

const char *SourceParser::skipSpacesAndComments(const char *line) const
{
    MY_ASSERT(line);
    bool commentState = false;
    for ( ; *line; ++line) {
        COUNT_LINES(
                if (*line == '\n') {
                    VERBAL_1(std::cout << _line << " ++_line '"
                    << std::string(line, M_MIN(20, strlen(line)))
                    << '\'' << std::endl;)
                    ++_line;
                })
        if (commentState) {
            if (!strncmp(line, "*/", 2)) {
                commentState = false;
                ++line;
            }
            continue;
        }
        if (*line == '/') {
            if (*(line + 1) == '*') {
                // comment
                commentState = true;
                ++line;
                continue;
            }
            else {
                return line;
            }
        }
        if (!isspace(*line))
            return line;
    }
    return line;
}

int SourceParser::skipSpacesAndCommentsR(const char *line, int len) const
{
    MY_ASSERT(line);
    bool commentState = false;

    const int len1 = len - 1;
    int d = 0;
    for ( ; d < len1; ++d) {
        if (commentState) {
            if (!strncmp(line - d - 1, "/*", 2)) {
                commentState = false;
                ++d;
            }
            continue;
        }
        const char ch = *(line - d);
        if (ch == '/') {
            if (*(line - d - 1) == '*') {
                // comment
                commentState = true;
                ++d;
                continue;
            }
            else {
                return d;
            }
        }
        if (!isspace(ch))
            return d;
    }
    return d;
}

const char *SourceParser::parseWord(const char *p, int &wordLength) const
{
    wordLength = 0;
    for ( ; *p; ++p) {
        const char c = *p;
        if (!(isalpha(c) || c == '_' || isdigit(c)))
            break;
        ++wordLength;
    }
    return p;
}

struct CharTreeNode
{
    typedef std::map<char, CharTreeNode*> LeafMap;

    bool finite;
    LeafMap leafs;

    CharTreeNode() : finite(false) {}
    ~CharTreeNode()
    {
        for (auto &n: leafs)
            delete n.second;
    }

    void print(int indent = 1)
    {
        std::string strIndents = makeIndents(indent);

        if (1 == indent)
            std::cout << "CharTreeNode_ROOT" << std::endl;
        for (auto &p: leafs) {
            std::cout << strIndents << p.first << ' ' << p.second->finite << std::endl;
            p.second->print(indent + 1);
        }
    }

    void insertRev(const std::string &key)
    {
        CharTreeNode *node = this;
        for (std::string::const_reverse_iterator it = key.rbegin();
             it < key.rend(); ++it) {
            node = node->insert(*it);
        }

        if (!node->finite)
            node->finite = true;
    }

    CharTreeNode *find(char ch) const
    {
        auto it = leafs.find(ch);
        if (it != leafs.end()) {
//            std::cout << "found " << ch << std::endl;
            return it->second;
        }
        return nullptr;
    }

    CharTreeNode *insert(char ch)
    {
        if (auto node = find(ch)) {
            // found
            return node;
        }

        CharTreeNode *leaf = new CharTreeNode();
        leafs.insert(std::make_pair(ch, leaf));
        return leaf;
    }
};

static CharTreeNode initOverloadingOperators()
{
    CharTreeNode root;
    const char* const operators[] = {
        "+",    "-",    "*",    "/",    "%",    "^",
        "&",    "|",    "~",    "!",    ",",    "=",
        "<",    ">",    "<=",   ">=",   "++",   "--",
        "<<",   ">>",   "==",   "!=",   "&&",   "||",
        "+=",   "-=",   "/=",   "%=",   "^=",   "&=",
        "|=",   "*=",   "<<=",  ">>=",  "[]",   "()",
        "->",   "->*",   "new",  "delete",
        "new []",   "delete []"
    }; // table is taken from https://www.tutorialspoint.com/cplusplus/cpp_overloading.htm
    for (auto &op : operators)
        root.insertRev(op);

    return root;
}

static CharTreeNode charTreeRevOverloadTokens = initOverloadingOperators();

int SourceParser::parseNameR(const char *p, int len, ScopedName &name) const
{
    MY_ASSERT(p);
    int d = 0;
    d += skipSpacesAndCommentsR(p, len);
    std::list<std::string> nsname;
    d += dealWithOperatorOverloadingR(p - d, len - d, nsname);
    bool operatorOverloading = !nsname.empty();
    bool namespaceState = false;
    for (;;) {
        if (!operatorOverloading) {
            d += skipTemplateR(p - d, len - d);
            int wl = parseWordR(p - d, len - d);
            if (0 == wl)
                break;
            d += wl;
            nsname.push_front(std::string(p - d + 1, wl));
        }
        else {
//            MY_PRINTEXT(operatorOverloading);
            operatorOverloading = false;
        }
        d += skipSpacesAndCommentsR(p - d, len - d);
        if (!strncmp(p - d - 1, "::", 2)) {
            d += 2;
            d += skipSpacesAndCommentsR(p - d, len - d);
            if (!namespaceState)
                namespaceState = true;
        }
        else {
            if (!namespaceState && d < len) {
                const char ch = *(p - d);
//                if (ch == '}' || ch == ';') {
//                    std::cout << std::string(p-d-20, 20) << std::endl;
//                    MY_PRINTEXT(ASSERT HERE);
//                }
                if (!(is_identifier_ch(ch)
                      || ch == '>'    // template
                      || ch =='&'     // reference
                      || ch =='*'     // pointer
                      )) {
                    nsname.clear();
                }
            }
            break;
        }
    }
    for (auto &n : nsname)
        name.append(n);
    return d;
}

int SourceParser::dealWithOperatorOverloadingR(const char *p, int len, std::list<std::__cxx11::string> &nsname) const
{
    CharTreeNode *node = &charTreeRevOverloadTokens;
    for (int d = 0; d < len; ++d) {
        const char ch = *(p - d);
        if (node = node->find(ch)) {
            if (!node->finite)
                continue;
        }
        else {
            return 0; // not operator overloading
        }
        // else - not overloading operator
        int offset = d + 1;
        offset += skipSpacesAndCommentsR(p - offset, len - offset);
        if (offset + sizeof(OPERATOR_TOKEN) >= len)
            return 0; // not operator overloading
        if (!strncmp(OPERATOR_TOKEN, p - offset - sizeof(OPERATOR_TOKEN) + 2, sizeof(OPERATOR_TOKEN) - 1)) {
            // ... operator <op>(...)
            offset += sizeof(OPERATOR_TOKEN) - 1;
            std::string opStrNormalized(p - offset + 1, offset);
            // remove spaces
            opStrNormalized.erase(
                        remove_if(opStrNormalized.begin(), opStrNormalized.end(), isspace),
                        opStrNormalized.end());
            // insert method name (without spaces)
            nsname.push_back(opStrNormalized);
            return offset;
        }
    }
    return 0;
}

int SourceParser::parseWordR(const char *p, int len) const
{
    int wordLength = 0;
//    std::cout << "parseWordR " << std::string(p, M_MIN(10, strlen(p))) << std::endl;
    for ( ; wordLength < len; ++wordLength) {
        const char c = *(p - wordLength);
        if (!(is_identifier_ch(c) || c == '<' ||
              c == '>' || c == '~'))
            break;
    }
    return wordLength;
}

bool SourceParser::checkIfFunctionHeadR(const char *p, int len) const
{
    int d = skipSpacesAndCommentsR(p, len);
    const char ch = *(p - d);
    std::cout << "checkIfFunctionHeadR " << ch << std::endl;

    return is_identifier_ch(ch) || ch == ':';
}

const char *SourceParser::readUntil(const char *p, const char *substr) const
{
    int substrLength = strlen(substr);
    if (*p == '\n')
        --_line;
    for ( ; *p; ++p) {
        if (!strncmp(p, substr, substrLength))
            return p + substrLength;
        COUNT_LINES(if (*p == '\n') {
                     VERBAL_1(std::cout << _line << " ++_line '"
                     << std::string(p, M_MIN(20, strlen(p)))
                     << '\'' << std::endl;)
                     ++_line;
                 })
    }
    return p;
}

const char *SourceParser::readUntilM(const char *p, const std::list<std::string> &substrings,
                                     std::list<std::string>::const_iterator &it) const
{
    if (*p == '\n')
        --_line;
    for ( ; *p; ++p) {
        for (auto & ss: substrings) {
            if (!strncmp(p, ss.c_str(), ss.size())) {
                it = std::find(substrings.begin(), substrings.end(), ss);
                return p + ss.size();
            }
        }
        COUNT_LINES(if (*p == '\n') {
                     VERBAL_1(std::cout << _line << " ++_line '"
                     << std::string(p, M_MIN(20, strlen(p)))
                     << '\'' << std::endl;)
                     ++_line;
                 })
    }
    return p;
}

const char *SourceParser::parseName(const char *p, ScopedName &name) const
{
    MY_ASSERT(p);
    std::list<std::string> nsname;
    for (;;) {
        int wl;
        p = skipSpacesAndComments(p);
        p = parseWord(p, wl);
        if (wl == 0)
            break;
        nsname.push_back(std::string(p - wl, wl));
        p = skipSpacesAndComments(p);
        if (!strncmp(p, "::", 2))
            p += 2;
        else
            break;
    }
    for (auto &n : nsname) {
        name.append(n);
    }
    return p;
}

std::string SourceParser::stateToString(SourceParser::SpecialState state)
{
    switch (state) {
    case NoSpecialState:        return "NoSpecialState";
    case HashSign:              return "HashSign";
    case IncludeState:          return "IncludeState";
    case NotIncludeMacroState:  return "NotIncludeMacroState";

    case ClassState:            return "ClassState";
    case StructState:           return "StructState";
    case UnionState:            return "UnionState";
    case TypedefState:          return "TypedefState";

    case UsingState:            return "UsingState";
    case NamespaceState:        return "NamespaceState";
    case TemplateState:         return "TemplateState";
    case Quotes:                return "Quotes";

    case SingleQuotes:          return "SingleQuotes";
    case OpenBracket:           return "OpenBracket";
    case MultiComments:         return "MultiComments";
    case SingleComments:        return "SingleComments";
    }
    return "UNKNOWN SpecialState";
}

SourceParser::SourceParser(const FileTree &ftree)
    : _fileTree(ftree)
{
    _currentNamespace.setNamespaceSeparator();
}

const char *SourceParser::skipTemplateAndSpaces(const char *p) const
{
    p = skipSpacesAndComments(p);

    int tmplDeep = 0;
    for ( ; *p; ++p)
    {
        const char ch = *p;
        switch (ch) {
        case '<':
            ++tmplDeep;
            break;
        case '>':
            --tmplDeep;
            if (0 == tmplDeep)
                return p + 1;
            continue;
        default:
            if (0 == tmplDeep)
                return p;
            break;
        }

        if (!strncmp(p, "/*", 2))
            p = skipSpacesAndComments(p) - 1;
    }
    return p;
}

int SourceParser::skipTemplateR(const char *line, int len) const
{
    int d = 0;

    int tmplDeep = 0;
    for (; d < len; ++d) {
        const char ch = line[-d];
        switch (ch) {
        case '>':
            ++tmplDeep;
            break;
        case '<':
            --tmplDeep;
            if (0 == tmplDeep)
                return d + 1;
            break;
        default:
            if (0 == tmplDeep)
                return d;
            break;
        }

        if (ch == '/') {
            if (d + 1 < len && *(line - d - 1) == '*') {
                d += skipSpacesAndCommentsR(line - d, len - d);
                --d;
            }
        }
    }
    return d;
}

const char *SourceParser::skipLine(const char *p) const
{
    COUNT_LINES(
                VERBAL_1(std::cout << _line << " ++_line '"
                         << std::string(p, M_MIN(20, strlen(p)))
                         << '\'' << std::endl;)
                ++_line);
    for ( ; *p; ++p) {
        COUNT_LINES(
            if (*(p - 1) == '\\' && *p == '\n') {
                VERBAL_1(std::cout << _line << " ++_line '"
                << std::string(p, M_MIN(20, strlen(p)))
                << '\'' << std::endl;)
                ++_line;
            })

        if (*p == '\n' && *(p - 1) != '\\')
            return p + 1;
    }
    return p;
}

static std::list<std::string> initChl()
{
    std::list<std::string> chl;
    chl.push_back(std::string(";"));
    chl.push_back(std::string("{"));

    return chl;
}

static std::list<std::string> initInheritanceChl()
{
    std::list<std::string> chl;
    chl.push_back(":");
    chl.push_back("{");

    return chl;
}

void SourceParser::parseFile(FileNode *node)
{
    if (!node->isRegularFile())
        return;
    node->setModified();

    std::string fname = (_fileTree.rootPath() + node->path()).joint();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if(!data) {
        errors() << "Failed to open the file" << '\'' + std::string(fname) + '\'';
        return;
    }
    long file_size = data_pair.second;
    int lcbrackets = 0;
    int nsbrackets = 0;
    std::list<int> listNsbracketsAt;
    std::list<int> listClassDeclAt;

    _line = 1;
    _currentFile = node;
    _state = NoSpecialState;
    _funcName.clear();
    _currentNamespace.clear();
    _listUsingNamespace.clear();

    for (const char *p = data; p - data < file_size;)
    {
        MY_ASSERTF(0 != *p);
        VERBAL_1(std::cout << "ch '" << *p << "' state " << stateToString(_state) << std::endl;);
        COUNT_LINES(if (*p == '\n') {
                     VERBAL_1(std::cout << _line << " ++_line '" <<
                     std::string(p, M_MIN(20, strlen(p))) << '\'' << std::endl;)
                     ++_line;
                 })
        switch (_state)
        {
        case NoSpecialState:
        {
            const char ch = *p;
            switch (ch)
            {
            case '\"':
                _state = Quotes;
                break;
            case '\'':
                _state = SingleQuotes;
                break;
            case '{':
                ++lcbrackets;
                break;
            case '}':
                MY_ASSERTF(lcbrackets > 0);
                --lcbrackets;
                if (!listNsbracketsAt.empty() &&
                        (lcbrackets == listNsbracketsAt.back())) {
                    listNsbracketsAt.pop_back();
                    --nsbrackets;
                    _currentNamespace.removeLast();
                }
                if (!listClassDeclAt.empty() &&
                        (lcbrackets == listClassDeclAt.back())) {
                    MY_ASSERTF(!_classNameDecl.empty());
                    if (!_classNameDecl.empty()) {
                        listClassDeclAt.pop_back();
                        p = skipSpacesAndComments(p + 1);
                        if (*p == ';')
                            node->record()._setClassDecl.insert(_classNameDecl);
                        --p;
                        _classNameDecl.clear();
                    }
                }
                break;
            case '#':
                _state = HashSign;
                break;
            case '/':
            {
                const char nch = *(p + 1);
                switch (nch) {
                case '/':
                    _state = SingleComments;
                    ++p;
                    break;
                case '*':
                    _state = MultiComments;
                    ++p;
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
            if (lcbrackets <= nsbrackets) {
                switch (ch) {
                case '(':
                {
                    _funcName = _currentNamespace;
                    parseNameR(p - 1, p - data - 1, _funcName);
                    break;
                }
                case ')':
                    if (!_funcName.empty() &&
                            _funcName != _currentNamespace) {
                        p = skipSpacesAndComments(p + 1);
                        if (*p == '{' || CMP_TOKEN(CONST_TOKEN)) {
                            // impl function/method
                            node->record()._setImplements.insert(_funcName);
                        }
                        else if (*p == ';') {
                            // global function decl
                            node->record()._setFuncDecl.insert(_funcName);
                        }
                        --p;
                        _funcName.clear();
                    }
                    break;
                default:
                    CHECK_TOKEN(USING_TOKEN, UsingState);
                    CHECK_TOKEN(NAMESPACE_TOKEN, NamespaceState);
                    CHECK_TOKEN(CLASS_TOKEN, ClassState);
                    CHECK_TOKEN(STRUCT_TOKEN, StructState);
                    CHECK_TOKEN(UNION_TOKEN, UnionState);
//                    CHECK_TOKEN(TYPEDEF_TOKEN, TypedefState); // no need to check for typedef
                    CHECK_TOKEN(TEMPLATE_TOKEN, TemplateState);

                    break;
                }
            }
            ++p;
            break;
        }
        case NotIncludeMacroState:
        {
            p = skipLine(p);
            _state = NoSpecialState;
            break;
        }
        case HashSign:
        {
            // check if include directive
            p = skipSpacesAndComments(p);
            if (CMP_TOKEN(INCLUDE_TOKEN)) {
                _state = IncludeState;
                p = skipSpacesAndComments(p + sizeof(INCLUDE_TOKEN));
            }
            else {
                _state = NotIncludeMacroState;
            }
            break;
        }
        case IncludeState:
        {
            // parse filename
            const char ch = *p;
            char pairChar;
            IncludeDirective dir;
            if (ch == '"') {
                pairChar = '"';
                dir.type = IncludeDirective::Quotes;
            }
            else if (ch == '<') {
                pairChar = '>';
                dir.type = IncludeDirective::Brackets;
            }
            else {
                std::cout << std::string(p) << std::endl;
                MY_ASSERT(false)
            }
            ++p;
            const char *end_of_filename = (char*) memchr(p, pairChar, strlen(p));
            MY_ASSERT(end_of_filename);
            COUNT_LINES(for (const char *s = p; s < end_of_filename; ++s) {
                if (*s == '\n') {
                    VERBAL_1(std::cout << _line << " ++_line '"
                              << std::string(p, M_MIN(20, strlen(p)))
                              << '\'' << std::endl;)
                    ++_line;
                }
            })

            dir.filename = std::string(p, end_of_filename - p);

            FileNode *includedFile = _fileTree.searchIncludedFile(dir, node);

            if (includedFile)
                node->record()._listIncludes.push_back(dir);

            _state = NoSpecialState;
            p = skipLine(end_of_filename);
            break;
        }
        case TemplateState:
        {
            p = skipTemplateAndSpaces(p);
            _state = NoSpecialState;
            break;
        }
        case UnionState:
        case StructState:  // no difference between parsing
        case ClassState:   // struct ... and class ...
        {
            ScopedName className = _currentNamespace;
            p = parseName(p, className);

            static std::list<std::string> chl = initChl();
            std::list<std::string>::const_iterator it;
            const char *pLast = readUntilM(p, chl, it);

            if (it == chl.end()) {
                MY_ASSERT(false);
            }
            else {
                const char ch = *it->c_str();
                switch (ch) {
                case '{':
                {
                    // decl
                    _classNameDecl = className;
                    listClassDeclAt.push_back(lcbrackets);

                    // parse inheritance
                    ScopedName baseClassName;
                    baseClassName.setNamespaceSeparator();

                    const char *pInh = p;
                    for (; pInh < pLast; ++pInh)
                    {
                        if (*pInh == ':' &&
                            *(pInh - 1) != ':' &&
                            *(pInh + 1) != ':')
                        {
                            ++pInh;
                            break;
                        }
                    }

                    int classHeaderLength = pInh - p;
                    ScopedName snClassName = _currentNamespace;
                    const char *pOneParsed = pInh - parseNameR(pInh, classHeaderLength, snClassName);
                    static HashedString hsFinal = std::string("final");
                    if (snClassName.last() == hsFinal) {
                        snClassName = _currentNamespace;
                        parseNameR(pOneParsed, pInh - pOneParsed, snClassName);
                    }

                    for (; pInh < pLast; )
                    {
                        pInh = skipSpacesAndComments(pInh);
                        if (*pInh == ',' || *pInh == '{') {
                            // add inherits
                            auto &inheritances = node->record()._setInheritances;

                            // insert base class to inheritances
                            inheritances.insert(baseClassName);
                            // also insert namespace version
                            if (!_currentNamespace.empty())
                                inheritances.insert(_currentNamespace + baseClassName);

                            ++pInh;
                        }
                        else {
                            baseClassName.clear();
                            static const std::list<std::string> chl = initInheritanceChl();
                            std::list<std::string>::const_iterator it;
                            const char *p = parseName(pInh, baseClassName);
                            if (std::find(chl.cbegin(), chl.cend(), std::string(*p, 1)) ==
                                    chl.cend())
                                pInh = readUntilM(p, chl, it);
                            else
                                pInh = p;
                        }
                    }

                    --p; // lcbracket++
                    break;
                }
                case ';':
                    // ref
                    // do nothing
                    break;
                default:
                    break;
                }
            }
            _state = NoSpecialState;
            break;
        }
        case NamespaceState:
        {
            int curNSSize = _currentNamespace.splitted().size();
            p = parseName(p, _currentNamespace);
            if (curNSSize < _currentNamespace.splitted().size()) {
                ++nsbrackets;
                listNsbracketsAt.push_back(lcbrackets);
            }

            _state = NoSpecialState;
            break;
        }
        case SingleQuotes:
        {
            const char ch = *p;
            switch (ch) {
            case '\\':
                p += 3;
                break;
            case '\'':
                p += 1;
                break;
            default:
                p += 2;
                break;
            }
            _state = NoSpecialState;
            break;
        }
        case UsingState:
        {
            p = skipSpacesAndComments(p);
            if (CMP_TOKEN(NAMESPACE_TOKEN)) {
                // using namespace ->ns
                ScopedName ns;
                ns.setNamespaceSeparator();
                p = parseName(p, ns);
                if (!ns.empty()) {
                    VERBAL_0(std::cout << "using namespace " << ns.joint() << std::endl;)
                    node->record()._listUsingNamespace.push_back(ns);
                }
            }
            _state = NoSpecialState;
        }
        case TypedefState:
        {
            // just skip
//            p = readUntil(p, ";");
            _state = NoSpecialState;
            break;
        }
        case Quotes:
        {
            for ( ; *p; ++p) {
                COUNT_LINES(if (*p == '\n') {
                             VERBAL_1(std::cout << _line << " ++_line '"
                             << std::string(p, M_MIN(20, strlen(p)))
                             << '\'' << std::endl;)
                             ++_line;
                         })
                if (*p == '\\')
                    ++p; // skip next
                else if (*p == '\"') {
                    _state = NoSpecialState;
                    ++p;
                    break;
                }
            }
            break;
        }
        case MultiComments:
        {
            p = readUntil(p, "*/");
            _state = NoSpecialState;
            break;
        }
        case SingleComments:
        {
            p = skipLine(p);
            _state = NoSpecialState;
            break;
        }
        }
    }

    // free file's data
    delete data;
}
