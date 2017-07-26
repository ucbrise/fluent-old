# redox creates a file named libredox.so. By naming this project something
# other than redox, we can link to libredox.so with the name redox.
ExternalProject_Add(redox_project
    GIT_REPOSITORY "https://github.com/hmartiro/redox"
    GIT_TAG "master"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    INSTALL_COMMAND ""
)
SET(REDOX_PROJECT redox_project)

ExternalProject_Get_Property(redox_project SOURCE_DIR)
ExternalProject_Get_Property(redox_project BINARY_DIR)
INCLUDE_DIRECTORIES(SYSTEM ${SOURCE_DIR}/include)
LINK_DIRECTORIES(${BINARY_DIR})
