#include "types/file_tree.hpp"

#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"

#include "types/file_system.hpp"

#include "command_line_args.hpp"
#include "directoryreader.hpp"
#include "dependency_analyzer.hpp"

#include "flatbuffers_schemes/file_tree_generated.h"

#include <external/flatbuffers/flatbuffers.h>

#include <sstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

FileRecord::FileRecord(const SplittedPath &path, Type type)
    : _path(path), _type(type), _isModified(false), _isManuallyLabeled(false),
      _isHashValid(false)
{
    _path.setUnixSeparator();
}

void FileRecord::calculateHash(const SplittedPath &dir_base)
{
    auto data_pair = readBinaryFile((dir_base + _path).c_str());
    char *data = data_pair.first;
    if (!data) {
        char buff[1000];
        snprintf(buff, sizeof(buff),
                 "LazyUT: Error: File \"%s\" can not be opened",
                 (dir_base + _path).c_str());
        errors() << std::string(buff);

        //        MY_ASSERT(false);
        return;
    }
    MD5 md5((unsigned char *)data, data_pair.second);
    md5.copyResultTo(_hashArray);

    _isHashValid = true;
}

void FileRecord::setHash(const unsigned char *hash)
{
    MY_ASSERT(!_isHashValid);

    copyHashArray(_hashArray, hash);
    _isHashValid = true;
}

std::__cxx11::string FileRecord::hashHex() const
{
    if (!_isHashValid)
        return std::string();

    char buf[33];
    for (int i = 0; i < 16; i++)
        sprintf(buf + i * 2, "%02x", _hashArray[i]);
    buf[32] = 0;

    return std::string(buf);
}

void FileRecord::swapParsedData(FileRecord &record)
{
    _listIncludes.swap(record._listIncludes);
    _setImplements.swap(record._setImplements);

    _setClassDecl.swap(record._setClassDecl);
    _setFuncDecl.swap(record._setFuncDecl);

    _listUsingNamespace.swap(record._listUsingNamespace);
}

FileNode::FileNode(const SplittedPath &path, FileRecord::Type type,
                   FileTree &fileTree)
    : _record(path, type), _parent(nullptr), _visited(false),
      _fileTree(fileTree)
{
}

FileNode::~FileNode()
{
    list< FileNode * >::iterator it = _childs.begin();
    while (it != _childs.end()) {
        delete *it;
        ++it;
    }
}

void FileNode::addChild(FileNode *child)
{
    MY_ASSERT(child->parent() == nullptr);

    child->setParent(this);
    _childs.push_back(child);
}

FileNode *FileNode::findOrNewChild(const HashedFileName &hfname,
                                   FileRecord::Type type)
{
    if (FileNode *foundChild = findChild(hfname))
        return foundChild;
    // not found, create new one
    FileNode *newChild;
    if (parent())
        newChild = new FileNode(_record._path + hfname, type, _fileTree);
    else
        newChild = new FileNode(SplittedPath(hfname, SplittedPath::unixSep()),
                                type, _fileTree);
    addChild(newChild);

    return newChild;
}

void FileNode::removeChild(FileNode *child)
{
    child->setParent(nullptr);
    _childs.remove(child);
}

void FileNode::setParent(FileNode *parent) { _parent = parent; }

SplittedPath FileNode::fullPath() const
{
    return _fileTree.rootPath() + path();
}

std::string FileNode::relativeName(const SplittedPath &base) const
{
    return relative_path(fullPath(), base).joint();
}

void FileNode::print(int indent) const
{
    std::string strIndents = makeIndents(indent);

    std::cout << strIndents << "file:" << _record._path.joint();
    if (_record._type == FileRecord::RegularFile)
        std::cout << "\thex:[" << _record.hashHex() << "]";
    std::cout << std::endl;
    //    printInherits(indent);
    //    printInheritsFiles(indent);
    printDependencies(indent);
    printDependentBy(indent);
    printImpls(indent);
    printImplFiles(indent);
    printDecls(indent);
    printFuncImpls(indent);
    printClassImpls(indent);

    list< FileNode * >::const_iterator it = _childs.begin();
    while (it != _childs.end()) {
        (*it)->print(indent + 1);
        ++it;
    }
}

void FileNode::printModified(int indent, bool modified,
                             const SplittedPath &base) const
{
    std::string strIndents = makeIndents(indent, 2);

    if (isRegularFile() && (isModified() == modified)) {
        std::cout << strIndents << (modified ? " + " : " - ")
                  << relativeName(base) << std::endl;
    }

    for (auto f : _childs)
        f->printModified(indent + 1, modified, base);
}

void FileNode::printDecls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &decl : _record._setClassDecl)
        std::cout << strIndents << string("class decl: ") << decl.joint()
                  << std::endl;
    for (auto &decl : _record._setFuncDecl)
        std::cout << strIndents << string("function decl: ") << decl.joint()
                  << std::endl;
}

void FileNode::printImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setImplements)
        std::cout << strIndents << string("impl: ") << impl.joint()
                  << std::endl;
}

void FileNode::printImplFiles(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl_file : _record._setImplementFiles)
        std::cout << strIndents << string("impl_file: ") << impl_file.joint()
                  << std::endl;
}

void FileNode::printFuncImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setFuncImplFiles)
        std::cout << strIndents << string("impl func: ") << impl.joint()
                  << std::endl;
}

void FileNode::printClassImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setClassImplFiles)
        std::cout << strIndents << string("impl class: ") << impl.joint()
                  << std::endl;
}

void FileNode::printDependencies(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &file : _setDependencies) {
        if (file == this)
            continue; // don't print the file itself
        std::cout << strIndents << string("dependecy: ") << file->name()
                  << std::endl;
    }
}

void FileNode::printDependentBy(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &file : _setDependentBy) {
        if (file == this)
            continue; // don't print the file itself
        std::cout << strIndents << string("dependent by: ") << file->name()
                  << std::endl;
    }
}

void FileNode::printInherits(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &inh : _record._setInheritances)
        std::cout << strIndents << string("inherits: ") << inh.joint()
                  << std::endl;
}

void FileNode::printInheritsFiles(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &inh : _record._setBaseClassFiles)
        std::cout << strIndents << string("inh_file: ") << inh.joint()
                  << std::endl;
}

bool FileNode::hasRegularFiles() const
{
    if (_record._type == FileRecord::RegularFile)
        return true;
    FileNodeConstIterator it(_childs.begin());
    while (it != _childs.end()) {
        if ((*it)->hasRegularFiles())
            return true;
        ++it;
    }
    return false;
}

void FileNode::destroy()
{
    if (_parent)
        _parent->removeChild(this);
    delete this;
}

void FileNode::installIncludes(const FileTree &fileTree)
{
    for (auto &include_directive : _record._listIncludes) {
        if (FileNode *includedFile =
                fileTree.searchIncludedFile(include_directive, this))
            installExplicitDep(includedFile);
    }
}

void FileNode::installInheritances(const FileTree &fileTree)
{
    for (auto &file_path : _record._setBaseClassFiles) {
        if (FileNode *baseClassFile = fileTree.searchInRoot(file_path))
            installExplicitDep(baseClassFile);
    }
}

void FileNode::installImplements(const FileTree &fileTree)
{
    for (auto &file_path : _record._setImplementFiles) {
        if (FileNode *implementedFile = fileTree.searchInRoot(file_path))
            installExplicitDepBy(implementedFile);
    }
}

void FileNode::installDependencies()
{
    installDepsPrivate(&FileNode::_setDependencies,
                       &FileNode::_setExplicitDependencies);
}

void FileNode::installDependentBy()
{
    installDepsPrivate(&FileNode::_setDependentBy,
                       &FileNode::_setExplicitDependendentBy);
}

void FileNode::clearVisited() { _visited = false; }

FileNode *FileNode::search(const SplittedPath &path)
{
    FileNode *current_dir = this;
    for (auto &fname : path.splitted()) {
        if ((current_dir = current_dir->findChild(fname)))
            continue;
        return nullptr;
    }
    return current_dir;
}

void FileNode::installExplicitDep(FileNode *includedNode)
{
    MY_ASSERT(includedNode);
    _setExplicitDependencies.insert(includedNode);
    includedNode->_setExplicitDependendentBy.insert(this);
}

void FileNode::installExplicitDepBy(FileNode *implementedNode)
{
    MY_ASSERT(implementedNode);
    implementedNode->installExplicitDep(this);
}

void FileNode::swapParsedData(FileNode *file)
{
    MY_ASSERT(file); // TODO use this instead of copy
    _record.swapParsedData(file->_record);
}

bool FileNode::isAffected() const
{
    // if some dependency is affected then this is affected too
    // (this uses affected)
    for (FileNode *dep : _setDependencies) {
        if (dep->isThisAffected())
            return true;
    }
    // if some dependent by is affected then this is affected too
    // (affected uses this)
    for (FileNode *dep : _setDependentBy) {
        if (dep->isThisAffected())
            return true;
    }

    return false;
}

void FileNode::installDepsPrivate(
    FileNode::SetFileNode FileNode::*getSetDeps,
    const FileNode::SetFileNode FileNode::*getSetExplicitDeps)
{
    _fileTree._filesystem->clearVisitedLabels();

    installDepsPrivateR(this, getSetDeps, getSetExplicitDeps);

    // File allways depends on himself
    (this->*getSetDeps).insert(this);
}

template < typename T >
static void append(T &lhs, const T &rhs)
{
    lhs.insert(rhs.begin(), rhs.end());
}

void FileNode::installDepsPrivateR(
    FileNode *node, FileNode::SetFileNode FileNode::*getSetDeps,
    const FileNode::SetFileNode FileNode::*getSetExplicitDeps)
{
    if (node->_visited)
        return;
    node->_visited = true;

    SetFileNode &deps = this->*getSetDeps;
    const SetFileNode &nodeDeps = node->*getSetDeps;

    const SetFileNode &nodeExplicitDeps = node->*getSetExplicitDeps;

    if (!nodeDeps.empty()) {
        append(deps, nodeDeps);
        return;
    }

    append(deps, nodeExplicitDeps);

    for (FileNode *explDepsNode : nodeExplicitDeps)
        installDepsPrivateR(explDepsNode, getSetDeps, getSetExplicitDeps);
}

FileNode *FileNode::findChild(const HashedFileName &hfname) const
{
    for (auto child : _childs) {
        if (child->fname() == hfname)
            return child;
    }
    return nullptr;
}

FileTree::FileTree()
    : _rootDirectoryNode(nullptr), _srcParser(*this), _filesystem(nullptr),
      _state(Clean)
{
    _relativeBasePath.setUnixSeparator();
}

void FileTree::clean()
{
    if (_rootDirectoryNode)
        delete _rootDirectoryNode;

    _state = Clean;
    _rootPath = SplittedPath();
    setRootDirectoryNode(nullptr);
}

void FileTree::removeEmptyDirectories()
{
    MY_ASSERT(_state == Filled);
    removeEmptyDirectories(_rootDirectoryNode);
    _state = Filtered;
}

void FileTree::calculateFileHashes()
{
    MY_ASSERT(_state == Filtered);
    calculateFileHashes(_rootDirectoryNode);
    _state = CachesCalculated;
}

void FileTree::parseFiles()
{
    MY_ASSERT(_state == CachesCalculated);
    if (_rootDirectoryNode) {
        parseFilesRecursive(_rootDirectoryNode);
        installModifiedFiles(_rootDirectoryNode);
    }
}

void FileTree::installIncludeNodes()
{
    if (_rootDirectoryNode)
        installIncludeNodesRecursive(*_rootDirectoryNode);
}

void FileTree::installInheritanceNodes()
{
    if (_rootDirectoryNode)
        installInheritanceNodesRecursive(*_rootDirectoryNode);
}

void FileTree::installImplementNodes()
{
    if (_rootDirectoryNode)
        installImplementNodesRecursive(*_rootDirectoryNode);
}

void FileTree::clearVisitedR()
{
    if (_rootDirectoryNode)
        recursiveCall(*_rootDirectoryNode, &FileNode::clearVisited);
}

void FileTree::installDependencies()
{
    if (_rootDirectoryNode)
        recursiveCall(*_rootDirectoryNode, &FileNode::installDependencies);
}

void FileTree::installDependentBy()
{
    if (_rootDirectoryNode)
        recursiveCall(*_rootDirectoryNode, &FileNode::installDependentBy);
}

void FileTree::installAffectedFiles()
{
    MY_ASSERT(_affectedFiles.empty());
    if (_rootDirectoryNode)
        installAffectedFilesRecursive(_rootDirectoryNode);
}

void FileTree::parseModifiedFiles(const FileTree &restored_file_tree)
{
    MY_ASSERT(_state == CachesCalculated);
    compareModifiedFilesRecursive(_rootDirectoryNode,
                                  restored_file_tree._rootDirectoryNode);
    parseModifiedFilesRecursive(_rootDirectoryNode);
}

void FileTree::print() const
{
    if (!_rootDirectoryNode) {
        std::cout << "Empty FileTree" << std::endl;
    }
    else {
        _rootDirectoryNode->print();
    }
}

void FileTree::printModified(const SplittedPath &base) const
{
    if (nullptr == _rootDirectoryNode)
        return;
    std::cout << "MODIFIED FILES " << _rootPath.joint() << std::endl;
    _rootDirectoryNode->printModified(0, true, base);
}

FileNode *FileTree::addFile(const SplittedPath &path)
{
    const std::list< HashedFileName > &splittedPath = path.splitted();
    if (!_rootDirectoryNode) {
        setRootDirectoryNode(
            new FileNode(SplittedPath(".", SplittedPath::unixSep()),
                         FileRecord::Directory, *this));
    }
    FileNode *currentNode = _rootDirectoryNode;
    MY_ASSERT(_rootDirectoryNode);
    for (const HashedFileName &fname : splittedPath) {
        FileRecord::Type type =
            ((fname == splittedPath.back()) ? FileRecord::RegularFile
                                            : FileRecord::Directory);
        currentNode = currentNode->findOrNewChild(fname, type);
        MY_ASSERT(currentNode);
    }
    return currentNode;
}

void FileTree::setRootDirectoryNode(FileNode *node)
{
    delete _rootDirectoryNode;

    _rootDirectoryNode = node;
}

void FileTree::setProjectDirectory(const SplittedPath &path)
{
    _projectDirectory = path;
    _projectDirectory.setUnixSeparator();
    updateRelativePath();
}

void FileTree::setRootPath(const SplittedPath &sp)
{
    _rootPath = sp;
    _rootPath.setUnixSeparator();
    updateRelativePath();
}

FileTree::State FileTree::state() const { return _state; }

void FileTree::setState(const State &state) { _state = state; }

void FileTree::setFileSystem(FileSystem *fs)
{
    MY_ASSERT(_filesystem == nullptr);
    _filesystem = fs;
}

void FileTree::removeEmptyDirectories(FileNode *node)
{
    if (!node)
        return;
    if (node->record()._type == FileRecord::RegularFile)
        return;

    FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeIterator it(childs.begin());

    while (it != childs.end()) {
        removeEmptyDirectories(*it);

        if ((*it)->hasRegularFiles())
            ++it;
        else
            it = childs.erase(it);
    }
}

void FileTree::calculateFileHashes(FileNode *node)
{
    if (!node)
        return;
    if (node->record()._type == FileRecord::RegularFile)
        node->record().calculateHash(_rootPath);

    FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeIterator it(childs.begin());

    while (it != childs.end()) {
        calculateFileHashes(*it);

        ++it;
    }
}

void FileTree::parseModifiedFilesRecursive(FileNode *node)
{
    if (node->isRegularFile() && node->isModified())
        _srcParser.parseFile(node);

    for (auto child : node->childs())
        parseModifiedFilesRecursive(child);
}

void FileTree::compareModifiedFilesRecursive(FileNode *node,
                                             FileNode *restored_node)
{
    auto thisChilds = node->childs();
    if (node->isRegularFile()) {
        if (restored_node->isRegularFile() &&
            compareHashArrays(node->record()._hashArray,
                              restored_node->record()._hashArray)) {
            // md5 hash sums match
            node->copy(restored_node);
        }
        else {
            // md5 hash sums don't match
            node->setModified();
        }
    }
    for (auto child : thisChilds) {
        if (FileNode *restored_child =
                restored_node->findChild(child->fname())) {
            compareModifiedFilesRecursive(child, restored_child);
        }
        else {
            if (clargs.verbal()) {
                std::cout << "this " << node->name() << " child "
                          << child->fname() << " not found" << std::endl;
            }
            installModifiedFiles(child);
        }
    }
}

void FileTree::installModifiedFiles(FileNode *node)
{
    if (node->isRegularFile()) {
        node->setModified();
        return;
    }
    // else, if directory
    for (auto child : node->childs())
        installModifiedFiles(child);
}

void FileTree::parseFilesRecursive(FileNode *node)
{
    if (node->isRegularFile()) {
        _srcParser.parseFile(node);
        return;
    }
    // else, if directory
    for (auto child : node->childs())
        parseFilesRecursive(child);
}

void FileTree::installIncludeNodesRecursive(FileNode &node)
{
    node.installIncludes(*this);
    for (auto &child : node.childs())
        installIncludeNodesRecursive(*child);
}

void FileTree::installInheritanceNodesRecursive(FileNode &node)
{
    node.installInheritances(*this);
    for (auto &child : node.childs())
        installInheritanceNodesRecursive(*child);
}

void FileTree::installImplementNodesRecursive(FileNode &node)
{
    node.installImplements(*this);
    for (auto &child : node.childs())
        installImplementNodesRecursive(*child);
}

void FileTree::installAffectedFilesRecursive(FileNode *node)
{
    if (node->isAffected()) {
        SplittedPath tmp = _relativeBasePath;
        tmp.appendPath(node->path());

        tmp.setOsSeparator();
        _affectedFiles.push_back(tmp);
    }
    for (auto child : node->childs())
        installAffectedFilesRecursive(child);
}

void FileTree::analyzeNodes()
{
    DependencyAnalyzer dep;
    dep.analyze(_rootDirectoryNode);

    installImplementNodes();
    installIncludeNodes();
    installInheritanceNodes();
}

void FileTree::propagateDeps()
{
    installDependencies();
    installDependentBy();
}

FileNode *FileTree::searchIncludedFile(const IncludeDirective &id,
                                       FileNode *node) const
{
    MY_ASSERT(node);
    const SplittedPath path(id.filename, SplittedPath::unixSep());
    if (id.isQuotes()) {
        // start from current dir
        if (FileNode *result = searchInCurrentDir(path, node->parent()))
            return result;
        return searchInIncludePaths(path);
    }
    else {
        // start from include paths
        if (FileNode *result = searchInIncludePaths(path))
            return result;
        return searchInCurrentDir(path, node->parent());
    }
}

FileNode *FileTree::searchInCurrentDir(const SplittedPath &path,
                                       FileNode *dir) const
{
    MY_ASSERT(dir && dir->isDirectory());
    if (!dir)
        return nullptr;
    return dir->search(path);
}

FileNode *FileTree::searchInIncludePaths(const SplittedPath &path) const
{
    MY_ASSERT(_filesystem);
    return _filesystem->searchInIncludepaths(path, _includePaths);
}

FileNode *FileTree::searchInRoot(const SplittedPath &path) const
{
    return searchInCurrentDir(path, _rootDirectoryNode);
}

void FileTree::updateRelativePath()
{
    if (!_projectDirectory.empty() && !_rootPath.empty())
        _relativeBasePath = relative_path(_rootPath, _projectDirectory);
}

std::__cxx11::string IncludeDirective::toPrint() const
{
    switch (type) {
    case Quotes:
        return '"' + filename + '"';
    case Brackets:
        return '<' + filename + '>';
    }
    return std::string();
}

void FileTreeFunc::readDirectory(FileTree &tree, const std::string &dirPath,
                                 const std::string &ignore_substrings)
{
    SplittedPath spOsSeparator(dirPath, SplittedPath::unixSep());
    spOsSeparator.setOsSeparator();

    DirectoryReader dirReader;
    dirReader._ignore_substrings = split(ignore_substrings, ",");
    dirReader.readDirectory(tree, spOsSeparator.joint());
}

void FileTreeFunc::parsePhase(FileTree &tree,
                              const std::__cxx11::string &dumpFileName)
{
    FileTree restoredTree;
    FileTreeFunc::deserialize(restoredTree, dumpFileName);
    if (restoredTree.state() == FileTree::Restored) {
        tree.parseModifiedFiles(restoredTree);
    }
    else {
        // if deserialization failed just parse all
        tree.parseFiles();
    }
}

void FileTreeFunc::printAffected(const FileTree &tree)
{
    std::cout << "FileTreeFunc::printAffected " << tree.rootPath().joint()
              << std::endl;

    std::string strIndents = makeIndents(0, 2);
    for (const auto &sp : tree._affectedFiles)
        std::cout << strIndents << sp.joint() << std::endl;
}

void FileTreeFunc::writeAffected(const FileTree &tree,
                                 const std::__cxx11::string &filename)
{
    /* open the file for writing*/
    if (FILE *fp = fopen(filename.c_str(), "w")) {
        for (const auto &sp : tree._affectedFiles)
            fprintf(fp, "%s\n", sp.joint().c_str());
        /* close the file*/
        fclose(fp);
    }
}

static bool containsMain(FileNode *file)
{
    static auto mainPrototype = std::string("main");

    const auto &impls = file->record()._setImplements;
    for (const auto &impl : impls) {
        if (impl.joint() == mainPrototype) {
            return true;
        }
    }
    // doesn't contain
    return false;
}

static void searchTestMainR(FileNode *file,
                            std::vector< FileNode * > &vTestMainFiles)
{
    if (vTestMainFiles.size() > 2)
        return;
    if (file->isRegularFile()) {
        if (containsMain(file))
            vTestMainFiles.push_back(file);
        return;
    }
    else {
        // directory
        for (auto childFile : file->childs())
            searchTestMainR(childFile, vTestMainFiles);
    }
}

static FileNode *searchTestMain(FileTree &testTree)
{
    std::vector< FileNode * > vTestMainFiles;
    vTestMainFiles.reserve(2);
    searchTestMainR(testTree._rootDirectoryNode, vTestMainFiles);
    // return file only if main() implemented in exactly one file
    if (vTestMainFiles.size() == 1)
        return vTestMainFiles.front();
    return nullptr;
}

void FileTreeFunc::labelMainAffected(FileTree &testTree)
{
    if (FileNode *testMainFile = searchTestMain(testTree))
        testMainFile->setLabeled();
}

static void writeModifiedR(const FileNode *file, FILE *fp)
{
    if (file == nullptr)
        return;

    if (file->isModified()) {
        SplittedPath tmp = file->path();
        tmp.setUnixSeparator();
        fprintf(fp, "%s\n", tmp.joint().c_str());
    }
    for (auto child : file->childs())
        writeModifiedR(child, fp);
}

void FileTreeFunc::writeModified(const FileTree &tree,
                                 const std::string &filename)
{
    /* open the file for writing*/
    if (FILE *fp = fopen(filename.c_str(), "w")) {
        writeModifiedR(tree._rootDirectoryNode, fp);
        /* close the file*/
        fclose(fp);
    }
}
