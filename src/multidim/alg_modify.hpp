#pragma once

#include <utility> // for std::move()

namespace multidim {
    template <typename InputIt, typename OutputIt>
    constexpr inline OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
        while (first != last) {
            *d_first++ = *first++;
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
    template <typename InputIt, typename OutputIt>
    constexpr inline OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
        while (first != last) {
            *d_first++ = std::move(*first++);
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
}
