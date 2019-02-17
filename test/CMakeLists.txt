message(test/CMakeLists.txt)
if (BUILD_TESTS)
	
    file(GLOB hdrs "*.h" "*.hpp")
    file(GLOB srcs "*.cpp")

    add_executable(testrunner ${hdrs} ${srcs})

    message(LIB_TARGET_NAME " " ${LIB_TARGET_NAME})
    target_link_libraries(testrunner ${LIB_TARGET_NAME})

    add_test(NAME testrunner
        COMMAND testrunner
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

endif()
