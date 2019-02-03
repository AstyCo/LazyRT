#ifndef DEPENDENCY_ANALYZER_HPP
#define DEPENDENCY_ANALYZER_HPP

#include "types/file_tree.hpp"

#include <map>

struct HashedStringNode
{
    typedef FileNode *FileNodePtr;
    typedef std::list< FileNodePtr > ExtraData;
    typedef ScopedName TSplittedString;
    typedef std::map< HashedString::HashType, HashedStringNode * > MapLeafs;

    HashedString hs;
    MapLeafs childs;

    ExtraData data;

    HashedStringNode(const HashedString &hs_);
    ~HashedStringNode();

    void insert(const TSplittedString &splittedString, FileNodePtr fnode);

    HashedStringNode *findOrNew(const HashedString &key);

    HashedStringNode *find(const HashedString &key) const;
    HashedStringNode *findSplitted(const TSplittedString &splittedString);

    /// DEBUG
    TSplittedString fullname() const;
    HashedStringNode *parent;
    void print(int indent = 0);
    ///
};

class DependencyAnalyzer
{
public:
    DependencyAnalyzer();

    void analyze(FileNode *fnode);

public:
    /// DEBUG
    void print();
    ///
private:
    void analyzeImpl(const ScopedName &impl, FileNode *fnode);
    void analyzeInheritance(const ScopedName &baseClass, FileNode *fnode);

    HashedStringNode *findClassForMethod(const ScopedName &impl,
                                         FileNode *fnode,
                                         HashedStringNode *hsnode);
    HashedStringNode *findClass(const ScopedName &impl, FileNode *fnode,
                                HashedStringNode *hsnode);

    enum SearchType { SearchClass, SearchMethod };

    HashedStringNode *findScopedPrivate(const ScopedName &impl, FileNode *fnode,
                                        HashedStringNode *hsnode,
                                        SearchType st);

    void addFunctionImpl(FileNode *implNode, HashedStringNode *hsnode);
    void addClassImpl(FileNode *implNode, HashedStringNode *hsnode);
    void addClassInheritance(FileNode *implNode, HashedStringNode *hsnode);

    void readDecls(FileNode *fnode);

    void analyzeDecls(FileNode *fnode);

    HashedStringNode _rootClassDecls;
    HashedStringNode _rootFuncDecls;
};

#endif // DEPENDENCY_ANALYZER_HPP
