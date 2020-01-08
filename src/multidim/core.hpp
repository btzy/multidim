#pragma once

#include <type_traits>
#include <utility> // for std::declval()

#include "fixed_buffer.hpp"
#include "dynamic_buffer.hpp"

#if defined(__GNUC__)
#define MULTIDIM_FORCEINLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define MULTIDIM_FORCEINLINE __forceinline
#else
#define MULTIDIM_FORCEINLINE 
#endif

namespace multidim {

	struct inner_container_base {};
	struct reference_base {};

	/**
	 * Inherit from this class to define an inner container type (e.g. inner_array or inner_dynarray).
	 */
	template <typename Container, typename Ref, typename ConstRef>
	struct enable_inner_container : public inner_container_base {
		using container_type = Container;
		using container_ref_type = Ref;
		using container_const_ref_type = ConstRef;
		using container_extents_type = typename Container::container_extents_type;
		static_assert(std::is_same_v<typename Container::container_extents_type, typename Ref::container_extents_type> && std::is_same_v<typename Ref::container_extents_type, typename ConstRef::container_extents_type>, "Container, Ref, and ConstRef must have the same extents_type");
	};

	/**
	 * Inherit from this class to indicate that it is a fake reference type.
	 * @param T the reference type (for CRTP)
	 */
	template <typename T>
	struct enable_reference : public reference_base {};



	/**
	 * Represents the extent of a base element type.  This class has no members, and any two instances of this class compare equal to each other.
	 */
	class unit_extent {
	public:
		/**
		 * Gets the number of base elements represented in this extent.  For unit_extent, this is always equal to 1.
		 */
		constexpr size_t stride() const noexcept {
			return 1;
		}
		friend bool operator==(const unit_extent&, const unit_extent&) noexcept { return true; }
		friend bool operator!=(const unit_extent& a, const unit_extent& b) noexcept { return !(a == b); }
	};



	/**
	 * Adds a dimension to the buffer.
	 * If Container is a fixed_buffer, this produces a fixed_buffer that is M times larger.
	 * If Container is a dynamic_buffer, this produces a dynamic_buffer.
	 */
	template <typename Container, size_t M>
	struct add_dim_to_buffer;

	template <typename T, size_t N, size_t M>
	struct add_dim_to_buffer<fixed_buffer<T, N>, M> {
		using type = fixed_buffer<T, N * M>;
	};

	template <typename T, size_t M>
	struct add_dim_to_buffer<dynamic_buffer<T>, M> {
		using type = dynamic_buffer<T>;
	};

	/**
	 * Convenience typedef for add_dim_to_buffer.
	 */
	template <typename Container, size_t M>
	using add_dim_to_buffer_t = typename add_dim_to_buffer<Container, M>::type;



	/**
	 * Gets the traits of a given type, for use as elements in a container.
	 * If T is not an inner container, then the member typedefs are like those in standard library containers.
	 * If T is an inner container, then reference and const_reference will not be real reference types, and value_type and pointer and const_pointer will be void.
	 */
	template <typename T, typename = void>
	struct element_traits {
		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using extents_type = unit_extent;
		using base_element = value_type;
		using buffer_type = fixed_buffer<T, 1>;
		constexpr static bool is_inner_container = false;
	};
	template <typename T>
	struct element_traits<T, std::enable_if_t<std::is_base_of_v<inner_container_base, T>>> {
		using value_type = void;
		using reference = typename T::container_ref_type;
		using const_reference = typename T::container_const_ref_type;
		using pointer = void;
		using const_pointer = void;
		using extents_type = typename T::container_extents_type;
		using base_element = typename reference::base_element;
		using buffer_type = typename reference::buffer_type;
		constexpr static bool is_inner_container = true;
	};



	/**
	 * Converts a possibly owning buffer to a pointer.
	 * If t is a pointer, then t is returned.
	 * If t.data() is well-formed, then t.data() is returned.
	 * Otherwise, to_pointer(t) is ill-formed.
	 * @param t the buffer
	 */
	template <typename T>
	constexpr inline decltype(std::declval<T>().data()) to_pointer(T&& t) noexcept {
		return std::forward<T>(t).data();
	}
	template <typename T, typename = std::enable_if_t<std::is_pointer_v<std::decay_t<T>>>>
	constexpr inline T to_pointer(T&& t) noexcept {
		return std::forward<T>(t);
	}
}