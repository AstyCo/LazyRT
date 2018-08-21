#include "types/file_tree.hpp"
#include "extensions/help_functions.hpp"

#include <sstream>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

FileRecord::FileRecord(const SplittedPath &path, Type type)
    : _path(path), _type(type), _isHashValid(false)
{

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

FileNode::FileNode(const SplittedPath &path, FileRecord::Type type)
    : _record(path, type)
{
    _parent = NULL;
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
    MY_ASSERT(child->parent() == NULL);

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
    child->setParent(NULL);
    _childs.remove(child);
}

void FileNode::setParent(FileNode *parent)
{
    _parent = parent;
}

void FileNode::print(int indent) const
{
    std::string strIndents;
    for (int i = 0; i < indent; ++i)
        strIndents.push_back('\t');

    std::cout << strIndents << _record._path.string();
    if (_record._type == FileRecord::RegularFile)
        std::cout << "\thex:[" <<  _record.hashHex() << "]";
    std::cout << std::endl;
//    for (auto &incl : _record._listIncludes)
//        std::cout << strIndents << "\tinclude: " << incl.filename << std::endl;
    printIncludes(indent);
    /// TODO
    for (auto &dep : _record._listDependents)
        std::cout << strIndents << "\tdependents: " << dep << std::endl;

    list<FileNode*>::const_iterator it = _childs.begin();
    while (it != _childs.end()) {
        (*it)->print(indent + 1);
        ++it;
    }
}

void FileNode::printIncludes(int indent) const
{
    std::string strIndents;
    for (int i = 0; i < indent; ++i)
        strIndents.push_back('\t');

    for (auto file : _listIncludes) {
        std::cout << strIndents << "f_incl:" << file->record()._path.string() << std::endl;
        file->printIncludes(indent + 1);
    }
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
           _listIncludes.push_back(includedFile);
    }
}

void FileNode::installDependencies()
{
//    for (auto inc)
    /// TODO
}

FileNode *FileNode::search(const SplittedPath &path)
{
    FileNode *current_dir = this;
    for (auto &fname : path.splittedPath()) {
        if ((current_dir = current_dir->findChild(fname)))
            continue;
        return NULL;
    }
    // found file
    VERBAL(std::cout << "found in '" << record()._path.string()
           << "' include file "
           << path.string() << std::endl);
    return current_dir;
}

FileNode *FileNode::findChild(const HashedFileName &hfname) const
{
    for (auto child : _childs) {
        if (child->record()._path.filename() == hfname)
            return child;
    }
    return NULL;
}

FileTree::FileTree()
    : _state(Clean), _rootDirectoryNode(NULL), _srcParser(*this)
{

}

void FileTree::clean()
{
    if (_rootDirectoryNode)
        delete _rootDirectoryNode;

    _state = Clean;
    _rootPath = SplittedPath();
    setRootDirectoryNode(NULL);
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
    std::cout << std::endl << "parse all files:" << std::endl;
    parseFilesRecursive(_rootDirectoryNode);
}

void FileTree::installIncludeNodes()
{
    MY_ASSERT(_rootDirectoryNode);
    installIncludeNodesRecursive(*_rootDirectoryNode);
}

void FileTree::parseModifiedFiles(const FileTree *restored_file_tree)
{
    MY_ASSERT(_state == CachesCalculated);
    std::cout << std::endl << "parseModifiedFiles:" << std::endl;
    parseModifiedFilesRecursive(_rootDirectoryNode, restored_file_tree->_rootDirectoryNode);
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
    const std::list<HashedFileName> &splittedPath = path.splittedPath();
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
        if (!(restored_node->isRegularFile()
              && compareHashArrays(node->record()._hashArray,
                                   restored_node->record()._hashArray))) {
            // md5 is different
            std::cout << node->record()._path.string() << " md5 is different" << std::endl;
            _srcParser.parseFile(node);
        }
    }
    for (auto child : thisChilds) {
        if (FileNode *restored_child =
                restored_node->findChild(child->record()._path.filename())) {
            parseModifiedFilesRecursive(child, restored_child);
        }
        else {
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

SourceParser::SourceParser(const FileTree &ftree)
    : _fileTree(ftree)
{

}

void SourceParser::parseFileOld(FileNode *node)
{
    MY_ASSERT(node->isRegularFile());
    VERBAL(std::cout << "parseFile " << node->record()._path.string() << std::endl;)

    static const int BUFFER_SIZE = 200/*16*1024*/;

    std::string fname = (_fileTree._rootPath + node->record()._path).string();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if(!data) {
        errors() << "Failed to open the file" << '\'' + std::string(fname) + '\'';
        return;
    }
    long bytes_read = data_pair.second;
    long offset = 0;
    for (char *p = data;;)
    {
        char *pnl = (char*) memchr(p, '\n', bytes_read - (p - data));
        // start looking for include
        if (pnl) {
            *pnl = 0;
            analyzeLine(p, node);
        }
        else {
            offset = bytes_read - (p - data);
            if (p == data) {
                errors() << "Too big line in file, more than" << numberToString(BUFFER_SIZE);
                return;
            }
            else {
                if (p - data < bytes_read) {
                    // copy
                    memcpy(data, p, offset);
                }
            }
            break;
        }
        p = pnl + 1;
    }
    // free file's data
    delete data;
}

const char *SourceParser::skipLine(const char *p) const
{
    for ( ; *p; ++p) {
        if (p == '\n')
            return p + 1;
    }
    return p;
}

void SourceParser::parseFile(FileNode *node)
{
    if (!node->isRegularFile())
        return;
    VERBAL(std::cout << "parseFile " << node->record()._path.string() << std::endl;)

    static const int BUFFER_SIZE = 200/*16*1024*/;

    std::string fname = (_fileTree._rootPath + node->record()._path).string();
    auto data_pair = readFile(fname.c_str(), "r");
    char *data = data_pair.first;
    if(!data) {
        errors() << "Failed to open the file" << '\'' + std::string(fname) + '\'';
        return;
    }
    long bytes_read = data_pair.second;
    long offset = 0;
    for (char *p = data; *p != 0;)
    {
        switch (_state) {
        case NoSpecialState:
            break;
        case NotIncludeMacroState:
            p = skipLine(p);
            break;
        case HashSign:
        {
            // check if include directive
            p = skipSpacesAndComments(p);
            static const char *str_include = "include";
            static const int str_include_len = strlen("include");
            for (int i = 0; i < str_include_len; ++i) {
                if (p[i] != str_include[i]) {
                    _state = NotIncludeMacroState;
                    break;
                }
            }
            p = skipSpacesAndComments(p + str_include_len);
            if (_state != NotIncludeMacroState)
                _state = IncludeState;
            break;
        }
        case IncludeState: {
            // parse filename
            char pairChar;
            IncludeDirective dir;
            if (*p == '"') {
                pairChar = '"';
                dir.type = IncludeDirective::Quotes;
            }
            else if (*p == '<') {
                pairChar = '>';
                dir.type = IncludeDirective::Brackets;
            }
            else {
                std::cout << std::string(p) << std::endl;
                MY_ASSERT(false)
            }
            ++p;
            const char *end_of_filename = (char*) memchr(p, pairChar, strlen(p));
            MY_ASSERT(end_of_filename)
            // TODO check for \<newline>
            dir.filename = std::string(p, end_of_filename - p);

            FileNode *includedFile = _fileTree.searchIncludedFile(dir, node);

            if (includedFile)
                node->record()._listIncludes.push_back(dir);
            VERBAL(else std::cout << "not found " << dir.filename << std::endl;)
            break;
        }
        case StructState:
            break;
        case ClassState:
            break;
        case Quotes:
            break;
        case MultiComments:
            break;
        case SingleComments:
            break;
        }

        char *pnl = (char*) memchr(p, '\n', bytes_read - (p - data));
        // start looking for include
        if (pnl) {
            *pnl = 0;
            p = skipSpacesAndComments(p);
            if (*p != '#')
                return;
            p = skipSpacesAndComments(p + 1);

        }
        else {
            offset = bytes_read - (p - data);
            if (p == data) {
                errors() << "Too big line in file, more than" << numberToString(BUFFER_SIZE);
                return;
            }
            else {
                if (p - data < bytes_read) {
                    // copy
                    memcpy(data, p, offset);
                }
            }
            break;
        }
        p = pnl + 1;
    }
    // free file's data
    delete data;
}

void FileTree::installIncludeNodesRecursive(FileNode &node)
{
    node.installIncludes(*this);
    for (auto &child : node.childs())
        installIncludeNodesRecursive(*child);
}

void FileTree::installDependenciesRecursive(FileNode &node)
{
    node.installDependencies();
    for (auto &child : node.childs())
        installDependenciesRecursive(*child);
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
        return NULL;
    return dir->search(path);
}

FileNode *FileTree::searchInIncludePaths(const SplittedPath &path) const
{
    for (auto includePath : _includePaths) {
        if (FileNode *includeFile = searchInCurrentDir(path, includePath))
            return includeFile;
    }
    // not found
    return NULL;
}

const char *SourceParser::skipSpaces(const char *line) const
{
    MY_ASSERT(line);
    for (;*line;++line) {
        if (!isspace(*line))
            return line;
    }
    return line;
}

const char *SourceParser::skipSpacesAndComments(const char *line) const
{
    MY_ASSERT(line);
    bool commentState = false;
    for (;*line;++line) {
        if (commentState) {
            if (!strncmp(line, "*/", 2)) {
                commentState = false;
                ++line;
            }
            continue;
        }
        if (*line == '/') {
            if (*(line + 1) == '*') {
                // comment
                commentState = true;
                ++line;
                continue;
            }
            else {
                return line;
            }
        }
        if (!isspace(*line))
            return line;
    }
    return line;
}

void SourceParser::analyzeLine(const char *line, FileNode *node)
{
    line = skipSpacesAndComments(line);
    if (*line != '#')
        return;
    line = skipSpacesAndComments(line + 1);
    static const char *str_include = "include";
    static const int str_include_len = strlen("include");
    for (int i = 0; i < str_include_len; ++i) {
        if (line[i] != str_include[i])
            return;
    }
    line = skipSpacesAndComments(line + str_include_len);
    // parse filename
    char pairChar;
    IncludeDirective dir;
    if (*line == '"') {
        pairChar = '"';
        dir.type = IncludeDirective::Quotes;
    }
    else if (*line == '<') {
        pairChar = '>';
        dir.type = IncludeDirective::Brackets;
    }
    else {
        std::cout << std::string(line) << std::endl;
        MY_ASSERT(false)
    }
    ++line;
    const char *end_of_filename = (char*) memchr(line, pairChar, strlen(line));
    MY_ASSERT(end_of_filename)
    dir.filename = std::string(line, end_of_filename - line);

    FileNode *includedFile = _fileTree.searchIncludedFile(dir, node);

    if (includedFile)
        node->record()._listIncludes.push_back(dir);
    VERBAL(else std::cout << "not found " << dir.filename << std::endl;)
}

std::__cxx11::string IncludeDirective::toPrint() const
{
    switch (type) {
    case Quotes: return '"' + filename + '"';
    case Brackets: return '<' + filename + '>';
    }
    return std::string();
}
