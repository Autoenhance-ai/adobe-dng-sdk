cmake_minimum_required(VERSION 3.0)

project(dng_validate)

# Set the source directory
set(SRC_DIR ../../source)

# Get the source files
file(GLOB LIB_CPP_FILES ${SRC_DIR}/*.cpp)

# Set the XMP SDK build directory if we don't want to install it system-wide
set(XMP_PUB_DIR ../../../xmp_sdk/public)
set(XMP_RELEASE ${XMP_PUB_DIR}/libraries/i80386linux_x64/release/)

# Set the compiler flags
include_directories(${XMP_PUB_DIR}/include ${SRC_DIR})

# Set the archive flags
set(ARFLAGS crus)

# Add the executable target
add_executable(dng_validate ${SRC_DIR}/dng_validate.cpp)

# Add the library target
add_library(dng_sdk STATIC ${LIB_CPP_FILES})

# Set the archive flags for the library target
set_target_properties(dng_sdk PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(dng_sdk PROPERTIES ARCHIVE_OUTPUT_NAME libdng_sdk.a)
set_target_properties(dng_sdk PROPERTIES ARCHIVE_FLAGS ${ARFLAGS})

# Set the libraries
find_library(JPEG_LIBRARY jpeg)
find_library(ZLIB_LIBRARY z)
find_library(PTHREAD_LIBRARY pthread)
find_library(DL_LIBRARY dl)

target_link_libraries(dng_validate dng_sdk ${LIBRARIES})

# Set the clean target
add_custom_target(clean-all
    COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*
    COMMAND rm -f dng_validate
)

#
# Compiler definitions.
#

# Enable compiling of the dng_validate target
#
target_compile_definitions(dng_validate PRIVATE -DqDNGValidateTarget)

# Specify we are compiling for Linux
#
target_compile_definitions(dng_sdk PRIVATE -DqLinux)
target_compile_definitions(dng_validate PRIVATE -DqLinux)
