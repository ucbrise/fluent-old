ExternalProject_Add(cereal_project
    GIT_REPOSITORY "https://github.com/USCiLab/cereal"
    GIT_TAG "v1.2.2"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(cereal_project SOURCE_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/include)
