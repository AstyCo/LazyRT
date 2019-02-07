set(FLATBUFFERS_PWD external/flatbuffers)
set(CLI11_PWD external/CLI11)
set(RAPIDJSON_PWD external/rapidjson)

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

set(RAPIDJSON_HEADERS
    ${RAPIDJSON_PWD}/allocators.h
    ${RAPIDJSON_PWD}/cursorstreamwrapper.h
    ${RAPIDJSON_PWD}/document.h
    ${RAPIDJSON_PWD}/encodedstream.h
    ${RAPIDJSON_PWD}/encodings.h
    ${RAPIDJSON_PWD}/error/en.h
    ${RAPIDJSON_PWD}/error/error.h
    ${RAPIDJSON_PWD}/filereadstream.h
    ${RAPIDJSON_PWD}/filewritestream.h
    ${RAPIDJSON_PWD}/fwd.h
    ${RAPIDJSON_PWD}/internal/biginteger.h
    ${RAPIDJSON_PWD}/internal/diyfp.h
    ${RAPIDJSON_PWD}/internal/dtoa.h
    ${RAPIDJSON_PWD}/internal/ieee754.h
    ${RAPIDJSON_PWD}/internal/itoa.h
    ${RAPIDJSON_PWD}/internal/meta.h
    ${RAPIDJSON_PWD}/internal/pow10.h
    ${RAPIDJSON_PWD}/internal/regex.h
    ${RAPIDJSON_PWD}/internal/stack.h
    ${RAPIDJSON_PWD}/internal/strfunc.h
    ${RAPIDJSON_PWD}/internal/strtod.h
    ${RAPIDJSON_PWD}/internal/swap.h
    ${RAPIDJSON_PWD}/istreamwrapper.h
    ${RAPIDJSON_PWD}/memorybuffer.h
    ${RAPIDJSON_PWD}/memorystream.h
    ${RAPIDJSON_PWD}/msinttypes/inttypes.h
    ${RAPIDJSON_PWD}/msinttypes/stdint.h
    ${RAPIDJSON_PWD}/ostreamwrapper.h
    ${RAPIDJSON_PWD}/pointer.h
    ${RAPIDJSON_PWD}/prettywriter.h
    ${RAPIDJSON_PWD}/rapidjson.h
    ${RAPIDJSON_PWD}/reader.h
    ${RAPIDJSON_PWD}/schema.h
    ${RAPIDJSON_PWD}/stream.h
    ${RAPIDJSON_PWD}/stringbuffer.h
    ${RAPIDJSON_PWD}/writer.h)

set(EXTERNAL_HEADERS
    ${FLATBUFFERS_HEADERS}
    ${CLI11_HEADERS}
    ${RAPIDJSON_HEADERS})

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
    directoryreader.hpp
    dependency_analyzer.hpp
    command_line_args.hpp
    json_reader.hpp
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
    main.cpp
    directoryreader.cpp
    dependency_analyzer.cpp
    command_line_args.cpp
    json_reader.cpp
    parsers/sourceparser.cpp
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
add_executable(${LAZYUT_TARGET_NAME} ${SRCS} ${HEADERS})

target_include_directories(${LAZYUT_TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(${LAZYUT_TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/external)

target_compile_features(${LAZYUT_TARGET_NAME} PUBLIC cxx_std_11)

# link boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.45.0 COMPONENTS filesystem system REQUIRED)

if(Boost_FOUND)
    target_include_directories(${LAZYUT_TARGET_NAME}
        PUBLIC
        ${Boost_INCLUDE_DIRS})
    target_link_libraries(${LAZYUT_TARGET_NAME}
        PUBLIC
        ${Boost_LIBRARIES})
endif()

if (WIN32)
    # disable autolinking in boost
    target_compile_definitions(${LAZYUT_TARGET_NAME} PUBLIC BOOST_ALL_NO_LIB)
    # force all boost libraries to dynamic link (we already disabled
    # autolinking, so I don't know why we need this, but we do!)
    target_compile_definitions(${LAZYUT_TARGET_NAME} PUBLIC BOOST_ALL_DYN_LINK)
endif()
