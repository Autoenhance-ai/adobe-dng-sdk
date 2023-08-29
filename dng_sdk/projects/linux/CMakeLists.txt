# This CMake file is based on the Makefile in https://github.com/yanburman/dng_sdk
# which originally was targeting the 1.5 version of the Adobe DNG SDK.
# 
# We have ported this to Cmake and updated it to build the 1.6 version of the SDK.
#
cmake_minimum_required(VERSION 3.0)

project(dng_validate)

# Set the source directory
set(SRC_DIR ../../source)

# Get the source files
file(GLOB LIB_CPP_FILES ${SRC_DIR}/*.cpp)

# Set the XMP SDK build directory if we don't want to install it system-wide
set(XMP_DIR ../../../xmp/toolkit/)

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
# Compiler flags.
#

# Specify where to find the headers
#
target_include_directories(dng_validate PRIVATE ${XMP_DIR}/public/include/)
target_include_directories(dng_sdk PRIVATE ${XMP_DIR}/public/include/)

# Enable compiling of the dng_validate target
#
target_compile_definitions(dng_validate PRIVATE -DqDNGValidateTarget)

# Specify we want the internal Adobe methods to be compiled
#
target_compile_definitions(dng_sdk PRIVATE -DAdobePrivate=1)


# # Specify we want the DOM model to be compiled
# #
# target_compile_definitions(dng_sdk PRIVATE -DENABLE_CPP_DOM_MODEL=1)

# Specify we are compiling for Linux
#
target_compile_definitions(dng_sdk PRIVATE -DqLinux)
target_compile_definitions(dng_sdk PRIVATE -DUNIX_ENV=1)
target_compile_definitions(dng_validate PRIVATE -DqLinux)
target_compile_definitions(dng_validate PRIVATE -DUNIX_ENV=1)



# "'XML_STATIC=1'",
# "'HAVE_EXPAT_CONFIG_H=1'",
# "'XMP_StaticBuild=1'",
# "'XMP_64=1'",
# "'BUILDING_XMPCORE_LIB=1'",
# "'XMP_COMPONENT_INT_NAMESPACE=AdobeXMPCore_Int'",
# "'BUILDING_XMPCORE_AS_STATIC=1'",