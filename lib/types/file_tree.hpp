#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include "extensions/md5.hpp"
#include "types/splitted_string.hpp"
#include "parsers/sourceparser.hpp"

#include <boost/filesystem.hpp>

#include <string>
#include <list>
#include <set>

using std::list;
using std::string;

typedef boost::filesystem::path BoostPath;

struct IncludeDirective
{
    enum SeqCharType { Quotes, Brackets };

    IncludeDirective(const std::string &fname = std::string())
        : type(Quotes), filename(fname)
    {
    }

    SeqCharType type;
    std::string filename;

    std::string toPrint() const;
    bool isQuotes() const { return type == Quotes; }
    bool isBrackets() const { return type == Brackets; }
};

class FileRecord
{
public:
    enum Type { Directory, RegularFile };

    FileRecord(const SplittedPath &path, Type type);

public:
    void calculateHash(const SplittedPath &dir_base);

    bool isRegularFile() const { return _type == RegularFile; }
    bool isDirectory() const { return _type == Directory; }

    void setHash(const unsigned char *hash);

    std::string hashHex() const;

    SplittedPath _path;
    Type _type;

    void swapParsedData(FileRecord &record);

    // Parse stage
    bool _isModified;
    bool _isManuallyLabeled;

    std::list< IncludeDirective > _listIncludes;
    std::set< ScopedName > _setImplements;
    std::set< ScopedName > _setClassDecl;
    std::set< ScopedName > _setFuncDecl;
    std::set< ScopedName > _setInheritances;
    std::list< ScopedName > _listUsingNamespace;

    // Analyze stage
    std::set< ScopedName > _setFuncImplFiles;
    std::set< ScopedName > _setClassImplFiles;
    std::set< ScopedName > _setBaseClassFiles;
    std::set< ScopedName > _setImplementFiles;

public:
    MD5::HashArray _hashArray;
    bool _isHashValid;
};

class FileTree;
class FileNode
{
public:
    typedef std::list< FileNode * > ListFileNode;
    typedef std::set< FileNode * > SetFileNode;
    typedef ListFileNode::iterator FileNodeIterator;
    typedef ListFileNode::const_iterator FileNodeConstIterator;

    typedef void (FileNode::*VoidProcedure)(void);
    typedef void (FileNode::*CFileTreeProcedure)(const FileTree &);

public:
    explicit FileNode(const SplittedPath &path, FileRecord::Type type,
                      FileTree &fileTree);
    virtual ~FileNode();

    FileNode *findChild(const HashedFileName &hfname) const;
    void addChild(FileNode *child);
    FileNode *findOrNewChild(const HashedFileName &hfname,
                             FileRecord::Type type);
    void removeChild(FileNode *child);

    const FileRecord &record() const { return _record; }
    FileRecord &record() { return _record; }

    FileNode *parent() const { return _parent; }
    void setParent(FileNode *parent);

    list< FileNode * > &childs() { return _childs; }
    const list< FileNode * > &childs() const { return _childs; }

    const SplittedPath &path() const { return _record._path; }
    const SplittedPath::HashedType &fname() const { return path().last(); }

    const std::string &name() const { return path().joint(); }
    SplittedPath fullPath() const;

    std::string relativeName(const SplittedPath &base) const;

    bool hasRegularFiles() const;
    bool isRegularFile() const { return _record.isRegularFile(); }
    bool isDirectory() const { return _record.isDirectory(); }

    void destroy();

    void installIncludes(const FileTree &fileTree);
    void installInheritances(const FileTree &fileTree);
    void installImplements(const FileTree &fileTree);

    void installDependencies();
    void installDependentBy();
    void clearVisited();

    FileNode *search(const SplittedPath &path);

    void installExplicitDep(FileNode *includedNode);
    void installExplicitDepBy(FileNode *implementedNode);

    void swapParsedData(FileNode *file);

    void copy(FileNode *storedNode) { _record = storedNode->record(); }

    void setModified() { _record._isModified = true; }
    bool isModified() const { return _record._isModified; }

    void setLabeled() { _record._isManuallyLabeled = true; }
    bool isManuallyLabeled() const { return _record._isManuallyLabeled; }

    bool isThisAffected() const { return isModified() || isManuallyLabeled(); }
    bool isAffected() const;

public:
    ///---Debug
    void print(int indent = 0) const;
    void printModified(int indent, bool modified,
                       const SplittedPath &base) const;
    void printDecls(int indent = 0) const;
    void printImpls(int indent = 0) const;
    void printImplFiles(int indent = 0) const;
    void printFuncImpls(int indent = 0) const;
    void printClassImpls(int indent = 0) const;
    void printDependencies(int indent = 0) const;
    void printDependentBy(int indent = 0) const;
    void printInherits(int indent = 0) const;
    void printInheritsFiles(int indent = 0) const;
    ///
private:
    void installDepsPrivate(SetFileNode FileNode::*getSetDeps,
                            const SetFileNode FileNode::*getSetExplicitDeps);
    void installDepsPrivateR(FileNode *node, SetFileNode FileNode::*getSetDeps,
                             const SetFileNode FileNode::*getSetExplicitDeps);

    FileNode *_parent;
    ListFileNode _childs;
    FileRecord _record;

public:
    SetFileNode _setExplicitDependencies;
    SetFileNode _setExplicitDependendentBy;

    SetFileNode _setDependencies;
    SetFileNode _setDependentBy;

    FileTree &_fileTree;

private:
    bool _visited;
};

namespace FileNodeFunc {

inline void setLabeled(FileNode *f)
{
    MY_ASSERT(f);
    f->setLabeled();
}

} // namespace FileNodeFunc

class FileSystem;
class FileTree
{
public:
    enum State {
        Clean,
        Restored,
        Filled,
        Filtered,
        CachesCalculated,
        IncludesInstalled,
        Error
    };

    FileTree();

    void clean();
    void removeEmptyDirectories();
    void calculateFileHashes();
    void parseFiles();
    void installIncludeNodes();
    void installInheritanceNodes();
    void installImplementNodes();

    void clearVisitedR();
    void installDependencies();
    void installDependentBy();

    void installAffectedFiles();

    void parseModifiedFiles(const FileTree &restored_file_tree);

    ///---Debug
    void print() const;
    void printModified(const SplittedPath &base) const;
    ///

    FileNode *addFile(const SplittedPath &path);

    void setRootDirectoryNode(FileNode *node);

    const SplittedPath &projectDirectory() const;
    void setProjectDirectory(const SplittedPath &path);

    const SplittedPath &relativePathSources() const;

    const SplittedPath &rootPath() const;
    void setRootPath(const SplittedPath &sp);

    State state() const;
    void setState(const State &state);

    void setFileSystem(FileSystem *fs);

public:
    FileNode *_rootDirectoryNode;
    SplittedPath _projectDirectory;

    std::list< SplittedPath > _includePaths;

    std::list< SplittedPath > _affectedFiles;

    FileSystem *_filesystem;

public:
    void removeEmptyDirectories(FileNode *node);
    void calculateFileHashes(FileNode *node);
    void parseModifiedFilesRecursive(FileNode *node);
    void compareModifiedFilesRecursive(FileNode *node, FileNode *restored_node);
    void installModifiedFiles(FileNode *node);
    void parseFilesRecursive(FileNode *node);

    void installIncludeNodesRecursive(FileNode &node);
    void installInheritanceNodesRecursive(FileNode &node);
    void installImplementNodesRecursive(FileNode &node);

    void installAffectedFilesRecursive(FileNode *node);

    void analyzeNodes();
    void propagateDeps();

    template < typename TFunc >
    void recursiveCall(FileNode &node, TFunc f)
    {
        (node.*f)();
        for (FileNode *child : node.childs())
            recursiveCall(*child, f);
    }

    FileNode *searchIncludedFile(const IncludeDirective &id,
                                 FileNode *node) const;

    FileNode *searchInCurrentDir(const SplittedPath &path,
                                 FileNode *current_dir) const;
    FileNode *searchInIncludePaths(const SplittedPath &path) const;
    FileNode *searchInRoot(const SplittedPath &path) const;

private:
    void updateRelativePath();

private:
    SplittedPath _rootPath;

    SourceParser _srcParser;
    SplittedPath _relativeBasePath;
    State _state;
};

// + INLINE FUNCTIONS
inline const SplittedPath &FileTree::projectDirectory() const
{
    return _projectDirectory;
}

inline const SplittedPath &FileTree::relativePathSources() const
{
    return _relativeBasePath;
}

inline const SplittedPath &FileTree::rootPath() const { return _rootPath; }

// - INLINE FUNCTIONS

typedef std::shared_ptr< FileTree > FileTreePtr;

namespace FileTreeFunc {

//  Reads directory and initializes tree structure of FileTree instance
void readDirectory(FileTree &tree, const std::string &dirPath,
                   const std::string &ignore_substrings);

//  Reads dump file, and checks for modified source/header files.
// Then it parses modified files, or just restores information
// from dump for non-modified files.
void parsePhase(FileTree &tree, const std::string &dumpFileName);

//  Firstly searchs for single .cpp with main() implementation
//  If such file founded, then installs affected flag on it, and
// its dependencies
void labelMainAffected(FileTree &testTree);

//  Prints list of files, affected by modifications to stdout.
void printAffected(const FileTree &tree);

//  Prints list of modified files;
void writeModified(const FileTree &tree, const std::__cxx11::string &filename);

//  Prints list of files, affected by modifications to specific file.
void writeAffected(const FileTree &tree, const std::string &filename);

} // namespace FileTreeFunc

#endif // FILE_TREE_HPP
