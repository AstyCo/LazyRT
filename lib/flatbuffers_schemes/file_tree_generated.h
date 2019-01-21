// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_FILETREE_LAZYUT_H_
#define FLATBUFFERS_GENERATED_FILETREE_LAZYUT_H_

#include "external/flatbuffers/flatbuffers.h"

namespace LazyUT {

struct ListSplitted;

struct FileRecord;

struct FileTree;

struct ListSplitted FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table
{
    enum { VT_SEPARATOR = 4, VT_SPLITTED_PATHS = 6 };
    const flatbuffers::String *separator() const
    {
        return GetPointer< const flatbuffers::String * >(VT_SEPARATOR);
    }
    const flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > *
    splitted_paths() const
    {
        return GetPointer< const flatbuffers::Vector<
            flatbuffers::Offset< flatbuffers::String > > * >(VT_SPLITTED_PATHS);
    }
    bool Verify(flatbuffers::Verifier &verifier) const
    {
        return VerifyTableStart(verifier) &&
               VerifyOffset(verifier, VT_SEPARATOR) &&
               verifier.Verify(separator()) &&
               VerifyOffset(verifier, VT_SPLITTED_PATHS) &&
               verifier.Verify(splitted_paths()) &&
               verifier.VerifyVectorOfStrings(splitted_paths()) &&
               verifier.EndTable();
    }
};

struct ListSplittedBuilder
{
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_separator(flatbuffers::Offset< flatbuffers::String > separator)
    {
        fbb_.AddOffset(ListSplitted::VT_SEPARATOR, separator);
    }
    void add_splitted_paths(
        flatbuffers::Offset<
            flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
            splitted_paths)
    {
        fbb_.AddOffset(ListSplitted::VT_SPLITTED_PATHS, splitted_paths);
    }
    explicit ListSplittedBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb)
    {
        start_ = fbb_.StartTable();
    }
    ListSplittedBuilder &operator=(const ListSplittedBuilder &);
    flatbuffers::Offset< ListSplitted > Finish()
    {
        const auto end = fbb_.EndTable(start_);
        auto o = flatbuffers::Offset< ListSplitted >(end);
        return o;
    }
};

inline flatbuffers::Offset< ListSplitted > CreateListSplitted(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset< flatbuffers::String > separator = 0,
    flatbuffers::Offset<
        flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
        splitted_paths = 0)
{
    ListSplittedBuilder builder_(_fbb);
    builder_.add_splitted_paths(splitted_paths);
    builder_.add_separator(separator);
    return builder_.Finish();
}

inline flatbuffers::Offset< ListSplitted > CreateListSplittedDirect(
    flatbuffers::FlatBufferBuilder &_fbb, const char *separator = nullptr,
    const std::vector< flatbuffers::Offset< flatbuffers::String > >
        *splitted_paths = nullptr)
{
    return LazyUT::CreateListSplitted(
        _fbb, separator ? _fbb.CreateString(separator) : 0,
        splitted_paths
            ? _fbb.CreateVector< flatbuffers::Offset< flatbuffers::String > >(
                  *splitted_paths)
            : 0);
}

struct FileRecord FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table
{
    enum {
        VT_PATH = 4,
        VT_MD5 = 6,
        VT_INCLUDES = 8,
        VT_IMPLEMENTS = 10,
        VT_INHERITANCES = 12,
        VT_CLASS_DECLS = 14,
        VT_FUNCTION_DECLS = 16,
        VT_USING_NAMESPACES = 18
    };
    const flatbuffers::String *path() const
    {
        return GetPointer< const flatbuffers::String * >(VT_PATH);
    }
    const flatbuffers::Vector< uint8_t > *md5() const
    {
        return GetPointer< const flatbuffers::Vector< uint8_t > * >(VT_MD5);
    }
    const flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > *
    includes() const
    {
        return GetPointer< const flatbuffers::Vector<
            flatbuffers::Offset< flatbuffers::String > > * >(VT_INCLUDES);
    }
    const ListSplitted *implements() const
    {
        return GetPointer< const ListSplitted * >(VT_IMPLEMENTS);
    }
    const ListSplitted *inheritances() const
    {
        return GetPointer< const ListSplitted * >(VT_INHERITANCES);
    }
    const ListSplitted *class_decls() const
    {
        return GetPointer< const ListSplitted * >(VT_CLASS_DECLS);
    }
    const ListSplitted *function_decls() const
    {
        return GetPointer< const ListSplitted * >(VT_FUNCTION_DECLS);
    }
    const ListSplitted *using_namespaces() const
    {
        return GetPointer< const ListSplitted * >(VT_USING_NAMESPACES);
    }
    bool Verify(flatbuffers::Verifier &verifier) const
    {
        return VerifyTableStart(verifier) && VerifyOffset(verifier, VT_PATH) &&
               verifier.Verify(path()) && VerifyOffset(verifier, VT_MD5) &&
               verifier.Verify(md5()) && VerifyOffset(verifier, VT_INCLUDES) &&
               verifier.Verify(includes()) &&
               verifier.VerifyVectorOfStrings(includes()) &&
               VerifyOffset(verifier, VT_IMPLEMENTS) &&
               verifier.VerifyTable(implements()) &&
               VerifyOffset(verifier, VT_INHERITANCES) &&
               verifier.VerifyTable(inheritances()) &&
               VerifyOffset(verifier, VT_CLASS_DECLS) &&
               verifier.VerifyTable(class_decls()) &&
               VerifyOffset(verifier, VT_FUNCTION_DECLS) &&
               verifier.VerifyTable(function_decls()) &&
               VerifyOffset(verifier, VT_USING_NAMESPACES) &&
               verifier.VerifyTable(using_namespaces()) && verifier.EndTable();
    }
};

struct FileRecordBuilder
{
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_path(flatbuffers::Offset< flatbuffers::String > path)
    {
        fbb_.AddOffset(FileRecord::VT_PATH, path);
    }
    void add_md5(flatbuffers::Offset< flatbuffers::Vector< uint8_t > > md5)
    {
        fbb_.AddOffset(FileRecord::VT_MD5, md5);
    }
    void add_includes(
        flatbuffers::Offset<
            flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
            includes)
    {
        fbb_.AddOffset(FileRecord::VT_INCLUDES, includes);
    }
    void add_implements(flatbuffers::Offset< ListSplitted > implements)
    {
        fbb_.AddOffset(FileRecord::VT_IMPLEMENTS, implements);
    }
    void add_inheritances(flatbuffers::Offset< ListSplitted > inheritances)
    {
        fbb_.AddOffset(FileRecord::VT_INHERITANCES, inheritances);
    }
    void add_class_decls(flatbuffers::Offset< ListSplitted > class_decls)
    {
        fbb_.AddOffset(FileRecord::VT_CLASS_DECLS, class_decls);
    }
    void add_function_decls(flatbuffers::Offset< ListSplitted > function_decls)
    {
        fbb_.AddOffset(FileRecord::VT_FUNCTION_DECLS, function_decls);
    }
    void
    add_using_namespaces(flatbuffers::Offset< ListSplitted > using_namespaces)
    {
        fbb_.AddOffset(FileRecord::VT_USING_NAMESPACES, using_namespaces);
    }
    explicit FileRecordBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb)
    {
        start_ = fbb_.StartTable();
    }
    FileRecordBuilder &operator=(const FileRecordBuilder &);
    flatbuffers::Offset< FileRecord > Finish()
    {
        const auto end = fbb_.EndTable(start_);
        auto o = flatbuffers::Offset< FileRecord >(end);
        return o;
    }
};

inline flatbuffers::Offset< FileRecord > CreateFileRecord(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset< flatbuffers::String > path = 0,
    flatbuffers::Offset< flatbuffers::Vector< uint8_t > > md5 = 0,
    flatbuffers::Offset<
        flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
        includes = 0,
    flatbuffers::Offset< ListSplitted > implements = 0,
    flatbuffers::Offset< ListSplitted > inheritances = 0,
    flatbuffers::Offset< ListSplitted > class_decls = 0,
    flatbuffers::Offset< ListSplitted > function_decls = 0,
    flatbuffers::Offset< ListSplitted > using_namespaces = 0)
{
    FileRecordBuilder builder_(_fbb);
    builder_.add_using_namespaces(using_namespaces);
    builder_.add_function_decls(function_decls);
    builder_.add_class_decls(class_decls);
    builder_.add_inheritances(inheritances);
    builder_.add_implements(implements);
    builder_.add_includes(includes);
    builder_.add_md5(md5);
    builder_.add_path(path);
    return builder_.Finish();
}

inline flatbuffers::Offset< FileRecord > CreateFileRecordDirect(
    flatbuffers::FlatBufferBuilder &_fbb, const char *path = nullptr,
    const std::vector< uint8_t > *md5 = nullptr,
    const std::vector< flatbuffers::Offset< flatbuffers::String > > *includes =
        nullptr,
    flatbuffers::Offset< ListSplitted > implements = 0,
    flatbuffers::Offset< ListSplitted > inheritances = 0,
    flatbuffers::Offset< ListSplitted > class_decls = 0,
    flatbuffers::Offset< ListSplitted > function_decls = 0,
    flatbuffers::Offset< ListSplitted > using_namespaces = 0)
{
    return LazyUT::CreateFileRecord(
        _fbb, path ? _fbb.CreateString(path) : 0,
        md5 ? _fbb.CreateVector< uint8_t >(*md5) : 0,
        includes
            ? _fbb.CreateVector< flatbuffers::Offset< flatbuffers::String > >(
                  *includes)
            : 0,
        implements, inheritances, class_decls, function_decls,
        using_namespaces);
}

struct FileTree FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table
{
    enum { VT_ROOTPATH = 4, VT_RECORDS = 6 };
    const flatbuffers::String *rootPath() const
    {
        return GetPointer< const flatbuffers::String * >(VT_ROOTPATH);
    }
    const flatbuffers::Vector< flatbuffers::Offset< FileRecord > > *
    records() const
    {
        return GetPointer<
            const flatbuffers::Vector< flatbuffers::Offset< FileRecord > > * >(
            VT_RECORDS);
    }
    bool Verify(flatbuffers::Verifier &verifier) const
    {
        return VerifyTableStart(verifier) &&
               VerifyOffset(verifier, VT_ROOTPATH) &&
               verifier.Verify(rootPath()) &&
               VerifyOffset(verifier, VT_RECORDS) &&
               verifier.Verify(records()) &&
               verifier.VerifyVectorOfTables(records()) && verifier.EndTable();
    }
};

struct FileTreeBuilder
{
    flatbuffers::FlatBufferBuilder &fbb_;
    flatbuffers::uoffset_t start_;
    void add_rootPath(flatbuffers::Offset< flatbuffers::String > rootPath)
    {
        fbb_.AddOffset(FileTree::VT_ROOTPATH, rootPath);
    }
    void add_records(flatbuffers::Offset<
                     flatbuffers::Vector< flatbuffers::Offset< FileRecord > > >
                         records)
    {
        fbb_.AddOffset(FileTree::VT_RECORDS, records);
    }
    explicit FileTreeBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb)
    {
        start_ = fbb_.StartTable();
    }
    FileTreeBuilder &operator=(const FileTreeBuilder &);
    flatbuffers::Offset< FileTree > Finish()
    {
        const auto end = fbb_.EndTable(start_);
        auto o = flatbuffers::Offset< FileTree >(end);
        return o;
    }
};

inline flatbuffers::Offset< FileTree >
CreateFileTree(flatbuffers::FlatBufferBuilder &_fbb,
               flatbuffers::Offset< flatbuffers::String > rootPath = 0,
               flatbuffers::Offset<
                   flatbuffers::Vector< flatbuffers::Offset< FileRecord > > >
                   records = 0)
{
    FileTreeBuilder builder_(_fbb);
    builder_.add_records(records);
    builder_.add_rootPath(rootPath);
    return builder_.Finish();
}

inline flatbuffers::Offset< FileTree > CreateFileTreeDirect(
    flatbuffers::FlatBufferBuilder &_fbb, const char *rootPath = nullptr,
    const std::vector< flatbuffers::Offset< FileRecord > > *records = nullptr)
{
    return LazyUT::CreateFileTree(
        _fbb, rootPath ? _fbb.CreateString(rootPath) : 0,
        records
            ? _fbb.CreateVector< flatbuffers::Offset< FileRecord > >(*records)
            : 0);
}

inline const LazyUT::FileTree *GetFileTree(const void *buf)
{
    return flatbuffers::GetRoot< LazyUT::FileTree >(buf);
}

inline const LazyUT::FileTree *GetSizePrefixedFileTree(const void *buf)
{
    return flatbuffers::GetSizePrefixedRoot< LazyUT::FileTree >(buf);
}

inline bool VerifyFileTreeBuffer(flatbuffers::Verifier &verifier)
{
    return verifier.VerifyBuffer< LazyUT::FileTree >(nullptr);
}

inline bool VerifySizePrefixedFileTreeBuffer(flatbuffers::Verifier &verifier)
{
    return verifier.VerifySizePrefixedBuffer< LazyUT::FileTree >(nullptr);
}

inline void FinishFileTreeBuffer(flatbuffers::FlatBufferBuilder &fbb,
                                 flatbuffers::Offset< LazyUT::FileTree > root)
{
    fbb.Finish(root);
}

inline void
FinishSizePrefixedFileTreeBuffer(flatbuffers::FlatBufferBuilder &fbb,
                                 flatbuffers::Offset< LazyUT::FileTree > root)
{
    fbb.FinishSizePrefixed(root);
}

} // namespace LazyUT

#endif // FLATBUFFERS_GENERATED_FILETREE_LAZYUT_H_
