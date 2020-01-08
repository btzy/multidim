#pragma once

#include <memory>
#include <type_traits>

#include "core.hpp"
#include "iterator.hpp"

/**
 * Uninitialized memory operations like in <memory>.
 * All functions here detect if ForwardIt::reference is an inner container or not.
 * If it is not an inner container, then the operation is equivalent to the std version.
 */

namespace multidim {

    template <typename It>
    struct is_iterator_to_inner_container : std::false_type {};
    template <typename T, bool IsConst>
    struct is_iterator_to_inner_container<multidim::iterator_lowest_impl<T, IsConst>> : std::false_type {};
    template <typename T, bool IsConst>
    struct is_iterator_to_inner_container<multidim::iterator_intermediate_impl<T, IsConst>> : std::true_type {};
    template <typename It>
    constexpr inline bool is_iterator_to_inner_container_v = is_iterator_to_inner_container<It>::value;

	template <typename InputIt, typename ForwardIt>
    constexpr inline ForwardIt uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first) {
        if constexpr (is_iterator_to_inner_container_v<InputIt>) {
            static_assert(is_iterator_to_inner_container_v<ForwardIt>);
            const auto raw_first = first->data();
            const auto raw_last = last->data();
            const auto raw_d_first = d_first->data();
            std::uninitialized_copy(raw_first, raw_last, raw_d_first);
            return d_first + (last - first);
        }
        else {
            return std::uninitialized_copy(first, last, d_first);
        }
    }
    template <typename InputIt, typename ForwardIt>
    constexpr inline ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt d_first) {
        if constexpr (is_iterator_to_inner_container_v<InputIt>) {
            static_assert(is_iterator_to_inner_container_v<ForwardIt>);
            const auto raw_first = first->data();
            const auto raw_last = last->data();
            const auto raw_d_first = d_first->data();
            std::uninitialized_move(raw_first, raw_last, raw_d_first);
            return d_first + (last - first);
        }
        else {
            return std::uninitialized_move(first, last, d_first);
        }
    }
    template <typename InputIt, typename ForwardIt>
    constexpr inline ForwardIt uninitialized_move_if_noexcept(InputIt first, InputIt last, ForwardIt d_first) {
        if constexpr (is_iterator_to_inner_container_v<InputIt>) {
            static_assert(is_iterator_to_inner_container_v<ForwardIt>);
            if constexpr (!std::is_nothrow_move_constructible_v<typename InputIt::base_element> && std::is_copy_constructible_v<typename InputIt::base_element>) {
                return std::uninitialized_copy(first, last, d_first);
            }
            else {
                return std::uninitialized_move(first, last, d_first);
            }
        }
        else {
            if constexpr (!std::is_nothrow_move_constructible_v<typename std::iterator_traits<InputIt>::value_type> && std::is_copy_constructible_v<typename std::iterator_traits<InputIt>::value_type>) {
                return std::uninitialized_copy(first, last, d_first);
            }
            else {
                return std::uninitialized_move(first, last, d_first);
            }
        }
    }

    template <typename ForwardIt, typename Size, typename T>
    constexpr inline ForwardIt uninitialized_fill_n(ForwardIt first, Size count, const T& value) {
        if constexpr (is_iterator_to_inner_container_v<ForwardIt>) {
            static_assert(std::is_base_of_v<multidim::reference_base, std::decay_t<T>>);
            while (count--) {
                first = std::uninitialized_copy_n(value.data(), value.size() * value.extents().stride(), first);
            }
            return first;
        }
        else {
            return std::uninitialized_fill_n(first, count, value);
        }
    }

    template <typename T, typename Reference>
    constexpr inline void uninitialized_copy_at(const T& val, Reference& dest) {
        if constexpr (std::is_base_of_v<multidim::reference_base, std::decay_t<Reference>>) {
            static_assert(std::is_base_of_v<multidim::reference_base, std::decay_t<T>>);
            const auto raw_first = dest.data();
            std::uninitialized_copy_n(val.data(), val.size() * val.extents().stride(), raw_first);
        }
        else {
            ::new (static_cast<void*>(std::addressof(dest))) std::decay_t<Reference>(val);
        }
    }

    template <typename T>
    constexpr inline void destroy_at(const T& val) {
        if constexpr (std::is_base_of_v<multidim::reference_base, std::decay_t<T>>) {
            std::destroy_n(val.data(), val.size() * val.extents().stride());
        }
        else {
            val.~T();
        }
    }
}
