# redox creates a file named libredox.so. By naming this project something
# other than redox, we can link to libredox.so with the name redox.
ExternalProject_Add(redox_project
    GIT_REPOSITORY "https://github.com/hmartiro/redox"
    GIT_TAG "master"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(redox_project SOURCE_DIR)
SET(REDOX_INCLUDE_DIRS ${SOURCE_DIR}/include)

ExternalProject_Get_Property(redox_project BINARY_DIR)
SET(REDOX_LINK_DIRS ${BINARY_DIR})
