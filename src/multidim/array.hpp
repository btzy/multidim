#pragma once

#include <memory> // for std::forward()
#include <type_traits>

#include "core.hpp"

namespace multidim {

	template <typename T, size_t N>
	class array;
	template <typename T, size_t N>
	class array_ref;

	template <typename T, size_t N>
	struct inner_array : public enable_inner_container<array<T, N>, array_ref<T, N>> {};



	template <typename E, size_t N>
	class static_extent {
	public:
		constexpr size_t stride() const noexcept {
			return N * element_extent_.stride();
		}
		constexpr size_t top_extent() const noexcept {
			return N;
		}
		constexpr const E& inner() const noexcept {
			return element_extent_;
		}
		constexpr static bool is_dynamic = false;
		template <typename... TNs>
		constexpr explicit static_extent(TNs... ns) noexcept : element_extent_(ns...) {}
	private:
		[[no_unique_address]] E element_extent_;
	};



	

	



	

	template <typename T, bool IsConst>
	class iterator_impl {

	};



	template <typename Array, typename T, size_t N, bool Owning>
	class array_base {
	public:
		using value_type = typename element_traits<T>::value_type;
		using reference = typename element_traits<T>::reference;
		using const_reference = const typename element_traits<T>::reference;
		using iterator = iterator_impl<T, false>;
		using const_iterator = iterator_impl<T, true>;
		using element_extents_type = typename element_traits<T>::extents_type;
		using container_extents_type = static_extent<element_extents_type, N>;
		using base_element = typename element_traits<T>::base_element;
		using buffer_type = typename add_dim_to_buffer_t<typename element_traits<T>::buffer_type, N>;

		constexpr size_t size() const noexcept {
			return N;
		}
		constexpr base_element* data() noexcept {
			return to_pointer(data_);
		}
		constexpr reference operator[](size_t index) noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return reference{ data() + index * extents_.stride(), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
	protected:
		using underlying_store = std::conditional_t<Owning, buffer_type, base_element*>;
		template <typename... Args>
		constexpr array_base(const element_extents_type& extents, Args&&... args) noexcept : data_(std::forward<Args>(args)...), extents_(extents) {}
		constexpr array_base(const array_base& other) = default;
		constexpr array_base(array_base&& other) = default;
		constexpr array_base& operator=(const array_base& other) = default;
		constexpr array_base& operator=(array_base&& other) = default;
		underlying_store data_;
		[[no_unique_address]] element_extents_type extents_;
	};

	template <typename T, size_t N>
	class array : public array_base<array<T, N>, T, N, true> {
	public:
		using B = array_base<array<T, N>, T, N, true>;
		template <typename... TNs>
		constexpr explicit array(TNs... ns) noexcept : B(B::element_extents_type(ns...)) {}
	};

	template <typename T, size_t N>
	class array_ref : public array_base<array_ref<T, N>, T, N, false> {
	private:
		using B = array_base<array_ref<T, N>, T, N, false>;
	public:
		constexpr array_ref() = delete;
		constexpr array_ref(const array_ref&) = default;
		constexpr array_ref(array_ref&&) = default;
		constexpr array_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.inner(), data) {
			static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
		}
		constexpr array_ref& operator=(const array_ref&) = default;
		constexpr array_ref& operator=(array_ref&&) = default;
	};

}
