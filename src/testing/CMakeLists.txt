CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(testing
    mock_clock.cc
)

MACRO(CREATE_TESTING_TEST NAME)
    CREATE_NAMED_TEST(testing_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(testing_${NAME}
        ${GLOG_LINK_DIRS}/libglog.a
        common
        pthread
        testing)
ENDMACRO(CREATE_TESTING_TEST)

CREATE_TESTING_TEST(captured_stdout_test)
CREATE_TESTING_TEST(mock_clock_test)
CREATE_TESTING_TEST(test_util_test)
