# AVI Project

project(mmir-irvl)

set(CMAKE_CXX_STANDARD 14)
set(OpenCV_DIR /opt/homebrew/Cellar/opencv/4.5.5_2/lib/cmake/opencv4)
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(demo main.cpp)
target_link_libraries(demo ${OpenCV_LIBS} )
