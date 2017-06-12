CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_LIBRARY(common
    error_code.cc
    status.cc
    string_util.cc
)

TARGET_LINK_LIBRARIES(common
    ${FMT_LINK_DIRS}/libfmt.a
    ${GLOG_LINK_DIRS}/libglog.a
)

MACRO(CREATE_COMMON_TEST NAME)
    CREATE_NAMED_TEST(common_${NAME} ${NAME})
    TARGET_LINK_LIBRARIES(common_${NAME} common ${GLOG_LINK_DIRS}/libglog.a)
ENDMACRO(CREATE_COMMON_TEST)

CREATE_COMMON_TEST(collection_util_test)
CREATE_COMMON_TEST(hash_util_test)
CREATE_COMMON_TEST(macros_test)
CREATE_COMMON_TEST(sizet_list_test)
CREATE_COMMON_TEST(static_assert_test)
CREATE_COMMON_TEST(string_util_test)
CREATE_COMMON_TEST(time_util_test)
CREATE_COMMON_TEST(tuple_util_test)
CREATE_COMMON_TEST(type_list_test)
CREATE_COMMON_TEST(type_traits_test)
