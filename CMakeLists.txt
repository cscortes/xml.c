# Project setup
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(xml LANGUAGES C VERSION 0.6.3)


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
		-Wall
		-Wextra
		-Wpedantic
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


# API documentation (Doxygen); optional if Doxygen is not installed
find_package(Doxygen QUIET)
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Doxygen_FOUND)
	set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	set(API_DOCS_XML "${CMAKE_CURRENT_BINARY_DIR}/api_docs/xml")
	set(API_DOCS_MD "${CMAKE_CURRENT_SOURCE_DIR}/docs/xml_api.md")
	set(DOXYGEN_XML_TO_MD "${CMAKE_CURRENT_SOURCE_DIR}/scripts/doxygen_xml_to_md.py")
	add_custom_target(api_docs
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM
	)
	if(Python3_FOUND)
		add_custom_command(
			TARGET api_docs
			POST_BUILD
			COMMAND ${Python3_EXECUTABLE} ${DOXYGEN_XML_TO_MD} ${API_DOCS_XML} ${API_DOCS_MD}
			COMMENT "Writing docs/xml_api.md"
		)
	endif()
else()
	message(STATUS "Doxygen not found; api_docs target not available")
endif()

