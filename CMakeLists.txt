set(CMAKE_VERBOSE_MAKEFILE ON)
cmake_minimum_required(VERSION 2.8)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()

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

set(FLATBUFFERS_SRCS
    src/flatbuffers/code_generators.cpp
    src/flatbuffers/idl_parser.cpp
    src/flatbuffers/idl_gen_text.cpp
    src/flatbuffers/reflection.cpp
    src/flatbuffers/util.cpp
    )

set(HEADERS
    ${FLATBUFFERS_HEADERS}
    include/parsers/parser.hpp
    include/extensions/error_reporter.hpp
    include/extensions/help_functions.hpp
    include/extensions/md5.hpp
    include/extensions/flatbuffers_extensions.hpp
    include/types/file_tree.hpp
    include/types/splitted_path.hpp)

add_sources(
#set(SRCS
    ${FLATBUFFERS_SRCS}
    src/main.cpp
    src/parsers/parser.cpp
    src/extensions/error_reporter.cpp
    src/extensions/help_functions.cpp
    src/extensions/profiling.cpp
    src/extensions/murmur_hash_2.cpp
    src/extensions/md5.cpp
    src/extensions/flatbuffers_extensions.cpp
    src/types/file_tree.cpp
    src/types/splitted_path.cpp)

# +++ C++11 flags block
if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
  # do not apply any global settings if the toolchain
  # is being configured externally
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Werror -Wextra -Wno-unused-parameter")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  if(CYGWIN)
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=gnu++11")
  else(CYGWIN)
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif(CYGWIN)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.4)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
      set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -faligned-new")
    endif()
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wunused-result -Werror=unused-result -Wunused-parameter -Werror=unused-parameter")
  endif()

  # Certain platforms such as ARM do not use signed chars by default
  # which causes issues with certain bounds checks.
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsigned-char")

elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -Werror -Wextra -Wno-unused-parameter")
  if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
  if(NOT ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD" OR
          "${CMAKE_SYSTEM_NAME}" MATCHES "Linux"))
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
  endif()
elseif(MSVC)
  # Visual Studio pedantic build settings
  # warning C4512: assignment operator could not be generated
  # warning C4316: object allocated on the heap may not be aligned
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /wd4512 /wd4316")
endif()
# --- C++11 flags block

add_executable(lazyrt ${SRCS} ${HEADERS})
target_include_directories(lazyrt PRIVATE ${PROJECT_SOURCE_DIR}/include)

# link boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_ROOT C:/include/boost)
set(BOOST_ROOT D:/Soft/boost-lib/boost_1_62_0)
#set(BOOST_INCLUDEDIR D:/Soft/boost_1_67_0)
#set(BOOST_LIBRARYDIR D:/Soft/boost_1_67_0/stage/lib)
#mapped_file - iostreams
find_package(Boost 1.45.0 COMPONENTS filesystem system REQUIRED)

if(Boost_FOUND)
    message("MESSAGE: Boost_FOUND!")
    target_include_directories(lazyrt PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(lazyrt ${Boost_LIBRARIES})
endif()

if (WIN32)
    # disable autolinking in boost
    add_definitions( -DBOOST_ALL_NO_LIB )

    # force all boost libraries to dynamic link (we already disabled
    # autolinking, so I don't know why we need this, but we do!)
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif()
