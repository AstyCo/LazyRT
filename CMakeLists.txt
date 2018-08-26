cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

##
## PROJECT
## name and version
##
project(lazyut VERSION 0.1.0 LANGUAGES CXX)

##
## INCLUDE
##
##

##
## CONFIGURATION
##
set(LAZYUT_TARGET_NAME                  ${PROJECT_NAME})
set(LAZYUT_CONFIG_INSTALL_DIR           "lib/cmake/${PROJECT_NAME}"
  CACHE INTERNAL "")
set(LAZYUT_INCLUDE_INSTALL_DIR          "include")
set(LAZYUT_TARGETS_EXPORT_NAME          "${PROJECT_NAME}Targets")
set(LAZYUT_CMAKE_CONFIG_TEMPLATE        "cmake/config.cmake.in")
set(LAZYUT_CMAKE_CONFIG_DIR             "${CMAKE_CURRENT_BINARY_DIR}")
set(LAZYUT_CMAKE_VERSION_CONFIG_FILE    "${LAZYUT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(LAZYUT_CMAKE_PROJECT_CONFIG_FILE    "${LAZYUT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(LAZYUT_CMAKE_PROJECT_TARGETS_FILE   "${LAZYUT_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")

##
## HEADERS
## project .h .hpp files
##
set(FLATBUFFERS_GENERATED_HEADERS
    lib/flatbuffers_schemes/file_tree_generated.h
    )
set(FLATBUFFERS_HEADERS
    ${FLATBUFFERS_GENERATED_HEADERS}
    lib/flatbuffers/code_generators.h
    lib/flatbuffers/base.h
    lib/flatbuffers/flatbuffers.h
    lib/flatbuffers/hash.h
    lib/flatbuffers/idl.h
    lib/flatbuffers/util.h
    lib/flatbuffers/reflection.h
    lib/flatbuffers/reflection_generated.h
    lib/flatbuffers/stl_emulation.h
    lib/flatbuffers/flexbuffers.h
    lib/flatbuffers/registry.h
    lib/flatbuffers/minireflect.h
    )
set(HEADERS
    ${FLATBUFFERS_HEADERS}
    lib/extensions/error_reporter.hpp
    lib/extensions/help_functions.hpp
    lib/extensions/md5.hpp
    lib/extensions/flatbuffers_extensions.hpp
    lib/types/file_tree.hpp
    lib/types/splitted_path.hpp
    lib/types/scoped_name.hpp
    lib/parsers/directoryparser.hpp
    lib/parsers/sourceparser.hpp
    lib/lazyut_global.hpp)

##
## SOURCES
## project .cpp files
##
set(FLATBUFFERS_SRCS
    lib/flatbuffers/code_generators.cpp
    lib/flatbuffers/idl_parser.cpp
    lib/flatbuffers/idl_gen_text.cpp
    lib/flatbuffers/reflection.cpp
    lib/flatbuffers/util.cpp
    )
set(SRCS
    ${FLATBUFFERS_SRCS}
    lib/main.cpp
    lib/parsers/directoryparser.cpp
    lib/parsers/sourceparser.cpp
    lib/extensions/error_reporter.cpp
    lib/extensions/help_functions.cpp
    lib/extensions/profiling.cpp
    lib/extensions/murmur_hash_2.cpp
    lib/extensions/md5.cpp
    lib/extensions/flatbuffers_extensions.cpp
    lib/types/file_tree.cpp
    lib/types/splitted_path.cpp
    lib/types/scoped_name.cpp)

##
## TARGET
## create target and add include path
##
add_executable(${LAZYUT_TARGET_NAME} ${SRCS} ${HEADERS})
target_include_directories(${LAZYUT_TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_compile_features(${LAZYUT_TARGET_NAME} PRIVATE cxx_std_11)

# link boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#C:/Soft/boost/boost_1_62_0

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
