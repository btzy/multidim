add_executable(unit_test
	main.cpp
	catch.hpp
)

add_test(unit_test unit_test)

target_link_libraries(unit_test multidim)
