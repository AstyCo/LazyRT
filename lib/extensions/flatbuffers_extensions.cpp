#include "extensions/flatbuffers_extensions.hpp"

template<typename FT, typename T>
void FileTreeFunc::copyVector(const FT &flatVector, T &v)
{
    std::transform(flatVector.begin(),
                   flatVector.end(),
                   std::inserter(v, v.end()),
                   [](const flatbuffers::String* value) {
                        return value->str();
                   }
    );
}

typedef flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String> > FB_VectorOfStrings;

template void FileTreeFunc::copyVector<FB_VectorOfStrings,
                                       std::list<IncludeDirective> >(
    const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> &,
    std::list<IncludeDirective> &);

void FileTreeFunc::deserialize(FileTree &tree, const std::string &fname)
{
    char *file_tree_dump = readBinaryFile(fname.c_str()).first;
    if (file_tree_dump) {
        auto file_tree = LazyUT::GetFileTree(file_tree_dump);

        tree._state = FileTree::Restored;
        tree._rootPath = file_tree->rootPath()->str();
        auto &records = *file_tree->records();
        for (const auto &record : records) {
            SplittedPath sp = record->path()->str();
            sp.setOsSeparator();
            FileNode *fnode = tree.addFile(sp);
            FileRecord &fileRecord = fnode->record();

            fileRecord.setHash(record->md5()->data()); // Hash
            copyVector(*record->includes(), fileRecord._listIncludes);
            copyVector(*record->implements(), fileRecord._setImplements);
            copyVector(*record->inheritances(), fileRecord._setInheritances);
            copyVector(*record->class_decls(), fileRecord._setClassDecl);
            copyVector(*record->function_decls(), fileRecord._setFuncDecl);
            copyVector(*record->using_namespaces(), fileRecord._listUsingNamespace);

            /// TODO install separators
        }

        delete file_tree_dump;
    }
}

static  flatbuffers::Offset<
            flatbuffers::Vector<
                flatbuffers::Offset<
                    flatbuffers::String>>>
    CreateVectorOfStrings(flatbuffers::FlatBufferBuilder &builder,
                          const std::list<IncludeDirective> &l)
{
    std::vector<flatbuffers::Offset<flatbuffers::String>> offsets(l.size());
    auto it = l.cbegin();
    int i = 0;
    for (auto &dir: l)
        offsets[i++] = builder.CreateString(dir.filename);
    return builder.CreateVector(offsets);
}

template <typename TContainer>
static  flatbuffers::Offset<
            flatbuffers::Vector<
                flatbuffers::Offset<
                    flatbuffers::String>>>
    CreateVectorOfStrings(flatbuffers::FlatBufferBuilder &builder,
                          const TContainer &container)
{
    std::vector<flatbuffers::Offset<flatbuffers::String>> offsets(container.size());
    auto it = container.cbegin();
    int i = 0;
    for (auto &scopedName: container)
        offsets[i++] = builder.CreateString(scopedName.joint());
    return builder.CreateVector(offsets);
}


static void pushFiles(flatbuffers::FlatBufferBuilder &builder,
               std::vector<flatbuffers::Offset<LazyUT::FileRecord> > &records, const FileNode *node)
{
    if (node->isRegularFile()) {
        auto frecord = node->record();
        auto fbs_frecord = LazyUT::CreateFileRecord(
                    builder,
                    builder.CreateString(frecord._path.joint()),
                    builder.CreateVector(frecord._hashArray, 16),
                    CreateVectorOfStrings(builder, frecord._listIncludes),
                    CreateVectorOfStrings(builder, frecord._setImplements),
                    CreateVectorOfStrings(builder, frecord._setInheritances),
                    CreateVectorOfStrings(builder, frecord._setClassDecl),
                    CreateVectorOfStrings(builder, frecord._setFuncDecl),
                    CreateVectorOfStrings(builder, frecord._listUsingNamespace)
                    );
        records.push_back(fbs_frecord);
    }

    const FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeConstIterator it(childs.cbegin());
    while (it != childs.cend()) {
        pushFiles(builder, records, *it);

        ++it;
    }
}

void FileTreeFunc::serialize(const FileTree &tree, const std::__cxx11::string &fileName)
{
    // store to binary file
    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<LazyUT::FileRecord> > fileRecords;
    pushFiles(builder, fileRecords, tree._rootDirectoryNode);


    auto fbs_file_tree = LazyUT::CreateFileTree(builder,
                                                     builder.CreateString(tree._rootPath.joint()),
                                                     builder.CreateVector(fileRecords));

    builder.Finish(fbs_file_tree);
    uint8_t *data = builder.GetBufferPointer();
    MY_ASSERT(data != nullptr);
    writeBinaryFile(fileName.c_str(), data, builder.GetSize());
}
