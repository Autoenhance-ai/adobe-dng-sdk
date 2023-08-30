# This CMake file is based on the Makefile in https://github.com/yanburman/dng_sdk
# which originally was targeting the 1.5 version of the Adobe DNG SDK.
# 
# We have ported this to Cmake and updated it to build the 1.6 version of the SDK.
#
cmake_minimum_required(VERSION 3.0)
project(dng)

set(SRC_DIR ../../source)
set(BUILD_DIR ../../build)

# XMP Toolkit
#

# We build dynamic libraries by default. Currently static libraries don't build
# as cmake tries to link using the standard .a extension but the XMP script moves
# them to an .ar extension which breaks the linking. 
#
set(XMP_BUILD_STATIC OFF)
message("XMP_BUILD_STATIC: ${XMP_BUILD_STATIC}")

# Keep track of the name of the target for the XMP libraries which changes
# depending on whether we are building static or dynamic libraries.
#
if(XMP_BUILD_STATIC)
  set(XMP_CORE_TARGET "XMPCoreStatic")
  set(XMP_FILES_TARGET "XMPFilesStatic")
else()
  set(XMP_CORE_TARGET "XMPCore")
  set(XMP_FILES_TARGET "XMPFiles")
endif()

# Note: The XMP bundled with the DNG SDK doesn't compile for Linux due to some missing
# build files. We have to use the XMP SDK from the XMP SDK repo, which is pointed
# by setting this variable when running cmake "cmake -DXMP_ROOT=../../xmp/toolkit"
#
add_subdirectory(${XMP_ROOT}/build/ ${BUILD_DIR}/xmp)

# Zlib
#
# For whatever reason the zlib symbols don't end up in the XMP library so we have to
# compile and link them into the DNG library ourselves.
#
add_subdirectory(${XMP_ROOT}/third-party/zlib ${BUILD_DIR}/zlib)

# Setup DNG SDK
# 
file(GLOB LIB_CPP_FILES ${SRC_DIR}/*.cpp)
add_library(dng_sdk STATIC ${LIB_CPP_FILES})

find_library(JPEG_LIBRARY jpeg)
find_library(PTHREAD_LIBRARY pthread)
find_library(DL_LIBRARY dl)

target_link_libraries(
    dng_sdk
    zlibstatic
    ${LIBRARIES}
    ${XMP_CORE_TARGET}
    ${XMP_FILES_TARGET}
)

# Setup DNG Validate binary
#
add_executable(dng_validate ${SRC_DIR}/dng_validate.cpp)

target_link_libraries(
    dng_validate 
    dng_sdk
    ${LIBRARIES}
    ${XMP_CORE_TARGET}
    ${XMP_FILES_TARGET}
)

# Create clean-all target to clean the build directory
#
add_custom_target(clean-all
    COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*
    COMMAND rm -f dng_validate
)

# Specify we are compiling for Linux so that the DNG SDK can configure itself
# for compiling on Linux.
#
target_compile_definitions(dng_sdk PRIVATE -DqLinux)
target_compile_definitions(dng_sdk PRIVATE -DUNIX_ENV=1)
target_compile_definitions(dng_validate PRIVATE -DqLinux)
target_compile_definitions(dng_validate PRIVATE -DUNIX_ENV=1)

# Specify where to find the headers
#
target_include_directories(dng_validate PRIVATE ${XMP_ROOT}/public/include/)
target_include_directories(dng_sdk PRIVATE ${XMP_ROOT}/public/include/)

# Do not enable XMP Document Operations since we don't require this feature
# in order to build the DNG SDK and there currently issues with the methods
# it requires being built as it required private methods to be built which
# currently don't appear to build even with the AdobePrivate flag set.
#
target_compile_definitions(dng_sdk PRIVATE -DqDNGXMPDocOps=0)

# Enable compiling of the dng_validate binary for dng_validate
# and setup the SDK to be built with the required methods needed to
# build the dng_validate binary.
#
target_compile_definitions(dng_validate PRIVATE -DqDNGValidateTarget)
target_compile_definitions(dng_sdk PRIVATE -DqDNGValidate)

# Specify we want the internal Adobe methods to be compiled for XMP
# which the DNG SDK uses.
#
target_compile_definitions(dng_sdk PRIVATE -DAdobePrivate=1)
target_compile_definitions(dng_validate PRIVATE -DAdobePrivate=1)

target_compile_definitions(dng_sdk PRIVATE -DAdobePrivate=1)
target_compile_definitions(dng_validate PRIVATE -DAdobePrivate=1)

# Make sure all the libraries are built before we build the DNG SDK
#
add_dependencies(dng_sdk zlibstatic ${XMP_CORE_TARGET} ${XMP_FILES_TARGET})