#include "parsers/sourceparser.hpp"

#include "types/file_tree.hpp"

#define CLASS_LITER "class"
#define STRUCT_LITER "struct"

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
        if (*line == '\n') {
            std::cout << "LINE1" << std::endl;
            std::cout << _line << " ++_line '" << std::string(line, 20) << '\'' << std::endl;
            ++_line;
        }
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

int SourceParser::parseNameR(const char *p, int len, ScopedName &name) const
{
    MY_ASSERT(p);
    int d = 0;
    std::cout << "parseNameR0 " << std::string(p, 10) << std::endl;
    d += skipSpacesAndCommentsR(p - d, len - d);
    for (;;) {
        int wl = parseWordR(p - d, len - d);
        if (0 == wl)
            break;
        d += wl;
        name.pushScopeOrName(std::string(p - d + 1, wl));
        std::cout << "parseNameR2 " << std::string(p - d + 1, wl) << std::endl;
        d += skipSpacesAndCommentsR(p - d, len - d);
        std::cout << "parseNameR3 " << std::string(p - d - 1, 10) << std::endl;
        if (!strncmp(p - d - 1, "::", 2)) {
            d += 2;
            d += skipSpacesAndCommentsR(p - d, len - d);
        }
        else
            break;
    }
    return d;
}

int SourceParser::parseWordR(const char *p, int len) const
{
    int wordLength = 0;
    std::cout << "parseWordR " << std::string(p, 10) << std::endl;
    for ( ; wordLength < len; ++wordLength) {
        const char c = *(p - wordLength);
        if (!(isalpha(c) || c == '_' ||
              isdigit(c) || c == '<' ||
              c == '>' || c == '~'))
            break;
    }
    return wordLength;
}

const char *SourceParser::readUntil(const char *p, const char *substr) const
{
    int substrLength = strlen(substr);
    if (*p == '\n')
        --_line;
    for ( ; *p; ++p) {
        if (!strncmp(p, substr, substrLength))
            return p + substrLength;
        if (*p == '\n') {
            std::cout << _line << " ++_line '" << std::string(p, 20) << '\'' << std::endl;
            ++_line;
        }
    }
    return p;
}

const char *SourceParser::parseName(const char *p, ScopedName &name) const
{
    MY_ASSERT(p);
    p = skipSpacesAndComments(p);
    bool endOfName = false;
    while (!endOfName) {
        int wl;
        p = parseWord(p, wl);
        std::cout << "wl " << wl << " word " << std::string(p - wl, wl) << std::endl;
        name.pushScopeOrName(std::string(p - wl, wl));
        p = skipSpacesAndComments(p);
        if (!strncmp(p, "::", 2))
            ++p;
        else
            endOfName = true;
    }
    return p;
}

void SourceParser::analyzeLine(const char *line, FileNode *node)
{
    line = skipSpacesAndComments(line);
    if (*line != '#')
        return;
    line = skipSpacesAndComments(line + 1);
    static const char *str_include = "include";
    static const int str_include_len = strlen("include");
    for (int i = 0; i < str_include_len; ++i) {
        if (line[i] != str_include[i])
            return;
    }
    line = skipSpacesAndComments(line + str_include_len);
    // parse filename
    char pairChar;
    IncludeDirective dir;
    if (*line == '"') {
        pairChar = '"';
        dir.type = IncludeDirective::Quotes;
    }
    else if (*line == '<') {
        pairChar = '>';
        dir.type = IncludeDirective::Brackets;
    }
    else {
        std::cout << std::string(line) << std::endl;
        MY_ASSERT(false)
    }
    ++line;
    const char *end_of_filename = (char*) memchr(line, pairChar, strlen(line));
    MY_ASSERT(end_of_filename)
    dir.filename = std::string(line, end_of_filename - line);

    FileNode *includedFile = _fileTree.searchIncludedFile(dir, node);

    if (includedFile)
        node->record()._listIncludes.push_back(dir);
    VERBAL(else std::cout << "not found " << dir.filename << std::endl;)
}

std::string SourceParser::stateToString(SourceParser::SpecialState state)
{
    switch (state) {
    case NoSpecialState:       return "NoSpecialState";
    case HashSign:             return "HashSign";
    case IncludeState:         return "IncludeState";
    case NotIncludeMacroState: return "NotIncludeMacroState";
    case StructState:          return "StructState";
    case ClassState:           return "ClassState";
    case Quotes:               return "Quotes";
    case OpenBracket:          return "OpenBracket";
    case MultiComments:        return "MultiComments";
    case SingleComments:       return "SingleComments";
    }
    return "UNKNOWN SpecialState";
}

SourceParser::SourceParser(const FileTree &ftree)
    : _fileTree(ftree)
{

}

void SourceParser::parseFileOld(FileNode *node)
{
    MY_ASSERT(node->isRegularFile());
    VERBAL(std::cout << "parseFile " << node->record()._path.string() << std::endl;)

    static const int BUFFER_SIZE = 200/*16*1024*/;

    std::string fname = (_fileTree._rootPath + node->record()._path).string();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if(!data) {
        errors() << "Failed to open the file" << '\'' + std::string(fname) + '\'';
        return;
    }
    long bytes_read = data_pair.second;
    long offset = 0;
    for (char *p = data;;)
    {
        char *pnl = (char*) memchr(p, '\n', bytes_read - (p - data));
        // start looking for include
        if (pnl) {
            *pnl = 0;
            analyzeLine(p, node);
        }
        else {
            offset = bytes_read - (p - data);
            if (p == data) {
                errors() << "Too big line in file, more than" << numberToString(BUFFER_SIZE);
                return;
            }
            else {
                if (p - data < bytes_read) {
                    // copy
                    memcpy(data, p, offset);
                }
            }
            break;
        }
        p = pnl + 1;
    }
    // free file's data
    delete data;
}

const char *SourceParser::skipTemplate(const char *p) const
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
            continue;
        default:
            break;
        }
        if (0 == tmplDeep)
            return p + 1;

        if (!strncmp(p, "/*", 2))
            p = skipSpacesAndComments(p);
    }
    return p;
}

int SourceParser::skipTemplateR(const char *line, int len) const
{
    int d = skipSpacesAndCommentsR(line, len);

    int tmplDeep = 0;
    for ( ; d < len; ++d)
    {
        const char ch = line[d];
        switch (ch) {
        case '>':
            ++tmplDeep;
            break;
        case '<':
            --tmplDeep;
            continue;
        default:
            break;
        }
        if (0 == tmplDeep)
            return d;

        if (len - d > 0 && !strncmp(line - d - 1, "*/", 2))
            d += skipSpacesAndCommentsR(line - d, len - d);
    }
    return d;
}

const char *SourceParser::skipLine(const char *p) const
{
    std::cout << _line << " ++_line '" << std::string(p, 20) << '\'' << std::endl;
    ++_line;
    for ( ; *p; ++p) {
        if (*p == '\n')
            return p + 1;
    }
    return p;
}

void SourceParser::parseFile(FileNode *node)
{
    if (!node->isRegularFile())
        return;
    VERBAL(std::cout << "parseFile " << node->record()._path.string() << std::endl;)

    std::string fname = (_fileTree._rootPath + node->record()._path).string();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if(!data) {
        errors() << "Failed to open the file" << '\'' + std::string(fname) + '\'';
        return;
    }
    long file_size = data_pair.second;
    int lcbrackets = 0;

    _line = 1;
    _currentFile = node;
    _state = NoSpecialState;
    _funcName.clear();
    _currentNamespace.clear();
    _listUsingNamespace.clear();
    for (const char *p = data; p - data < file_size;)
    {
        std::cout << "ch '" << *p << "' state " << stateToString(_state) << std::endl;
        std::cout << p - data << " size " << file_size << std::endl;
        if (*p == '\n') {
            std::cout << _line << " ++_line '" << std::string(p, 20) << '\'' << std::endl;
            ++_line;
        }
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
            case '{':
                ++lcbrackets;
                if (!_funcName.isEmpty()) {
                    // impl global func
                    node->record()._listImpl.push_back(_funcName);
                    _funcName.clear();
                }
                break;
            case '}':
                MY_ASSERTF(lcbrackets > 0);
                --lcbrackets;
                break;

            default:
                break;
            }
            if (lcbrackets == 0) {
                switch (ch) {
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
                case '(':
                {
                    _funcName = _currentNamespace;
                    /// TODO check if function
                    parseNameR(p - 1, p - data - 1, _funcName);
                    break;
                }
                case ';':
                {
                    if (!_funcName.isEmpty()) {
                        // decl func
                        node->record()._listDecl.push_back(_funcName);
                        _funcName.clear();
                    }
                }
                default:
                    break;
                }
                if (!strncmp(p, CLASS_LITER, sizeof(CLASS_LITER) - 1)) {
                    p += sizeof(CLASS_LITER) - 2;
                    _state = ClassState;
                }
                else if (!strncmp(p, STRUCT_LITER, sizeof(STRUCT_LITER) - 1)) {
                    p += sizeof(STRUCT_LITER) - 2;
                    _state = StructState;
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
            static const char *str_include = "include";
            static const int str_include_len = strlen("include");
            if (!strncmp(p, str_include, str_include_len)) {
                _state = IncludeState;
                p += str_include_len;
            }
            else {
                _state = NotIncludeMacroState;
            }
            p = skipSpacesAndComments(p);
            break;
        }
        case IncludeState: {
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
            MY_ASSERT(end_of_filename)
            for (const char *s = p; s < end_of_filename; ++s) {
                if (*s == '\n') {
                    std::cout << _line << " ++_line '" << std::string(p, 20) << '\'' << std::endl;
                    ++_line;
                }
            }

            dir.filename = std::string(p, end_of_filename - p);

            FileNode *includedFile = _fileTree.searchIncludedFile(dir, node);

            if (includedFile)
                node->record()._listIncludes.push_back(dir);
            VERBAL(else std::cout << "not found " << dir.filename << std::endl;)
            _state = NoSpecialState;
            p = skipLine(end_of_filename);
            break;
        }
        case StructState:  // no difference between parsing
        case ClassState:   // struct ... and class ...
        {
            ScopedName className = _currentNamespace;
            p = parseName(p, className);
            p = skipSpacesAndComments(p);
            const char ch = *p;
            if (ch == ';') {
                // ref
                // do nothing
            }
            else if (ch == '{') {
                // decl
                node->record()._listDecl.push_back(className);
            }
            _state = NoSpecialState;
            break;
        }
        case Quotes:
        {
            for ( ; *p; ++p) {
                if (*p == '\n') {
                    std::cout << _line << " ++_line '" << std::string(p, 20) << '\'' << std::endl;
                    ++_line;
                }
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
