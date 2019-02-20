#ifndef PARSERS_UTILS_HPP
#define PARSERS_UTILS_HPP

#include <map>

template < typename T >
class TreeNode
{
public:
    using LeafMap = std::map< T, TreeNode * >;
    using LengthType = int;

public:
    TreeNode();

    ~TreeNode();

    void insert(const T *key, int length);
    void insertRev(const T *key, int length);

    TreeNode *find(T ch) const;
    TreeNode *insertCh(T ch);

    const TreeNode *find(const T *s, size_t len) const;

    bool hasChilds() const { return !leafs.empty(); }

public:
    bool finite;
    LeafMap leafs;
};

namespace Debug {

template < typename T >
void printCharTreeR(const TreeNode< T > *n, int indent = 1);
template < typename T >
void printCharTree(const TreeNode< T > *n, int indent = 1);

} // namespace Debug

#endif // PARSERS_UTILS_HPP
