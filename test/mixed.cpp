#include "catch.hpp"

#include <multidim/array.hpp>
#include <multidim/dynarray.hpp>

TEST_CASE("mixed array types 1", "[mixed]") {
	multidim::dynarray<multidim::inner_array<int, 4>> arr(60);
	for (int i = 0; i < 60; ++i) {
		for (int j = 0; j < 4; ++j) {
			arr[i][j] = i * 4 + j;
		}
	}
	REQUIRE(arr[0][0] == 0);
	REQUIRE(arr[59][3] == 239);
	auto copy = arr;
	REQUIRE(copy == arr);
	multidim::array<int, 4> row;
	for (int i = 0; i < 4; ++i) {
		row[i] = 8 + i;
	}
	REQUIRE(arr[2] == row);
	REQUIRE(row == arr[2]);
	arr[4] = row;
	REQUIRE(arr[4] == row);
	REQUIRE(arr != copy);
	multidim::dynarray<multidim::inner_array<int, 4>>::reference row_ref = arr[5];
	multidim::dynarray<multidim::inner_array<int, 4>> arr2(60);
	REQUIRE(row_ref == arr[5]);
	REQUIRE(row_ref != arr2[5]);
	swap(arr, arr2); // ADL swap of dynamic_buffer will swap pointers to the heap buffer, so row_ref will point to the row in arr2 now
	REQUIRE(row_ref == arr2[5]);
	REQUIRE(row_ref != arr[5]);
	multidim::dynarray<multidim::inner_array<int, 4>> arr3; // zero size dynarray
	swap(arr2, arr3);
	REQUIRE(row_ref == arr3[5]);
}

TEST_CASE("mixed array types 2", "[mixed]") {
	multidim::array<multidim::inner_dynarray<int>, 60> arr(4);
	for (int i = 0; i < 60; ++i) {
		for (int j = 0; j < 4; ++j) {
			arr[i][j] = i * 4 + j;
		}
	}
	REQUIRE(arr[0][0] == 0);
	REQUIRE(arr[59][3] == 239);
	auto copy = arr;
	REQUIRE(copy == arr);
	multidim::dynarray<int> row(4);
	for (int i = 0; i < 4; ++i) {
		row[i] = 8 + i;
	}
	REQUIRE(arr[2] == row);
	REQUIRE(row == arr[2]);
	arr[4] = row;
	REQUIRE(arr[4] == row);
	REQUIRE(arr != copy);
}
