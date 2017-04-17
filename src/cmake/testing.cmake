CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# CREATE_NAMED_TEST(foo bar.cc) creates a test named `foo` from the file
# `bar.cc`. Refer to [1] and [2] for documentation on how ADD_TEST works.
#
# [1]: http://stackoverflow.com/a/21413672/3187068
# [2]: https://cmake.org/cmake/help/v3.0/command/add_test.html
MACRO(CREATE_NAMED_TEST NAME FILENAME)
    ADD_EXECUTABLE(${NAME} ${FILENAME})
    ADD_TEST(NAME ${NAME} COMMAND ${NAME})
    ADD_DEPENDENCIES(${NAME} googletest)
    TARGET_LINK_LIBRARIES(${NAME}
        ${GTEST_LIBS_DIR}/libgtest.a
        ${GTEST_LIBS_DIR}/libgtest_main.a
        ${GMOCK_LIBS_DIR}/libgmock.a
        ${GMOCK_LIBS_DIR}/libgmock_main.a
        pthread
    )
ENDMACRO(CREATE_NAMED_TEST)

# CREATE_TEST(foo) creates a test named `foo` from the file `foo.cc`.
MACRO(CREATE_TEST NAME)
    CREATE_NAMED_TEST(${NAME} ${NAME}.cc)
ENDMACRO(CREATE_TEST)
