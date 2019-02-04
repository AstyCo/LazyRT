#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP

#include "types/file_tree.hpp"

#include <list>

class FileSystem
{
public:
    FileSystem();

    void setProjectDirectory(const SplittedPath &path);
    void setIncludePaths(const std::vector< SplittedPath > &includePaths);

    void installIncludeProjectDir();
    void installExtraDependencies(const std::string &extra_dependencies);
    void installAffectedFiles();

    void analyzePhase();

    FileNode *
    searchInIncludepaths(const SplittedPath &path,
                         const std::vector< SplittedPath > &includePaths);
    FileNode *search(const SplittedPath &fullpath);

    void clearVisitedLabels();

    FileTree srcTree;
    FileTree testTree;
    FileTree extraDepsTree;

    std::vector< FileTree * > _trees;
    std::vector< FileTree * > _treesToAnalyze;

private:
    void propagateDeps();
};

namespace Debug {

void test_analyse_phase(FileSystem &trees);

} // namespace Debug

#endif // FILE_SYSTEM_HPP
