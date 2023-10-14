cmake_minimum_required(VERSION 3.6)

# Define yombir project
project( yombir VERSION 0.0.1 DESCRIPTION "Native C++ app for thermal image USB cameras that use the Infiray Tiny1-B Micro LWIR Thermal Module." )

# Include yombir sources
include_directories( ${PROJECT_SOURCE_DIR}/src )
file( GLOB_RECURSE ceresSources
    src/*.h
    src/*.cpp
)

# Include OpenCV library
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Include libv4l2cpp library
include_directories( ${PROJECT_SOURCE_DIR}/lib/libv4l2cpp/inc )
file( GLOB_RECURSE libv4l2cppSources
    ${PROJECT_SOURCE_DIR}/lib/libv4l2cpp/inc/*.h
    ${PROJECT_SOURCE_DIR}/lib/libv4l2cpp/src/*.cpp
)


# Define executable
add_executable( ${PROJECT_NAME} src/main.cpp ${libv4l2cppSources} )
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/lib/libv4l2cpp/inc
)
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )


#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
