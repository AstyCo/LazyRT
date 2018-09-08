#ifndef FLATBUFFERS_EXTENSIONS_HPP
#define FLATBUFFERS_EXTENSIONS_HPP

#include "extensions/help_functions.hpp"
#include "flatbuffers_schemes/file_tree_generated.h"
#include "types/file_tree.hpp"


namespace FileTreeFunc {

void deserialize(FileTree &tree,const std::string &fileName);
void serialize(const FileTree &tree, const std::string &fileName);

template <typename FT, typename T>
void copyVector(const FT& flatVector, T &v);

} // namespace FileTreeFunc

#endif // FLATBUFFERS_EXTENSIONS_HPP
