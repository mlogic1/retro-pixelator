cmake_minimum_required(VERSION 3.10.2)
project(retro)
# add_subdirectory(lib/libjpeg-turbo)

include(ExternalProject)

set(LIBJPEG_INSTALL_DIR ${CMAKE_BINARY_DIR}/libjpeg_install)

ExternalProject_Add(libjpeg_turbo
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
    GIT_TAG        3.0.2
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${LIBJPEG_INSTALL_DIR}
        -DENABLE_SHARED=OFF
        -DENABLE_STATIC=ON
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
)

add_library(libjpeg INTERFACE)
add_dependencies(libjpeg libjpeg_turbo)

# Include and link directories
target_include_directories(libjpeg INTERFACE ${LIBJPEG_INSTALL_DIR}/include)
target_link_directories(libjpeg INTERFACE ${LIBJPEG_INSTALL_DIR}/lib)
target_link_libraries(libjpeg INTERFACE jpeg)

add_subdirectory(pixelator)