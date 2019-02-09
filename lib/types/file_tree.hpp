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

using BoostPath = boost::filesystem::path;

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
    std::vector< IncludeDirective > _listIncludes;
    std::set< ScopedName > _setImplements;
    std::set< ScopedName > _setClassDecl;
    std::set< ScopedName > _setFuncDecl;
    std::set< ScopedName > _setInheritances;
    std::vector< ScopedName > _listUsingNamespace;

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
    enum Flags {
        Nothing = 0x0,
        Modified = 0x1,
        Labeled = 0x2,
        SourceFile = 0x4,
        TestFile = 0x8
    };
    using FlagsType = uint8_t;

    using ListFileNode = std::vector< FileNode * >;
    using SetFileNode = std::set< FileNode * >;
    using FileNodeIterator = ListFileNode::iterator;
    using FileNodeConstIterator = ListFileNode::const_iterator;

    using VoidProcedurePtr = void (FileNode::*)(void);
    using BoolProcedureCPtr = bool (FileNode::*)(void) const;
    using CFileTreeProcedure = void (FileNode::*)(const FileTree &);

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

    std::vector< FileNode * > &childs() { return _childs; }
    const std::vector< FileNode * > &childs() const { return _childs; }

    const SplittedPath &path() const { return _record._path; }
    const SplittedPath::HashedType &fname() const { return path().last(); }

    const std::string &name() const { return path().joint(); }
    SplittedPath fullPath() const;

    std::string relativeName(const SplittedPath &base) const;

    bool hasRegularFiles() const;
    bool isRegularFile() const { return _record.isRegularFile(); }
    bool isDirectory() const { return _record.isDirectory(); }

    void destroy();

    void installDependencies();
    void installDependentBy();
    void clearVisited();

    void initExplicitDeps();

    FileNode *search(const SplittedPath &path);

    void addDependency(FileNode *node);
    void addDependencies(const SetFileNode &nodes);

    void addDependentBy(FileNode *node);

    void addExplicitDep(FileNode *includedNode);
    void addExplicitDepBy(FileNode *implementedNode);

    void swapParsedData(FileNode *file);

    void setModified() { _flags |= Flags::Modified; }
    bool isModified() const { return _flags & Flags::Modified; }

    void setLabeled() { _flags |= Flags::Labeled; }
    bool isManuallyLabeled() const { return _flags & Flags::Labeled; }

    void setSourceFile() { _flags |= Flags::SourceFile; }
    bool isSourceFile() const { return _flags & Flags::SourceFile; }

    void setTestFile() { _flags |= Flags::TestFile; }
    bool isTestFile() const { return _flags & Flags::TestFile; }

    bool isThisAffected() const { return isModified() || isManuallyLabeled(); }
    bool isAffected() const;

    FlagsType flags() const { return _flags; }
    bool checkFlags(FlagsType fls) const { return _flags & fls; }

    bool isAffectedSource() const { return isAffected() && !isTestFile(); }
    bool isAffectedTest() const { return isAffected() && isTestFile(); }

    std::vector< FileNode * > getFiles() const;

public:
    ///---Debug
    void print(int indent = 0) const;
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
    void installIncludes();
    void installInheritances();
    void installImplements();

    void installDependenciesR(FileNode *node);

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
    FlagsType _flags;
};

namespace FileNodeFunc {

inline void setLabeled(FileNode *f)
{
    assert(f);
    f->setLabeled();
}

} // namespace FileNodeFunc

class CommandLineArgs;
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

    void clearVisitedR();

    void installAffectedFiles();

    void parseModifiedFiles(const FileTree &restored_file_tree);

    ///---Debug
    void print() const;
    void printAll() const;
    ///

    FileNode *rootNode() const { return _rootDirectoryNode; }
    FileNode *addFile(const SplittedPath &relPath);

    const SplittedPath &projectDirectory() const;
    void setProjectDirectory(const SplittedPath &path);

    const SplittedPath &relativePathSources() const;

    const SplittedPath &rootPath() const;
    void setRootPath(const SplittedPath &sp);

    State state() const;
    void setState(const State &state);

    void readFiles(const CommandLineArgs &clargs);
    void parsePhase(const SplittedPath &spFtreeDump);
    void writeAffectedFiles(const CommandLineArgs &clargs);
    void labelTestMain();

    void readSources(const std::vector< SplittedPath > &relPaths,
                     const std::vector< std::string > &ignoreSubstrings);
    void readTests(const std::vector< SplittedPath > &relPaths,
                   const std::vector< std::string > &ignoreSubstrings);

    void labelTests(const std::vector< SplittedPath > &relPaths);
    void labelTest(const SplittedPath &relPath);

    void analyzePhase();

    void printPaths(std::ostream &os,
                    const std::vector< SplittedPath > &paths) const;
    void writePaths(const SplittedPath &path,
                    const std::vector< SplittedPath > &paths) const;

    void writeFiles(const SplittedPath &path,
                    FileNode::BoolProcedureCPtr checkSatisfy) const;
    void writeFiles(std::ostream &os,
                    FileNode::BoolProcedureCPtr checkSatisfy) const;

    void installExtraDependencies(const SplittedPath &pathToExtraDeps);

public:
    SplittedPath _projectDirectory;

    void addIncludePaths(const std::vector< SplittedPath > &paths);

    void addIncludePath(const SplittedPath &path);

    const std::vector< FileNode * > &includePaths() const
    {
        return _includePaths;
    }

    std::vector< SplittedPath > affectedFiles(const SplittedPath &spBase,
                                              FileNode::FlagsType flags) const;

public:
    void removeEmptyDirectories(FileNode *node);
    void calculateFileHashes(FileNode *node);
    void parseModifiedFilesRecursive(FileNode *node);
    void compareModifiedFilesRecursive(FileNode *node, FileNode *restored_node);
    void installModifiedFiles(FileNode *node);
    void parseFilesRecursive(FileNode *node);

    void installAffectedFilesRecursive(FileNode *node);

    void analyzeNodes();
    void propagateDeps();

    void recursiveCall(FileNode &node, FileNode::VoidProcedurePtr f);

    FileNode *searchIncludedFile(const IncludeDirective &id,
                                 FileNode *node) const;

    FileNode *searchInCurrentDir(const SplittedPath &path,
                                 FileNode *current_dir) const;
    FileNode *searchInIncludePaths(const SplittedPath &path) const;
    FileNode *searchInRoot(const SplittedPath &path) const;

private:
    void updateRoot();

private:
    FileNode *_rootDirectoryNode;
    std::vector< FileNode * > _includePaths;
    std::vector< FileNode * > _affectedFiles;

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

using FileTreePtr = std::shared_ptr< FileTree >;

#endif // FILE_TREE_HPP
