#define CATCH_CONFIG_MAIN

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

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
	static_assert(std::is_same_v<decltype(*(static_cast<std::add_const_t<decltype(arr)>>(arr).begin())), const int&>, "constness must be propagated through iterator");
	SECTION("begin end") {
		std::vector<int> tmp;
		for (auto it = arr.begin(); it != arr.end(); ++it) {
			tmp.push_back(*it);
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
	SECTION("begin end it++") {
		std::vector<int> tmp;
		for (auto it = arr.begin(); it != arr.end(); it++) {
			tmp.push_back(*it);
		}
		REQUIRE(tmp == ans);
	}
	SECTION("cbegin cend it++") {
		std::vector<int> tmp;
		for (auto it = arr.cbegin(); it != arr.cend(); it++) {
			tmp.push_back(*it);
		}
		REQUIRE(tmp == ans);
	}
	SECTION("begin end") {
		std::vector<int> tmp;
		for (auto it = arr.end(); it != arr.begin(); ) {
			--it;
			tmp.push_back(*it);
		}
		std::reverse(tmp.begin(), tmp.end());
		REQUIRE(tmp == ans);
	}
	SECTION("random access iterators") {
		auto it = arr.begin();
		REQUIRE(*(it + 2) == 9);
		auto it2 = it;
		it2 += 2;
		REQUIRE(*it2 == 9);
		REQUIRE(it2 - it == 2);
		REQUIRE(it2[-2] == 5);
		REQUIRE(it < it2);
		it2 -= 1;
		REQUIRE(*it2 == 7);
		REQUIRE(it[2] == 9);
		--it2;
		REQUIRE(it2 == it);
		REQUIRE(it == it2);
		REQUIRE(!(it != it2));
	}
}

TEST_CASE("2D array iterator", "[2d][array][iterator]") {
	multidim::array<multidim::inner_array<std::pair<int, int>, 6>, 10> arr;
	std::vector<std::pair<int, int>> ans;
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 6; ++j) {
			const int ii = i + 1;
			const int jj = j + 1;
			const auto pr = std::make_pair(ii * ii, jj * jj);
			arr[i][j] = pr;
			ans.push_back(pr);
		}
	}
	REQUIRE(arr[0][0] == std::make_pair(1, 1));
	REQUIRE(arr[9][5] == std::make_pair(100, 36));
	SECTION("non-const iterator") {
		std::vector<std::pair<int, int>> tmp;
		for (auto x : arr) {
			for (auto y : x) {
				tmp.push_back(y);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("const iterator") {
		std::vector<std::pair<int, int>> tmp;
		for (auto x : static_cast<std::add_const_t<decltype(arr)>>(arr)) {
			for (auto y : x) {
				tmp.push_back(y);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("begin end") {
		std::vector<std::pair<int, int>> tmp;
		for (auto it = arr.begin(); it != arr.end(); ++it) {
			for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
				tmp.push_back(*it2);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("cbegin cend mixed") {
		std::vector<std::pair<int, int>> tmp;
		for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
			for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
				tmp.push_back(*it2);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("cbegin cend") {
		std::vector<std::pair<int, int>> tmp;
		for (auto it = arr.cbegin(); it != arr.cend(); ++it) {
			for (auto it2 = it->cbegin(); it2 != it->cend(); ++it2) {
				tmp.push_back(*it2);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("begin end it++") {
		std::vector<std::pair<int, int>> tmp;
		for (auto it = arr.begin(); it != arr.end(); it++) {
			for (auto it2 = it->begin(); it2 != it->end(); it2++) {
				tmp.push_back(*it2);
			}
		}
		REQUIRE(tmp == ans);
	}
	SECTION("iterating in reverse direction") {
		std::vector<std::pair<int, int>> tmp;
		for (auto it = arr.end(); it != arr.begin(); ) {
			--it;
			for (auto it2 = it->end(); it2 != it->begin(); ) {
				--it2;
				tmp.push_back(*it2);
			}
		}
		std::reverse(tmp.begin(), tmp.end());
		REQUIRE(tmp == ans);
	}
	SECTION("random access iterators") {
		auto it = arr.begin();
		REQUIRE((*(it + 2))[0] == std::make_pair(9, 1));
		REQUIRE(*((it + 2)->begin()) == std::make_pair(9, 1));
		auto it2 = it;
		it2 += 2;
		REQUIRE((*it2)[0] == std::make_pair(9, 1));
		REQUIRE(it2 - it == 2);
		REQUIRE(it2[-2][0] == std::make_pair(1, 1));
		REQUIRE(it2[0][3] == std::make_pair(9, 16));
		REQUIRE(it < it2);
		it2 -= 1;
		REQUIRE((*it2)[0] == std::make_pair(4, 1));
		REQUIRE(it[2][1] == std::make_pair(9, 4));
		--it2;
		REQUIRE(it2 == it);
		REQUIRE(it == it2);
		REQUIRE(!(it != it2));
	}
}

TEST_CASE("2D array zero length subarray", "[2d][array][iterator]") {
	multidim::array<multidim::inner_array<int, 0>, 10> arr;
	auto begin = arr.begin();
	auto end = arr.end();
	REQUIRE(begin != end);
	REQUIRE(end - begin == 10);
	REQUIRE(begin - end == -10);
	REQUIRE(begin + 10 == end);
	SECTION("++iterator") {
		for (int i = 0; i < 10; ++i) {
			REQUIRE(begin != end);
			++begin;
		}
		REQUIRE(begin == end);
	}
	SECTION("--iterator") {
		for (int i = 0; i < 10; ++i) {
			REQUIRE(begin != end);
			--end;
		}
		REQUIRE(begin == end);
	}
}

TEST_CASE("2D array <0,0> special case", "[2d][array][iterator]") {
	multidim::array<multidim::inner_array<int, 0>, 0> arr;
	auto begin = arr.begin();
	auto end = arr.end();
	REQUIRE(begin == end);
}

TEST_CASE("2D array <10,0> special case", "[2d][array][iterator]") {
	multidim::array<multidim::inner_array<int, 10>, 0> arr;
	auto begin = arr.begin();
	auto end = arr.end();
	REQUIRE(begin == end);
}

