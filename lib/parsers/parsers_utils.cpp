#include "parsers_utils.hpp"
#include "tokenizer.hpp"
#include "extensions/help_functions.hpp" // makeIndents

template < typename T >
TreeNode< T >::TreeNode() : finite(false)
{
}

template < typename T >
TreeNode< T >::~TreeNode()
{
    for (auto &n : leafs)
        delete n.second;
}

template < typename T >
void insert_g(TreeNode< T > *node, const T *key, int length, int inc)
{
    int first = (inc > 0 ? 0 : length - 1);
    int last = (inc > 0 ? length : -1);

    for (int i = first; i != last; i += inc)
        node = node->insertCh(key[i]);

    node->finite = true;
}

template < typename T >
void TreeNode< T >::insert(const T *key, int length)
{
    insert_g(this, key, length, 1);
}

template < typename T >
void TreeNode< T >::insertRev(const T *key, int length)
{
    insert_g(this, key, length, -1);
}

template < typename T >
TreeNode< T > *TreeNode< T >::find(T ch) const
{
    auto it = leafs.find(ch);
    if (it != leafs.end()) {
        return it->second;
    }
    return nullptr;
}

template < typename T >
TreeNode< T > *TreeNode< T >::insertCh(T ch)
{
    if (auto node = find(ch)) {
        // found
        return node;
    }

    TreeNode< T > *leaf = new TreeNode< T >();
    leafs.insert(std::make_pair(ch, leaf));
    return leaf;
}

template < typename T >
const TreeNode< T > *TreeNode< T >::find(const T *s, size_t len) const
{
    auto currentNode = this;
    for (size_t i = 0; i < len; ++i) {
        if ((currentNode = currentNode->find(s[i])) == nullptr)
            return nullptr;
    }
    return currentNode;
}

template < typename T >
void Debug::printCharTreeR(const TreeNode< T > *n, int indent)
{
    std::string strIndents = makeIndents(indent);

    for (auto &p : n->leafs) {
        std::cout << strIndents << p.first << ' ' << p.second->finite
                  << std::endl;
        printCharTreeR(p.second, indent + 1);
    }
}

template < typename T >
void Debug::printCharTree(const TreeNode< T > *n, int indent)
{
    std::string strIndents = makeIndents(indent);
    std::cout << strIndents << "CharTreeNode_ROOT" << std::endl;

    printCharTreeR(n, indent + 1);
}

// Explicit template specialization (so it can be defined in .cpp)
template class TreeNode< char >;
template class TreeNode< TokenName >;
