##
## HEADERS
## project .h .hpp files
##
set(FLATBUFFERS_GENERATED_HEADERS
    flatbuffers_schemes/file_tree_generated.h
    )
set(FLATBUFFERS_HEADERS
    ${FLATBUFFERS_GENERATED_HEADERS}
    flatbuffers/code_generators.h
    flatbuffers/base.h
    flatbuffers/flatbuffers.h
    flatbuffers/hash.h
    flatbuffers/idl.h
    flatbuffers/util.h
    flatbuffers/reflection.h
    flatbuffers/reflection_generated.h
    flatbuffers/stl_emulation.h
    flatbuffers/flexbuffers.h
    flatbuffers/registry.h
    flatbuffers/minireflect.h
    )
set(HEADERS
    ${FLATBUFFERS_HEADERS}
    extensions/error_reporter.hpp
    extensions/help_functions.hpp
    extensions/md5.hpp
    extensions/flatbuffers_extensions.hpp
    types/file_tree.hpp
    types/splitted_path.hpp
    types/scoped_name.hpp
    parsers/sourceparser.hpp
    dependency_analyzer.hpp
    directoryreader.hpp
    lazyut_global.hpp)

##
## SOURCES
## project .cpp files
##
set(FLATBUFFERS_SRCS
    flatbuffers/code_generators.cpp
    flatbuffers/idl_parser.cpp
    flatbuffers/idl_gen_text.cpp
    flatbuffers/reflection.cpp
    flatbuffers/util.cpp
    )
set(SRCS
    ${FLATBUFFERS_SRCS}
    main.cpp
    directoryreader.cpp
    dependency_analyzer.cpp
    parsers/sourceparser.cpp
    extensions/error_reporter.cpp
    extensions/help_functions.cpp
    extensions/profiling.cpp
    extensions/murmur_hash_2.cpp
    extensions/md5.cpp
    extensions/flatbuffers_extensions.cpp
    types/file_tree.cpp
    types/splitted_path.cpp
    types/scoped_name.cpp)

##
## TARGET
## create target and add include path
##
add_executable(${LAZYUT_TARGET_NAME} ${SRCS} ${HEADERS})
target_include_directories(${LAZYUT_TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_compile_features(${LAZYUT_TARGET_NAME} PUBLIC cxx_std_11)

# link boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.45.0 COMPONENTS filesystem system REQUIRED)

if(Boost_FOUND)
    target_include_directories(${LAZYUT_TARGET_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(${LAZYUT_TARGET_NAME} PUBLIC ${Boost_LIBRARIES})
endif()

if (WIN32)
    # disable autolinking in boost
    add_definitions( -DBOOST_ALL_NO_LIB )

    # force all boost libraries to dynamic link (we already disabled
    # autolinking, so I don't know why we need this, but we do!)
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif()
