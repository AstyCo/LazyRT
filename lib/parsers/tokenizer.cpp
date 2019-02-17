#include "tokenizer.hpp"

#include <algorithm> // for_each

#include <cstring> // strncmp

static decltype(auto) initSymbolTree()
{
    CharTreeNode root;

    auto map_size = special_symbols.size();
    for (int i = 0; i < map_size; ++i)
        root.insert(special_symbols[i].second);

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
    static const CharTreeNode symbolsTree = initSymbolTree();
    std::string fname = path.jointOs();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if (!data) {
        errors() << "Failed to open the file"
                 << '\"' + std::string(fname) + '\"';
        return;
    }
    auto file_size = data_pair.second;

    TokenizerState state = TokenizerState::Default;
    char *word_start = nullptr;

    initDebug(fname);

    for (long offset = 0; offset < file_size; ++offset) {
        auto p = data + offset;
        int ch = *p;

        updateDebug(ch);

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
            auto tmp = symbolsTree.find(word_start, p - word_start + 1);
            if (tmp)
                continue;
            emplaceToken(Token(word_start, p - word_start));
            dealWithSpecialTokens(data, offset);
            updateState(data + offset, state, word_start);
            break;
        }
        default:
            assert(false);
        }
    }
}

const Tokenizer::TokenVector &Tokenizer::tokens() const { return _tokens; }

// Control characters:

// Numeric character references:
// 1) \ + up to 3 octal digits
// 2) \x + any number of hex digits
// 3) \u + 4 hex digits (Unicode BMP, new in C++11)
// 4) \U + 8 hex digits (Unicode astral planes, new in C++11)

static bool ishexdigit(char ch)
{
    return isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static int read_char(const char *data)
{
    if (*data == '\\') {
        const char fch = data[1];
        int offset = 1;
        if (isdigit(fch)) {
            // case 1)
            for (int i = 0; i < 3; ++i) {
                if (!isdigit(data[offset]))
                    break;
                ++offset;
            }
            return offset;
        }
        else if (fch == 'x') {
            // case 2)
            ++offset;
            while (ishexdigit(data[offset]))
                ++offset;
        }
        else if (fch == 'u') {
            ++offset;
            for (int i = 0; i < 4; ++i) {
                // TODO remove this cycle
                assert(ishexdigit(data[offset + i + offset]));
            }
            return offset + 4;
        }
        else if (fch == 'U') {
            ++offset;
            for (int i = 0; i < 8; ++i) {
                // TODO remove this cycle
                assert(ishexdigit(data[offset + i + offset]));
            }
            return offset + 8;
        }
        else {
            assert(false);
        }
    }
    else {
        return 1;
    }
}

void Tokenizer::dealWithSpecialTokens(const char *data, long &offset)
{
    assert(!_tokens.empty());
    Token &last_token = _tokens.back();
    switch (last_token.name) {
    case TokenName::SingleQuote: {
        long tmp = read_char(data + offset);
        assert(data[offset + tmp] == '\'');
        offset += tmp + 1;
        break;
    }
    case TokenName::DoubleQuote: {
        for (;;) {
            auto tmp = read_char(data + offset);
            if (tmp == 1 && data[offset] == '\"') {
                offset += tmp;
                break;
            }
            offset += tmp;
        }
        const char *token_start = last_token.lexeme;

        last_token.length = data + offset - token_start;
        last_token.name = TokenName::String;
        break;
    }
    case TokenName::DoubleSlash: {
        readUntil(data, offset, [](const char *data, long offset) {
            return (data[offset] == '\n' && data[offset - 1] != '\\');
        });
        ++offset;
        _tokens.pop_back(); // skip comments
        break;
    }
    case TokenName::SlashStar: {
        readUntil(data, offset, [](const char *data, long offset) {
            return strncmp(data + offset, "*/", 2) == 0;
        });
        offset += sizeof("*/");
        _tokens.pop_back(); // skip comments
        break;
    }
    default:
        return;
    }
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
    _n_line = 0;
    _n_char = 0;
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
        _n_char = 0;
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
