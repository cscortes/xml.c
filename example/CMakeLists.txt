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
