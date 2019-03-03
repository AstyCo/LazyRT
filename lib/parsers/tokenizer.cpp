#include "tokenizer.hpp"

#include <algorithm> // for_each

#include <cstring> // strncmp
#include <sstream> // stringstream

static decltype(auto) initSymbolTree()
{
    TreeNode< char > root;

    auto map_size = special_symbols.size();
    for (int i = 0; i < map_size; ++i) {
        auto str = special_symbols[i].second;
        root.insert(str, strlen(str));
    }

    return root;
}

static bool istextual(int ch)
{
    return isalpha(ch) || isdigit(ch) || ch == '_';
}

static TokenizerState new_state(int ch)
{
    if (istextual(ch))
        return TokenizerState::Textual;
    else if (isspace(ch))
        return TokenizerState::Default;
    else
        return TokenizerState::Symbol;
}

static void update_state(char *p, TokenizerState &state, char *&word_start)
{
    state = new_state(*p);
    word_start = p;
}

void Tokenizer::tokenize(const SplittedPath &path)
{
    static const auto symbolsTree = initSymbolTree();
    std::string fname = path.jointOs();
    _fileData = readFile(fname.c_str(), "r");

    char *data = _fileData.data.get();
    auto file_size = _fileData.size;
    if (!data) {
        errors() << "Failed to open the file"
                 << '\"' + std::string(fname) + '\"';
        return;
    }

    TokenizerState state = TokenizerState::Default;
    char *word_start = nullptr;

    initDebug(fname);

    for (long offset = 0; offset < file_size; increment(data, offset)) {
        auto p = data + offset;
        int ch = *p;

        switch (state) {
        case TokenizerState::Default:
            updateState(p, state, word_start);
            break;
        case TokenizerState::Textual:
            if (istextual(ch))
                continue;
            emplaceToken(Token(word_start, p - word_start));
            updateState(p, state, word_start);
            break;
        case TokenizerState::Symbol: {
            if (symbolsTree.find(word_start, p - word_start + 1))
                continue;
            auto tmp = symbolsTree.find(word_start, p - word_start);
            while (!tmp->finite) { // fix >>= (>) case
                --offset;
                tmp = symbolsTree.find(word_start, data + offset - word_start);
            }

            emplaceToken(Token(word_start, data + offset - word_start));
            dealWithSpecialTokens(data, offset, file_size);
            updateState(data + offset, state, word_start);
            break;
        }
        default:
            assert(false);
        }
    }
}

const Tokenizer::TokenVector &Tokenizer::tokens() const { return _tokens; }

int read_string(const char *data, char qch, long max)
{
    for (int offset = 0; offset < max; ++offset) {
        if (data[offset] == '\\')
            ++offset;
        else if (data[offset] == qch)
            return offset;
    }
    assert(false);
    return max;
}

void Tokenizer::dealWithSpecialTokens(const char *data, long &offset,
                                      long file_size)
{
    assert(!_tokens.empty());
    Token &last_token = _tokens.back();
    switch (last_token.name) {
    case TokenName::SingleQuote:
    case TokenName::DoubleQuote: {
        const char qch =
            ((last_token.name == TokenName::SingleQuote) ? '\'' : '\"');
        int str_len = read_string(data + offset, qch, file_size - offset);

        ++last_token.lexeme;
        last_token.length = str_len;
        last_token.name = TokenName::String;

        offset += str_len + 1;
        break;
    }
    case TokenName::DoubleSlash: {
        readUntil(data, offset, [](const char *data, long offset) {
            return (data[offset] == '\n' && data[offset - 1] != '\\');
        });
        increment(data, offset);
        _tokens.pop_back(); // skip comments
        break;
    }
    case TokenName::SlashStar: {
        readUntil(data, offset, [](const char *data, long offset) {
            return strncmp(data + offset, "*/", 2) == 0;
        });

        increment_n(data, offset, sizeof("*/") - 1);
        _tokens.pop_back(); // skip comments
        break;
    }
    default:
        return;
    }
}

void Tokenizer::increment_n(const char *data, long &offset, int n)
{
    for (int i = 0; i < n; ++i)
        increment(data, offset);
}

void Tokenizer::emplaceToken(Token &&token)
{
    _tokens.emplace_back(std::forward< Token >(token));

    Token &emplaced_token = _tokens.back();

    emplaced_token.filename = _filename;
    emplaced_token.n_line = _n_line;
    emplaced_token.n_char = _n_char - emplaced_token.length;
}

void Tokenizer::initDebug(const std::__cxx11::string &fname)
{
    _filename = fname;
    _n_line = 1;
    _n_char = 1;
    _n_token_line = 0;
    _n_token_char = 0;
}

void Tokenizer::updateDebug(int ch)
{
    if (ch == '\n') {
        ++_n_token_line;
        _n_token_char = 0;
    }
    else {
        ++_n_token_char;
    }
}

void Tokenizer::updateState(char *p, TokenizerState &state, char *&word_start)
{
    update_state(p, state, word_start);
    newTokenDebug();
}

void Tokenizer::newTokenDebug()
{
    if (_n_token_line > 0) {
        _n_line += _n_token_line;
        _n_char = 1;
    }
    else {
        _n_char += _n_token_char;
    }
    _n_token_char = 0;
    _n_token_line = 0;
}

Token::Token(const char *p, LengthType size)
    : name(TokenName::Undefined), lexeme(p), length(size)
{
    name = tok(lexeme, length);
}

bool Token::isClass() const
{
    return name == TokenName::Class || name == TokenName::Struct;
}

bool Token::isInheritance() const
{
    return name == TokenName::Private || name == TokenName::Protected ||
           name == TokenName::Public;
}

bool Token::isKeyWord() const { return key_words.findLexeme(name) != nullptr; }

bool Token::isEndif() const
{
    return name == TokenName::Identifier && str_equal(lexeme_str(), "endif");
}

bool Token::isElseMacro() const
{
    return name == TokenName::Identifier && str_equal(lexeme_str(), "else");
}

bool Token::isIfMacro() const
{
    return name == TokenName::Identifier && str_equal(lexeme_str(), "if");
}

bool Token::isIfdef() const
{
    return name == TokenName::Identifier && str_equal(lexeme_str(), "ifdef");
}

bool Token::isElif() const
{
    return name == TokenName::Identifier && str_equal(lexeme_str(), "elif");
}

std::__cxx11::string Token::toString() const
{
    std::string str;
    if (name == TokenName::Identifier || name == TokenName::String)
        str = lexeme_str();
    else
        str = ttos(name);
    std::stringstream ss;
    ss << '\'' << str << '\'' << " file: " << filename << " line: " << n_line
       << " offset: " << n_char;
    return ss.str();
}

void Debug::printTokens(const std::vector< Token > &tokens)
{
    int line = 0;
    std::for_each(tokens.begin(), tokens.end(), [&line](const Token &t) {
        if (t.n_line > line) {
            ++line;
            std::cout << std::endl;
        }
        std::cout << strToken(t) << std::endl;
    });
}

std::__cxx11::string Debug::strToken(const Token &token)
{
    std::stringstream ss;
    ss << std::string(token.lexeme, token.length)
       << " Type: " << ttos(token.name);
    return ss.str();
}

std::__cxx11::string ttos(const TokenName &t)
{
    ValueType cstr;
    if ((cstr = key_words.findLexeme(t)))
        return cstr;
    if ((cstr = special_symbols.findLexeme(t)))
        return cstr;
    switch (t) {
    case TokenName::Identifier:
        return "Identifier";
    case TokenName::Comment:
        return "Comment";
    case TokenName::String:
        return "String";
    default:
        assert(false);
        return std::string();
    }
}
