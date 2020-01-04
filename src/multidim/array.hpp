#pragma once

#include <iterator> // for std::reverse_iterator
#include <memory> // for std::forward()
#include <type_traits>
#include <utility> // for declaration of std::tuple_size / std::tuple_element

#include "core.hpp"
#include "iterator.hpp"

namespace multidim {

	template <typename T, size_t N>
	class array;
	template <typename T, size_t N>
	class array_ref;
	template <typename T, size_t N>
	class array_const_ref;

	/**
	 * A tag type to specify nested arrays except the topmost one.
	 */
	template <typename T, size_t N>
	struct inner_array : public enable_inner_container<array<T, N>, array_ref<T, N>, array_const_ref<T, N>> {};



	/**
	 * An extent that is known at compilation time.  This class is immutable unless reassigned using operator=.
	 * @tparam E the extent of the next inner dimension (it will be unit_extent if there are no more inner dimensions)
	 * @tparam N the size of this dimension
	 */
	template <typename E, size_t N>
	class static_extent {
	public:
		/**
		 * Gets the number of base elements represented in this extent.  For static_extent, this is always equal to N times of the stride of the inner extent.
		 */
		constexpr size_t stride() const noexcept {
			return N * element_extent_.stride();
		}
		/**
		 * Gets the number of elements (not necessarily base elements) represented in this extent.  For static_extent, this is always equal to N.
		 */
		constexpr size_t top_extent() const noexcept {
			return N;
		}
		/**
		 * Gets a reference to the inner extent.
		 */
		constexpr const E& inner() const noexcept {
			return element_extent_;
		}
		/**
		 * Trait to detect whether this extent is dynamic, for static_extent this is false.
		 */
		constexpr static bool is_dynamic = false;
		/**
		 * Constructs a static_extent.  All parameters are forwarded to the inner extent.
		 */
		template <typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TNs>...>>>
		constexpr explicit static_extent(TNs... ns) noexcept : element_extent_(ns...) {}
		constexpr explicit static_extent(const E& element_extent) noexcept : element_extent_(element_extent) {}
		friend bool operator==(const static_extent& a, const static_extent& b) noexcept { return a.element_extent_ == b.element_extent_; }
		friend bool operator!=(const static_extent& a, const static_extent& b) noexcept { return !(a == b); }

	private:
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4848)
#endif
		[[no_unique_address]] E element_extent_;
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
	};



	/**
	 * Base class for array.  This is an internal library implementation and should not be used directly by users. 
	 */
	template <typename Array, typename T, size_t N, bool Owning, bool IsConst>
	class array_base {
	public:
		using value_type = typename element_traits<T>::value_type;
		using reference = typename element_traits<T>::reference;
		using const_reference = typename element_traits<T>::const_reference;
		using pointer = typename element_traits<T>::pointer;
		using const_pointer = typename element_traits<T>::const_pointer;
		using iterator = iterator_impl<T, false>;
		using const_iterator = iterator_impl<T, true>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using difference_type = ptrdiff_t;
		using size_type = size_t;
		using element_extents_type = typename element_traits<T>::extents_type;
		using container_extents_type = static_extent<element_extents_type, N>;
		using base_element = typename element_traits<T>::base_element;
		using buffer_type = add_dim_to_buffer_t<typename element_traits<T>::buffer_type, N>;

		constexpr size_type size() const noexcept { return N; }
		constexpr size_type max_size() const noexcept { return N; }
		[[nodiscard]] constexpr bool empty() const noexcept { return N == 0; }
		constexpr const_iterator cbegin() const noexcept { return multidim::const_iterator<T>(static_cast<const Array&>(*this).data(), extents_, 0); }
		constexpr const_iterator cend() const noexcept { return multidim::const_iterator<T>(static_cast<const Array&>(*this).data_offset(N), extents_, N); }
		constexpr const_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
		constexpr const_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }
	protected:
		using underlying_store = std::conditional_t<Owning, buffer_type, std::conditional_t<IsConst, const base_element*, base_element*>>;
		template <typename... Args>
		constexpr array_base(const element_extents_type& extents, Args&&... args) noexcept : data_(std::forward<Args>(args)...), extents_(extents) {}
		constexpr array_base() = default;
		constexpr array_base(const array_base&) = delete;
		constexpr array_base& operator=(const array_base&) = delete;
		// no move constructor or move assignment operator, derived classes should use the first constructor of array_base to do it.

		/**
		 * Swaps the content of this array with another one.
		 */
		constexpr void swap(array_base& other) noexcept {
			using std::swap;
			swap(data_, other.data_);
			swap(extents_, other.extents_);
		}

		/**
		 * Gets the extents of elements that are stored in this array.
		 */
		constexpr const element_extents_type& extents() const noexcept { return extents_; }

		underlying_store data_;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4848)
#endif
		[[no_unique_address]] element_extents_type extents_;
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
	};

	/**
	 * Represents a multidimensional array whose outermost dimension is an array with a size that is fixed at compilation time.
	 * @tparam T the element type; if this is the innermost dimension then T is the base element type, otherwise T is an inner container (i.e. something that extends from enable_inner_container)
	 * @tparam N the number of elements in this array
	 */
	template <typename T, size_t N>
	class array : public array_base<array<T, N>, T, N, true, false> {
	public:
		using B = array_base<array<T, N>, T, N, true, false>;
		constexpr array(const array& other) : B(other.extents_, other.data_.clone(N * other.extents_.stride())) {}
		constexpr array(array&& other) noexcept(std::is_nothrow_move_constructible_v<typename B::buffer_type>) : B(other.extents_, std::move(other.data_)) {
			other.extents_ = typename B::element_extents_type();
		}
		/**
		 * Constructs an array from the given element_extents_type.  This should not generally be used directly.
		 */
		constexpr explicit array(const typename B::element_extents_type& extents) noexcept : B(extents, N * extents.stride()) {}
		/**
		 * Constructs an array from the given dimensions.
		 * Note: Dimensions are only specified for dynarray layers.  Compile-time fixed arrays do not need a dimension parameter.
		 */
		template <typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TNs>...>>>
		constexpr explicit array(TNs... ns) noexcept : array(typename B::element_extents_type(ns...)) {}
		constexpr array& operator=(const array& other) {
			this->extents_ = other.extents_;
			this->data_ = other.data_.clone(N * other.extents_.stride());
			return *this;
		}
		constexpr array& operator=(array&& other) noexcept(std::is_nothrow_move_assignable_v<typename B::buffer_type>) {
			this->extents_ = other.extents_;
			this->data_ = std::move(other.data_); // will reset other.data_
			other.extents_ = typename B::element_extents_type();
			return *this;
		};

		/**
		 * Swaps two arrays.  This will invalidate references to the arrays if any inner array is an inner_dynarray (in practice, any existing references for one array will now refer to something in the other array).  If all inner arrays have compile-time fixed size, then this function does an element-wise swap.
		 */
		friend constexpr void swap(array& a, array& b) noexcept(std::is_nothrow_swappable_v<typename B::buffer_type>) {
			a.swap(b);
		}
		/**
		 * Swaps this array with another one.  This will invalidate references to the the arrays if any inner array is an inner_dynarray (in practice, any references for one array will now refer to something in the other array).  If all inner arrays have compile-time fixed size, then this function does an element-wise swap.
		 */
		constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<typename B::buffer_type>) {
			B::swap(other);
		}


		/**
		 * Converting operator to array_ref.
		 */
		constexpr operator array_ref<T, N>() noexcept {
			return array_ref<T, N>{ this->data_.data(), typename B::container_extents_type{ this->extents_ } };
		}
		/**
		 * Converting operator to array_const_ref.
		 */
		constexpr operator array_const_ref<T, N>() const noexcept {
			return array_const_ref<T, N>{ this->data_.data(), typename B::container_extents_type{ this->extents_ } };
		}

		/**
		 * Gets a pointer to the underlying base elements.
		 */
		constexpr typename B::base_element* data() noexcept { return to_pointer(this->data_); }
		constexpr const typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		/**
		 * Gets a pointer to the underlying base elements, offsetted by some index.
		 */
		constexpr typename B::base_element* data_offset(typename B::size_type index) noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		/**
		 * Gets a reference to the element at the specified index.
		 */
		constexpr typename B::reference operator[](typename B::size_type index) noexcept {
			assert(index < N);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			assert(index < N);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::reference at(typename B::size_type index) noexcept { if (index >= N) throw std::out_of_range("element access index out of range"); else return operator[](index); }
		constexpr typename B::const_reference at(typename B::size_type index) const noexcept { if (index >= N) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::const_iterator begin() const noexcept { return this->cbegin(); }
		constexpr typename B::iterator begin() noexcept { return multidim::iterator<T>(data(), this->extents_, 0); }
		constexpr typename B::const_iterator end() const noexcept { return this->cend(); }
		constexpr typename B::iterator end() noexcept { return multidim::iterator<T>(data_offset(N), this->extents_, N); }
		constexpr typename B::const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }
		constexpr typename B::reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::reference front() noexcept { return operator[](0); }
		constexpr typename B::const_reference front() const noexcept { return operator[](0); }
		constexpr typename B::reference back() noexcept { return operator[](N - 1); }
		constexpr typename B::const_reference back() const noexcept { return operator[](N - 1); }

		template <typename B::size_type I>
		friend constexpr typename B::reference get(array& arr) noexcept { static_assert(I < N, "index out of bounds"); return arr[I]; }
		template <typename B::size_type I>
		friend constexpr typename B::const_reference get(const array& arr) noexcept { static_assert(I < N, "index out of bounds");  return arr[I]; }

		constexpr void fill(typename B::const_reference value) noexcept(std::is_nothrow_assignable_v<typename B::reference, typename B::const_reference>) {
			for (typename B::reference x : *this) {
				x = value;
			}
		}

		/**
		 * Compares if two dynarrays are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const array& a, const array_const_ref<T, N>& b) {
			return static_cast<array_const_ref<T, N>>(a) == b;
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const array& a, const array_const_ref<T, N>& b) { return !(a == b); };
		// Note: We don't provide lexicographical comparison because it isn't clear what it means to compare arrays of different shape.
	};

	/**
	 * Represents a reference to a (slice of a) multidimensional array whose outermost dimension is an array with a size that is fixed at compilation time.
	 * Note: This type has the semantics of a reference type:  Copy/move construction will construct an array_ref that refers to the same data as the other one.  Copy/move assignment will copy/move the other data to the place that the current array_ref refers to.
	 * @tparam T the element type; if this is the innermost dimension then T is the base element type, otherwise T is an inner container (i.e. something that extends from enable_inner_container)
	 * @tparam N the number of elements in this array
	 */
	template <typename T, size_t N>
	class array_ref : public array_base<array_ref<T, N>, T, N, false, false> {
	private:
		using B = array_base<array_ref<T, N>, T, N, false, false>;
		static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
	public:
		/**
		 * Default-constructed array_ref.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==.
		 */
		constexpr array_ref() noexcept = default;
		/**
		 * Copy-constructs an array_ref.  The new array_ref refers to the same data as the old one.
		 */
		constexpr array_ref(const array_ref& other) noexcept : B(other.extents_, other.data_) {}
		//constexpr array_ref(array_ref&&) noexcept : B(extents_, data_) {}
		/**
		 * Constructs an array_ref from the given data and container extents.
		 * Users should not need to use this function unless they have acquired the data from an external source.
		 */
		constexpr array_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.inner(), data) {}
		/**
		 * Copies the data from another array_ref to this one.
		 * This does an element-wise copy of the data that these array_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 * Note: The overloads for const array_ref& is necessary otherwise an implicitly defined one will be generated that does something different.  But since we define this, we have to define one for const array& as well, otherwise three would be overload resolution ambiguities.
		 * Note: There might be pessimisation here, because extents are copied when we static_cast the refs.
		 */
		constexpr array_ref& operator=(const array<T, N>& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) { return *this = static_cast<array_const_ref<T, N>>(other); }
		constexpr array_ref& operator=(const array_ref& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) { return *this = static_cast<array_const_ref<T, N>>(other); }
		constexpr array_ref& operator=(const array_const_ref<T, N>& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) {
			assert(this->extents_ == other.extents_);
			std::copy_n(other.data_, N * this->extents_.stride(), this->data_);
			return *this;
		}
		// Moving from array_ref disabled for now, pending design review.  We may need an array_move_ref class instead.
		/*constexpr array_ref& operator=(array_ref&& other) noexcept(std::is_nothrow_move_assignable_v<typename B::base_element>) {
			assert(this->extents_ == other.extents_);
			std::copy_n(std::make_move_iterator(other.data_), N * this->extents_.stride(), this->data_);
			return *this;
		}*/

		/**
		 * Swaps the content of two array_refs.
		 * This does an element-wise swap of the data that these array_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 */
		friend constexpr void swap(const array_ref& a, const array_ref& b) noexcept(std::is_nothrow_swappable_v<typename B::base_element>) {
			assert(a.extents_ == b.extents_);
			std::swap_ranges(a.data_, a.data_ + N * a.extents_.stride(), b.data_);
		}
		/**
		 * Swaps the content of this array_ref with another one.
		 * This does an element-wise swap of the data that these array_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 */
		constexpr void swap(const array_ref& other) const noexcept(std::is_nothrow_swappable_v<typename B::base_element>) { swap(*this, other); }

		/**
		 * Converting operator to array_const_ref
		 */
		constexpr operator array_const_ref<T, N>() const noexcept {
			return array_const_ref<T, N>{this->data_, typename B::container_extents_type{ this->extents_ } };
		}

		constexpr typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		constexpr typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		constexpr typename B::reference operator[](typename B::size_type index) const noexcept {
			assert(index < N);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::reference at(typename B::size_type index) const noexcept { if (index >= N) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::iterator begin() const noexcept { return multidim::iterator<T>(data(), this->extents_, 0); }
		constexpr typename B::iterator end() const noexcept { return multidim::iterator<T>(data_offset(N), this->extents_, N); }
		constexpr typename B::reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::reference front() const noexcept { return operator[](0); }
		constexpr typename B::reference back() const noexcept { return operator[](N - 1); }

		template <typename B::size_type I>
		friend constexpr typename B::reference get(const array_ref& arr) noexcept { static_assert(I < N, "index out of bounds"); return arr[I]; }

		constexpr void fill(typename B::const_reference value) noexcept(std::is_nothrow_assignable_v<typename B::reference, typename B::const_reference>) {
			for (typename B::reference x : *this) {
				x = value;
			}
		}

		/**
		 * Compares if two array_refs are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const array_ref& a, const array_const_ref<T, N>& b) {
			return static_cast<array_const_ref<T, N>>(a) == b;
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const array_ref& a, const array_const_ref<T, N>& b) { return !(a == b); };

		constexpr operator array_const_ref<T, N>() noexcept { return array_const_ref<T, N>{ this->data_, typename B::container_extents_type{ this->extents_ } }; }

		/**
		 * Rebinds this reference to another array_ref.
		 */
		constexpr inline void rebind(const array_ref& other) noexcept {
			this->data_ = other.data_;
			this->extents_ = other.extents_;
		}
		/**
		 * Rebinds this reference to another object.
		 */
		constexpr inline void rebind(typename B::base_element* data) noexcept { this->data_ = data; }
		/**
		 * Rebinds this reference to another object that is n objects away from the current object.
		 */
		constexpr inline void rebind_relative(typename B::difference_type n) noexcept { this->data_ += n * N * this->extents_.stride(); }
	};

	template <typename T, size_t N>
	class array_const_ref : public array_base<array_const_ref<T, N>, T, N, false, true> {
	private:
		using B = array_base<array_const_ref<T, N>, T, N, false, true>;
		static_assert(!B::container_extents_type::is_dynamic, "extents_type must be static");
		friend class array_ref<T, N>;
	public:
		/**
		 * Default-constructed array_const_ref.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==.
		 */
		constexpr array_const_ref() noexcept = default;
		constexpr array_const_ref(const array_const_ref& other) noexcept : B(other.extents_, other.data_) {}
		//constexpr array_const_ref(array_const_ref&&) = default;
		constexpr array_const_ref(const typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.inner(), data) {}

		constexpr const typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			assert(index < N);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_reference at(typename B::size_type index) const noexcept { if (index >= N) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::const_iterator begin() const noexcept { return this->cbegin(); }
		constexpr typename B::const_iterator end() const noexcept { return this->cend(); }
		constexpr typename B::const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::const_reference front() const noexcept { return operator[](0); }
		constexpr typename B::const_reference back() const noexcept { return operator[](N - 1); }

		template <typename B::size_type I>
		friend constexpr typename B::const_reference get(const array_const_ref& arr) noexcept { static_assert(I < N, "index out of bounds"); return arr[I]; }

		/**
		 * Compares if two array_const_refs are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr bool operator==(const array_const_ref& a, const array_const_ref& b) {
			return a.extents_ == b.extents_ && std::equal(a.data(), a.data_offset(N), b.data());
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const array_const_ref& a, const array_const_ref& b) { return !(a == b); };

		/**
		 * Rebinds this reference to another array_const_ref.
		 */
		constexpr inline void rebind(const array_const_ref& other) noexcept {
			this->data_ = other.data_;
			this->extents_ = other.extents_;
		}
		/**
		 * Rebinds this reference to another object.
		 */
		constexpr inline void rebind(const typename B::base_element* data) noexcept { this->data_ = data; }
		/**
		 * Rebinds this reference to another object that is n objects away from the current object.
		 */
		constexpr inline void rebind_relative(typename B::difference_type n) noexcept { this->data_ += n * N * this->extents_.stride(); }
	};

}

/**
 * Specialisations for std::tuple_size and std::tuple_element
 */
namespace std {
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
	template <typename T, size_t N>
	struct tuple_size<multidim::array<T, N>> : public std::integral_constant<size_t, N> {};
	template <size_t I, typename T, size_t N>
	struct tuple_element<I, multidim::array<T, N>> {
		using type = typename multidim::element_traits<T>::value_type;
	};
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
