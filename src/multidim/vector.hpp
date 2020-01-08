#pragma once

#include <iterator> // for std::reverse_iterator and iterator_category
#include <memory> // for std::forward() and uninitialized_copy_n() et al
#include <type_traits>

#include "dynarray.hpp"
#include "uninitialized_dynamic_buffer.hpp"
#include "core.hpp"
#include "iterator.hpp"
#include "memory.hpp"

namespace multidim {

	/**
	 * Represents a multidimensional array whose outermost dimension is a growable vector.
	 * @tparam T the element type; if this is the innermost dimension then T is the base element type, otherwise T is an inner container (i.e. something that extends from enable_inner_container)
	 */
	template <typename T>
	class vector {
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
		using buffer_type = uninitialized_dynamic_buffer<base_element>;


		constexpr vector(const vector& other) : data_(other.size_* other.extents_.stride()), size_(other.size_), capacity_(other.size_), extents_(other.extents_) {
			std::uninitialized_copy_n(other.data_.data(), other.size_ * other.extents_.stride(), data_.data());
		}
		constexpr vector(vector&& other) noexcept(std::is_nothrow_move_constructible_v<buffer_type>) : data_(std::move(other.data_)), size_(other.size_), capacity_(other.capacity_), extents_(other.extents_) {
			other.size_ = 0;
			other.capacity_ = 0;
			//other.extents_ = element_extents_type(); don't actually need to do this, since size() is zero already.
		}
		/**
		 * Constructs an dynarray from the given size (current dimension) and element_extents_type (inner dimensions).  This should not generally be used directly.
		 */
		constexpr explicit vector(const element_extents_type& extents) noexcept : size_(0), capacity_(0), extents_(extents) {}
		/**
		 * Constructs an vector from the given dimensions.
		 * Note: Dimensions are only specified for dynarray layers.  Vectors and compile-time fixed arrays do not need a dimension parameter.
		 */
		template <typename... TNs, typename = std::enable_if_t<std::conjunction_v<std::is_convertible<size_t, TNs>...>>>
		constexpr explicit vector(TNs... ns) noexcept : vector(element_extents_type(ns...)) {}
		constexpr vector& operator=(const vector& other) {
			clear();
			if (extents_ == other.extents_) {
				if (capacity_ >= other.size_) {
					// our existing data_ has enough space
					size_ = other.size_;
					std::uninitialized_copy_n(other.data_.data(), size_ * extents_.stride(), data_.data()); // construct/copy the new stuff
					return *this;
				}
			}
			else {
				extents_ = other.extents_;
			}
			size_ = other.size_;
			capacity_ = other.size_;
			data_ = buffer_type(size_ * extents_.stride());
			std::uninitialized_copy_n(other.data_.data(), size_ * extents_.stride(), data_.data()); // construct/copy the new stuff
			return *this;
		}
		constexpr vector& operator=(vector&& other) noexcept(std::is_nothrow_move_assignable_v<buffer_type>) {
			clear();
			size_ = other.size_;
			capacity_ = other.capacity_;
			extents_ = other.extents_;
			data_ = std::move(other.data_); // will reset other.data_
			other.size_ = 0;
			other.capacity_ = 0;
			// other.extents_ = element_extents_type();
			return *this;
		};

#if defined(__cpp_lib_constexpr_dynamic_alloc) && __cpp_lib_constexpr_dynamic_alloc >= 201907
		constexpr ~vector() {
#else
		~vector() {
#endif
			std::destroy_n(data_.data(), size_ * extents_.stride()); // destroy existing data
		}

		/**
		 * Swaps two vectors.  This will invalidate references to both vectors (in practice, if the element extents are the same, any existing references for one vector will now refer to something in the other vector).
		 */
		friend constexpr void swap(vector& a, vector& b) noexcept(std::is_nothrow_swappable_v<buffer_type>) {
			a.swap(b);
		}
		/**
		 * Swaps this vector with another one.  This will invalidate references to both arrays (in practice, if the element extents are the same, any existing references for one vector will now refer to something in the other vector).
		 */
		constexpr void swap(vector& other) noexcept(std::is_nothrow_swappable_v<buffer_type>) {
			using std::swap;
			swap(data_, other.data_);
			swap(size_, other.size_);
			swap(capacity_, other.capacity_);
			swap(extents_, other.extents_);
		}

		/**
		 * Assign to this container some data starting from the given iterator.  Behaviour is undefined if the iterator points to an inner container that does not have matching extents.
		 */
		template <typename InputIt>
		constexpr std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>> assign(InputIt first, InputIt last) {
			static_assert(std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, "SFINAE bug");
			const size_type dist = std::distance(first, last);
			clear();
			size_ = dist;
			if (capacity_ >= dist) {
				// our existing data_ has enough space
			}
			else {
				data_ = buffer_type(dist * extents_.stride());
				capacity_ = dist;
			}
			multidim::uninitialized_copy(first, last, begin()); // construct/copy the new stuff
		}
		template <typename InputIt>
		constexpr std::enable_if_t<!std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>> assign(InputIt first, InputIt last) {
			static_assert(!std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, "SFINAE bug");
			clear();
			for (; first != last; ++first) {
				emplace_back(*first);
			}
		}
		constexpr void assign(std::initializer_list<std::decay_t<const_reference>> ilist) {
			assign(ilist.begin(), ilist.end());
		}
		constexpr void assign(size_type count, const_reference value) {
			clear();
			size_ = count;
			if (capacity_ >= count) {
				// our existing data_ has enough space
			}
			else {
				data_ = buffer_type(count * extents_.stride());
				capacity_ = count;
			}
			multidim::uninitialized_fill_n(begin(), count, value); // construct/copy the new stuff
		}


		/**
		 * Converting operator to dynarray_ref.
		 */
		constexpr operator dynarray_ref<T>() noexcept {
			return dynarray_ref<T>{ this->data_.data(), container_extents_type{ this->size_, this->extents_ } };
		}
		/**
		 * Converting operator to dynarray_const_ref.
		 */
		constexpr operator dynarray_const_ref<T>() const noexcept {
			return dynarray_const_ref<T>{ this->data_.data(), container_extents_type{ this->size_, this->extents_ } };
		}

		/**
		 * Gets a pointer to the underlying base elements.
		 */
		constexpr base_element* data() noexcept { return multidim::to_pointer(this->data_); }
		constexpr const base_element* data() const noexcept { return multidim::to_pointer(this->data_); }
	private:
		/**
		 * Gets a pointer to the underlying base elements, offsetted by some index.  It is undefined behaviour if index > size().  If index == size() then this function is value, but the returned pointer may not be dereferenced.
		 */
		constexpr base_element* data_offset(size_type index) noexcept { return multidim::to_pointer(this->data_) + index * this->extents_.stride(); }
		constexpr const base_element* data_offset(size_type index) const noexcept { return multidim::to_pointer(this->data_) + index * this->extents_.stride(); }

		/**
		 * Gets a reference to the element at the specified index.  It is undefined behaviour if index >= size().
		 * This takes in an additional base parameter, so that push_back() et al can access uninitialized elements in the new buffer.
		 */
		constexpr reference get_element(base_element* base, size_type index) noexcept {
			if constexpr (element_traits<T>::is_inner_container) {
				return reference{ base + index * extents_.stride(), extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return base[index];
			}
		}
		constexpr const_reference get_element(base_element* base, size_type index) const noexcept {
			assert(index < this->size_);
			if constexpr (element_traits<T>::is_inner_container) {
				return const_reference{ base + index * extents_.stride(), this->extents_ };
			}
			else {
				static_assert(std::is_same_v<element_extents_type, unit_extent>, "extents_type must be unit_extent if there is no inner container");
				return base[index];
			}
		}
	public:
		/**
		 * Gets a reference to the element at the specified index.  It is undefined behaviour if index >= size().
		 */
		constexpr reference operator[](size_type index) noexcept {
			assert(index < this->size_);
			return get_element(data(), index);
		}
		constexpr const_reference operator[](size_type index) const noexcept {
			assert(index < this->size_);
			return get_element(data(), index);
		}
		constexpr reference at(size_type index) noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }
		constexpr const_reference at(size_type index) const noexcept { if (index >= this->size_) throw std::out_of_range("element access index out of range"); else return operator[](index); }

		constexpr size_type size() const noexcept { return size_; }
		constexpr size_type max_size() const noexcept { return size_; }
		[[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
		constexpr size_type capacity() const noexcept { return capacity_; }

		constexpr const_iterator cbegin() const noexcept { return multidim::const_iterator<T>(data(), extents_, 0); }
		constexpr const_iterator cend() const noexcept { return multidim::const_iterator<T>(data_offset(size_), extents_, size_); }
		constexpr const_iterator crbegin() const noexcept { return std::make_reverse_iterator(cend()); }
		constexpr const_iterator crend() const noexcept { return std::make_reverse_iterator(cbegin()); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator begin() noexcept { return multidim::iterator<T>(data(), extents_, 0); }
		constexpr const_iterator end() const noexcept { return cend(); }
		constexpr iterator end() noexcept { return multidim::iterator<T>(data_offset(size_), extents_, size_); }
		constexpr const_reverse_iterator rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		constexpr reverse_iterator rbegin() noexcept { return std::make_reverse_iterator(end()); }
		constexpr const_reverse_iterator rend() const noexcept { return std::make_reverse_iterator(begin()); }
		constexpr reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }

		constexpr reference front() noexcept { return operator[](0); }
		constexpr const_reference front() const noexcept { return operator[](0); }
		constexpr reference back() noexcept { return operator[](size_ - 1); }
		constexpr const_reference back() const noexcept { return operator[](size_ - 1); }



		constexpr void reserve(size_type new_cap) {
			if (new_cap <= capacity_) return;
			buffer_type tmp_buf(new_cap * extents_.stride());
			multidim::uninitialized_move_if_noexcept(data(), data_offset(size_), tmp_buf.data());
			std::destroy(data(), data_offset(size_)); // destroy existing data
			data_ = std::move(tmp_buf);
			capacity_ = new_cap;
		}

		constexpr void clear() noexcept {
			std::destroy(data(), data_offset(size_)); // destroy existing data
			size_ = 0;
		}

	private:
		/**
		 * Creates and returns a new buffer of at least the desired capacity, but also at least twice of the original capacity (in order to provide amortized guarantees).
		 */
		constexpr buffer_type create_new_buffer_amortized(size_type min_capacity, size_type& out_capacity) {
			const size_type new_capacity = std::max(min_capacity, capacity_ * 2);
			buffer_type new_buffer(new_capacity); // might throw std::bad_alloc()
			out_capacity = new_capacity; // assign the new capacity after allocating the buffer, in order to provide strong exception guarantee
			return new_buffer; // implicit move
		}

	public:
		constexpr void push_back(const_reference value) noexcept {
			const size_type new_size = size_ + 1;
			if (new_size <= capacity_) {
				// enough space
				multidim::uninitialized_copy_at(value, get_element(data_.data(), size_));
				size_ = new_size;
			}
			else {
				// reserve space and update capacity
				buffer_type tmp_buf = create_new_buffer_amortized(new_size, capacity_);
				// copy the new element
				multidim::uninitialized_copy_at(value, get_element(tmp_buf.data(), size_));
				// copy/move the existing elements
				multidim::uninitialized_move_if_noexcept(data(), data_offset(size_), tmp_buf.data());
				std::destroy(data(), data_offset(size_));
				// swap the buffer in
				data_ = std::move(tmp_buf);
				// save the new size
				size_ = new_size;
			}
		}
		template <typename... Args>
		constexpr void emplace_back(Args&&... args) noexcept {
			static_assert(!element_traits<T>::is_inner_container, "emplace_back() only allowed for deepest level container");
			const size_type new_size = size_ + 1;
			if (new_size <= capacity_) {
				// enough space
				::new (static_cast<void*>(data_offset(size_))) value_type(std::forward<Args>(args)...);
				size_ = new_size;
			}
			else {
				// reserve space and update capacity
				buffer_type tmp_buf = create_new_buffer_amortized(new_size, capacity_);
				// copy the new element
				static_assert(extents_.stride() == 1);
				::new (static_cast<void*>(tmp_buf.data() + size_)) value_type(std::forward<Args>(args)...);
				// copy/move the existing elements
				multidim::uninitialized_move_if_noexcept(data(), data_offset(size_), tmp_buf.data());
				std::destroy(data(), data_offset(size_));
				// swap the buffer in
				data_ = std::move(tmp_buf);
				// save the new size
				size_ = new_size;
			}
		}

		void pop_back() noexcept {
			--size_;
			multidim::destroy_at(get_element(data_.data(), size_));
		}


		/**
		 * Gets the extents of elements that are stored in this vector.
		 */
		constexpr const element_extents_type& extents() const noexcept { return extents_; }


		/**
		 * Compares if two vectors are elementwise equal.  If they have different shape or different number of elements, then it will also return false.
		 */
		friend constexpr MULTIDIM_FORCEINLINE bool operator==(const vector& a, const vector& b) {
			return a.size_ == b.size_ && a.extents_ == b.extents_ && std::equal(a.data(), a.data_offset(a.size_), b.data());
		}
		friend constexpr MULTIDIM_FORCEINLINE bool operator!=(const vector& a, const vector& b) { return !(a == b); };
		// Note: We don't provide lexicographical comparison because it isn't clear what it means to compare arrays of different shape.

	private:

		buffer_type data_;
		size_t size_; // the size of the current dimension
		size_t capacity_; // the capacity of the current dimension
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

}
