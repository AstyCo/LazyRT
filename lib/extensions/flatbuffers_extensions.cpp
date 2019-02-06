#include "extensions/flatbuffers_extensions.hpp"

template < typename FT, typename T >
void FileTreeFunc::copyVector(const FT &flatVector, T &v)
{
    std::transform(
        flatVector.begin(), flatVector.end(), std::inserter(v, v.end()),
        [](const flatbuffers::String *value) { return value->str(); });
}

typedef flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > >
    FB_VectorOfStrings;
template void
FileTreeFunc::copyVector< FB_VectorOfStrings, std::vector< IncludeDirective > >(
    const flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > &,
    std::vector< IncludeDirective > &);

template < typename TContainer >
void FileTreeFunc::copyListSplitted(const LazyUT::ListSplitted &fv,
                                    TContainer &v)
{
    std::string sep = fv.separator()->str();
    const auto &flatVector = *fv.splitted_paths();
    std::transform(flatVector.begin(), flatVector.end(),
                   std::inserter(v, v.end()),
                   [&sep](const flatbuffers::String *value) {
                       return ScopedName(value->str(), sep);
                   });
}

typedef std::set< ScopedName > SetScopedName;
template void
FileTreeFunc::copyListSplitted< SetScopedName >(const LazyUT::ListSplitted &fv,
                                                SetScopedName &v);

void FileTreeFunc::deserialize(FileTree &tree, const SplittedPath &sp)
{
    char *file_tree_dump = readBinaryFile(sp.jointOs().c_str()).first;
    if (file_tree_dump) {
        auto file_tree = LazyUT::GetFileTree(file_tree_dump);

        SplittedPath spRootPath(file_tree->rootPath()->str(),
                                SplittedPath::unixSep());
        tree.setRootPath(spRootPath);
        auto &records = *file_tree->records();
        for (const auto &record : records) {
            SplittedPath sp(record->path()->str(), SplittedPath::unixSep());
            FileNode *fnode = tree.addFile(sp);
            FileRecord &fileRecord = fnode->record();

            fileRecord.setHash(record->md5()->data()); // Hash
            copyVector(*record->includes(), fileRecord._listIncludes);
            copyListSplitted(*record->implements(), fileRecord._setImplements);
            copyListSplitted(*record->inheritances(),
                             fileRecord._setInheritances);
            copyListSplitted(*record->class_decls(), fileRecord._setClassDecl);
            copyListSplitted(*record->function_decls(),
                             fileRecord._setFuncDecl);
            copyListSplitted(*record->using_namespaces(),
                             fileRecord._listUsingNamespace);

            /// TODO install separators
        }

        tree.setState(FileTree::Restored);
        delete file_tree_dump;
    }
}

static flatbuffers::Offset<
    flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
CreateVectorOfStrings(flatbuffers::FlatBufferBuilder &builder,
                      const std::vector< IncludeDirective > &l)
{
    std::vector< flatbuffers::Offset< flatbuffers::String > > offsets(l.size());
    auto it = l.cbegin();
    int i = 0;
    for (auto &dir : l)
        offsets[i++] = builder.CreateString(dir.filename);
    return builder.CreateVector(offsets);
}

template < typename TContainer >
static flatbuffers::Offset< LazyUT::ListSplitted >
CreateListSplittedH(flatbuffers::FlatBufferBuilder &builder,
                    const TContainer &container, const std::string &separator)
{
    return LazyUT::CreateListSplitted(
        builder, builder.CreateString(separator),
        CreateVectorOfStrings(builder, container));
}

template < typename TContainer >
static flatbuffers::Offset<
    flatbuffers::Vector< flatbuffers::Offset< flatbuffers::String > > >
CreateVectorOfStrings(flatbuffers::FlatBufferBuilder &builder,
                      const TContainer &container)
{
    std::vector< flatbuffers::Offset< flatbuffers::String > > offsets(
        container.size());
    auto it = container.cbegin();
    int i = 0;
    for (auto &scopedName : container)
        offsets[i++] = builder.CreateString(scopedName.joint());
    return builder.CreateVector(offsets);
}

static void
pushFiles(flatbuffers::FlatBufferBuilder &builder,
          std::vector< flatbuffers::Offset< LazyUT::FileRecord > > &records,
          const FileNode *node)
{
    if (node->isRegularFile()) {
        auto frecord = node->record();
        auto fbs_frecord = LazyUT::CreateFileRecord(
            builder, builder.CreateString(frecord._path.joint()),
            builder.CreateVector(frecord._hashArray, 16),
            CreateVectorOfStrings(builder, frecord._listIncludes),
            CreateListSplittedH(builder, frecord._setImplements,
                                SplittedPath::namespaceSep()),
            CreateListSplittedH(builder, frecord._setInheritances,
                                SplittedPath::namespaceSep()),
            CreateListSplittedH(builder, frecord._setClassDecl,
                                SplittedPath::namespaceSep()),
            CreateListSplittedH(builder, frecord._setFuncDecl,
                                SplittedPath::namespaceSep()),
            CreateListSplittedH(builder, frecord._listUsingNamespace,
                                SplittedPath::namespaceSep()));
        records.push_back(fbs_frecord);
    }

    const FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeConstIterator it(childs.cbegin());
    while (it != childs.cend()) {
        pushFiles(builder, records, *it);

        ++it;
    }
}

void FileTreeFunc::serialize(const FileTree &tree, const SplittedPath &sp)
{
    MY_ASSERT(tree.rootNode());
    // store to binary file
    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector< flatbuffers::Offset< LazyUT::FileRecord > > fileRecords;
    pushFiles(builder, fileRecords, tree.rootNode());

    auto fbs_file_tree = LazyUT::CreateFileTree(
        builder, builder.CreateString(tree.rootPath().joint()),
        builder.CreateVector(fileRecords));

    builder.Finish(fbs_file_tree);
    uint8_t *data = builder.GetBufferPointer();
    MY_ASSERT(data != nullptr);
    writeBinaryFile(sp.jointOs().c_str(), data, builder.GetSize());
}
