add_executable(unit_test
	main.cpp
	catch.hpp
)

set_property(TARGET unit_test PROPERTY CXX_STANDARD 17)
set_property(TARGET unit_test PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET unit_test PROPERTY CXX_EXTENSIONS OFF)

add_test(unit_test unit_test)

target_link_libraries(unit_test multidim)
