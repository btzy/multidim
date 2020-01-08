#include "catch.hpp"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <multidim/vector.hpp>

/**
 * Counts the number of constructions/destructions of itself.
 */
template <typename T>
struct Tracker {
	Tracker() {
		inc();
	}
	Tracker(const T& val) : val(val) {
		inc();
	}
	Tracker(const Tracker& other) : val(other.val) {
		inc();
	}
	void inc() {
		if (net < 0) throw std::runtime_error("Too many destructions");
		++ctr;
		++net;
	}
	~Tracker() {
		--net;
	}
	operator const T& () const noexcept { return val; }
	static void validate_ctr(int ctr_count) {
		if (net < 0) throw std::runtime_error("Too many destructions");
		if (ctr_count != ctr) {
			throw std::runtime_error("Incorrect number of constructors called, expected " + std::to_string(ctr));
		}
	}
	static void validate_net(int net_count = 0) {
		if (net < 0) throw std::runtime_error("Too many destructions");
		if (net_count != net) {
			throw std::runtime_error("Incorrect net number of constructors/destructors called, expected " + std::to_string(net));
		}
	}
	static void reset() noexcept {
		ctr = 0;
		net = 0;
	}
	static inline int ctr = 0, net = 0;
	T val;
};

TEST_CASE("1D vector basic operations", "[1d][vector]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	REQUIRE(arr.begin() == arr.end());
	REQUIRE(arr.empty());
	arr.push_back(5);
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 5);
	arr.push_back(7);
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 7);
	arr.push_back(9);
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 9);
	arr.push_back(10);
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 10);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	REQUIRE(arr[0] == 5);
	REQUIRE(arr[1] == 7);
	REQUIRE(arr[2] == 9);
	REQUIRE(arr[3] == 10);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	auto it = arr.begin();
	REQUIRE(it == arr.begin());
	REQUIRE(*it++ == 5);
	REQUIRE(*it++ == 7);
	REQUIRE(*it++ == 9);
	REQUIRE(*it++ == 10);
	REQUIRE(it == arr.end());
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 10);
	arr.pop_back();
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 9);
	arr.pop_back();
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 7);
	arr.pop_back();
	REQUIRE(arr.front() == 5);
	REQUIRE(arr.back() == 5);
	arr.pop_back();
	REQUIRE(arr.empty());
	REQUIRE(arr.begin() == arr.end());
	REQUIRE_NOTHROW(Tracker<int>::validate_net());
}

TEST_CASE("1D vector operator==", "[1d][vector][equality]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	multidim::vector<Tracker<int>> arr2;
	arr2.push_back(5);
	REQUIRE(arr != arr2);
	arr2.push_back(7);
	REQUIRE(arr != arr2);
	arr2.push_back(9);
	REQUIRE(arr != arr2);
	arr2.push_back(10);
	REQUIRE(arr == arr2);
	arr2.push_back(11);
	REQUIRE(arr != arr2);
	arr2.pop_back();
	REQUIRE(arr == arr2); // to check that the extra capacity doesn't matter
	multidim::vector<Tracker<int>> arr3;
	arr3.push_back(6);
	arr3.push_back(7);
	arr3.push_back(9);
	arr3.push_back(10);
	REQUIRE(arr != arr3);
	multidim::vector<Tracker<int>> arr4;
	arr4.push_back(5);
	arr4.push_back(7);
	arr4.push_back(9);
	arr4.push_back(11);
	REQUIRE(arr != arr4);
	multidim::vector<Tracker<int>> arr5;
	arr5.push_back(5);
	arr5.push_back(7);
	arr5.push_back(8);
	arr5.push_back(10);
	REQUIRE(arr != arr5);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(20));
}

TEST_CASE("1D vector copy/move", "[1d][vector][copy][move]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	multidim::vector<Tracker<int>> arr2 = arr; // copy construction
	REQUIRE(arr2 == arr);
	multidim::vector<Tracker<int>> arr3; // copy assignment
	arr3 = arr;
	REQUIRE(arr3 == arr);
	multidim::vector<Tracker<int>> arr4 = std::move(arr); // move construction
	REQUIRE(arr4 != arr);
	REQUIRE(arr4 == arr2);
	multidim::vector<Tracker<int>> arr5; // move assignment
	REQUIRE(arr5 == arr);
	REQUIRE(arr5 != arr4);
	arr5 = std::move(arr4);
	REQUIRE(arr5 != arr4);
	REQUIRE(arr5 == arr2);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(12));
}

TEST_CASE("1D vector reserve", "[1d][vector]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.reserve(4);
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	REQUIRE_NOTHROW(Tracker<int>::validate_ctr(8));
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
}

TEST_CASE("1D vector swap and clear", "[1d][vector][swap][clear]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	multidim::vector<Tracker<int>> arr2;
	arr2.push_back(3);
	arr2.push_back(5);
	REQUIRE(arr != arr2);
	multidim::vector<Tracker<int>> tmp_arr = arr;
	multidim::vector<Tracker<int>> tmp_arr2 = arr2;
	{
		using std::swap;
		swap(arr, arr2);
	}
	REQUIRE(tmp_arr == arr2);
	REQUIRE(tmp_arr2 == arr);
	REQUIRE(tmp_arr != arr);
	REQUIRE(tmp_arr2 != arr2);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(12));
	arr.clear();
	arr2.clear();
	REQUIRE(arr.size() == 0);
	REQUIRE(arr2.size() == 0);
	REQUIRE(arr.empty());
	REQUIRE(arr2.empty());
	REQUIRE(arr == arr2);
	REQUIRE_NOTHROW(Tracker<int>::validate_net(6));
	tmp_arr.clear();
	tmp_arr2.clear();
	REQUIRE_NOTHROW(Tracker<int>::validate_net());
}

TEST_CASE("1D vector assign", "[1d][vector][assign]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	multidim::vector<Tracker<int>> arr2;
	arr2.push_back(3);
	arr2.push_back(5);
	arr2.assign(arr.begin(), arr.end());
	REQUIRE(arr == arr2);
	arr2.clear();
	REQUIRE(arr != arr2);
	arr2.assign(arr.begin(), arr.end());
	REQUIRE(arr == arr2);
	arr2.assign(arr.begin(), arr.end());
	REQUIRE(arr == arr2);
	arr2.assign(3, 99);
	{
		multidim::vector<Tracker<int>> ans;
		ans.push_back(99);
		ans.push_back(99);
		ans.push_back(99);
		REQUIRE(arr2 == ans);
	}
	arr2.assign({ 11,12,14,15,18 });
	{
		multidim::vector<Tracker<int>> ans;
		ans.push_back(11);
		ans.push_back(12);
		ans.push_back(14);
		ans.push_back(15);
		ans.push_back(18);
		REQUIRE(arr2 == ans);
	}
	REQUIRE_NOTHROW(Tracker<int>::validate_net(9));
	arr2.clear();
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
}

TEST_CASE("1D vector conversions", "[1d][vector]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.reserve(4);
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	REQUIRE_NOTHROW(Tracker<int>::validate_ctr(8));
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	multidim::dynarray_const_ref<Tracker<int>> ref = arr;
	REQUIRE_NOTHROW(Tracker<int>::validate_ctr(8));
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	REQUIRE(ref == arr);
	multidim::dynarray_const_ref<Tracker<int>> ref2 = arr;
	REQUIRE_NOTHROW(Tracker<int>::validate_ctr(8));
	REQUIRE_NOTHROW(Tracker<int>::validate_net(4));
	REQUIRE(ref == ref2);
}

TEST_CASE("1D vector iterator", "[1d][vector][iterator]") {
	Tracker<int>::reset();
	multidim::vector<Tracker<int>> arr;
	arr.push_back(5);
	arr.push_back(7);
	arr.push_back(9);
	arr.push_back(10);
	std::vector<int> tmp;
	for (const Tracker<int>& x : arr) {
		tmp.push_back(x);
	}
	REQUIRE(tmp == std::vector<int>{5, 7, 9, 10});
}
