message(test/CMakeLists.txt)
if (BUILD_TESTS)
	
    file(GLOB hdrs "*.h" "*.hpp")
    file(GLOB srcs "*.cpp")

    add_executable(testrunner ${hdrs} ${srcs})

    message(LAZYUT_TARGET_NAME ${LAZYUT_TARGET_NAME})
    add_dependencies(testrunner ${LAZYUT_TARGET_NAME})

    add_test(NAME testrunner COMMAND testrunner WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

endif()
