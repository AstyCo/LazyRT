#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "parsers_utils.hpp"

#include <types/splitted_string.hpp>

#include <unordered_set>
#include <vector>

using LengthType = uint16_t;

enum class TokenName {
    // Unreserved name
    Identifier,
    // Special Tokens
    Comment,
    String,

    // Key Words
    Auto,
    Class,
    Const,
    Constexpr,
    Decltype,
    Default,
    Delete,
    Explicit,
    Export,
    Extern,
    Final,
    Implicit,
    Include,
    Inline,
    Namespace,
    New,
    Operator,
    Private,
    Protected,
    Public,
    Struct,
    Template,
    Typedef,
    Typename,
    Union,
    Using,

    // Symbols
    Semicolon,           // --- ;
    Hash,                // --- #
    SingleQuote,         // --- '
    DoubleQuote,         // --- "
    Backslash,           // --- \    ---
    DoubleSlash,         // --- //
    SlashStar,           // --- /*
    StarSlash,           // --- */
    Colon,               // --- :
    DoubleColon,         // --- ::
    Dot,                 // --- .
    Arrow,               // --- ->
    BracketLeft,         // --- (
    BracketRight,        // --- )
    BracketSquareLeft,   // --- [
    BracketSquareRight,  // --- ]
    BracketCurlyLeft,    // --- {
    BracketCurlyRight,   // --- }
    Less,                // --- <
    Greater,             // --- >
    DoublePlus,          // --- ++
    DoubleMinus,         // --- --
    Ampersand,           // --- &
    DoubleAmpersand,     // --- &&
    VerticalBar,         // --- |
    DoubleVerticalBar,   // --- ||
    Tilde,               // --- ~
    Caret,               // --- ^
    Exclamation,         // --- !
    Plus,                // --- +
    Minus,               // --- -
    Equals,              // --- =
    DoubleEquals,        // --- ==
    ExclamationEqual,    // --- !=
    LessEquals,          // --- <=
    GreaterEquals,       // --- >=
    Slash,               // --- /
    Star,                // --- *
    Percent,             // --- %
    SlashEquals,         // --- /=
    StarEquals,          // --- *=
    PercentEquals,       // --- %=
    PlusEquals,          // --- +=
    MinusEquals,         // --- -=
    DoubleLessEquals,    // --- <<=
    DoubleGreaterEquals, // --- >>=
    AmpersandEquals,     // --- &=
    VerticalBarEquals,   // --- |=
    CaretEquals,         // --- ^=
    Question,            // --- ?
    Comma,               // --- ,

    Undefined
};

struct EnumClassHash
{
    template < typename T >
    std::size_t operator()(T t) const
    {
        return static_cast< std::size_t >(t);
    }
};
constexpr LengthType cstr_len(const char *s)
{
    assert(s);
    LengthType l = 0;
    while (*s++)
        ++l;
    return l;
}

constexpr void install_sz_for_null_terminated(const char *s, LengthType &sz)
{
    if (sz != static_cast< LengthType >(-1))
        return;       // use explicit size
    sz = cstr_len(s); // null terminated string
}

constexpr bool cstr_equal(const char *lhs, LengthType sz_lhs, const char *rhs,
                          LengthType sz_rhs)
{
    assert(lhs && rhs);
    install_sz_for_null_terminated(lhs, sz_lhs);
    install_sz_for_null_terminated(rhs, sz_rhs);

    if (sz_lhs != sz_rhs)
        return false;
    for (LengthType i = 0; i < sz_lhs; ++i) {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}
// common versions
constexpr bool cstr_equal(const char *lhs, LengthType sz_lhs, const char *rhs)
{
    return cstr_equal(lhs, sz_lhs, rhs, static_cast< LengthType >(-1));
}
constexpr bool cstr_equal(const char *lhs, const char *rhs, LengthType sz_rhs)
{
    return cstr_equal(lhs, static_cast< LengthType >(-1), rhs, sz_rhs);
}
constexpr bool cstr_equal(const char *lhs, const char *rhs)
{
    return cstr_equal(lhs, static_cast< LengthType >(-1), rhs,
                      static_cast< LengthType >(-1));
}

using ValueType = const char *;
using TokenPair = std::pair< TokenName, ValueType >;

template < int N >
constexpr decltype(auto) token_pairs_sz(const TokenPair (&map)[N])
{
    return sizeof(map) / sizeof(map[0]);
}

template < int N >
class TokenMap
{
    TokenPair _map[N];
    int _size;

public:
    constexpr TokenMap(const TokenPair (&map)[N]) : _size(N)
    {
        for (int i = 0; i < N; ++i) {
            _map[i].first = map[i].first;
            _map[i].second = map[i].second;
        }
    }

    constexpr ValueType findLexeme(TokenName key) const
    {
        return findLexemeR(key, N);
    }

    constexpr TokenName
    findToken(ValueType value,
              LengthType length = static_cast< LengthType >(-1)) const
    {
        return findTokenR(value, length, N);
    }

    decltype(auto) size() const { return _size; }
    constexpr const TokenPair &operator[](int i) const { return _map[i]; }

private:
    constexpr ValueType findLexemeR(TokenName key, int range) const
    {
        return (range == 0) ? nullptr
                            : (_map[range - 1].first == key)
                                  ? _map[range - 1].second
                                  : findLexemeR(key, range - 1);
    };

    constexpr TokenName findTokenR(ValueType value, LengthType length,
                                   int range) const
    {
        return (range == 0)
                   ? TokenName::Undefined
                   : (cstr_equal(_map[range - 1].second, value, length))
                         ? _map[range - 1].first
                         : findTokenR(value, length, range - 1);
    };
};

constexpr decltype(auto) makeKeyWords()
{
    constexpr TokenPair KWmap[] = {{TokenName::Auto, "auto"},
                                   {TokenName::Class, "class"},
                                   {TokenName::Const, "const"},
                                   {TokenName::Constexpr, "constexpr"},
                                   {TokenName::Decltype, "decltype"},
                                   {TokenName::Default, "default"},
                                   {TokenName::Delete, "delete"},
                                   {TokenName::Explicit, "explicit"},
                                   {TokenName::Export, "export"},
                                   {TokenName::Extern, "extern"},
                                   {TokenName::Final, "final"},
                                   {TokenName::Implicit, "implicit"},
                                   {TokenName::Include, "include"},
                                   {TokenName::Inline, "inline"},
                                   {TokenName::Namespace, "namespace"},
                                   {TokenName::New, "new"},
                                   {TokenName::Operator, "operator"},
                                   {TokenName::Private, "private"},
                                   {TokenName::Protected, "protected"},
                                   {TokenName::Public, "public"},
                                   {TokenName::Struct, "struct"},
                                   {TokenName::Template, "template"},
                                   {TokenName::Typedef, "typedef"},
                                   {TokenName::Typename, "typename"},
                                   {TokenName::Union, "union"},
                                   {TokenName::Using, "using"}};
    return TokenMap< token_pairs_sz(KWmap) >(KWmap);
}

constexpr decltype(auto) makeSymbols()
{
    constexpr TokenPair Smap[] = {{TokenName::Semicolon, ";"},
                                  {TokenName::Hash, "#"},
                                  {TokenName::SingleQuote, "\'"},
                                  {TokenName::DoubleQuote, "\""},
                                  {TokenName::Backslash, "\\"},
                                  {TokenName::DoubleSlash, "//"},
                                  {TokenName::SlashStar, "/*"},
                                  {TokenName::StarSlash, "*/"},
                                  {TokenName::Colon, ":"},
                                  {TokenName::DoubleColon, "::"},
                                  {TokenName::Dot, "."},
                                  {TokenName::Arrow, "->"},
                                  {TokenName::BracketLeft, "("},
                                  {TokenName::BracketRight, ")"},
                                  {TokenName::BracketSquareLeft, "["},
                                  {TokenName::BracketSquareRight, "]"},
                                  {TokenName::BracketCurlyLeft, "{"},
                                  {TokenName::BracketCurlyRight, "}"},
                                  {TokenName::Less, "<"},
                                  {TokenName::Greater, ">"},
                                  {TokenName::DoublePlus, "++"},
                                  {TokenName::DoubleMinus, "--"},
                                  {TokenName::Ampersand, "&"},
                                  {TokenName::DoubleAmpersand, "&&"},
                                  {TokenName::VerticalBar, "|"},
                                  {TokenName::DoubleVerticalBar, "||"},
                                  {TokenName::Tilde, "~"},
                                  {TokenName::Caret, "^"},
                                  {TokenName::Exclamation, "!"},
                                  {TokenName::Plus, "+"},
                                  {TokenName::Minus, "-"},
                                  {TokenName::Equals, "="},
                                  {TokenName::DoubleEquals, "=="},
                                  {TokenName::ExclamationEqual, "!="},
                                  {TokenName::LessEquals, "<="},
                                  {TokenName::GreaterEquals, ">="},
                                  {TokenName::Slash, "/"},
                                  {TokenName::Star, "*"},
                                  {TokenName::Percent, "%"},
                                  {TokenName::SlashEquals, "/="},
                                  {TokenName::StarEquals, "*="},
                                  {TokenName::PercentEquals, "%="},
                                  {TokenName::PlusEquals, "+="},
                                  {TokenName::MinusEquals, "-="},
                                  {TokenName::DoubleLessEquals, "<<="},
                                  {TokenName::DoubleGreaterEquals, ">>="},
                                  {TokenName::AmpersandEquals, "&="},
                                  {TokenName::VerticalBarEquals, "|="},
                                  {TokenName::CaretEquals, "^="},
                                  {TokenName::Question, "?"},
                                  {TokenName::Comma, ","}};
    return TokenMap< token_pairs_sz(Smap) >(Smap);
}

constexpr auto key_words = makeKeyWords();
constexpr auto special_symbols = makeSymbols();

static_assert(key_words.findToken(key_words.findLexeme(TokenName::Class)) ==
                  TokenName::Class,
              "inverse");
static_assert(special_symbols.findToken(
                  special_symbols.findLexeme(TokenName::Dot)) == TokenName::Dot,
              "inverse");

constexpr TokenName tok(const char *s,
                        LengthType length = static_cast< LengthType >(-1))
{
    install_sz_for_null_terminated(s, length);
    TokenName t = key_words.findToken(s, length);
    if (t == TokenName::Undefined)
        t = special_symbols.findToken(s, length);
    if (t == TokenName::Undefined)
        return TokenName::Identifier;
    return t;
}

static_assert(tok(".") == TokenName::Dot, "tok");

std::string ttos(const TokenName &t);

class Token
{
public:
    Token(const char *p, LengthType size);

    TokenName name;
    const char *lexeme;
    LengthType length;

    std::string lexeme_str() const { return std::string(lexeme, length); }
    // Debug
    std::string filename;
    uint16_t n_line;
    uint16_t n_char;

public:
    // common methods
    bool isClass() const;
    bool isInheritance() const;
    bool isKeyWord() const;
    bool isEndif() const;
    bool isElseMacro() const;
    bool isIfMacro() const;
    bool isIfdef() const;
    bool isElif() const;

    std::string toString() const;
};

enum class TokenizerState {
    Default,
    Textual, // Doesn't matter if number or identifier, so just 'Textual'
    Symbol
};

class Tokenizer
{
public:
    using TokenVector = std::vector< Token >;
    using StringSet = std::unordered_set< std::string >;

public:
    void tokenize(const SplittedPath &path);

    const TokenVector &tokens() const;

private:
    void dealWithSpecialTokens(const char *data, long &offset);
    void emplaceToken(Token &&tok);

    void increment(const char *data, long &offset)
    {
        updateDebug(data[offset]);
        ++offset;
    }
    void increment_n(const char *data, long &offset, int n);

private:
    TokenVector _tokens;
    FileData _fileData;

    // Debug
    void initDebug(const std::string &fname);
    void updateDebug(int ch);
    void updateState(char *p, TokenizerState &state, char *&word_start);
    void newTokenDebug();

    template < typename TFunc >
    void readUntil(const char *data, long &offset, TFunc f)
    {
        while (!f(data, offset))
            increment(data, offset);
    }

    std::string _filename;
    uint16_t _n_line;
    uint16_t _n_char;

    uint16_t _n_token_line;
    uint16_t _n_token_char;
};

namespace Debug {

std::string strToken(const Token &token);
void printTokens(const std::vector< Token > &tokens);

} // namespace Debug

#endif // TOKENIZER_HPP
