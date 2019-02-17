#ifndef PARSERS_UTILS_HPP
#define PARSERS_UTILS_HPP

#include <map>

class CharTreeNode
{
public:
    using LeafMap = std::map< char, CharTreeNode * >;

public:
    CharTreeNode();

    ~CharTreeNode();

    void insert(const std::string &key);
    void insertRev(const std::string &key);

    CharTreeNode *find(char ch) const;
    CharTreeNode *insertCh(char ch);

    const CharTreeNode *find(const char *s, size_t len) const;

    bool hasChilds() const { return !leafs.empty(); }

public:
    bool finite;
    LeafMap leafs;
};

namespace Debug {

void printCharTreeR(const CharTreeNode *n, int indent = 1);
void printCharTree(const CharTreeNode *n, int indent = 1);

} // namespace Debug

#endif // PARSERS_UTILS_HPP
