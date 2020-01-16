#pragma once

#include <utility> // for std::move()
#include <type_traits>

#include "multidim/alg_nonmodify.hpp" // for some helpers, e.g. multidim::find()

namespace multidim {
    template <typename InputIt, typename OutputIt>
    constexpr inline OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
        while (first != last) {
            *d_first++ = *first++;
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt, typename UnaryPredicate>
    constexpr inline OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred) {
        while (first != last) {
            if (pred(*first))
                *d_first++ = *first;
            ++first;
        }
        return d_first;
    }
    template <typename InputIt, typename Size, typename OutputIt>
    constexpr inline OutputIt copy_n(InputIt first, Size count, OutputIt result) {
        if (count > 0) {
            *result++ = *first;
            for (Size i = 1; i < count; ++i) {
                *result++ = *++first;
            }
        }
        return result;
    }
    template <typename BidirIt1, typename BidirIt2>
    constexpr inline BidirIt2 copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
        while (first != last) {
            *--d_last = *--last;
        }
        return d_last;
    }

    template <typename InputIt, typename OutputIt>
    constexpr inline OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
        while (first != last) {
            *d_first++ = std::move(*first++);
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt, typename UnaryPredicate>
    constexpr inline OutputIt move_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred) {
        while (first != last) {
            if (pred(*first))
                *d_first++ = std::move(*first);
            ++first;
        }
        return d_first;
    }
    template <typename InputIt, typename Size, typename OutputIt>
    constexpr inline OutputIt move_n(InputIt first, Size count, OutputIt result) {
        if (count > 0) {
            *result++ = std::move(*first);
            for (Size i = 1; i < count; ++i) {
                *result++ = std::move(*++first);
            }
        }
        return result;
    }
    template <typename BidirIt1, typename BidirIt2>
    constexpr inline BidirIt2 move_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
        while (first != last) {
            *--d_last = std::move(*--last);
        }
        return d_last;
    }

    template <typename ForwardIt, typename T>
    constexpr inline void fill(ForwardIt first, ForwardIt last, const T& value) {
        for (; first != last; ++first) {
            *first = value;
        }
    }
    template <typename OutputIt, typename Size, typename T>
    constexpr inline OutputIt fill_n(OutputIt first, Size count, const T& value) {
        for (Size i = 0; i < count; i++) {
            *first++ = value;
        }
        return first;
    }

    template <typename ForwardIt, typename T>
    constexpr inline ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
        first = multidim::find(first, last, value);
        if (first != last) {
            ForwardIt reader = first;
            ++reader;
            while (reader != last) {
                if (!(*reader == value)) *first++ = std::move(*reader);
                ++reader;
            }
        }
        return first;
    }
    template <typename ForwardIt, typename UnaryPredicate>
    constexpr inline ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPredicate pred) {
        first = multidim::find_if(first, last, pred);
        if (first != last) {
            ForwardIt reader = first;
            ++reader;
            while (reader != last) {
                if (!pred(*reader)) *first++ = std::move(*reader);
                ++reader;
            }
        }
        return first;
    }
    template <typename InputIt, typename OutputIt, typename T>
    constexpr inline OutputIt remove_copy(InputIt first, InputIt last, OutputIt d_first, const T& value) {
        for (; first != last; ++first) {
            if (!(first == value)) {
                *d_first++ = *first;
            }
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt, typename UnaryPredicate>
    constexpr inline OutputIt remove_copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred) {
        for (; first != last; ++first) {
            if (!pred(first)) {
                *d_first++ = *first;
            }
        }
        return d_first;
    }
    template <typename ForwardIt, typename T>
    constexpr inline void replace(ForwardIt first, ForwardIt last, const T& old_value, const T& new_value) {
        for (; first != last; ++first) {
            if (*first == old_value) {
                *first = new_value;
            }
        }
    }
    template <typename ForwardIt, typename UnaryPredicate, typename T>
    constexpr inline void replace_if(ForwardIt first, ForwardIt last, UnaryPredicate p, const T& new_value) {
        for (; first != last; ++first) {
            if (p(*first)) {
                *first = new_value;
            }
        }
    }
    template <typename InputIt, typename OutputIt, typename T>
    constexpr inline OutputIt replace_copy(InputIt first, InputIt last, OutputIt d_first, const T& old_value, const T& new_value) {
        for (; first != last; ++first) {
            if (*first == old_value) {
                *d_first++ = new_value;
            }
            else {
                *d_first++ = *first;
            }
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename T>
    constexpr inline OutputIt replace_copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate p, const T& new_value) {
        for (; first != last; ++first) {
            if (p(*first)) {
                *d_first++ = new_value;
            }
            else {
                *d_first++ = *first;
            }
        }
        return d_first;
    }

    template <typename ForwardIt1, typename ForwardIt2>
    constexpr inline void iter_swap(ForwardIt1 a, ForwardIt2 b) {
        using std::swap;
        swap(*a, *b);
    }

    template <typename ForwardIt1, typename ForwardIt2>
    constexpr inline ForwardIt2 swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2) {
        for (; first1 != last1; ++first1, ++first2) {
            multidim::iter_swap(first1, first2);
        }
        return first2;
    }

    template <typename BidirIt>
    constexpr inline void reverse(BidirIt first, BidirIt last) {
        while (first != last && first != --last) {
            multidim::iter_swap(first, last);
            ++first;
        }
    }
    template <typename BidirIt, typename OutputIt>
    constexpr inline OutputIt reverse_copy(BidirIt first, BidirIt last, OutputIt d_first) {
        while (first != last) {
            *d_first++ = *--last;
        }
        return d_first;
    }

    template <typename ForwardIt>
    constexpr inline ForwardIt rotate(ForwardIt first, ForwardIt n_first, ForwardIt last) {
        if (first == n_first) return last;
        if (n_first == last) return first;
        {
            ForwardIt it = n_first;
            do {
                multidim::iter_swap(first, it);
                ++first;
                ++it;
                if (first == n_first) {
                    n_first = it;
                }
            } while (it != last);
        }
        ForwardIt ret = first;
        while (n_first != last) {
            assert(first != n_first);
            ForwardIt it = n_first;
            do {
                multidim::iter_swap(first, it);
                ++first;
                ++it;
                if (first == n_first) {
                    n_first = it;
                }
            } while (it != last);
        }
        return ret;
    }
    template <typename ForwardIt, typename OutputIt>
    constexpr inline ForwardIt rotate_copy(ForwardIt first, ForwardIt n_first, ForwardIt last, OutputIt d_first) {
        d_first = multidim::copy(n_first, last, d_first);
        return multidim::copy(first, n_first, d_first);
    }

    template <typename ForwardIt>
    constexpr inline std::enable_if_t<!std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> shift_left(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type n) {
        if (n <= 0) return last;
        ForwardIt it = first;
        for (; n > 0; --n, ++it) {
            if (it == last) return first;
        }
        return multidim::move(it, last, first);
    }
    template <typename ForwardIt>
    constexpr inline std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> shift_left(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type n) {
        if (n <= 0) return last;
        if (last - first <= n) return first;
        return multidim::move(first + n, last, first);
    }
    template <typename ForwardIt>
    constexpr inline std::enable_if_t<!std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> shift_right(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type n) {
        if (n <= 0) return first;
        ForwardIt it = first;
        for (; n > 0; --n, ++it) {
            if (it == last) return last;
        }
        ForwardIt ret = it;
        ForwardIt ret_it = first;
        assert(ret_it != ret);
        if (it != last) {
            multidim::iter_swap(ret_it, it);
            ++ret_it;
            for (++it; it != last; ++it) {
                if (ret_it == ret) ret_it = first;
                multidim::iter_swap(ret_it, it);
                ++ret_it;
            }
        }
        return ret;
    }
    template <typename ForwardIt>
    constexpr inline std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category> && !std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> shift_right(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type n) {
        if (n <= 0) return first;
        ForwardIt it = last;
        for (; n > 0; --n, --it) {
            if (it == first) return last;
        }
        return multidim::move_backward(first, it, last);
    }
    template <typename ForwardIt>
    constexpr inline std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> shift_right(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type n) {
        if (n <= 0) return first;
        if (last - first <= n) return last;
        return multidim::move_backward(first, last - n, last);
    }

    template <typename ForwardIt>
    constexpr inline ForwardIt unique(ForwardIt first, ForwardIt last) {
        if (first == last) return last;
        ForwardIt prev = first;
        ++first;
        while (true) {
            if (first == last) return last;
            if (*prev == *first) break;
            ++prev;
            ++first;
        }
        assert(first != last);
        while (++first != last) {
            if (!(*prev == *first)) {
                *++prev = std::move(*first);
            }
        }
        return ++prev;
    }
    template <typename ForwardIt, typename BinaryPredicate>
    constexpr inline ForwardIt unique(ForwardIt first, ForwardIt last, BinaryPredicate p) {
        if (first == last) return last;
        ForwardIt prev = first;
        ++first;
        while (true) {
            if (first == last) return last;
            if (p(*prev, *first)) break;
            ++prev;
            ++first;
        }
        assert(first != last);
        while (++first != last) {
            if (!p(*prev, *first)) {
                *++prev = std::move(*first);
            }
        }
        return ++prev;
    }

    /**
     * Copies the first element in every consecutive group of unique elements to the output range.
     * InputIt must satisfy LegacyInputIterator, and OutputIt must satisfy LegacyOutputIterator.
     * Furthermore, either InputIt or OutputIt must satisfy LegacyForwardIterator.
     */
    template <typename InputIt, typename OutputIt>
    constexpr inline std::enable_if<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, OutputIt> unique_copy(InputIt first, InputIt last, OutputIt d_first) {
        if (first == last) return d_first;
        InputIt prev = first;
        *d_first++ = *first;
        while (++first != last) {
            if (!(*prev == *first)) {
                *d_first++ = *first;
                prev = first;
            }
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt, typename BinaryPredicate>
    constexpr inline std::enable_if<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, OutputIt> unique_copy(InputIt first, InputIt last, OutputIt d_first, BinaryPredicate p) {
        if (first == last) return d_first;
        InputIt prev = first;
        *d_first++ = *first;
        while (++first != last) {
            if (!p(*prev, *first)) {
                *d_first++ = *first;
                prev = first;
            }
        }
        return d_first;
    }
    template <typename InputIt, typename OutputIt>
    constexpr inline std::enable_if<!std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, OutputIt> unique_copy(InputIt first, InputIt last, OutputIt d_first) {
        static_assert(std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<OutputIt>::iterator_category>);
        if (first == last) return d_first;
        *d_first = *first;
        while (++first != last) {
            if (!(*d_first == *first)) {
                *++d_first = *first;
            }
        }
        return ++d_first;
    }
    template <typename InputIt, typename OutputIt, typename BinaryPredicate>
    constexpr inline std::enable_if<!std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, OutputIt> unique_copy(InputIt first, InputIt last, OutputIt d_first, BinaryPredicate p) {
        static_assert(std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<OutputIt>::iterator_category>);
        if (first == last) return d_first;
        *d_first = *first;
        while (++first != last) {
            if (!p(*d_first, *first)) {
                *++d_first = *first;
            }
        }
        return ++d_first;
    }

}
