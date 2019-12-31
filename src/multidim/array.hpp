#pragma once

#include <array>
#include <memory>
#include <type_traits>

namespace multidim {

	template <typename Container, typename Ref>
	struct enable_inner_container {
		using container_type = Container;
		using container_ref_type = Ref;
		using container_extents_type = typename Container::container_extents_type;
		static_assert(std::is_same_v<typename Container::container_extents_type, typename Ref::container_extents_type>, "Container and Ref must have the same extents_type");
	};

	template <typename T, size_t N>
	class array;
	template <typename T, size_t N>
	class array_ref;

	template <typename T, size_t N>
	struct inner_array : public enable_inner_container<array<T, N>, array_ref<T, N>> {};



	class unit_extent {
	public:
		constexpr size_t stride() const noexcept {
			return 1;
		}
	};

	template <typename E, size_t N>
	class static_extent {
	public:
		constexpr size_t stride() const noexcept {
			return N * element_extent_.stride();
		}
		constexpr size_t top_extent() const noexcept {
			return N;
		}
		constexpr E inner() const noexcept {
			return element_extent_;
		}
		constexpr static bool is_dynamic = false;
		template <typename... TNs>
		constexpr explicit static_extent(TNs... ns) noexcept : element_extent_(ns...) {}
	private:
		[[no_unique_address]] E element_extent_;
	};

	template <typename E>
	class dynamic_extent {
	public:
		constexpr size_t stride() const noexcept {
			return size_ * element_extent_.stride();
		}
		constexpr size_t top_extent() const noexcept {
			return size_;
		}
		constexpr E inner() const noexcept {
			return element_extent_;
		}
		constexpr static bool is_dynamic = true;
		template <typename TN, typename... TNs>
		constexpr explicit dynamic_extent(TN n, TNs... ns) noexcept : size_(n), element_extent_(ns...) {}
	private:
		size_t size_;
		[[no_unique_address]] E element_extent_;
	};



	template <typename T, size_t N>
	using array_buffer = std::array<T, N>;

	template <typename Container, size_t M>
	struct add_dim_to_buffer;

	template <typename T, size_t N, size_t M>
	struct add_dim_to_buffer<array_buffer<T, N>, M> {
		using type = array_buffer<T, N * M>;
	};

	template <typename Container, size_t M>
	using add_dim_to_buffer_t = typename add_dim_to_buffer<Container, M>::type;



	template <typename T, typename = void>
	struct element_traits {
		using value_type = T;
		using reference = T&;
		using extents_type = unit_extent;
		using base_element = value_type;
		using buffer_type = array_buffer<T, 1>;
		constexpr static bool is_inner_container = false;
	};
	template <typename T>
	struct element_traits<T, std::enable_if_t<std::is_base_of_v<enable_inner_container<typename T::container_type, typename T::container_ref_type>, T>>>{
		using value_type = void;
		using reference = typename T::container_ref_type;
		using extents_type = typename T::container_extents_type;
		using base_element = typename reference::base_element;
		using buffer_type = typename reference::buffer_type;
		constexpr static bool is_inner_container = true;
	};
	/*template <typename T>
	struct element_traits<T, std::enable_if_t<!std::is_base_of_v<enable_inner_container<typename T::container_type, typename T::container_ref_type>, T>>> {
		using value_type = T;
		using reference = T&;
		using extents_type = unit_extent;
		using base_element = value_type;
		constexpr static bool is_inner_container = false;
	};*/

	template <typename T, bool IsConst>
	class iterator_impl {

	};



	template <typename T/*, typename = std::void_t<decltype(std::declval<T>().data())>*/>
	constexpr inline decltype(std::declval<T>().data())/*typename std::decay_t<T>::pointer*/ to_pointer(T&& t) noexcept {
		return std::forward<T>(t).data();
	}
	template <typename T, typename = std::enable_if_t<std::is_pointer_v<std::decay_t<T>>>>
	constexpr inline T/*std::decay_t<T>*/ to_pointer(T&& t) noexcept {
		return std::forward<T>(t);
	}



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
			return extents_.top_extent();
		}
		constexpr base_element* data() noexcept {
			return to_pointer(data_);
		}
		constexpr reference operator[](size_t index) noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				const auto& element_extents = extents_.inner();
				return reference{ data() + index * element_extents.stride(), element_extents };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
	protected:
		using underlying_store = std::conditional_t<Owning, buffer_type, base_element*>;
		template <typename... Args>
		constexpr array_base(const container_extents_type& extents, Args&&... args) noexcept : data_(std::forward<Args>(args)...), extents_(extents) {}
		constexpr array_base(const array_base& other) = default;
		constexpr array_base(array_base&& other) = default;
		constexpr array_base& operator=(const array_base& other) = default;
		constexpr array_base& operator=(array_base&& other) = default;
		underlying_store data_;
		[[no_unique_address]] container_extents_type extents_;
	};

	template <typename T, size_t N>
	class array : public array_base<array<T, N>, T, N, true> {
	public:
		using B = array_base<array<T, N>, T, N, true>;
		template <typename... TNs>
		constexpr explicit array(TNs... ns) noexcept : B(B::container_extents_type(ns...)) {}
	};

	template <typename T, size_t N>
	class array_ref : public array_base<array_ref<T, N>, T, N, false> {
	public:
		template <typename FArray, typename FT, size_t FN, bool FOwning>
		friend class array_base;
	private:
		using B = array_base<array_ref<T, N>, T, N, false>;
		constexpr array_ref() = delete;
		constexpr array_ref(const array_ref&) = delete;
		constexpr array_ref(array_ref&&) = delete;
		constexpr array_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents, data) {
			static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
		}
		constexpr array_ref& operator=(const array_ref& other) = default;
		constexpr array_ref& operator=(array_ref&& other) = default;
	};

}
