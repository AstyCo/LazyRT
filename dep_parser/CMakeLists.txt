# just create an executable and link it against object library

add_executable(${DEP_PARSER_TARGET_NAME} main.cpp)

target_link_libraries(${DEP_PARSER_TARGET_NAME} ${LAZYUT_LIB_NAME})
