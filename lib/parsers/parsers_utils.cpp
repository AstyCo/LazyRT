#include "parsers_utils.hpp"

#include "extensions/help_functions.hpp" // makeIndents

static CharTreeNode initOverloadingOperators()
{
    CharTreeNode root;
    const char
        *const operators[] =
            {"+",     "-",       "*",  "/",  "%",  "^",   "&",   "|",
             "~",     "!",       ",",  "=",  "<",  ">",   "<=",  ">=",
             "++",    "--",      "<<", ">>", "==", "!=",  "&&",  "||",
             "+=",    "-=",      "/=", "%=", "^=", "&=",  "|=",  "*=",
             "<<=",   ">>=",     "[]", "()", "->", "->*", "new", "delete",
             "new[]", "delete[]"}; // table is taken from
                                   // https://www.tutorialspoint.com/cplusplus/cpp_overloading.htm
    for (auto &op : operators)
        root.insertRev(op);

    return root;
}

static CharTreeNode charTreeRevOverloadTokens = initOverloadingOperators();

CharTreeNode::CharTreeNode() : finite(false) {}

CharTreeNode::~CharTreeNode()
{
    std::cout << "~CharTreeNode map_size " << leafs.size() << std::endl;
    for (auto &n : leafs)
        delete n.second;
}

void CharTreeNode::insert(const std::__cxx11::string &key)
{
    CharTreeNode *node = this;
    for (auto it = key.begin(); it < key.end(); ++it)
        node = node->insertCh(*it);

    node->finite = true;
}

void CharTreeNode::insertRev(const std::__cxx11::string &key)
{
    auto rev_key = std::string(key.rbegin(), key.rend());
    insert(rev_key);
}

CharTreeNode *CharTreeNode::find(char ch) const
{
    auto it = leafs.find(ch);
    if (it != leafs.end()) {
        return it->second;
    }
    return nullptr;
}

CharTreeNode *CharTreeNode::insertCh(char ch)
{
    if (auto node = find(ch)) {
        // found
        return node;
    }

    CharTreeNode *leaf = new CharTreeNode();
    leafs.insert(std::make_pair(ch, leaf));
    return leaf;
}

const CharTreeNode *CharTreeNode::find(const char *s, size_t len) const
{
    const CharTreeNode *currentNode = this;
    for (size_t i = 0; i < len; ++i) {
        if ((currentNode = currentNode->find(s[i])) == nullptr)
            return nullptr;
    }
    return currentNode;
}

void Debug::printCharTreeR(const CharTreeNode *n, int indent)
{
    std::string strIndents = makeIndents(indent);

    for (auto &p : n->leafs) {
        std::cout << strIndents << p.first << ' ' << p.second->finite
                  << std::endl;
        printCharTreeR(p.second, indent + 1);
    }
}

void Debug::printCharTree(const CharTreeNode *n, int indent)
{
    std::string strIndents = makeIndents(indent);
    std::cout << strIndents << "CharTreeNode_ROOT" << std::endl;

    printCharTreeR(n, indent + 1);
}
