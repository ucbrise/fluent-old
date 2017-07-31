ExternalProject_Add(aws_project
    GIT_REPOSITORY "https://github.com/aws/aws-sdk-cpp"
    GIT_TAG "1.1.7"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS "-DBUILD_ONLY=s3"
    INSTALL_COMMAND ""
)
SET(AWS_PROJECT aws_project)

ExternalProject_Get_Property(aws_project SOURCE_DIR)
ExternalProject_Get_Property(aws_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/aws-cpp-sdk-core/include)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/aws-cpp-sdk-s3/include)
LINK_DIRECTORIES(${BINARY_DIR}/aws-cpp-sdk-core)
LINK_DIRECTORIES(${BINARY_DIR}/aws-cpp-sdk-s3)
