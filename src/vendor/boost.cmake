# Resources:
#  - http://bit.ly/2dfK5L1

ExternalProject_Add(boost_project
    URL "http://kent.dl.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.gz"
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

SET(BOOST_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src/boost_project)
