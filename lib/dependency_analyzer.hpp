#ifndef DEPENDENCY_ANALYZER_HPP
#define DEPENDENCY_ANALYZER_HPP

#include "types/file_tree.hpp"

#include <map>

struct HashedStringNode
{
    typedef std::map<HashedString::HashType, HashedStringNode *> MapLeafs;

    HashedString str;
    MapLeafs childs;

//    void insert(const Sc)
//    HashedStringNode *find(const ScopedName &listHS) const;
};

class DependencyAnalyzer
{
public:

};


#endif // DEPENDENCY_ANALYZER_HPP
