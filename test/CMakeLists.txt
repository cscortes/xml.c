# xml.c / test
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)



# Test input files (tests run from build dir with input/ subdir)
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/test.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/test-attributes.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/test-attributes-0.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/test-attributes-1.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/test-attributes-2.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/input/minimal.xml"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/input")



# C unit tests (single executable: runner + modules; one report)
add_executable(
	"${PROJECT_NAME}-test-c"
	"${CMAKE_CURRENT_LIST_DIR}/test_main.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-cdata.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-compliance.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-null.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-pi.c"
	"${CMAKE_CURRENT_LIST_DIR}/wrap_realloc.c"
)

target_include_directories(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_compile_options(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)

target_link_libraries(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		xml
		cmocka
)

# Allow tests to mock realloc (e.g. make it return NULL) via cmocka will_return_ptr(__wrap_realloc, NULL)
target_link_options(
	"${PROJECT_NAME}-test-c"
	PRIVATE
		"LINKER:--wrap=realloc"
)

add_test(
	NAME "${PROJECT_NAME}-test-c"
	COMMAND "${PROJECT_NAME}-test-c"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Valgrind memory-check test (optional; added only if valgrind is installed)
find_program(VALGRIND_EXECUTABLE NAMES valgrind)
if(VALGRIND_EXECUTABLE)
	add_test(
		NAME "${PROJECT_NAME}-test-c-valgrind"
		COMMAND "${VALGRIND_EXECUTABLE}" --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-test-c"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	)
endif()
