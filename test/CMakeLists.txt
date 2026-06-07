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
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-ampersand-reject.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-cdata.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-doctype.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-encoding.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-entities.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-name-production.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-namespace.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-null.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-pi.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-string-clone.c"
	"${CMAKE_CURRENT_LIST_DIR}/unit-c-unique-attributes.c"
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
set_tests_properties("${PROJECT_NAME}-test-c" PROPERTIES LABELS "lowlevel")

# Feature unit tests (spec-driven; from docs/FeatureTestCases.md)
add_executable(
	"${PROJECT_NAME}-test-features"
	"${CMAKE_CURRENT_LIST_DIR}/ut_main.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_parse_document.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_open_document.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_document_root.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_node_content.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_node_attributes.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_easy_child.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_c_string_api.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_string_api.c"
	"${CMAKE_CURRENT_LIST_DIR}/ut_integration.c"
	"${CMAKE_CURRENT_LIST_DIR}/wrap_realloc.c"
)

target_include_directories(
	"${PROJECT_NAME}-test-features"
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_compile_options(
	"${PROJECT_NAME}-test-features"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)

target_link_libraries(
	"${PROJECT_NAME}-test-features"
	PRIVATE
		xml
		cmocka
)

target_link_options(
	"${PROJECT_NAME}-test-features"
	PRIVATE
		"LINKER:--wrap=realloc"
)

add_test(
	NAME "${PROJECT_NAME}-test-features"
	COMMAND "${PROJECT_NAME}-test-features"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
set_tests_properties("${PROJECT_NAME}-test-features" PROPERTIES LABELS "unit")


# Valgrind memory-check test (optional; added only if valgrind is installed)
find_program(VALGRIND_EXECUTABLE NAMES valgrind)
if(VALGRIND_EXECUTABLE)
	add_test(
		NAME "${PROJECT_NAME}-test-c-valgrind"
		COMMAND "${VALGRIND_EXECUTABLE}" --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-test-c"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	)
endif()
