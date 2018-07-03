#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include <string>
#include <list>
#include <boost/filesystem.hpp>

#include "extensions/md5.hpp"
#include "splitted_path.hpp"

using std::string;
using std::list;

//typedef boost::filesystem::path Path;
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

    list<string> _listIncludes;
    list<string> _listDependents;
public:
    MD5::HashArray _hashArray;
    bool _isHashValid;
};

class FileNode
{
public:
    explicit FileNode(const SplittedPath &path, FileRecord::Type type);
    virtual ~FileNode();

    void addChild(FileNode *child);
    FileNode *createChild(const HashedFileName &hfname, FileRecord::Type type);
    void removeChild(FileNode *child);
    void setParent(FileNode *parent);

    FileNode *parent() const { return _parent;}

    list<FileNode*> &childs() { return _childs;}
    const list<FileNode*> &childs() const { return _childs;}

    ///---Debug
    void print(int indent = 0) const;
    ///

    typedef std::list<FileNode*> child_list;
    typedef child_list::iterator child_iterator;
    typedef child_list::const_iterator child_const_iterator;

    bool hasRegularFiles() const;
    void destroy();
    const FileRecord &record() const { return _record;}
    FileRecord &record() { return _record;}
    bool isRegularFile() const { return _record.isRegularFile();}
    bool isDirectory() const { return _record.isDirectory();}
    FileNode *findChild(const HashedFileName &hfname) const;

private:
    FileNode *_parent;
    std::list<FileNode*> _childs;
    FileRecord _record;
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
        CachesCalculated
    };

    FileTree();

    void clean();
    void removeEmptyDirectories();
    void calculateFileHashes();
    void parseModifiedFiles(const FileTree *restored_file_tree);

    ///---Debug
    void print() const;
    ///

    FileNode *createNode(const SplittedPath &path);

    void setRootDirectoryNode(FileNode *node);

public:
    State _state;
    SplittedPath _rootPath;
    FileNode *_rootDirectoryNode;

    std::list<FileNode*> _includePaths;

private:
    void removeEmptyDirectories(FileNode *node);
    void calculateFileHashes(FileNode *node);
    void parseModifiedFilesRecursive(FileNode *node, FileNode *restored_node);
    void parseFilesRecursive(FileNode *node);
    void parseFile(FileNode *node);

    FileNode *searchIncludedFile(const IncludeDirective &id, FileNode *node);

    FileNode *searchInCurrentDir(const SplittedPath &path, FileNode *current_dir);
    FileNode *searchInIncludePaths(const SplittedPath &path);

    const char *skipSpaces(const char *line);
    void analyzeLine(const char *line, FileNode *node);
};

#endif // FILE_TREE_HPP

