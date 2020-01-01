#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <multidim.hpp>

TEST_CASE("1D array operator[]", "[1d][array][operator bracket]") {
	multidim::array<int, 4> arr;
	arr[0] = 5;
	arr[1] = 7;
	arr[2] = 9;
	arr[3] = 10;
	REQUIRE(arr[0] == 5);
	REQUIRE(arr[1] == 7);
	REQUIRE(arr[2] == 9);
	REQUIRE(arr[3] == 10);
}

TEST_CASE("2D array operator[]", "[2d][array][operator bracket]") {
	multidim::array<multidim::inner_array<int, 3>, 3> arr2;
	arr2[1][1] = 45;
	REQUIRE(arr2[1][1] == 45);
}

TEST_CASE("1D array iterator", "[1d][array][iterator]") {
	multidim::array<int, 4> arr;
	arr[0] = 5;
	arr[1] = 7;
	arr[2] = 9;
	arr[3] = 10;
	std::vector<int> ans{ 5,7,9,10 };
	SECTION("non-const iterator") {
		std::vector<int> tmp;
		for (auto x : arr) {
			tmp.push_back(x);
		}
		REQUIRE(tmp == ans);
	}
	SECTION("const iterator") {
		std::vector<int> tmp;
		for (auto x : static_cast<std::add_const_t<decltype(arr)>>(arr)) {
			tmp.push_back(x);
		}
		REQUIRE(tmp == ans);
	}
	SECTION("cbegin cend") {
		std::vector<int> tmp;
		for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
			tmp.push_back(*it);
		}
		REQUIRE(tmp == ans);
	}
	
}
