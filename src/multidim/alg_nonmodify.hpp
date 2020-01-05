#pragma once

#include <iterator>
#include <utility>

namespace multidim {

	template <typename InputIt, typename T>
	constexpr inline InputIt find(InputIt first, InputIt last, const T& val) {
		for (; first != last; ++first) {
			if (*first == val) return first;
		}
		return last;
	}
	template <typename InputIt, typename UnaryPredicate>
	constexpr inline InputIt find_if(InputIt first, InputIt last, UnaryPredicate p) {
		for (; first != last; ++first) {
			if (p(*first)) return first;
		}
		return last;
	}
	template <typename InputIt, typename UnaryPredicate>
	constexpr inline InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate p) {
		for (; first != last; ++first) {
			if (!p(*first)) return first;
		}
		return last;
	}

	template <typename InputIt, typename UnaryPredicate>
	constexpr inline bool all_of(InputIt first, InputIt last, UnaryPredicate p) {
		return multidim::find_if_not(first, last, std::move(p)) == last;
	}
	template <typename InputIt, typename UnaryPredicate>
	constexpr inline bool any_of(InputIt first, InputIt last, UnaryPredicate p) {
		return multidim::find_if(first, last, std::move(p)) != last;
	}
	template <typename InputIt, typename UnaryPredicate>
	constexpr inline bool none_of(InputIt first, InputIt last, UnaryPredicate p) {
		return multidim::find_if(first, last, std::move(p)) == last;
	}

	template <typename InputIt, typename Function>
	constexpr inline Function for_each(InputIt first, InputIt last, Function f) {
		for (; first != last; ++first) {
			f(*first);
		}
		return f;
	}
	template <typename InputIt, typename Size, typename Function>
	constexpr inline InputIt for_each_n(InputIt first, Size n, Function f) {
		for (Size i = 0; i < n; ++i, ++first) {
			f(*first);
		}
		return first;
	}
	template <typename Function, typename InputIt, typename... InputIts>
	constexpr inline Function for_eachs(Function f, InputIt first, InputIt last, InputIts... firsts) {
		for (; first != last; ++first, ((++firsts), ...)) {
			f(*first, (*firsts)...);
		}
		return f;
	}
	template <typename Function, typename InputIt, typename Size, typename... InputIts>
	constexpr inline InputIt for_eachs_n(Function f, InputIt first, Size n, InputIts... firsts) {
		for (Size i = 0; i < n; ++i, ++first, ((++firsts), ...)) {
			f(*first, (*firsts)...);
		}
		return first;
	}

	template <typename InputIt, typename T>
	constexpr inline typename std::iterator_traits<InputIt>::difference_type count(InputIt first, InputIt last, const T& val) {
		typename std::iterator_traits<InputIt>::difference_type ret = 0;
		for (; first != last; ++first) {
			if (*first == val) ++ret;
		}
		return ret;
	}
	template <typename InputIt, typename UnaryPredicate>
	constexpr inline typename std::iterator_traits<InputIt>::difference_type count(InputIt first, InputIt last, UnaryPredicate p) {
		typename std::iterator_traits<InputIt>::difference_type ret = 0;
		for (; first != last; ++first) {
			if (p(*first)) ++ret;
		}
		return ret;
	}

	template <typename InIt1, typename InIt2>
	constexpr inline std::pair<InIt1, InIt2> mismatch(InIt1 first1, InIt1 last1, InIt2 first2) {
		while (first1 != last1 && *first1 == *first2) {
			++first1;
			++first2;
		}
		return std::make_pair(first1, first2);
	}
	template <typename InIt1, typename InIt2, typename BinaryPredicate>
	constexpr inline std::pair<InIt1, InIt2> mismatch(InIt1 first1, InIt1 last1, InIt2 first2, BinaryPredicate p) {
		while (first1 != last1 && p(*first1, *first2)) {
			++first1;
			++first2;
		}
		return std::make_pair(first1, first2);
	}
	template <typename InIt1, typename InIt2>
	constexpr inline std::pair<InIt1, InIt2> mismatch(InIt1 first1, InIt1 last1, InIt2 first2, InIt2 last2) {
		while (first1 != last1 && first2 != last2 && *first1 == *first2) {
			++first1;
			++first2;
		}
		return std::make_pair(first1, first2);
	}
	template <typename InIt1, typename InIt2, typename BinaryPredicate>
	constexpr inline std::pair<InIt1, InIt2> mismatch(InIt1 first1, InIt1 last1, InIt2 first2, InIt2 last2, BinaryPredicate p) {
		while (first1 != last1 && first2 != last2 && p(*first1, *first2)) {
			++first1;
			++first2;
		}
		return std::make_pair(first1, first2);
	}

	template <typename InputIt, typename ForwardIt>
	constexpr inline InputIt first_first_of(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last) {
		for (; first != last; ++first) {
			for (ForwardIt s_it = s_first; s_it != s_last; ++s_it) {
				if (*first == *s_it) return first;
			}
		}
		return last;
	}
	template <typename InputIt, typename ForwardIt, typename BinaryPredicate>
	constexpr inline InputIt first_first_of(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last, BinaryPredicate p) {
		for (; first != last; ++first) {
			for (ForwardIt s_it = s_first; s_it != s_last; ++s_it) {
				if (p(*first, *s_it)) return first;
			}
		}
		return last;
	}

	template <typename ForwardIt>
	constexpr inline ForwardIt adjacent_find(ForwardIt first, ForwardIt last) {
		if (first == last) return last;
		ForwardIt prev = first;
		++first;
		while (first != last) {
			if (*prev == *first) return prev;
			prev = first;
			++first;
		}
		return last;
	}
	template <typename ForwardIt, typename BinaryPredicate>
	constexpr inline ForwardIt adjacent_find(ForwardIt first, ForwardIt last, BinaryPredicate p) {
		if (first == last) return last;
		ForwardIt prev = first;
		++first;
		while (first != last) {
			if (p(*prev, *first)) return prev;
			prev = first;
			++first;
		}
		return last;
	}

	template <typename ForwardIt1, typename ForwardIt2>
	constexpr inline ForwardIt1 search(ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first, ForwardIt2 s_last) {
		while (true) {
			ForwardIt1 it = first;
			ForwardIt2 s_it = s_first;
			while (true) {
				if (s_it == s_last) return first;
				if (it == last) return last;
				if (!(*it == *s_it)) break;
				++s_it;
				++it;
			}
			++first;
		}
	}
	template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
	constexpr inline ForwardIt1 search(ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first, ForwardIt2 s_last, BinaryPredicate p) {
		while (true) {
			ForwardIt1 it = first;
			ForwardIt2 s_it = s_first;
			while (true) {
				if (s_it == s_last) return first;
				if (it == last) return last;
				if (!p(*it, *s_it)) break;
				++s_it;
				++it;
			}
			++first;
		}
	}

	template <typename ForwardIt, typename Size, typename T>
	constexpr inline ForwardIt find_consecutive(ForwardIt first, ForwardIt last, Size count, const T& val) {
		if (count <= 0) return first;
		if (count == 1) return multidim::find(first, last, val);
		for (; first != last; ++first) {
			if (!(*first == val)) continue;
			ForwardIt ret = first;
			Size curr = 1;
			for (++first; true; ++first) {
				if (first == last) return last;
				if (!(*first == val)) break;
				++curr;
				if (curr == count) return ret;
			}
		}
		return last;
	}
	template <typename ForwardIt, typename Size, typename UnaryPredicate>
	constexpr inline ForwardIt find_consecutive_if(ForwardIt first, ForwardIt last, Size count, UnaryPredicate p) {
		if (count <= 0) return first;
		if (count == 1) return multidim::find_if(first, last, p);
		for (; first != last; ++first) {
			if (!p(*first)) continue;
			ForwardIt ret = first;
			Size curr = 1;
			for (++first; true; ++first) {
				if (first == last) return last;
				if (!p(*first)) break;
				++curr;
				if (curr == count) return ret;
			}
		}
		return last;
	}
	template <typename ForwardIt, typename Size, typename T>
	constexpr inline ForwardIt search_n(ForwardIt first, ForwardIt last, Size count, const T& val) {
		return multidim::find_consecutive(first, last, count, val);
	}
	template <typename ForwardIt, typename Size, typename T, typename BinaryPredicate>
	constexpr inline ForwardIt search_n(ForwardIt first, ForwardIt last, Size count, const T& val, BinaryPredicate p) {
		return multidim::find_consecutive_if(first, last, count, [&](const auto& ref) {
			return p(ref, val);
		});
	}

	template <typename ForwardIt1, typename ForwardIt2>
	constexpr inline ForwardIt1 find_end(ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first, ForwardIt2 s_last) {
		if (s_first == s_last) return last;
		ForwardIt1 ret = last;
		while (true) {
			ForwardIt1 res = multidim::search(first, last, s_first, s_last);
			if (res == last) return ret;
			first = ret = res;
			++first;
		}
	}
	template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
	constexpr inline ForwardIt1 find_end(ForwardIt1 first, ForwardIt1 last, ForwardIt2 s_first, ForwardIt2 s_last, BinaryPredicate p) {
		if (s_first == s_last) return last;
		ForwardIt1 ret = last;
		while (true) {
			ForwardIt1 res = multidim::search(first, last, s_first, s_last, p);
			if (res == last) return ret;
			first = ret = res;
			++first;
		}
	}
}
