# xml.c / example
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

add_executable(
	"${PROJECT_NAME}-example"
	"${CMAKE_CURRENT_LIST_DIR}/example.c"
)

target_compile_options(
	"${PROJECT_NAME}-example"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)

target_link_libraries(
	"${PROJECT_NAME}-example"
	PRIVATE
		xml
)

add_test(
	NAME "${PROJECT_NAME}-example"
	COMMAND "${PROJECT_NAME}-example"
)

# Second example: parses one XML document that demonstrates most new features
add_executable(
	"${PROJECT_NAME}-example2"
	"${CMAKE_CURRENT_LIST_DIR}/example2.c"
)

target_compile_options(
	"${PROJECT_NAME}-example2"
	PRIVATE
		-std=c11
		-Wall
		-Wextra
		-Wpedantic
)

target_link_libraries(
	"${PROJECT_NAME}-example2"
	PRIVATE
		xml
)

add_test(
	NAME "${PROJECT_NAME}-example2"
	COMMAND "${PROJECT_NAME}-example2"
)

# Valgrind memory-check for examples (optional; added only if valgrind is installed)
find_program(VALGRIND_EXECUTABLE NAMES valgrind)
if(VALGRIND_EXECUTABLE)
	add_test(
		NAME "${PROJECT_NAME}-example-valgrind"
		COMMAND "${VALGRIND_EXECUTABLE}" --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-example"
	)
	add_test(
		NAME "${PROJECT_NAME}-example2-valgrind"
		COMMAND "${VALGRIND_EXECUTABLE}" --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-example2"
	)
endif()
