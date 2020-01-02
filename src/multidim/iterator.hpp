#pragma once

#include <iterator>
#include <type_traits>

#include "core.hpp"

namespace std {
	// detection mechanism in case contiguous_iterator_tag is unavailable in the compiler.
	struct contiguous_iterator_tag;
}

namespace multidim {

	template <typename T, typename = void>
	struct is_complete : public std::false_type {};
	template <typename T>
	struct is_complete<T, std::enable_if_t<(sizeof(T) > 0)>> : public std::true_type {};
	template <typename T>
	constexpr inline bool is_complete_v = is_complete<T>::value;

	using contiguous_iterator_tag = std::conditional_t<is_complete_v<std::contiguous_iterator_tag>, std::contiguous_iterator_tag, std::random_access_iterator_tag>;

	/**
	 * Iterator base class for any container.
	 * Iterators may only be safely compared with other iterators that were obtained from the same array.
	 * @param T the base element type or the inner container type that this iterator points to.
	 * @param IsConst whether this iterator is a const_iterator.
	 */
	template <typename T, bool IsConst>
	class iterator_base_impl {
	public:
		using value_type = std::conditional_t<IsConst, const typename element_traits<T>::value_type, typename element_traits<T>::value_type>;
		using reference = std::conditional_t<IsConst, typename element_traits<T>::const_reference, typename element_traits<T>::reference>;
		using const_reference = const typename element_traits<T>::reference;
		using difference_type = ptrdiff_t;
		using size_type = size_t;
		using pointer = std::conditional_t<IsConst, typename element_traits<T>::const_pointer, typename element_traits<T>::pointer>;
		using iterator_category = contiguous_iterator_tag;
		using element_extents_type = typename element_traits<T>::extents_type;
		using base_element = std::conditional_t<IsConst, const typename element_traits<T>::base_element, typename element_traits<T>::base_element>;
	};

	/**
	 * Iterator class for the lowest-level container.  They satisfy LegacyContiguousIterator as much as is possible.
	 * The main difference is that std::iterator_traits<Iter>::value_type and std::iterator_traits<Iter>::pointer are void.
	 * In addition std::iterator_traits<Iter>::reference is not a real reference, but an actual type (something like a reference wrapper) instead.
	 */
	template <typename T, bool IsConst>
	class iterator_intermediate_impl : public iterator_base_impl<T, IsConst> {
	public:
		constexpr iterator_intermediate_impl(base_element* data, const element_extents_type& extents, size_type index) noexcept : ref_(data, extents), index_(index) {}
		/**
		 * Default-constructed iterator.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==, as required by LegacyForwardIterator.
		 */
		constexpr iterator_intermediate_impl() noexcept = default;
		constexpr iterator_intermediate_impl(const iterator_intermediate_impl&) noexcept = default;
		constexpr iterator_intermediate_impl& operator=(const iterator_intermediate_impl&) noexcept = default;

		constexpr reference operator*() const noexcept { return ref_; }
		constexpr const reference* operator->() const noexcept { return &ref_; }

		constexpr iterator_intermediate_impl& operator++() noexcept { ref_.rebind_relative(1); ++index_; return *this; }
		constexpr iterator_intermediate_impl operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }
		constexpr iterator_intermediate_impl& operator--() noexcept { ref_.rebind_relative(-1); --index_; return *this; }
		constexpr iterator_intermediate_impl operator--(int) noexcept { auto tmp = *this; --(*this); return tmp; }
		constexpr iterator_intermediate_impl& operator+=(difference_type n) noexcept { ref_.rebind_relative(n); index_ += n; return *this; }
		constexpr iterator_intermediate_impl operator+(difference_type n) const noexcept { auto tmp = *this; return tmp += n; }
		friend constexpr iterator_intermediate_impl operator+(difference_type n, const iterator_intermediate_impl& it) noexcept { return it + n; }
		constexpr iterator_intermediate_impl& operator-=(difference_type n) noexcept { ref_.rebind_relative(-n); index_ -= n; return *this; }
		constexpr iterator_intermediate_impl operator-(difference_type n) const noexcept { auto tmp = *this; return tmp -= n; }
		friend constexpr difference_type operator-(const iterator_intermediate_impl& b, const iterator_intermediate_impl& a) noexcept { return b.index_ - a.index_; }

		constexpr reference operator[](difference_type n) const noexcept { return *(*this + n); }

		friend constexpr bool operator==(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return a.index_ == b.index_; }
		friend constexpr bool operator!=(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return !(a == b); }
#ifdef __cpp_impl_three_way_comparison
		friend constexpr auto operator<=>(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return a.index_ <=> b.index_; }
#else
		friend constexpr auto operator<(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return a.index_ < b.index_; }
		friend constexpr auto operator>(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return b < a; }
		friend constexpr auto operator<=(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return !(b < a); }
		friend constexpr auto operator>=(const iterator_intermediate_impl& a, const iterator_intermediate_impl& b) noexcept { return !(a < b); }
#endif

	private:
		using B = iterator_base_impl<T, IsConst>;
		static_assert(!std::is_same_v<element_extents_type, unit_extent>, "extents_type must not be unit_extent for intermediate level iterators");
		typename B::reference ref_;
		size_type index_; // the index of the element pointed to; we need to store this in case one of the extents is zero, so that comparison with end() will work properly
	};

	/**
	 * Iterator class for the lowest-level container.  They satisfy LegacyContiguousIterator.
	 */
	template <typename T, bool IsConst>
	class iterator_lowest_impl : public iterator_base_impl<T, IsConst> {
	public:
		static_assert(std::is_same_v<element_extents_type, unit_extent>, "element_extents_type must be unit_extent");
		constexpr iterator_lowest_impl(base_element* data, const element_extents_type&, size_type) noexcept : ptr_(data) {}
		/**
		 * Default-constructed iterator.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==, as required by LegacyForwardIterator.
		 */
		constexpr iterator_lowest_impl() noexcept = default;
		constexpr iterator_lowest_impl(const iterator_lowest_impl&) noexcept = default;
		constexpr iterator_lowest_impl& operator=(const iterator_lowest_impl&) noexcept = default;

		constexpr reference operator*() const noexcept { return *ptr_; }
		constexpr pointer operator->() const noexcept { return ptr; }

		constexpr iterator_lowest_impl& operator++() noexcept { ++ptr_; return *this; }
		constexpr iterator_lowest_impl operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }
		constexpr iterator_lowest_impl& operator--() noexcept { --ptr_; return *this; }
		constexpr iterator_lowest_impl operator--(int) noexcept { auto tmp = *this; --(*this); return tmp; }
		constexpr iterator_lowest_impl& operator+=(difference_type n) noexcept { ptr_ += n; return *this; }
		constexpr iterator_lowest_impl operator+(difference_type n) const noexcept { auto tmp = *this; return tmp += n; }
		friend constexpr iterator_lowest_impl operator+(difference_type n, const iterator_lowest_impl& it) noexcept { return it + n; }
		constexpr iterator_lowest_impl& operator-=(difference_type n) noexcept { ptr_ -= n; return *this; }
		constexpr iterator_lowest_impl operator-(difference_type n) const noexcept { auto tmp = *this; return tmp -= n; }
		friend constexpr difference_type operator-(const iterator_lowest_impl& b, const iterator_lowest_impl& a) noexcept { return b.ptr_ - a.ptr_; }

		constexpr reference operator[](difference_type n) const noexcept { return *(*this + n); }

		friend constexpr bool operator==(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return a.ptr_ == b.ptr_; }
		friend constexpr bool operator!=(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return !(a == b); }
#ifdef __cpp_impl_three_way_comparison
		friend constexpr auto operator<=>(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return a.ptr_ <=> b.ptr_; }
#else
		friend constexpr auto operator<(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return a.ptr_ < b.ptr_; }
		friend constexpr auto operator>(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return b < a; }
		friend constexpr auto operator<=(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return !(b < a); }
		friend constexpr auto operator>=(const iterator_lowest_impl& a, const iterator_lowest_impl& b) noexcept { return !(a < b); }
#endif

	private:
		using B = iterator_base_impl<T, IsConst>;
		static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
		typename B::pointer ptr_;
	};


	template <typename T, bool IsConst>
	using iterator_impl = std::conditional_t<std::is_base_of_v<inner_container_base, T>, iterator_intermediate_impl<T, IsConst>, iterator_lowest_impl<T, IsConst>>;

	template <typename T>
	using iterator = iterator_impl<T, false>;
	template <typename T>
	using const_iterator = iterator_impl<T, true>;
	
}
