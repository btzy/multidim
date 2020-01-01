#pragma once

#include <memory> // for std::forward()
#include <type_traits>

#include "core.hpp"
#include "iterator.hpp"

namespace multidim {

	template <typename T, size_t N>
	class array;
	template <typename T, size_t N>
	class array_ref;
	template <typename T, size_t N>
	class array_const_ref;

	template <typename T, size_t N>
	struct inner_array : public enable_inner_container<array<T, N>, array_ref<T, N>, array_const_ref<T, N>> {};



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



	template <typename Array, typename T, size_t N, bool Owning>
	class array_base {
	public:
		using value_type = typename element_traits<T>::value_type;
		using reference = typename element_traits<T>::reference;
		using const_reference = typename element_traits<T>::const_reference;
		using iterator = iterator_impl<T, false>;
		using const_iterator = iterator_impl<T, true>;
		using difference_type = ptrdiff_t;
		using size_type = size_t;
		using element_extents_type = typename element_traits<T>::extents_type;
		using container_extents_type = static_extent<element_extents_type, N>;
		using base_element = typename element_traits<T>::base_element;
		using buffer_type = typename add_dim_to_buffer_t<typename element_traits<T>::buffer_type, N>;

		constexpr size_type size() const noexcept { return N; }
		constexpr const_iterator cbegin() const noexcept { return multidim::const_iterator<T>(static_cast<const Array&>(*this).data(), extents_, 0); }
		constexpr const_iterator cend() const noexcept { return multidim::const_iterator<T>(static_cast<const Array&>(*this).data_offset(N), extents_, N); }
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
		constexpr array(const array&) noexcept = default;
		constexpr array(array&& other) noexcept = delete;
		template <typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TNs>...>>>
		constexpr explicit array(TNs... ns) noexcept : B(B::element_extents_type(ns...)) {}
		constexpr array& operator=(const array&) noexcept = default;
		constexpr array& operator=(array && other) noexcept = delete;

		constexpr typename B::base_element* data() noexcept { return to_pointer(data_); }
		constexpr const typename B::base_element* data() const noexcept { return to_pointer(data_); }
	private:
		friend class B;
		constexpr typename B::base_element* data_offset(typename B::size_type index) noexcept { return to_pointer(data_) + index * extents_.stride(); }
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(data_) + index * extents_.stride(); }
	public:
		constexpr typename B::reference operator[](typename B::size_type index) noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return B::reference{ data_offset(index), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}

		constexpr typename B::const_iterator begin() const noexcept { return cbegin(); }
		constexpr typename B::iterator begin() noexcept { return multidim::iterator<T>(data(), extents_, 0); }
		constexpr typename B::const_iterator end() const noexcept { return cend(); }
		constexpr typename B::iterator end() noexcept { return multidim::iterator<T>(data_offset(N), extents_, N); }
	};

	template <typename T, size_t N>
	class array_ref : public array_base<array_ref<T, N>, T, N, false> {
	private:
		using B = array_base<array_ref<T, N>, T, N, false>;
		static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
	public:
		constexpr array_ref() = delete;
		constexpr array_ref(const array_ref&) = default;
		constexpr array_ref(array_ref&&) = default;
		constexpr array_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.inner(), data) {}
		constexpr array_ref& operator=(const array_ref&) = default;
		constexpr array_ref& operator=(array_ref&&) = default;

		constexpr typename B::base_element* data() const noexcept { return to_pointer(data_); }
	private:
		friend class B;
		constexpr typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(data_) + index * extents_.stride(); }
	public:
		constexpr typename B::reference operator[](typename B::size_type index) const noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::reference{ data_offset(index), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::iterator begin() const noexcept { return multidim::iterator<T>(data(), extents_, 0); }
		constexpr typename B::iterator end() const noexcept { return multidim::iterator<T>(data_offset(N), extents_, N); }
	};

	template <typename T, size_t N>
	class array_const_ref : public array_base<array_const_ref<T, N>, T, N, false> {
	private:
		using B = array_base<array_const_ref<T, N>, T, N, false>;
		static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
	public:
		constexpr array_const_ref() = delete;
		constexpr array_const_ref(const array_const_ref&) = default;
		constexpr array_const_ref(array_const_ref&&) = default;
		constexpr array_const_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.inner(), data) {}
		constexpr array_const_ref& operator=(const array_const_ref&) = default;
		constexpr array_const_ref& operator=(array_const_ref&&) = default;

		constexpr const typename B::base_element* data() const noexcept { return to_pointer(data_); }
	private:
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(data_) + index * extents_.stride(); }
	public:
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_iterator begin() const noexcept { return cbegin(); }
		constexpr typename B::const_iterator end() const noexcept { return cend(); }
	};

}
