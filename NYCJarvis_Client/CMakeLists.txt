cmake_minimum_required(VERSION 3.17)
project(NYCJarvisClient CXX)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES NYCJarvis_Client.cpp)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTKMM REQUIRED gtk4)

set(ASIO_INCLUDE_DIRS "C:/Program Files/asio-1.28.0/include")


ADD_LIBRARY( asio "C:/Program Files/asio-1.28.0/include/asio.hpp" )

set(NYCJARVIS_INCLUDE_DIRS "../NYCJarvis_Common")

include(CMakePrintHelpers)
cmake_print_variables(GTK_INCLUDE_DIRS)
cmake_print_variables(NYCJARVIS_INCLUDE_DIRS)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} asio "C:/Windows/System32/ws2_32.dll" "C:/Windows/System32/mswsock.dll")

target_include_directories(
    ${PROJECT_NAME} PRIVATE ${GTKMM_INCLUDE_DIRS} PRIVATE ${NYCJARVIS_INCLUDE_DIRS}
    asio ${ASIO_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME} PRIVATE ${GTKMM_LIBRARY_DIRS})

set_target_properties(asio PROPERTIES LINKER_LANGUAGE CXX)