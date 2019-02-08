# just create an executable and link it against object library

add_executable(${LAZYUT_TARGET_NAME} main.cpp)

target_link_libraries(${LAZYUT_TARGET_NAME} ${LIB_TARGET_NAME})
