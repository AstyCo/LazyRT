#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include "extensions/md5.hpp"
#include "types/splitted_string.hpp"
#include "parsers/sourceparser.hpp"

#include <boost/filesystem.hpp>

#include <string>
#include <list>
#include <set>

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

    IncludeDirective(const std::string &fname = std::string())
        : type(Quotes), filename(fname) {}

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

    void swapParsedData(FileRecord &record);

    // Parse stage
    bool _isModified;

    list<IncludeDirective> _listIncludes;

    list<ScopedName> _listImplements;

    list<ScopedName> _listClassDecl;
    list<ScopedName> _listFuncDecl;

    list<ScopedName> _listUsingNamespace;

    // Analyze stage
    std::set<ScopedName> _setFuncImpl;
    std::set<ScopedName> _setClassImpl;
    std::set<ScopedName> _setImplementFiles;
public:
    MD5::HashArray _hashArray;
    bool _isHashValid;
};

class FileTree;
class FileNode
{
public:
    typedef std::list<FileNode*> ListFileNode;
    typedef std::set<FileNode*> SetFileNode;
    typedef ListFileNode::iterator FileNodeIterator;
    typedef ListFileNode::const_iterator FileNodeConstIterator;

    typedef void (FileNode::*VoidProcedure)(void);
    typedef void (FileNode::*CFileTreeProcedure)(const FileTree &);
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

    const SplittedPath &path() const { return _record._path;}
    const std::string &name() const { return path().joint();}

    bool hasRegularFiles() const;
    bool isRegularFile() const { return _record.isRegularFile();}
    bool isDirectory() const { return _record.isDirectory();}

    void destroy();

    void installIncludes(const FileTree &fileTree);
    void installImplements(const FileTree &fileTree);

    void installDependencies();
    void installDependentBy();

    FileNode *search(const SplittedPath &path);

    void addInclude(FileNode *includedNode);
    void addImplements(FileNode *implementedNode);

    void swapParsedData(FileNode *file);

    void setModified() { _record._isModified = true;}
    bool isModified() const { return _record._isModified;}

public:
    ///---Debug
    void print(int indent = 0) const;
    void printModified(int indent = 0, bool modified = true) const;
    void printIncludes(int indent = 0, int extra = 2) const;
    void printImplementNodes(int indent, int extra = 2) const;
    void printDecls(int indent = 0) const;
    void printImpls(int indent = 0) const;
    void printFuncImpls(int indent = 0) const;
    void printClassImpls(int indent = 0) const;
    void printDependencies(int indent = 0) const;
    void printDependentBy(int indent = 0) const;
    ///
private:
    void addDependencyPrivate(FileNode &file, SetFileNode FileNode::*deps, const ListFileNode FileNode::*incls,
                              const ListFileNode FileNode::*impls, bool FileNode::*called);
    void installDepsPrivate(SetFileNode FileNode::*deps, const ListFileNode FileNode::*incls,
                            const ListFileNode FileNode::*impls, bool FileNode::*called);

    FileNode *_parent;
    ListFileNode _childs;
    FileRecord _record;

public:
    ListFileNode _listIncludes;
    ListFileNode _listIncludedBy;

    ListFileNode _listImplements;
    ListFileNode _listImplementedBy;

    SetFileNode _setDependencies;
    SetFileNode _setDependentBy;
private:
    bool _installDependenciesCalled;
    bool _installDependentByCalled;
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
    void installImplementNodes();

    void installDependencies();
    void installDependentBy();

    void parseModifiedFiles(const FileTree &restored_file_tree);

    ///---Debug
    void print() const;
    void printModified() const;
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
    void installImplementNodesRecursive(FileNode &node);

    template <typename TFunc>
    void recursiveCall(FileNode &node, TFunc f)
    {
        (node.*f)();
        for (auto &child : node.childs())
            recursiveCall(*child, f);
    }

    FileNode *searchIncludedFile(const IncludeDirective &id, FileNode *node) const;

    FileNode *searchInCurrentDir(const SplittedPath &path, FileNode *current_dir) const;
    FileNode *searchInIncludePaths(const SplittedPath &path) const;
    FileNode *searchInRoot(const SplittedPath &path) const;

private:
    SourceParser _srcParser;
};
typedef std::shared_ptr<FileTree> FileTreePtr;

namespace FileTreeFunc {

//  Reads directory and initializes tree structure of FileTree instance
void readDirectory(FileTree &tree, const std::string &dirPath);

//  Reads dump file, and checks for modified source/header files.
// Then it parses modified files, or just restores information
// from dump for non-modified files.
void parsePhase(FileTree &tree, const std::string &dumpFileName);

//  Installs all the dependencies against FileNode's according
// to the data, gathered on parsing phase.
void analyzePhase(FileTree &tree);

//  Prints list of files, affected by modifications to stdout.
void printAffected(const FileTree &tree);

//  Prints list of files, affected by modifications to specific file.
void writeAffected(const FileTree &tree, const std::string &filename);


} // namespace FileTreeFunc


#endif // FILE_TREE_HPP
