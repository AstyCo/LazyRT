#ifndef FILE_TREE_FOREST_HPP
#define FILE_TREE_FOREST_HPP

#include "types/file_tree.hpp"

#include <list>

class FileTreeForest
{
public:
    FileTreeForest();

    void setProjectDirectory(const SplittedPath &path);
    void installIncludeSources();
    void installExtraDependencies(const std::string &extra_dependencies);
    void installAffectedFiles();

    FileTree srcTree;
    FileTree testTree;
    FileTree extraDepsTree;

    std::list<FileTree*> _trees;
};

#endif // FILE_TREE_FOREST_HPP
