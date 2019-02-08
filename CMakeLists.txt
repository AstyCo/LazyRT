cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

##
## PROJECT
## name and version
##
project(lazyut VERSION 0.1.1 LANGUAGES CXX)

##
## INCLUDE
##
include(cmake/options.cmake REQUIRED)

##
## TESTING
##
if (BUILD_TESTS)
    enable_testing()
endif()


##
## SUBDIRECTORIES
##
add_subdirectory(lib) # source files used in subprojects
add_subdirectory(lazyut) # lazyut executable
#add_subdirectory(dep_parser) # utility for parsing extra dependencies
add_subdirectory(test) # Tests

include(cmake/printInfo.cmake REQUIRED) # prints general configuration
