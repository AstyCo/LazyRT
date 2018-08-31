#ifndef DEPENDENCY_ANALYZER_HPP
#define DEPENDENCY_ANALYZER_HPP

#include "types/file_tree.hpp"

#include <map>

struct HashedStringNode
{
    typedef std::map<HashedString::HashType, HashedStringNode *> MapLeafs;

    HashedString string;
    MapLeafs childs;


};

class DependencyAnalyzer
{
public:

};


#endif // DEPENDENCY_ANALYZER_HPP
