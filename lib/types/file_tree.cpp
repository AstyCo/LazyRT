#include "types/file_tree.hpp"
#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"

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
    : _path(path), _type(type), _isModified(false),
      _isManuallyLabeled(false), _isHashValid(false)
{
    _path.setOsSeparator();
}

void FileRecord::calculateHash(const SplittedPath &dir_base)
{
    auto data_pair = readBinaryFile((dir_base + _path).c_str());
    char *data = data_pair.first;
    if (!data) {
        std::cout << dir_base.joint() << std::endl;
        std::cout << _path.joint() << std::endl;
        std::cout << (dir_base + _path).joint() << std::endl;

        MY_ASSERT(false);
        return;
    }
    MD5 md5((unsigned char*)data, data_pair.second);
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
    for (int i=0; i<16; i++)
        sprintf(buf+i*2, "%02x", _hashArray[i]);
    buf[32]=0;

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

FileNode::FileNode(const SplittedPath &path, FileRecord::Type type)
    : _record(path, type), _parent(nullptr), _installDependenciesCalled(false)
{

}

FileNode::~FileNode()
{
    list<FileNode*>::iterator it = _childs.begin();
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

FileNode *FileNode::findOrNewChild(const HashedFileName &hfname, FileRecord::Type type)
{
    if (FileNode *foundChild = findChild(hfname))
        return foundChild;
    // not found, create new one
    FileNode *newChild;
    if (parent())
        newChild = new FileNode(_record._path + hfname, type);
    else
        newChild = new FileNode(hfname, type);
    addChild(newChild);

    return newChild;
}

void FileNode::removeChild(FileNode *child)
{
    child->setParent(nullptr);
    _childs.remove(child);
}

void FileNode::setParent(FileNode *parent)
{
    _parent = parent;
}

void FileNode::print(int indent) const
{
    std::string strIndents = makeIndents(indent);

    std::cout << strIndents << _record._path.joint();
    if (_record._type == FileRecord::RegularFile)
        std::cout << "\thex:[" <<  _record.hashHex() << "]";
    std::cout << std::endl;
//    printInherits(indent);
//    printInheritsFiles(indent);
    printDependencies(indent);
//    printDependentBy(indent);
    printImpls(indent);
    printImplFiles(indent);
//    printDecls(indent);
//    printFuncImpls(indent);
//    printClassImpls(indent);

    list<FileNode*>::const_iterator it = _childs.begin();
    while (it != _childs.end()) {
        (*it)->print(indent + 1);
        ++it;
    }
}

void FileNode::printDecls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &decl : _record._setClassDecl)
        std::cout << strIndents << string("class decl: ") << decl.joint() << std::endl;
    for (auto &decl : _record._setFuncDecl)
        std::cout << strIndents << string("function decl: ") << decl.joint() << std::endl;
}

void FileNode::printImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setImplements)
        std::cout << strIndents << string("impl: ") << impl.joint() << std::endl;
}

void FileNode::printImplFiles(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl_file: _record._setImplementFiles)
        std::cout << strIndents << string("impl_file: ") << impl_file.joint() << std::endl;
}

void FileNode::printFuncImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setFuncImplFiles)
        std::cout << strIndents << string("impl func: ") << impl.joint() << std::endl;
}

void FileNode::printClassImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setClassImplFiles)
        std::cout << strIndents << string("impl class: ") << impl.joint() << std::endl;
}

void FileNode::printDependencies(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &file : _setDependencies) {
        if (file == this)
            continue; // don't print the file itself
        std::cout << strIndents << string("dependecy: ") << file->name() << std::endl;
    }
}

void FileNode::printDependentBy(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &file : _setDependentBy) {
        if (file == this)
            continue; // don't print the file itself
        std::cout << strIndents << string("dependent by: ") << file->name() << std::endl;
    }
}

void FileNode::printInherits(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &inh : _record._setInheritances)
        std::cout << strIndents << string("inherits: ") << inh.joint() << std::endl;
}

void FileNode::printInheritsFiles(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &inh : _record._setBaseClassFiles)
        std::cout << strIndents << string("inh_file: ") << inh.joint() << std::endl;
}

void FileNode::installDepsPrivate(FileNode::SetFileNode FileNode::*deps, const FileNode::SetFileNode FileNode::*explicitDeps,
                                  /*const FileNode::ListFileNode FileNode::*impls, */bool FileNode::*called)
{
    this->*called = true;

    // File allways depends on himself
    (this->*deps).insert(this);

    for (auto node_ptr: (this->*explicitDeps))
        addDependencyPrivate(*node_ptr, deps, explicitDeps, called);
//    for (auto node_ptr: (this->*impls))
//        addDependencyPrivate(*node_ptr, deps, incls, impls, called);
}

bool FileNode::hasRegularFiles() const
{
    if (_record._type == FileRecord::RegularFile)
        return true;
    FileNodeConstIterator it(_childs.begin());
    while(it != _childs.end()) {
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
       if (FileNode *includedFile = fileTree.searchIncludedFile(include_directive, this))
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
                       &FileNode::_setExplicitDependencies,
                       &FileNode::_installDependenciesCalled);
}

void FileNode::installDependentBy()
{
    installDepsPrivate(&FileNode::_setDependentBy,
                       &FileNode::_setExplicitDependendentBy,
                       &FileNode::_installDependentByCalled);
}

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
    if (!includedNode) {
        MY_ASSERT(false);
        return;
    }
    _setExplicitDependencies.insert(includedNode);
    includedNode->_setExplicitDependendentBy.insert(this);
}

void FileNode::installExplicitDepBy(FileNode *implementedNode)
{
    if (!implementedNode) {
        MY_ASSERT(false);
        return;
    }
    implementedNode->installExplicitDep(this);
//    _setExplicitDependendentBy.insert(implementedNode);
//    implementedNode->_setExplicitDependencies.insert(this);
}

void FileNode::swapParsedData(FileNode *file)
{
    if (!file) {
        MY_ASSERT(false);
        return;
    }
    _record.swapParsedData(file->_record);
}

void FileNode::setLabeledDependencies()
{
    std::for_each(_setDependencies.begin(),
                  _setDependencies.end(), FileNodeFunc::setLabeled);
}

//void FileNode::addDependency(FileNode &file)
//{
//    // install dependencies of dependency first
//    if (!file._installDependenciesCalled)
//        file.installDependencies();

//    const auto &otherFileDependencies = file._setDependencies;
//    _setDependencies.insert(otherFileDependencies.begin(), otherFileDependencies.end());
//}

void FileNode::addDependencyPrivate(FileNode &file, FileNode::SetFileNode FileNode::*deps,
//                                    const ListFileNode FileNode::*incls,
                                    const SetFileNode FileNode::*explicitDeps,
                                    bool FileNode::*called)
{
    // install dependencies of dependency first
    if (!(file.*called))
        file.installDepsPrivate(deps, explicitDeps, called);

    const auto &otherFileDependencies = file.*deps;
    (this->*deps).insert(otherFileDependencies.begin(), otherFileDependencies.end());
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
    : _state(Clean), _rootDirectoryNode(nullptr), _srcParser(*this)
{

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
    parseFilesRecursive(_rootDirectoryNode);
}

void FileTree::installIncludeNodes()
{
    MY_ASSERT(_rootDirectoryNode);
    installIncludeNodesRecursive(*_rootDirectoryNode);
}

void FileTree::installInheritanceNodes()
{
    MY_ASSERT(_rootDirectoryNode);
    installInheritanceNodesRecursive(*_rootDirectoryNode);
}

void FileTree::installImplementNodes()
{
    MY_ASSERT(_rootDirectoryNode);
    installImplementNodesRecursive(*_rootDirectoryNode);
}

void FileTree::installDependencies()
{
    MY_ASSERT(_rootDirectoryNode);
    recursiveCall(*_rootDirectoryNode, &FileNode::installDependencies);
}

void FileTree::installDependentBy()
{
    MY_ASSERT(_rootDirectoryNode);
    recursiveCall(*_rootDirectoryNode, &FileNode::installDependentBy);
}

void FileTree::parseModifiedFiles(const FileTree &restored_file_tree)
{
    MY_ASSERT(_state == CachesCalculated);
    parseModifiedFilesRecursive(_rootDirectoryNode, restored_file_tree._rootDirectoryNode);
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

FileNode *FileTree::addFile(const SplittedPath &path)
{
    const std::list<HashedFileName> &splittedPath = path.splitted();
    if (!_rootDirectoryNode)
        setRootDirectoryNode(new FileNode(string("."), FileRecord::Directory));

    FileNode *currentNode = _rootDirectoryNode;
    MY_ASSERT(_rootDirectoryNode);
    for (const HashedFileName &fname : splittedPath) {
        FileRecord::Type type = ((fname == splittedPath.back())
                                 ? FileRecord::RegularFile
                                 : FileRecord::Directory);
        currentNode = currentNode->findOrNewChild(fname, type);
        MY_ASSERT(currentNode);
    }
    return currentNode;
}

void FileTree::setRootDirectoryNode(FileNode *node)
{
    if (_rootDirectoryNode) {
        _includePaths.remove(_rootDirectoryNode);
        delete _rootDirectoryNode;
    }
    if (node)
        _includePaths.push_back(node);

    _rootDirectoryNode = node;
}

void FileTree::removeEmptyDirectories(FileNode *node)
{
    if (!node)
        return;
    if (node->record()._type == FileRecord::RegularFile)
        return;

    FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeIterator it(childs.begin());

    while(it != childs.end()) {
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

    while(it != childs.end()) {
        calculateFileHashes(*it);

        ++it;
    }
}

void FileTree::parseModifiedFilesRecursive(FileNode *node, FileNode *restored_node)
{
    auto thisChilds = node->childs();
    if (node->isRegularFile()) {
        if (restored_node->isRegularFile() &&
                compareHashArrays(node->record()._hashArray,
                                  restored_node->record()._hashArray)) {
            // md5 hash sums match
            node->swapParsedData(restored_node);
        }
        else {
            // md5 hash sums don't match
            std::cout << node->name() << " md5 is different" << std::endl;
            _srcParser.parseFile(node);
        }
    }
    for (auto child : thisChilds) {
        if (FileNode *restored_child =
                restored_node->findChild(child->fname())) {
            parseModifiedFilesRecursive(child, restored_child);
        }
        else {
            std::cout << "this "<< node->name() << " child " << child->fname() << " not found" << std::endl;
            parseFilesRecursive(child);
        }
    }
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

FileNode *FileTree::searchIncludedFile(const IncludeDirective &id, FileNode *node) const
{
    MY_ASSERT(node);
    const SplittedPath &path = id.filename;
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

FileNode *FileTree::searchInCurrentDir(const SplittedPath &path, FileNode *dir) const
{
    MY_ASSERT(dir && dir->isDirectory());
    if (!dir)
        return nullptr;
    return dir->search(path);
}

FileNode *FileTree::searchInIncludePaths(const SplittedPath &path) const
{
    for (auto includePath : _includePaths) {
        if (FileNode *includeFile = searchInCurrentDir(path, includePath))
            return includeFile;
    }
    // not found
    return nullptr;
}

FileNode *FileTree::searchInRoot(const SplittedPath &path) const
{
    return searchInCurrentDir(path, _rootDirectoryNode);
}

std::__cxx11::string IncludeDirective::toPrint() const
{
    switch (type) {
    case Quotes: return '"' + filename + '"';
    case Brackets: return '<' + filename + '>';
    }
    return std::string();
}

void FileTreeFunc::readDirectory(FileTree &tree, const std::__cxx11::string &dirPath)
{
    DirectoryReader dirReader;
    dirReader.readDirectory(tree, dirPath);
}

void FileTreeFunc::parsePhase(FileTree &tree, const std::__cxx11::string &dumpFileName)
{
    FileTree restoredTree;
    FileTreeFunc::deserialize(restoredTree, dumpFileName);
    if (restoredTree._state == FileTree::Restored) {
        tree.parseModifiedFiles(restoredTree);
    }
    else {
        tree.parseFiles();
    }
}

static void testDeps(FileNode *fnode)
{
    for (auto &file: fnode->_setDependencies) {
        MY_ASSERT(file->_setDependentBy.find(fnode) != file->_setDependentBy.end());
    }
}

void FileTreeFunc::analyzePhase(FileTree &tree)
{
    DependencyAnalyzer dep;
    dep.setRoot(tree._rootDirectoryNode);

    tree.installImplementNodes();
    tree.installIncludeNodes();
    tree.installInheritanceNodes();

    tree.installDependencies();

    DEBUG(
        tree.installDependentBy();
        testDeps(tree._rootDirectoryNode)
    );
}

static bool isAffected(const FileNode *file)
{
    if (file->isManuallyLabeled())
        return true;
    for (auto &dep: file->_setDependencies) {
        if (dep->isModified()) {
//            std::cout << dep->name() << " is modified" << std::endl;
            return true;
        }
    }
    return false;
}

static void printAffectedR(const FileNode *file)
{
    if (file == nullptr)
        return;

    if (isAffected(file)) {
        std::string strIndents = makeIndents(0, 2);
        std::cout << strIndents << file->name() << std::endl;
    }
    for (auto child: file->childs())
        printAffectedR(child);
}

void FileTreeFunc::printAffected(const FileTree &tree)
{
    std::cout << "FileTreeFunc::printAffected " << tree._rootPath.joint() << std::endl;

    printAffectedR(tree._rootDirectoryNode);
}

static void writeAffectedR(const FileNode *file, FILE *fp)
{
    if (file == nullptr)
        return;

    if (isAffected(file)) {
        SplittedPath tmp = file->path();
        tmp.setUnixSeparator();
        fprintf(fp, "%s\n", tmp.joint().c_str());
    }
    for (auto child: file->childs())
        writeAffectedR(child, fp);
}

void FileTreeFunc::writeAffected(const FileTree &tree, const std::__cxx11::string &filename)
{
    /* open the file for writing*/
    if (FILE * fp = fopen (filename.c_str(),"w")) {
        writeAffectedR(tree._rootDirectoryNode, fp);
        /* close the file*/
        fclose (fp);
    }
}

static bool containsMain(FileNode *file)
{
    static std::string mainPrototype = "main";
    const auto &impls = file->record()._setImplements;
    for (const auto &impl: impls) {
        if (impl.joint() == mainPrototype) {
            std::cout << file->name() << " contains main() {}" << std::endl;
            return true;
        }
    }
    // doesn't contain
    return false;
}

static void searchTestMainR(FileNode *file, std::vector<FileNode *> &vTestMainFiles)
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
        for (auto childFile: file->childs())
            searchTestMainR(childFile, vTestMainFiles);
    }
}

static FileNode *searchTestMain(FileTree &testTree)
{
    std::vector<FileNode *> vTestMainFiles;
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
        testMainFile->setLabeledDependencies();
}

static void writeModifiedR(const FileNode *file, FILE * fp)
{
    if (file == nullptr)
        return;

    if (file->isModified()) {
        SplittedPath tmp = file->path();
        tmp.setUnixSeparator();
        fprintf(fp, "%s\n", tmp.joint().c_str());
    }
    for (auto child: file->childs())
        writeModifiedR(child, fp);
}

void FileTreeFunc::writeModified(const FileTree &tree, const std::string &filename)
{
    /* open the file for writing*/
    if (FILE * fp = fopen (filename.c_str(),"w")) {
        writeModifiedR(tree._rootDirectoryNode, fp);
        /* close the file*/
        fclose (fp);
    }
}
