#include "directoryreader.hpp"
#include "extensions/error_reporter.hpp"
#include "extensions/help_functions.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

std::vector< std::string > initSourceFileExtensions()
{
    using CharArray = const char[5];
    const CharArray exts_c[] = {".c",   ".cpp", ".cc",  ".C",  ".cxx",
                                ".c++", ".h",   ".hpp", ".hh", ".H",
                                ".hxx", ".h++", ".tpp", ".ipp"};
    int size = sizeof(exts_c) / sizeof(CharArray);

    std::vector< std::string > exts;

    for (int i = 0; i < size; ++i)
        exts.push_back(exts_c[i]);

    return exts;
}

std::vector< std::string > DirectoryReader::_sourceFileExtensions =
    initSourceFileExtensions();
std::vector< std::string > DirectoryReader::_ignore_substrings =
    std::vector< std::string >();

void DirectoryReader::setTestPatterns(
    const DirectoryReader::StringVector &patterns)
{
    _testPatterns = patterns;
}

void DirectoryReader::readSources(const SplittedPath &relPath,
                                  FileTree &filetree)
{
    assert(filetree.rootNode());
    readSources(relPath, filetree.rootNode());
}

void DirectoryReader::readSources(const SplittedPath &relPath, FileNode *parent)
{
    SplittedPath fullpath = parent->fullPath() + relPath;
    if (isIgnored(fullpath))
        return;
    if (!exists(fullpath)) {
        errors() << "warning: file " << fullpath.joint() << " doesn't exists";
        return;
    }
    const auto &splitted = relPath.splitted();
    if (splitted.empty()) {
        SplittedPath dirPath = parent->fullPath();
        if (!is_directory(dirPath.jointOs().c_str()))
            return;
        DirectoryIterator it(dirPath);
        DirectoryIterator end;
        while (it != end) {
            SplittedPath fileName = *it;
            fileName.setSeparator(SplittedPath::unixSep());

            readSources(fileName, parent);
            ++it;
        }
        return;
    }
    FileNode *child =
        parent->findOrNewChild(splitted.front(), getFileType(fullpath));
    assert(child);
    if (child->isRegularFile() && isSourceFile(fullpath))
        child->setSourceFile();

    SplittedPath nextRelPath = relPath;
    nextRelPath.removeFirst();
    readSources(nextRelPath, child);
}

void DirectoryReader::removeEmptyDirectories(FileTree &fileTree)
{
    fileTree.removeEmptyDirectories();
}

bool DirectoryReader::isSourceFile(const SplittedPath &sp) const
{
    assert(is_file(sp.jointOs().c_str()));
    std::string ext = extension(sp);
    for (const auto &sourceFileExt : _sourceFileExtensions) {
        if (sourceFileExt == ext)
            return true;
    }
    return false;
}

bool DirectoryReader::isIgnored(const SplittedPath &sp) const
{
    return checkPatterns(sp.jointUnix(), _ignore_substrings);
}

bool DirectoryReader::isIgnoredOsSep(const std::string &path) const
{
    // check if path is ignored
    return isIgnored(SplittedPath(path, SplittedPath::osSep()));
}

FileRecord::Type DirectoryReader::getFileType(const SplittedPath &sp) const
{
    if (is_directory(sp.jointOs().c_str()))
        return FileRecord::Directory;
    return FileRecord::RegularFile;
}

class DirectoryIteratorImpl
{

public:
    DirectoryIteratorImpl() : offset(-1) {}

    void increment()
    {
        if (offset >= 0)
            ++offset;
        updateOffset();
    }

    bool cmpOffsets(const DirectoryIteratorImpl &o) const
    {
        return offset == o.offset;
    }

    const SplittedPath &value() const
    {
        static const SplittedPath emptyPath;
        if (offset >= 0 && offset < files.size())
            return files[offset];
        throw "DirectoryIteratorImpl:: invalid value access";
        return emptyPath;
    }

    void readDirectory(const SplittedPath &path)
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(path.jointOs().c_str())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                HashedFileName fname(ent->d_name);
                if (fname.isDot() || fname.isDotDot())
                    continue;
                files.push_back(SplittedPath(fname, SplittedPath::osSep()));
            }
            closedir(dir);
        }
        else {
            std::cerr << "could not open directory " + path.jointOs()
                      << std::endl;
        }
        setToBegin();
    }

private:
    void setToBegin()
    {
        offset = 0;
        updateOffset();
    }

    void updateOffset()
    {
        if (offset >= files.size())
            offset = -1;
    }

private:
    std::vector< SplittedPath > files;
    int offset;
};

DirectoryIterator::DirectoryIterator(const SplittedPath &path)
    : _impl(std::make_unique< DirectoryIteratorImpl >())
{
    if (!path.empty())
        _impl->readDirectory(path);
}

template < class T >
std::unique_ptr< T > copy_unique(const std::unique_ptr< T > &source)
{
    return source ? std::make_unique< T >(*source) : nullptr;
}

DirectoryIterator::DirectoryIterator(const DirectoryIterator &o)
    : _impl(copy_unique(o._impl))
{
}

DirectoryIterator &DirectoryIterator::operator++()
{
    _impl->increment();
    return *this;
}

DirectoryIterator DirectoryIterator::operator++(int)
{
    DirectoryIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool DirectoryIterator::operator==(const DirectoryIterator &o) const
{
    return _impl->cmpOffsets(*o._impl);
}

const SplittedPath &DirectoryIterator::operator*() const
{
    return _impl->value();
}
