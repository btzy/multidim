#include "catch.hpp"

#include <array>
#include <forward_list>
#include <iterator>
#include <algorithm>
#include <multidim/alg_partition.hpp>

TEST_CASE("algorithms is_partitioned", "[algorithm]") {
	std::array<int, 10> arr = { { 1,5,3,4,7,9,6,10,8,8 } };
	REQUIRE(multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 5; }));
	REQUIRE(multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 1; }));
	REQUIRE(!multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 6; }));
	REQUIRE(multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 0; }));
	REQUIRE(multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 10; }));
	REQUIRE(!multidim::is_partitioned(arr.begin(), arr.end(), [](int x) {return x <= 8; }));
}

TEST_CASE("algorithms partition and partition_copy", "[algorithm]") {
	auto test_bidir = [](const std::array<int, 10>& orig, auto pred, int expect) {
		std::array<int, 10> arr = orig;
		auto it = multidim::partition(arr.begin(), arr.end(), pred);
		REQUIRE(std::distance(arr.begin(), it) == expect);
		REQUIRE(std::is_partitioned(arr.begin(), arr.end(), pred));
	};
	auto test_forward = [](const std::array<int, 10>& orig, auto pred, int expect) {
		std::forward_list<int> arr(orig.begin(), orig.end());
		auto it = multidim::partition(arr.begin(), arr.end(), pred);
		REQUIRE(std::distance(arr.begin(), it) == expect);
		REQUIRE(std::is_partitioned(arr.begin(), arr.end(), pred));
	};
	auto test_copy = [](const std::array<int, 10>& arr, auto pred, int expect) {
		std::vector<int> true_1, false_1, true_2, false_2;
		multidim::partition_copy(arr.begin(), arr.end(), std::back_inserter(true_1), std::back_inserter(false_1), pred);
		std::partition_copy(arr.begin(), arr.end(), std::back_inserter(true_2), std::back_inserter(false_2), pred);
		REQUIRE(true_1 == true_2);
		REQUIRE(false_1 == false_1);
		REQUIRE(true_1.size() == static_cast<size_t>(expect));
	};
	auto test_stable = [](const std::array<int, 10>& orig, auto pred, int expect) {
		std::array<int, 10> arr_1 = orig, arr_2 = orig;
		auto it = multidim::stable_partition(arr_1.begin(), arr_1.end(), pred);
		std::stable_partition(arr_2.begin(), arr_2.end(), pred);
		REQUIRE(arr_1 == arr_2);
		REQUIRE(std::distance(arr_1.begin(), it) == expect);
	};
	{
		std::array<int, 10> arr = { { 1,5,3,4,7,9,6,10,8,8 } };
		auto lambda = [](int x) {return x % 2 == 0; };
		test_bidir(arr, lambda, 5);
		test_forward(arr, lambda, 5);
		test_copy(arr, lambda, 5);
		test_stable(arr, lambda, 5);
	}
	{
		std::array<int, 10> arr = { { 1,5,3,4,7,9,6,10,8,8 } };
		auto lambda = [](int x) {return x % 2 != 0; };
		test_bidir(arr, lambda, 5);
		test_forward(arr, lambda, 5);
		test_copy(arr, lambda, 5);
		test_stable(arr, lambda, 5);
	}
	{
		std::array<int, 10> arr = { { 1,5,3,4,7,9,6,10,8,8 } };
		auto lambda = [](int x) {return x <= 7; };
		test_bidir(arr, lambda, 6);
		test_forward(arr, lambda, 6);
		test_copy(arr, lambda, 6);
		test_stable(arr, lambda, 6);
	}
}

TEST_CASE("algorithms partition_point", "[algorithm]") {
	std::array<int, 16> arr = { { 1,1,5,6,6,8,10,10,10,11,12,16,17,19,19,25 } };
	REQUIRE(std::is_sorted(arr.begin(), arr.end()));
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= -1; })) == 0);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 0; })) == 0);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 1; })) == 2);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 2; })) == 2);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 8; })) == 6);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 9; })) == 6);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 10; })) == 9);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 11; })) == 10);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 12; })) == 11);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 13; })) == 11);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 24; })) == 15);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 25; })) == 16);
	REQUIRE(std::distance(arr.begin(), multidim::partition_point(arr.begin(), arr.end(), [](int x) {return x <= 30; })) == 16);
}
