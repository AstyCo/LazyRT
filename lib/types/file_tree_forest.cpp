#include "file_tree_forest.hpp"

#include "json_reader.hpp"

FileTreeForest::FileTreeForest()
{
    _trees.push_back(&srcTree);
    _trees.push_back(&testTree);
    _trees.push_back(&extraDepsTree);
}

void FileTreeForest::setProjectDirectory(const SplittedPath &path)
{
    for (FileTree *p : _trees)
        p->setProjectDirectory(path);
}

void FileTreeForest::installIncludeSources()
{
    if (srcTree._rootDirectoryNode)
        testTree._includePaths.push_back(srcTree._rootDirectoryNode);
}

void FileTreeForest::installExtraDependencies(
    const std::string &extra_dependencies)
{
    if (extra_dependencies.empty())
        return;
    JsonReader jr;
    jr.read_extra_dependencies(extra_dependencies, extraDepsTree);

    extraDepsTree.removeEmptyDirectories();
    extraDepsTree.calculateFileHashes();
}

void FileTreeForest::installAffectedFiles()
{
    for (FileTree *p : _trees)
        p->installAffectedFiles();
}
