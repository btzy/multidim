#pragma once

#include <utility> // for std::pair
#include <iterator>

#include "multidim/alg_nonmodify.hpp" // for some helpers, e.g. multidim::find()
#include "multidim/alg_modify.hpp" // for some helpers, e.g. multidim::iter_swap()

namespace multidim {
    template <typename InputIt, typename UnaryPredicate>
    constexpr inline bool is_partitioned(InputIt first, InputIt last, UnaryPredicate p) {
        first = multidim::find_if_not(first, last, p);
        return multidim::none_of(first, last, p);
    }
    template <typename ForwardIt, typename UnaryPredicate>
    constexpr inline std::enable_if_t<!std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>, ForwardIt> partition(ForwardIt first, ForwardIt last, UnaryPredicate p) {
        first = multidim::find_if_not(first, last, p);
        if (first == last) return first;
        ForwardIt front = first;
        for (++front; front != last; ++front) {
            if (p(*front)) {
                multidim::iter_swap(first, front);
                ++first;
            }
        }
        return first;
    }
    template <typename BidirIt, typename UnaryPredicate>
    constexpr inline std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<BidirIt>::iterator_category>, BidirIt> partition(BidirIt first, BidirIt last, UnaryPredicate p) {
        if (first == last) return first;
        while (true) {
            while (p(*first)) {
                ++first;
                if (first == last) return first;
            }
            while (true) {
                if (first == --last) return first;
                if (p(*last))break;
            }
            multidim::iter_swap(first, last);
            ++first;
            if (first == last) return first;
        }
    }
    template <typename InputIt, typename OutputIt1, typename OutputIt2, typename UnaryPredicate>
    constexpr inline std::pair<OutputIt1, OutputIt2> partition_copy(InputIt first, InputIt last, OutputIt1 d_first_true, OutputIt2 d_first_false, UnaryPredicate p)
    {
        for (; first != last; ++first) {
            if (p(*first)) {
                *d_first_true++ = *first;
            }
            else {
                *d_first_false++ = *first;
            }
        }
        return std::pair<OutputIt1, OutputIt2>(d_first_true, d_first_false);
    }


    namespace detail {
        // forward declaration for the mutual recursion
        template <typename ForwardIt, typename UnaryPredicate>
        constexpr inline ForwardIt stable_partition_impl_with_sh(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type len, UnaryPredicate p);
        
        // assumes that first != last (so len > 0)
        template <typename ForwardIt, typename UnaryPredicate>
        constexpr inline ForwardIt stable_partition_impl(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type len, UnaryPredicate p) {
            typename std::iterator_traits<ForwardIt>::difference_type left_len = len / 2;
            if (left_len == 0) {
                // at this point, first + 1 == last
                return p(*first) ? last : first;
            }
            ForwardIt mid = first;
            std::advance(mid, left_len);
            ForwardIt left_ans = stable_partition_impl(first, mid, left_len, p);
            typename std::iterator_traits<ForwardIt>::difference_type right_len = len - left_len;
            // at this point, right_len > 0 (so mid != last), since we know that right_len >= left_len
            // skip all the falsy elements before going into recursion on the right side
            while (true) {
                if (!p(*mid)) {
                    ++mid;
                    --right_len;
                    if (right_len == 0) return left_ans;
                }
                else {
                    break;
                }
            }
            // at this point, still, right_len > 0 (so mid != last)
            ForwardIt right_ans = stable_partition_impl_with_sh(mid, last, right_len, p);
            return multidim::rotate(left_ans, mid, right_ans);
        }
        
        // assumes that first != last (so len > 0)
        template <typename ForwardIt, typename UnaryPredicate>
        constexpr inline ForwardIt stable_partition_impl_with_sh(ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::difference_type len, UnaryPredicate p) {
            // skip all the truthy elements before going into recursion
            while (true) {
                if (p(*first)) {
                    ++first;
                    --len;
                    if (len == 0) return last;
                }
                else {
                    break;
                }
            }
            return stable_partition_impl(first, last, len, std::move(p));
        }
    }

    template <typename ForwardIt, typename UnaryPredicate>
    constexpr inline ForwardIt stable_partition(ForwardIt first, ForwardIt last, UnaryPredicate p) {
        if (first == last) return first;
        typename std::iterator_traits<ForwardIt>::difference_type len = std::distance(first, last);
        return detail::stable_partition_impl_with_sh(first, last, len, std::move(p));
    }

    template <typename ForwardIt, typename UnaryPredicate>
    constexpr inline ForwardIt partition_point(ForwardIt first, ForwardIt last, UnaryPredicate p) {
        typename std::iterator_traits<ForwardIt>::difference_type len = std::distance(first, last);
        while (len != 0) {
            ForwardIt mid = first;
            typename std::iterator_traits<ForwardIt>::difference_type left_len = len / 2;
            std::advance(mid, left_len);
            if (p(*mid)) {
                first = mid;
                ++first;
                len -= left_len;
                --len; // note: guaranteed that len>=0 after this line, because original len >= 1
            }
            else {
                last = mid;
                len = left_len;
            }
        }
        return first;
    }
}