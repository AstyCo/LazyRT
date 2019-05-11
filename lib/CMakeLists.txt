set(FLATBUFFERS_PWD external/flatbuffers)
set(CLI11_PWD external/CLI11)

##
## HEADERS
## project .h .hpp files
##
set(FLATBUFFERS_GENERATED_HEADERS
    flatbuffers_schemes/file_tree_generated.h)
set(FLATBUFFERS_HEADERS
    ${FLATBUFFERS_GENERATED_HEADERS}
    ${FLATBUFFERS_PWD}/base.h
    ${FLATBUFFERS_PWD}/code_generators.h
    ${FLATBUFFERS_PWD}/flatbuffers.h
    ${FLATBUFFERS_PWD}/flatc.h
    ${FLATBUFFERS_PWD}/flexbuffers.h
    ${FLATBUFFERS_PWD}/grpc.h
    ${FLATBUFFERS_PWD}/hash.h
    ${FLATBUFFERS_PWD}/idl.h
    ${FLATBUFFERS_PWD}/minireflect.h
    ${FLATBUFFERS_PWD}/reflection.h
    ${FLATBUFFERS_PWD}/reflection_generated.h
    ${FLATBUFFERS_PWD}/registry.h
    ${FLATBUFFERS_PWD}/stl_emulation.h
    ${FLATBUFFERS_PWD}/util.h)

set(CLI11_HEADERS
    ${CLI11_PWD}/CLI11.hpp)

set(EXTERNAL_HEADERS
    ${FLATBUFFERS_HEADERS}
    ${CLI11_HEADERS})

set(HEADERS
    ${FLATBUFFERS_HEADERS}
    ${CLI11_HEADERS}
    extensions/error_reporter.hpp
    extensions/help_functions.hpp
    extensions/md5.hpp
    extensions/flatbuffers_extensions.hpp
    types/file_tree.hpp
    types/splitted_string.hpp
    parsers/sourceparser.hpp
    parsers/tokenizer.hpp
    parsers/parsers_utils.hpp
    directoryreader.hpp
    dependency_analyzer.hpp
    command_line_args.hpp
    extra_dependency_reader.hpp
    lazyut_global.hpp)

##
## SOURCES
## project .cpp files
##
set(FLATBUFFERS_SRCS
    external/flatbuffers/code_generators.cpp
    external/flatbuffers/idl_parser.cpp
    external/flatbuffers/idl_gen_text.cpp
    external/flatbuffers/reflection.cpp
    external/flatbuffers/util.cpp
    )
set(SRCS
    ${FLATBUFFERS_SRCS}
    directoryreader.cpp
    dependency_analyzer.cpp
    command_line_args.cpp
    extra_dependency_reader.cpp
    parsers/sourceparser.cpp
    parsers/tokenizer.cpp
    parsers/parsers_utils.cpp
    extensions/error_reporter.cpp
    extensions/help_functions.cpp
    extensions/profiling.cpp
    extensions/murmur_hash_2.cpp
    extensions/md5.cpp
    extensions/flatbuffers_extensions.cpp
    types/file_tree.cpp
    types/splitted_string.cpp
    )

##
## TARGET
## create target and add include path
##
add_library(${LIB_TARGET_NAME} OBJECT ${SRCS} ${HEADERS})

target_include_directories(${LIB_TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(${LIB_TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/external)

target_compile_features(${LIB_TARGET_NAME} PUBLIC cxx_std_14)
