CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ExternalProject)

# redox creates a file named libredox.so. By naming this project something
# other than redox, we can link to libredox.so with the name redox.
ExternalProject_Add(redoxlib
    GIT_REPOSITORY "https://github.com/hmartiro/redox"
    GIT_TAG "master"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(redoxlib SOURCE_DIR)
set(REDOX_INCLUDE_DIRS ${SOURCE_DIR}/include PARENT_SCOPE)

ExternalProject_Get_Property(redoxlib BINARY_DIR)
set(REDOX_LINK_DIRS ${BINARY_DIR} PARENT_SCOPE)
