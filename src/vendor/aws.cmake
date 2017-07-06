ExternalProject_Add(aws_project
    GIT_REPOSITORY "https://github.com/aws/aws-sdk-cpp"
    GIT_TAG "1.1.7"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    CMAKE_ARGS "-DBUILD_ONLY=s3"
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(aws_project SOURCE_DIR)
ExternalProject_Get_Property(aws_project BINARY_DIR)

SET(AWS_INCLUDE_DIRS ${SOURCE_DIR}/aws-cpp-sdk-core/include)
SET(AWS_S3_INCLUDE_DIRS ${SOURCE_DIR}/aws-cpp-sdk-s3/include)

SET(AWS_LINK_DIRS ${BINARY_DIR}/aws-cpp-sdk-core)
SET(AWS_S3_LINK_DIRS ${BINARY_DIR}/aws-cpp-sdk-s3)
