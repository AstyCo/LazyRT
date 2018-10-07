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
    : _path(path), _type(type), _isModified(false), _isHashValid(false)
{
    _path.setOsSeparator();
}

void FileRecord::calculateHash(const SplittedPath &dir_base)
{
    auto data_pair = readBinaryFile((dir_base + _path).c_str());
    char *data = data_pair.first;
    if (!data) {
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
    _listImplements.swap(record._listImplements);

    _listClassDecl.swap(record._listClassDecl);
    _listFuncDecl.swap(record._listFuncDecl);

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
    printDependencies(indent);
//    printDependentBy(indent);
//    printIncludes(indent);
//    printImplementNodes(indent);
//    printImpls(indent);
//    printDecls(indent);
//    printFuncImpls(indent);
//    printClassImpls(indent);

    list<FileNode*>::const_iterator it = _childs.begin();
    while (it != _childs.end()) {
        (*it)->print(indent + 1);
        ++it;
    }
}

void FileNode::printModified(int indent, bool modified) const
{
    std::string strIndents = makeIndents(indent, 2);

    if (isModified() == modified)
        std::cout<< strIndents << (modified ? " + " : " - " ) << name() << std::endl;

    for (auto f: _childs)
        printModified(indent + 1, modified);
}

void FileNode::printIncludes(int indent, int extra) const
{
    std::string strIndents = makeIndents(indent, extra);

    for (auto file : _listIncludes) {
        std::cout << strIndents << "include: " << file->name() << std::endl;
        file->printIncludes(indent, extra + 2);
    }
}

void FileNode::printImplementNodes(int indent, int extra) const
{
    std::string strIndents = makeIndents(indent, extra);

    for (auto file : _listImplementedBy) {
        std::cout << strIndents << "implemented by: " << file->name() << std::endl;
        file->printImplementNodes(indent, extra + 2);
    }
}

void FileNode::printDecls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &decl : _record._listClassDecl)
        std::cout << strIndents << string("class decl: ") << decl.joint() << std::endl;
    for (auto &decl : _record._listFuncDecl)
        std::cout << strIndents << string("function decl: ") << decl.joint() << std::endl;
}

void FileNode::printImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._listImplements)
        std::cout << strIndents << string("impl: ") << impl.joint() << std::endl;
}

void FileNode::printFuncImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setFuncImpl)
        std::cout << strIndents << string("impl func: ") << impl.joint() << std::endl;
}

void FileNode::printClassImpls(int indent) const
{
    std::string strIndents = makeIndents(indent, 2);

    for (auto &impl : _record._setClassImpl)
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

void FileNode::installDepsPrivate(FileNode::SetFileNode FileNode::*deps, const FileNode::ListFileNode FileNode::*incls, const FileNode::ListFileNode FileNode::*impls, bool FileNode::*called)
{
    this->*called = true;

    // File allways depends on himself
    (this->*deps).insert(this);

    for (auto node_ptr: (this->*incls))
        addDependencyPrivate(*node_ptr, deps, incls, impls, called);
    for (auto node_ptr: (this->*impls))
        addDependencyPrivate(*node_ptr, deps, incls, impls, called);
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
           addInclude(includedFile);
    }
}

void FileNode::installImplements(const FileTree &fileTree)
{
    for (auto &file_path : _record._setImplementFiles) {

       if (FileNode *implementedFile = fileTree.searchInRoot(file_path))
           addImplements(implementedFile);
    }
}

void FileNode::installDependencies()
{
    installDepsPrivate(&FileNode::_setDependencies,
                       &FileNode::_listIncludes,
                       &FileNode::_listImplementedBy,
                       &FileNode::_installDependenciesCalled);
}

void FileNode::installDependentBy()
{
    installDepsPrivate(&FileNode::_setDependentBy,
                       &FileNode::_listIncludedBy,
                       &FileNode::_listImplements,
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

void FileNode::addInclude(FileNode *includedNode)
{
    if (!includedNode) {
        MY_ASSERT(false);
        return;
    }
    _listIncludes.push_back(includedNode);
    includedNode->_listIncludedBy.push_back(this);
}

void FileNode::addImplements(FileNode *implementedNode)
{
    if (!implementedNode) {
        MY_ASSERT(false);
        return;
    }
    _listImplements.push_back(implementedNode);
    implementedNode->_listImplementedBy.push_back(this);
}

void FileNode::swapParsedData(FileNode *file)
{
    if (!file) {
        MY_ASSERT(false);
        return;
    }
    _record.swapParsedData(file->_record);
}

//void FileNode::addDependency(FileNode &file)
//{
//    // install dependencies of dependency first
//    if (!file._installDependenciesCalled)
//        file.installDependencies();

//    const auto &otherFileDependencies = file._setDependencies;
//    _setDependencies.insert(otherFileDependencies.begin(), otherFileDependencies.end());
//}

void FileNode::addDependencyPrivate(FileNode &file, FileNode::SetFileNode FileNode::*deps, const ListFileNode FileNode::*incls, const ListFileNode FileNode::*impls, bool FileNode::*called)
{
    // install dependencies of dependency first
    if (!(file.*called))
        file.installDepsPrivate(deps, incls, impls, called);

    const auto &otherFileDependencies = file.*deps;
    (this->*deps).insert(otherFileDependencies.begin(), otherFileDependencies.end());
}

FileNode *FileNode::findChild(const HashedFileName &hfname) const
{
    for (auto child : _childs) {
        if (child->path().last() == hfname)
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

void FileTree::printModified() const
{
    MY_ASSERT(_rootDirectoryNode);
    _rootDirectoryNode->printModified(0, true);
    _rootDirectoryNode->printModified(0, false);
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
                restored_node->findChild(child->path().last())) {
            parseModifiedFilesRecursive(child, restored_child);
        }
        else {
            std::cout << "this "<< node->name() << " child " << child->path().last() << " not found" << std::endl;
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

    tree.installDependencies();

    DEBUG(
        tree.installDependentBy();
        testDeps(tree._rootDirectoryNode)
    );
}

static bool isAffected(const FileNode *file)
{
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
        tmp.setSeparator("/");
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
