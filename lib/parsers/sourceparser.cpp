#include "parsers/sourceparser.hpp"

#include "types/file_tree.hpp"

#include "parsers_utils.hpp"

#include <set>
#include <map>

#define M_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define M_MAX(a, b) (((a) > (b)) ? (a) : (b))

// template < typename TInList >
// static decltype(auto) makeTokenSet(TInList &&il)
//{
//    TokenName tkns[] = il;
//    return std::unordered_set< TokenName >(tkns, tkns + sizeof(tkns) /
//                                                            sizeof(tkns[0]));
//}

static decltype(auto) initOverloadingOperators()
{
    std::vector< std::vector< TokenName > > operators = {
        {TokenName::Plus},
        {TokenName::Minus},
        {TokenName::Star},
        {TokenName::Slash},
        {TokenName::Percent},
        {TokenName::Caret},
        {TokenName::Ampersand},
        {TokenName::VerticalBar},
        {TokenName::Tilde},
        {TokenName::Exclamation},
        {TokenName::Comma},
        {TokenName::Equals},
        {TokenName::Less},
        {TokenName::Greater},
        {TokenName::LessEquals},
        {TokenName::GreaterEquals},
        {TokenName::DoublePlus},
        {TokenName::DoubleMinus},
        {TokenName::DoubleLess},
        {TokenName::Greater, TokenName::Greater},
        {TokenName::DoubleEquals},
        {TokenName::ExclamationEqual},
        {TokenName::DoubleAmpersand},
        {TokenName::DoubleVerticalBar},
        {TokenName::PlusEquals},
        {TokenName::MinusEquals},
        {TokenName::SlashEquals},
        {TokenName::PercentEquals},
        {TokenName::CaretEquals},
        {TokenName::AmpersandEquals},
        {TokenName::VerticalBarEquals},
        {TokenName::StarEquals},
        {TokenName::DoubleLessEquals},
        {TokenName::DoubleGreaterEquals},
        {TokenName::BracketSquareLeft, TokenName::BracketSquareRight},
        {TokenName::BracketLeft, TokenName::BracketRight},
        {TokenName::Arrow},
        {TokenName::Arrow, TokenName::Star},
        {TokenName::New},
        {TokenName::Delete},
        {TokenName::New, TokenName::BracketSquareLeft,
         TokenName::BracketCurlyRight},
        {TokenName::Delete, TokenName::BracketSquareLeft,
         TokenName::BracketSquareRight}};

    TreeNode< TokenName > root;
    for (auto &op : operators)
        root.insertRev(op.data(), op.size());

    return root;
}

bool SourceParser::parseScopedName(const SourceParser::TokenVector &v,
                                   int start, int end, SplittedPath &name)
{
    bool inserted = false;
    for (; start < end; ++start) {
        const Token &token = v[start];

        switch (token.name) {
        case TokenName::Identifier:
            name.append(token.lexeme_str());
            inserted = true;
            break;
        case TokenName::Final:
        case TokenName::DoubleColon:
            break;
        case TokenName::Operator:
            // exactly operator's type doesn't matter
            name.append(ttos(TokenName::Operator));
            inserted = true;
            return true;
        case TokenName::Less:
            skipTemplate(v, start);
            --start;
            break;
        default:
            return false;
        }
    }
    return inserted;
}

int SourceParser::getIdentifierStart(const SourceParser::TokenVector &v,
                                     int offset) const
{
    --offset;
    skipOperatorOverloadingReverse(v, offset);
    skipTemplateReverse(v, offset);

    static const auto name_type =
        TokenNameSet({TokenName::Identifier, TokenName::Operator,
                      TokenName::DoubleColon, TokenName::Final});

    for (; offset > 0; --offset) {
        if (name_type.find(v[offset - 1].name) == name_type.end() ||
            v[offset - 1].name == v[offset].name) {
            return offset;
        }
    }
    return 0;
}

void SourceParser::skipOperatorOverloadingReverse(
    const SourceParser::TokenVector &v, int &offset) const
{
    static auto treeOverloadingOperator = initOverloadingOperators();
    std::vector< TokenName > revOperator;
    auto node = &treeOverloadingOperator;
    auto i = offset;

    for (; i >= 0; --i) {
        if (auto tmp = node->find(v[i].name)) {
            node = tmp;
            continue;
        }
        break;
    }
    if (v[i].name == TokenName::Operator && node->finite)
        offset = i;
}

void SourceParser::skipLine(const SourceParser::TokenVector &tokens,
                            int &offset)
{
    int line = tokens[offset].n_line;
    int sizeM1 = tokens.size() - 1;
    for (; offset < sizeM1;) {
        if (tokens[offset].name == TokenName::Backslash)
            line = tokens[offset + 1].n_line;

        increment(tokens, offset);
        if (tokens[offset].n_line > line)
            return;
    }
    increment(tokens, offset);
}

void SourceParser::skipTemplate(const SourceParser::TokenVector &tokens,
                                int &offset)
{
    int lbcount = 0;
    do {
        const Token &token = tokens[offset];
        increment(tokens, offset);
        switch (token.name) {
        case TokenName::Less:
            ++lbcount;
            break;
        case TokenName::Greater:
            --lbcount;
            break;
        default:
            break;
        }
    } while (lbcount > 0);
}

void SourceParser::skipTemplateReverse(const SourceParser::TokenVector &tokens,
                                       int &offset) const
{
    if (tokens[offset].name != TokenName::Greater)
        return;
    int lbcount = 0;
    do {
        const Token &token = tokens[offset--];
        switch (token.name) {
        case TokenName::Less:
            --lbcount;
            break;
        case TokenName::Greater:
            ++lbcount;
            break;
        default:
            break;
        }
        assert(offset >= 0);
    } while (lbcount > 0);
}

void SourceParser::prepare()
{
    _openBracketCount = 0;
    _openCurlyBracketCount = 0;
    _namespaceDeep = 0;
    _stackNamespaceBrackets.clear();

    _currentNamespace.clear();
    _listUsingNamespace.clear();
}

TokenName SourceParser::readUntil(const SourceParser::TokenVector &tokens,
                                  int &offset, const TokenNameSet &tokenNames)
{
    for (; offset < tokens.size(); increment(tokens, offset)) {
        auto it = tokenNames.find(tokens[offset].name);
        if (it != tokenNames.end())
            return *it;
    }
    assert(false);
    return TokenName::Undefined;
}

void SourceParser::increment(const TokenVector &tokens, int &offset)
{
    const Token &token = tokens[offset];
    switch (token.name) {
    case TokenName::BracketCurlyLeft:
        ++_openCurlyBracketCount;
        break;
    case TokenName::BracketCurlyRight:
        assert(_openCurlyBracketCount > 0);
        --_openCurlyBracketCount;
        if (_stackNamespaceBrackets.isOnTop(_openCurlyBracketCount)) {
            _stackNamespaceBrackets.pop();
            --_namespaceDeep;
            _currentNamespace.removeLast();
        }
        break;
    case TokenName::BracketLeft:
        ++_openBracketCount;
        break;
    case TokenName::BracketRight:
        --_openBracketCount;
        break;
    default:
        break;
    }
    ++offset;
}

bool SourceParser::isTopLevelCB() const
{
    return _namespaceDeep == _openCurlyBracketCount;
}

void SourceParser::setNamespace()
{
    ++_namespaceDeep;
    _stackNamespaceBrackets.push(_openCurlyBracketCount);
}

void SourceParser::dealWithClassDeclaration(
    const SourceParser::TokenVector &tokens, int offset)
{
    ScopedName className = _currentNamespace;
    auto fstart = getIdentifierStart(tokens, offset);
    if (parseScopedName(tokens, fstart, offset, className)) {
        if (isClassToken(tokens, fstart - 1) ||
            isClassToken(tokens, fstart - 2)) {
            // class/struct declaration
            _node->record()._setClassDecl.insert(className);
        }
        else if (isInheritanceToken(tokens, fstart - 1)) {
            // class/struct inheritance
            _node->record()._setInheritances.insert(className);
            int nextOffset = fstart - 1;
            if (nextOffset >= 0) {
                switch (tokens[nextOffset].name) {
                case TokenName::Comma:
                case TokenName::DoubleColon:
                    dealWithClassDeclaration(tokens, nextOffset);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

bool SourceParser::checkOffset(const SourceParser::TokenVector &tokens,
                               int offset)
{
    return offset >= 0 && offset < tokens.size();
}

bool SourceParser::isClassToken(const SourceParser::TokenVector &tokens,
                                int offset)
{
    if (!checkOffset(tokens, offset))
        return false;
    return tokens[offset].isClass();
}

bool SourceParser::isInheritanceToken(const SourceParser::TokenVector &tokens,
                                      int offset)
{
    if (!checkOffset(tokens, offset))
        return false;
    return tokens[offset].isInheritance();
}

SourceParser::SourceParser(const FileTree &ftree) : _fileTree(ftree)
{
    _currentNamespace.setNamespaceSeparator();
}

void SourceParser::parseFile(FileNode *node)
{
    if (!node->isSourceFile())
        return;
    _node = node;

    Tokenizer tkn;
    const SplittedPath &filename = node->fullPath();
    tkn.tokenize(filename);

    const auto &tokens = tkn.tokens();

    prepare();

    for (int i = 0; i < tokens.size();) {
        if (!isTopLevelCB()) {
            increment(tokens, i);
            continue;
        }
        switch (tokens[i].name) {
        case TokenName::Hash:
            // check if include directive
            if (tokens[i + 1].name == TokenName::Include) { // include directive
                const Token *curr_tok = tokens.data() + i + 2;
                const Token &textualToken = *curr_tok;
                IncludeDirective dir;
                bool error = false;
                if (textualToken.name == TokenName::String) {
                    dir.type = IncludeDirective::Quotes;
                    dir.filename = std::string(textualToken.lexeme + 1,
                                               textualToken.length - 2);
                }
                else if (textualToken.name == TokenName::Less) {
                    dir.type = IncludeDirective::Brackets;
                    const Token &fnameToken = *(++curr_tok);
                    if (fnameToken.name != TokenName::Identifier)
                        error = true;
                    else
                        dir.filename = fnameToken.lexeme_str();
                }
                else {
                    error = true;
                }
                if (error) {
                    errors() << "warning: skip include directive in file"
                             << filename.jointOs() << "on the line"
                             << ntos(tokens[i].n_line);
                }
                else {
                    if (FileNode *includedFile =
                            _fileTree.searchIncludedFile(dir, node))
                        node->record()._listIncludes.push_back(dir);
                }
            }
            skipLine(tokens, i);
            continue;
        case TokenName::BracketLeft: {
            if (_openBracketCount > 0)
                break;
            ScopedName funcName = _currentNamespace;
            auto fstart = getIdentifierStart(tokens, i);
            if (parseScopedName(tokens, fstart, i, funcName)) {
                static const TokenNameSet typeTokens = {
                    TokenName::BracketCurlyLeft, TokenName::Semicolon};
                auto t = readUntil(tokens, i, typeTokens);
                switch (t) {
                case TokenName::BracketCurlyLeft:
                    // impl function/method
                    node->record()._setImplements.insert(funcName);
                    break;
                case TokenName::Semicolon:
                    // global function decl
                    node->record()._setFuncDecl.insert(funcName);
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            break;
        }
        case TokenName::BracketCurlyLeft: {
            dealWithClassDeclaration(tokens, i);
            break;
        }
        case TokenName::Using:
            increment(tokens, i);
            if (tokens[i].name == TokenName::Namespace) {
                // using namespace ->ns
                ScopedName ns;
                ns.setNamespaceSeparator();
                increment(tokens, i);
                parseScopedName(tokens, i, tokens.size(), ns);
                if (!ns.empty())
                    node->record()._listUsingNamespace.push_back(ns);
            }
            break;
        case TokenName::Namespace:
            increment(tokens, i);
            parseScopedName(tokens, i, tokens.size(), _currentNamespace);
            setNamespace();
            break;
        default:
            break;
        }
        increment(tokens, i);
    }
}
