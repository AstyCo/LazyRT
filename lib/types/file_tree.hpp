#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include "extensions/md5.hpp"
#include "types/scoped_name.hpp"
#include "types/splitted_path.hpp"
#include "parsers/sourceparser.hpp"

#include <boost/filesystem.hpp>

#include <string>
#include <list>

using std::string;
using std::list;

typedef boost::filesystem::path BoostPath;

struct IncludeDirective
{
    enum SeqCharType
    {
        Quotes,
        Brackets
    };
    SeqCharType type;
    std::string filename;

    std::string toPrint() const;
    bool isQuotes() const { return type == Quotes;}
    bool isBrackets() const { return type == Brackets;}
};

class FileRecord
{
public:
    enum Type
    {
        Directory,
        RegularFile
    };

    FileRecord(const SplittedPath &path, Type type);
public:

    void calculateHash(const SplittedPath &dir_base);

    bool isRegularFile() const { return _type == RegularFile;}
    bool isDirectory() const { return _type == Directory;}

    void setHash(const unsigned char *hash);

    std::string hashHex() const;

    SplittedPath _path;
    Type _type;

    list<IncludeDirective> _listIncludes;
    list<string> _listDependents;

    list<ScopedName> _listClassDecl;
    list<ScopedName> _listFuncDecl;

    list<ScopedName> _listImpl;
public:
    MD5::HashArray _hashArray;
    bool _isHashValid;
};

class FileTree;
class FileNode
{
public:
    typedef std::list<FileNode*> ListFileNode;
    typedef ListFileNode::iterator FileNodeIterator;
    typedef ListFileNode::const_iterator FileNodeConstIterator;
public:

    explicit FileNode(const SplittedPath &path, FileRecord::Type type);
    virtual ~FileNode();

    FileNode *findChild(const HashedFileName &hfname) const;
    void addChild(FileNode *child);
    FileNode *findOrNewChild(const HashedFileName &hfname, FileRecord::Type type);
    void removeChild(FileNode *child);

    const FileRecord &record() const { return _record;}
    FileRecord &record() { return _record;}

    FileNode *parent() const { return _parent;}
    void setParent(FileNode *parent);

    list<FileNode*> &childs() { return _childs;}
    const list<FileNode*> &childs() const { return _childs;}

    bool hasRegularFiles() const;
    bool isRegularFile() const { return _record.isRegularFile();}
    bool isDirectory() const { return _record.isDirectory();}

    void destroy();

    void installIncludes(const FileTree &fileTree);
    void installDependencies();

    FileNode *search(const SplittedPath &path);

public:
    ///---Debug
    void print(int indent = 0) const;
    void printIncludes(int indent = 0) const;
    void printDecls(int indent = 0) const;
    void printImpls(int indent = 0) const;
    ///
private:
    FileNode *_parent;
    ListFileNode _childs;
    FileRecord _record;

    ListFileNode _listIncludes;
    ListFileNode _listImplements;

    ListFileNode _listDependencies;
};

class FileTree
{
public:
    enum State
    {
        Clean,
        Restored,
        Filled,
        Filtered,
        CachesCalculated,
        IncludesInstalled
    };

    FileTree();

    void clean();
    void removeEmptyDirectories();
    void calculateFileHashes();
    void parseFiles();
    void installIncludeNodes();
    void parseModifiedFiles(const FileTree *restored_file_tree);

    ///---Debug
    void print() const;
    ///

    FileNode *addFile(const SplittedPath &path);

    void setRootDirectoryNode(FileNode *node);

public:
    State _state;
    SplittedPath _rootPath;
    FileNode *_rootDirectoryNode;

    std::list<FileNode*> _includePaths;

public:
    void removeEmptyDirectories(FileNode *node);
    void calculateFileHashes(FileNode *node);
    void parseModifiedFilesRecursive(FileNode *node, FileNode *restored_node);
    void parseFilesRecursive(FileNode *node);

    void installIncludeNodesRecursive(FileNode &node);
    void installDependenciesRecursive(FileNode &node);

    FileNode *searchIncludedFile(const IncludeDirective &id, FileNode *node) const;

    FileNode *searchInCurrentDir(const SplittedPath &path, FileNode *current_dir) const;
    FileNode *searchInIncludePaths(const SplittedPath &path) const;

private:
    SourceParser _srcParser;
};

#endif // FILE_TREE_HPP

