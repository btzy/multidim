#include "catch.hpp"

#include <array>
#include <vector>

#include <multidim/array.hpp>
#include <multidim/dynarray.hpp>

#include <multidim/alg_nonmodify.hpp>
#include <multidim/alg_modify.hpp> // for multidim::copy

TEST_CASE("algorithms find and for_eachs", "[algorithm]") {
	multidim::array<multidim::inner_array<int, 5>, 10> arr;
	multidim::dynarray<multidim::inner_dynarray<int>> dynarr(10, 5);

	std::vector<std::array<int, 5>> stdvec(10);
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 5; ++j) {
			arr[i][j] = i * 10 + j;
			dynarr[i][j] = i * 10 + j;
			stdvec[i][j] = i * 10 + j;
		}
	}
	SECTION("find") {
		std::array<int, 5> stdseek{ { 30,31,32,33,34 } };
		multidim::array<int, 5> arrseek;
		multidim::dynarray<int> dynarrseek(5);
		multidim::copy(stdseek.begin(), stdseek.end(), arrseek.begin());
		multidim::copy(stdseek.begin(), stdseek.end(), dynarrseek.begin());
		auto it1 = std::find(stdvec.begin(), stdvec.end(), stdseek);
		auto it2 = multidim::find(stdvec.begin(), stdvec.end(), stdseek);
		auto it3 = multidim::find(arr.begin(), arr.end(), arrseek);
		auto it4 = multidim::find(dynarr.begin(), dynarr.end(), dynarrseek);
		REQUIRE(it1 == it2);
		REQUIRE(std::equal(it1->begin(), it1->end(), it3->begin(), it3->end()));
		REQUIRE(std::equal(it1->begin(), it1->end(), it4->begin(), it4->end()));
	}

	SECTION("for_eachs") {
		int i = 0;
		multidim::for_eachs([&](const std::array<int, 5>& stdr, const multidim::array_const_ref<int, 5> r1, const multidim::dynarray_const_ref<int> r2) {
			REQUIRE(&stdr == &stdvec[i++]);
			REQUIRE(std::equal(stdr.begin(), stdr.end(), r1.begin(), r1.end()));
			REQUIRE(std::equal(stdr.begin(), stdr.end(), r2.begin(), r2.end()));
			}, stdvec.begin(), stdvec.end(), arr.begin(), dynarr.begin());
	}

	SECTION("for_eachs_n") {
		int i = 0;
		multidim::for_eachs_n([&](const std::array<int, 5>& stdr, const multidim::array_const_ref<int, 5> r1, const multidim::dynarray_const_ref<int> r2) {
			REQUIRE(&stdr == &stdvec[i++]);
			REQUIRE(std::equal(stdr.begin(), stdr.end(), r1.begin(), r1.end()));
			REQUIRE(std::equal(stdr.begin(), stdr.end(), r2.begin(), r2.end()));
			}, stdvec.begin(), stdvec.size(), arr.begin(), dynarr.begin());
	}
}

TEST_CASE("algorithms find_consecutive", "[algorithm]") {
	std::array<int, 11> arr{ {4, 3, 3, 2, 3, 3, 3, 4, 4, 4, 6} };
	REQUIRE(multidim::search_n(arr.begin(), arr.end(), 3, 3) == arr.begin() + 4);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 3, 3) == arr.begin() + 4);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 0, 3) == arr.begin());
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), -1, 3) == arr.begin());
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 4, 3) == arr.end());
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 2, 3) == arr.begin() + 1);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 1, 4) == arr.begin());
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 2, 4) == arr.begin() + 7);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 3, 4) == arr.begin() + 7);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 1, 6) == arr.begin() + 10);
	REQUIRE(multidim::find_consecutive(arr.begin(), arr.end(), 2, 6) == arr.end());
	REQUIRE(multidim::find_consecutive_if(arr.begin(), arr.end(), 2, [](const int& x) { return x % 2 == 0; }) == arr.begin() + 7);
	REQUIRE(multidim::find_consecutive_if(arr.begin(), arr.end(), 4, [](const int& x) { return x % 2 == 0; }) == arr.begin() + 7);
	REQUIRE(multidim::find_consecutive_if(arr.begin(), arr.end(), 5, [](const int& x) { return x % 2 == 0; }) == arr.end());
}
