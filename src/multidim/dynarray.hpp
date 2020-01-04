#pragma once

#include <iterator> // for std::reverse_iterator
#include <memory> // for std::forward()
#include <type_traits>

#include "core.hpp"
#include "iterator.hpp"

namespace multidim {

	template <typename T>
	class dynarray;
	template <typename T>
	class dynarray_ref;
	template <typename T>
	class dynarray_const_ref;

	/**
	 * A tag type to specify nested dynarrays except the topmost one.
	 */
	template <typename T>
	struct inner_dynarray : public enable_inner_container<dynarray<T>, dynarray_ref<T>, dynarray_const_ref<T>> {};



	/**
	 * An extent that is only known at construction time.  This class is immutable unless reassigned using operator=.
	 * @tparam E the extent of the next inner dimension (it will be unit_extent if there are no more inner dimensions)
	 */
	template <typename E>
	class dynamic_extent {
	public:
		/**
		 * Gets the number of base elements represented in this extent.  For dynamic_extent, this is equal to the size of this dimension times of the stride of the inner extent.
		 */
		constexpr size_t stride() const noexcept {
			return size_ * element_extent_.stride();
		}
		/**
		 * Gets the number of elements (not necessarily base elements) represented in this extent, i.e. the size of this dimension.
		 */
		constexpr size_t top_extent() const noexcept {
			return size_;
		}
		/**
		 * Gets a reference to the inner extent.
		 */
		constexpr const E& inner() const noexcept {
			return element_extent_;
		}
		/**
		 * Trait to detect whether this extent is dynamic, for dynamic_extent this is false.
		 */
		constexpr static bool is_dynamic = true;
		/**
		 * Constructs a dynamic_extent with a size of n.  Additional parameters are forwarded to the inner extent.
		 */
		template <typename TN, typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TN>, std::is_convertible<size_t, TNs>...>>>
		constexpr explicit dynamic_extent(TN n, TNs... ns) noexcept : size_(n), element_extent_(ns...) {}
		/**
		 * Constructs a dynamic_extent with a size of zero.  All inner extents will also be default constructed.
		 */
		constexpr explicit dynamic_extent() noexcept : size_(0), element_extent_() {}
		constexpr explicit dynamic_extent(size_t size, const E& element_extent) noexcept : size_(size), element_extent_(element_extent) {}
		friend bool operator==(const dynamic_extent& a, const dynamic_extent& b) noexcept { return a.size_ == b.size_ && a.element_extent_ == b.element_extent_; }
		friend bool operator!=(const dynamic_extent& a, const dynamic_extent& b) noexcept { return !(a == b); }

	private:
		size_t size_;
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
	 * Base class for dynarray.  This is an internal library implementation and should not be used directly by users.
	 */
	template <typename Dynarray, typename T, bool Owning, bool IsConst>
	class dynarray_base {
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
		using container_extents_type = dynamic_extent<element_extents_type>;
		using base_element = typename element_traits<T>::base_element;
		using buffer_type = dynamic_buffer<base_element>;

		constexpr size_type size() const noexcept { return size_; }
		constexpr size_type max_size() const noexcept { return size_; }
		[[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
		constexpr const_iterator cbegin() const noexcept { return multidim::const_iterator<T>(static_cast<const Dynarray&>(*this).data(), extents_, 0); }
		constexpr const_iterator cend() const noexcept { return multidim::const_iterator<T>(static_cast<const Dynarray&>(*this).data_offset(size_), extents_, size_); }
		constexpr const_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
		constexpr const_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }
	protected:
		using underlying_store = std::conditional_t<Owning, buffer_type, std::conditional_t<IsConst, const base_element*, base_element*>>;
		template <typename... Args>
		constexpr dynarray_base(size_t size, const element_extents_type& extents, Args&&... args) noexcept : data_(std::forward<Args>(args)...), size_(size), extents_(extents) {}
		constexpr dynarray_base() noexcept : size_(0) {}
		constexpr dynarray_base(const dynarray_base&) = delete;
		constexpr dynarray_base& operator=(const dynarray_base&) = delete;
		// no move constructor or move assignment operator, derived classes should use the first constructor of dynarray_base to do it.

		/**
		 * Swaps the content of this dynarray with another one.
		 */
		constexpr void swap(dynarray_base& other) noexcept {
			using std::swap;
			swap(data_, other.data_);
			swap(size_, other.size_);
			swap(extents_, other.extents_);
		}

		/**
		 * Gets the extents of elements that are stored in this dynarray.
		 */
		constexpr const element_extents_type& extents() const noexcept { return extents_; }

		underlying_store data_;
		size_t size_; // the size of the current dimension
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
	 * Represents a multidimensional array whose outermost dimension is an array with a size that is known at construction time.
	 * @tparam T the element type; if this is the innermost dimension then T is the base element type, otherwise T is an inner container (i.e. something that extends from enable_inner_container)
	 */
	template <typename T>
	class dynarray : public dynarray_base<dynarray<T>, T, true, false> {
	public:
		using B = dynarray_base<dynarray<T>, T, true, false>;
		constexpr dynarray(const dynarray& other) : B(other.size_, other.extents_, other.data_.clone(other.size_ * other.extents_.stride())) {}
		constexpr dynarray(dynarray&& other) noexcept(std::is_nothrow_move_constructible_v<typename B::buffer_type>) : B(other.size_, other.extents_, std::move(other.data_)) {
			other.size_ = 0;
			other.extents_ = typename B::element_extents_type();
		}
		/**
		 * Constructs an dynarray from the given size (current dimension) and element_extents_type (inner dimensions).  This should not generally be used directly.
		 */
		constexpr explicit dynarray(size_t size, const typename B::element_extents_type& extents) noexcept : B(size, extents, size * extents.stride()) {}
		/**
		 * Constructs an dynarray from the given dimensions.
		 * Note: Dimensions are only specified for dynarray layers.  Compile-time fixed arrays do not need a dimension parameter.
		 */
		template <typename TN, typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TN>, std::is_convertible<size_t, TNs>...>>>
		constexpr explicit dynarray(TN n, TNs... ns) noexcept : dynarray(n, typename B::element_extents_type(ns...)) {}
		constexpr explicit dynarray() noexcept : dynarray(0, typename B::element_extents_type()) {}
		constexpr dynarray& operator=(const dynarray& other) {
			this->size_ = other.size_;
			this->extents_ = other.extents_;
			this->data_ = other.data_.clone(other.size_ * other.extents_.stride());
			return *this;
		}
		constexpr dynarray& operator=(dynarray&& other) noexcept(std::is_nothrow_move_assignable_v<typename B::buffer_type>) {
			this->size_ = other.size_;
			this->extents_ = other.extents_;
			this->data_ = std::move(other.data_); // will reset other.data_
			other.size_ = 0;
			other.extents_ = typename B::element_extents_type();
			return *this;
		};

		/**
		 * Swaps two dynarrays.  This will invalidate references to both arrays (in practice, any existing references for one dynarray will now refer to something in the other dynarray).
		 */
		friend constexpr void swap(dynarray& a, dynarray& b) noexcept(std::is_nothrow_swappable_v<typename B::buffer_type>) {
			a.swap(b);
		}
		/**
		 * Swaps this dynarray with another one.  This will invalidate references to both arrays (in practice, any existing references for one dynarray will now refer to something in the other dynarray).
		 */
		constexpr void swap(dynarray& other) noexcept(std::is_nothrow_swappable_v<typename B::buffer_type>) {
			B::swap(other);
		}


		/**
		 * Converting operator to dynarray_ref.
		 */
		constexpr operator dynarray_ref<T>() noexcept {
			return dynarray_ref<T>{ this->data_.data(), typename B::container_extents_type{ this->size_, this->extents_ } };
		}
		/**
		 * Converting operator to dynarray_const_ref.
		 */
		constexpr operator dynarray_const_ref<T>() const noexcept {
			return dynarray_const_ref<T>{ this->data_.data(), typename B::container_extents_type{ this->size_, this->extents_ } };
		}

		/**
		 * Gets a pointer to the underlying base elements.
		 */
		constexpr typename B::base_element* data() noexcept { return to_pointer(this->data_); }
		constexpr const typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		/**
		 * Gets a pointer to the underlying base elements, offsetted by some index.  It is undefined behaviour if index > size().  If index == size() then this function is value, but the returned pointer may not be dereferenced.
		 */
		constexpr typename B::base_element* data_offset(typename B::size_type index) noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		/**
		 * Gets a reference to the element at the specified index.  It is undefined behaviour if index >= size().
		 */
		constexpr typename B::reference operator[](typename B::size_type index) noexcept {
			assert(index < this->size_);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			assert(index < this->size_);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::reference at(typename B::size_type index) noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }
		constexpr typename B::const_reference at(typename B::size_type index) const noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::const_iterator begin() const noexcept { return this->cbegin(); }
		constexpr typename B::iterator begin() noexcept { return multidim::iterator<T>(data(), this->extents_, 0); }
		constexpr typename B::const_iterator end() const noexcept { return this->cend(); }
		constexpr typename B::iterator end() noexcept { return multidim::iterator<T>(data_offset(this->size_), this->extents_, this->size_); }
		constexpr typename B::const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }
		constexpr typename B::reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::reference front() noexcept { return operator[](0); }
		constexpr typename B::const_reference front() const noexcept { return operator[](0); }
		constexpr typename B::reference back() noexcept { return operator[](this->size_ - 1); }
		constexpr typename B::const_reference back() const noexcept { return operator[](this->size_ - 1); }

		constexpr void fill(typename B::const_reference value) noexcept(std::is_nothrow_assignable_v<typename B::reference, typename B::const_reference>) {
			for (typename B::reference x : *this) {
				x = value;
			}
		}

		/**
		 * Compares if two dynarrays are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const dynarray& a, const dynarray_const_ref<T>& b) {
			return static_cast<dynarray_const_ref<T>>(a) == b;
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const dynarray& a, const dynarray_const_ref<T>& b) { return !(a == b); };
		// Note: We don't provide lexicographical comparison because it isn't clear what it means to compare arrays of different shape.
	};

	/**
	 * Represents a reference to a (slice of a) multidimensional array whose outermost dimension is an dynarray.
	 * Note: This type has the semantics of a reference type:  Copy/move construction will construct an dynarray_ref that refers to the same data as the other one.  Copy/move assignment will copy/move the other data to the place that the current dynarray_ref refers to.
	 * @tparam T the element type; if this is the innermost dimension then T is the base element type, otherwise T is an inner container (i.e. something that extends from enable_inner_container)
	 */
	template <typename T>
	class dynarray_ref : public dynarray_base<dynarray_ref<T>, T, false, false> {
	private:
		using B = dynarray_base<dynarray_ref<T>, T, false, false>;
		static_assert(B::container_extents_type::is_dynamic, "extents_type must be dynamic");
	public:
		/**
		 * Default-constructed dynarray_ref.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==.
		 */
		constexpr dynarray_ref() noexcept = default;
		/**
		 * Copy-constructs a dynarray_ref.  The new dynarray_ref refers to the same data as the old one.
		 */
		constexpr dynarray_ref(const dynarray_ref& other) noexcept : B(other.size_, other.extents_, other.data_) {}
		//constexpr array_ref(array_ref&&) noexcept : B(extents_, data_) {}
		/**
		 * Constructs an array_ref from the given data and container extents.
		 * Users should not need to use this function unless they have acquired the data from an external source.
		 */
		constexpr dynarray_ref(typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.top_extent(), extents.inner(), data) {}
		/**
		 * Copies the data from another dynarray_ref to this one.
		 * This does an element-wise copy of the data that these dynarray_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 */
		constexpr MULTIDIM_FORCEINLINE dynarray_ref& operator=(const dynarray<T>& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) { return *this = static_cast<dynarray_const_ref<T>>(other); }
		constexpr MULTIDIM_FORCEINLINE dynarray_ref& operator=(const dynarray_ref& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) { return *this = static_cast<dynarray_const_ref<T>>(other); }
		constexpr MULTIDIM_FORCEINLINE dynarray_ref& operator=(const dynarray_const_ref<T>& other) noexcept(std::is_nothrow_copy_assignable_v<typename B::base_element>) {
			assert(this->size_ == other.size_);
			assert(this->extents_ == other.extents_);
			std::copy_n(other.data_, this->size_ * this->extents_.stride(), this->data_);
			return *this;
		}
		// Moving from dynarray_ref disabled for now, pending design review.
		/*constexpr dynarray_ref& operator=(dynarray_ref&& other) noexcept(std::is_nothrow_move_assignable_v<typename B::base_element>) {
			assert(this->size_ == other.size_);
			assert(this->extents_ == other.extents_);
			std::copy_n(std::make_move_iterator(other.data_), this->size_ * this->extents_.stride(), this->data_);
			return *this;
		}*/

		/**
		 * Swaps the content of two dynarray_refs.
		 * This does an element-wise swap of the data that these dynarray_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 */
		friend constexpr void swap(const dynarray_ref& a, const dynarray_ref& b) noexcept(std::is_nothrow_swappable_v<typename B::base_element>) {
			assert(a.size_ == b.size_);
			assert(a.extents_ == b.extents_);
			std::swap_ranges(a.data_, a.data_ + a.size_ * a.extents_.stride(), b.data_);
		}
		/**
		 * Swaps the content of this dynarray_ref with another one.
		 * This does an element-wise swap of the data that these array_refs refer to.
		 * If the two arrays do not have the same extents, then behaviour is undefined.
		 */
		constexpr void swap(const dynarray_ref& other) const noexcept(std::is_nothrow_swappable_v<typename B::base_element>) { swap(*this, other); }

		/**
		 * Converting operator to dynarray_const_ref
		 */
		constexpr operator dynarray_const_ref<T>() const noexcept {
			return dynarray_const_ref<T>{this->data_, typename B::container_extents_type{ this->size_, this->extents_ } };
		}

		constexpr typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		constexpr typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		constexpr typename B::reference operator[](typename B::size_type index) const noexcept {
			assert(index < this->size_);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::reference at(typename B::size_type index) const noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::iterator begin() const noexcept { return multidim::iterator<T>(data(), this->extents_, 0); }
		constexpr typename B::iterator end() const noexcept { return multidim::iterator<T>(data_offset(this->size_), this->extents_, this->size_); }
		constexpr typename B::reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::reference front() const noexcept { return operator[](0); }
		constexpr typename B::reference back() const noexcept { return operator[](this->size_ - 1); }

		constexpr void fill(typename B::const_reference value) noexcept(std::is_nothrow_assignable_v<typename B::reference, typename B::const_reference>) {
			for (typename B::reference x : *this) {
				x = value;
			}
		}

		/**
		 * Compares if two dynarray_refs are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const dynarray_ref& a, const dynarray_const_ref<T>& b) {
			return static_cast<dynarray_const_ref<T>>(a) == b;
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const dynarray_ref& a, const dynarray_const_ref<T>& b) { return !(a == b); };

		constexpr operator dynarray_const_ref<T>() noexcept { return dynarray_const_ref<T>{ this->data_, typename B::container_extents_type{ this->size_, this->extents_ } }; }

		/**
		 * Rebinds this reference to another dynarray_ref.
		 */
		constexpr inline void rebind(const dynarray_ref& other) noexcept {
			this->data_ = other.data_;
			this->size_ = other.size_;
			this->extents_ = other.extents_;
		}
		/**
		 * Rebinds this reference to another object.
		 */
		constexpr inline void rebind(typename B::base_element* data) noexcept { this->data_ = data; }
		/**
		 * Rebinds this reference to another object that is n objects away from the current object.
		 */
		constexpr inline void rebind_relative(typename B::difference_type n) noexcept { this->data_ += n * this->size_ * this->extents_.stride(); }
	};

	template <typename T>
	class dynarray_const_ref : public dynarray_base<dynarray_const_ref<T>, T, false, true> {
	private:
		using B = dynarray_base<dynarray_const_ref<T>, T, false, true>;
		static_assert(B::container_extents_type::is_dynamic, "extents_type must be dynamic");
		friend class dynarray_ref<T>;
	public:
		/**
		 * Default-constructed dynarray_const_ref.
		 * This should not be used for anything apart from reassignment, but is provided for convenience of some algorithms.
		 * Two value-initialised instances are guaranteed to compare equal with operator==.
		 */
		constexpr dynarray_const_ref() noexcept = default;
		constexpr dynarray_const_ref(const dynarray_const_ref& other) noexcept : B(other.size_, other.extents_, other.data_) {}
		//constexpr array_const_ref(array_const_ref&&) = default;
		constexpr dynarray_const_ref(const typename B::base_element* data, const typename B::container_extents_type& extents) noexcept : B(extents.top_extent(), extents.inner(), data) {}

		constexpr const typename B::base_element* data() const noexcept { return to_pointer(this->data_); }
	private:
		friend B;
		constexpr const typename B::base_element* data_offset(typename B::size_type index) const noexcept { return to_pointer(this->data_) + index * this->extents_.stride(); }
	public:
		constexpr typename B::const_reference operator[](typename B::size_type index) const noexcept {
			assert(index < this->size_);
			if constexpr (element_traits<T>::is_inner_container) {
				return typename B::const_reference{ data_offset(index), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<typename B::element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return data()[index];
			}
		}
		constexpr typename B::const_reference at(typename B::size_type index) const noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr typename B::const_iterator begin() const noexcept { return this->cbegin(); }
		constexpr typename B::const_iterator end() const noexcept { return this->cend(); }
		constexpr typename B::const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr typename B::const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }

		constexpr typename B::const_reference front() const noexcept { return operator[](0); }
		constexpr typename B::const_reference back() const noexcept { return operator[](this->size_ - 1); }


		/**
		 * Compares if two dynarray_const_refs are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const dynarray_const_ref& a, const dynarray_const_ref& b) {
			return a.size_ == b.size_ && a.extents_ == b.extents_ && std::equal(a.data(), a.data_offset(a.size_), b.data());
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const dynarray_const_ref& a, const dynarray_const_ref& b) { return !(a == b); };

		/**
		 * Rebinds this reference to another dynarray_const_ref.
		 */
		constexpr inline void rebind(const dynarray_const_ref& other) noexcept {
			this->data_ = other.data_;
			this->size_ = other.size_;
			this->extents_ = other.extents_;
		}
		/**
		 * Rebinds this reference to another object.
		 */
		constexpr inline void rebind(const typename B::base_element* data) noexcept { this->data_ = data; }
		/**
		 * Rebinds this reference to another object that is n objects away from the current object.
		 */
		constexpr inline void rebind_relative(typename B::difference_type n) noexcept { this->data_ += n * this->size_ * this->extents_.stride(); }
	};
}
