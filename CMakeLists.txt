# Project setup
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(xml LANGUAGES C VERSION 0.3.3)


# Fetch cmocka (C unit test framework) at a fixed version
set(CMOCKA_VERSION "2.0.0")
set(UNIT_TESTING OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
	cmocka
	URL "https://cmocka.org/files/2.0/cmocka-${CMOCKA_VERSION}.tar.xz"
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(cmocka)


# Define main library target
add_library(xml STATIC "")


# Compiler setup
target_compile_options(
	xml
	PRIVATE
		-std=c11
)


# Options
option(XML_PARSER_VERBOSE "Enable to be told everything the xml parser does" OFF)

if(XML_PARSER_VERBOSE)
	target_compile_definitions(
		xml
		PRIVATE
			XML_PARSER_VERBOSE
	)
endif(XML_PARSER_VERBOSE)


# Sources
target_sources(
	xml
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/src/xml.c"
)


target_include_directories(
	xml
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/src/"
)


# Example program
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/example")

# Build unit cases
enable_testing()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")

