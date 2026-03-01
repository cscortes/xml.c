# xml.c / example
cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

add_executable(
	"${PROJECT_NAME}-example"
	"${CMAKE_CURRENT_LIST_DIR}/example.c"
)

target_compile_options(
	"${PROJECT_NAME}-example"
	PRIVATE
		-std=c11
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
