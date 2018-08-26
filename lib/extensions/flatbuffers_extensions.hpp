#ifndef FLATBUFFERS_EXTENSIONS_HPP
#define FLATBUFFERS_EXTENSIONS_HPP

#include "extensions/help_functions.hpp"
#include "flatbuffers_schemes/file_tree_generated.h"
#include "types/file_tree.hpp"

FileTree *restoreFileTree(const char *fname);


#endif // FLATBUFFERS_EXTENSIONS_HPP
