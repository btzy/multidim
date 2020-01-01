#pragma once

#include <type_traits>
#include <utility> // for std::declval()

#include "fixed_buffer.hpp"

namespace multidim {

	template <typename Container, typename Ref>
	struct enable_inner_container {
		using container_type = Container;
		using container_ref_type = Ref;
		using container_extents_type = typename Container::container_extents_type;
		static_assert(std::is_same_v<typename Container::container_extents_type, typename Ref::container_extents_type>, "Container and Ref must have the same extents_type");
	};



	class unit_extent {
	public:
		constexpr size_t stride() const noexcept {
			return 1;
		}
	};



	template <typename Container, size_t M>
	struct add_dim_to_buffer;

	template <typename T, size_t N, size_t M>
	struct add_dim_to_buffer<fixed_buffer<T, N>, M> {
		using type = fixed_buffer<T, N * M>;
	};

	template <typename Container, size_t M>
	using add_dim_to_buffer_t = typename add_dim_to_buffer<Container, M>::type;



	template <typename T, typename = void>
	struct element_traits {
		using value_type = T;
		using reference = T&;
		using extents_type = unit_extent;
		using base_element = value_type;
		using buffer_type = fixed_buffer<T, 1>;
		constexpr static bool is_inner_container = false;
	};
	template <typename T>
	struct element_traits<T, std::enable_if_t<std::is_base_of_v<enable_inner_container<typename T::container_type, typename T::container_ref_type>, T>>> {
		using value_type = void;
		using reference = typename T::container_ref_type;
		using extents_type = typename T::container_extents_type;
		using base_element = typename reference::base_element;
		using buffer_type = typename reference::buffer_type;
		constexpr static bool is_inner_container = true;
	};



	template <typename T/*, typename = std::void_t<decltype(std::declval<T>().data())>*/>
	constexpr inline decltype(std::declval<T>().data())/*typename std::decay_t<T>::pointer*/ to_pointer(T&& t) noexcept {
		return std::forward<T>(t).data();
	}
	template <typename T, typename = std::enable_if_t<std::is_pointer_v<std::decay_t<T>>>>
	constexpr inline T/*std::decay_t<T>*/ to_pointer(T&& t) noexcept {
		return std::forward<T>(t);
	}
}