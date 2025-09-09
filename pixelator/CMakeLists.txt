cmake_minimum_required(VERSION 3.10.2)
project(pixelator C)

set(PIXELATOR_SRC_FILES
	src/pixelator.c
)

if(PIXELATOR_BUILD_AS_EXECUTABLE)
	add_executable(pixelator ${PIXELATOR_SRC_FILES}
						 	 src/main.c)
else()
	add_library(pixelator SHARED ${PIXELATOR_SRC_FILES})
endif()


target_include_directories(pixelator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pixelator PRIVATE libjpeg turbojpeg)