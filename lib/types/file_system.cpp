#include "file_system.hpp"

#include "json_reader.hpp"

FileSystem::FileSystem()
{
    _trees.push_back(&srcTree);
    _trees.push_back(&testTree);
    _trees.push_back(&extraDepsTree);

    _treesToAnalyze.push_back(&srcTree);
    _treesToAnalyze.push_back(&testTree);

    for (FileTree *pTree : _trees)
        pTree->setFileSystem(this);
}

void FileSystem::setProjectDirectory(const SplittedPath &path)
{
    for (FileTree *p : _trees)
        p->setProjectDirectory(path);
}

void FileSystem::setIncludePaths(
    const std::vector< SplittedPath > &includePaths)
{
    for (const auto &path : includePaths) {
        for (FileTree *pTree : _trees) {
            pTree->_includePaths.push_back(path);
        }
    }
}

void FileSystem::installIncludeProjectDir()
{
    srcTree._includePaths.push_back(srcTree._projectDirectory);
    testTree._includePaths.push_back(testTree._projectDirectory);
}

void FileSystem::installExtraDependencies(const std::string &extra_dependencies)
{
    if (extra_dependencies.empty())
        return;
    JsonReader jr;
    jr.read_extra_dependencies(extra_dependencies, extraDepsTree);

    extraDepsTree.removeEmptyDirectories();
    extraDepsTree.calculateFileHashes();
}

void FileSystem::installAffectedFiles()
{
    for (FileTree *p : _trees)
        p->installAffectedFiles();
}

namespace Debug {

// This function checks whether set "dependencies" and set "dependent by" match
static void test_dependency_dependentyBy_match(FileNode *fnode)
{
    if (fnode == nullptr)
        return;
    for (auto &&file : fnode->_setDependencies) {
        if (file->_setDependentBy.find(fnode) == file->_setDependentBy.end()) {
            std::cerr << "ERROR: FILE " << file->fullPath().joint() << " FNODE "
                      << fnode->fullPath().joint() << std::endl;
            file->_fileTree.print();
            fnode->_fileTree.print();
            MY_ASSERT(false);
        }
    }

    for (auto &&child : fnode->childs())
        test_dependency_dependentyBy_match(child);
}

static void
test_dependencies_recursion_h(FileNode *depNode,
                              const std::set< FileNode * > &totalDependencies,
                              std::set< FileNode * > &testedNodes)
{
    MY_ASSERT(depNode);

    for (auto &&dep : depNode->_setDependencies) {
        MY_ASSERT(totalDependencies.find(dep) != totalDependencies.end());

        // test is recursive also
        if (testedNodes.find(dep) == testedNodes.end()) {
            testedNodes.insert(dep);
            test_dependencies_recursion_h(dep, totalDependencies, testedNodes);
        }
    }
}

// Checks whether dependency set contains the dependencies of dependencies
static void test_dependencies_recursion(FileNode *fnode)
{
    MY_ASSERT(fnode);
    const auto &deps = fnode->_setDependencies;

    std::set< FileNode * > testedNodes;
    testedNodes.insert(fnode);

    for (auto &&file : fnode->_setDependencies)
        test_dependencies_recursion_h(file, deps, testedNodes);

    for (auto &&child : fnode->childs())
        test_dependencies_recursion(child);
}

static void test_dependencies_recursion(FileTree &ftree)
{
    test_dependencies_recursion(ftree._rootDirectoryNode);
}

static void test_dependency_dependentyBy_match(FileTree &ftree)
{
    test_dependency_dependentyBy_match(ftree._rootDirectoryNode);
}

void test_analyse_phase(FileSystem &trees)
{
    std::array< FileTree *, 2 > treesToAnalyze = {&trees.srcTree,
                                                  &trees.testTree};

    for (auto pTree : treesToAnalyze) {
        Debug::test_dependency_dependentyBy_match(*pTree);
        Debug::test_dependencies_recursion(*pTree);
    }
}

} // namespace Debug

void FileSystem::analyzePhase()
{
    for (auto pTree : _treesToAnalyze)
        pTree->analyzeNodes();

    propagateDeps();

    DEBUG(Debug::test_analyse_phase(*this));
}

FileNode *FileSystem::searchInIncludepaths(
    const SplittedPath &path, const std::vector< SplittedPath > &includePaths)
{
    for (auto includePath : includePaths) {
        SplittedPath fullpath = includePath + path;
        if (FileNode *foundNode = search(fullpath))
            return foundNode;
    }
    return nullptr;
}

FileNode *FileSystem::search(const SplittedPath &fullpath)
{
    for (FileTree *fileTree : _trees) {
        const SplittedPath &rootPath = fileTree->rootPath();
        bool error = false;
        SplittedPath rel_path = relative_path(fullpath, rootPath, &error);
        if (error)
            continue;
        if (fileTree->_rootDirectoryNode == nullptr)
            continue;
        FileNode *file = fileTree->_rootDirectoryNode->search(rel_path);
        if (file)
            return file;
    }
    return nullptr;
}

void FileSystem::clearVisitedLabels()
{
    for (FileTree *pTree : _treesToAnalyze)
        pTree->clearVisitedR();
}

void FileSystem::propagateDeps()
{
    for (FileTree *pTree : _treesToAnalyze)
        pTree->installDependencies();

    for (FileTree *pTree : _treesToAnalyze)
        pTree->installDependentBy();
}