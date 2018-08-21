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
    include/flatbuffers_schemes/file_tree_generated.h
    )
set(FLATBUFFERS_HEADERS
    ${FLATBUFFERS_GENERATED_HEADERS}
    include/flatbuffers/code_generators.h
    include/flatbuffers/base.h
    include/flatbuffers/flatbuffers.h
    include/flatbuffers/hash.h
    include/flatbuffers/idl.h
    include/flatbuffers/util.h
    include/flatbuffers/reflection.h
    include/flatbuffers/reflection_generated.h
    include/flatbuffers/stl_emulation.h
    include/flatbuffers/flexbuffers.h
    include/flatbuffers/registry.h
    include/flatbuffers/minireflect.h
    )
set(HEADERS
    ${FLATBUFFERS_HEADERS}
    include/directoryparser.hpp
    include/extensions/error_reporter.hpp
    include/extensions/help_functions.hpp
    include/extensions/md5.hpp
    include/extensions/flatbuffers_extensions.hpp
    include/types/file_tree.hpp
    include/types/splitted_path.hpp
    include/lazyut_global.hpp)

##
## SOURCES
## project .cpp files
##
set(FLATBUFFERS_SRCS
    src/flatbuffers/code_generators.cpp
    src/flatbuffers/idl_parser.cpp
    src/flatbuffers/idl_gen_text.cpp
    src/flatbuffers/reflection.cpp
    src/flatbuffers/util.cpp
    )
set(SRCS
    ${FLATBUFFERS_SRCS}
    src/main.cpp
    src/directoryparser.cpp
    src/extensions/error_reporter.cpp
    src/extensions/help_functions.cpp
    src/extensions/profiling.cpp
    src/extensions/murmur_hash_2.cpp
    src/extensions/md5.cpp
    src/extensions/flatbuffers_extensions.cpp
    src/types/file_tree.cpp
    src/types/splitted_path.cpp)

##
## TARGET
## create target and add include path
##
add_executable(${LAZYUT_TARGET_NAME} ${SRCS} ${HEADERS})
target_include_directories(${LAZYUT_TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
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
