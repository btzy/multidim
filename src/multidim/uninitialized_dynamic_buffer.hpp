#pragma once

#include <type_traits>

#include "dynamic_buffer.hpp"

namespace multidim {
	/**
	 * A dynamic_buffer that does not construct/destruct its elements.
	 */
	template <typename T>
	class uninitialized_dynamic_buffer {
	public:
		using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

		constexpr T* data() noexcept { return reinterpret_cast<T*>(buf_.data()); }
		constexpr const T* data() const noexcept { return reinterpret_cast<const T*>(buf_.data()); }
		constexpr uninitialized_dynamic_buffer() = default;
		constexpr uninitialized_dynamic_buffer(size_t sz) noexcept : buf_(std::make_unique<storage[]>(sz)) {}
		constexpr uninitialized_dynamic_buffer(const uninitialized_dynamic_buffer&) noexcept = delete;
		constexpr uninitialized_dynamic_buffer(uninitialized_dynamic_buffer&&) noexcept = default;
		constexpr uninitialized_dynamic_buffer& operator=(const uninitialized_dynamic_buffer&) noexcept = delete;
		constexpr uninitialized_dynamic_buffer& operator=(uninitialized_dynamic_buffer&&) noexcept = default;
		
		friend void swap(uninitialized_dynamic_buffer& a, uninitialized_dynamic_buffer& b) noexcept {
			using std::swap;
			swap(a.buf_, b.buf_);
		}
	private:
		multidim::dynamic_buffer<storage> buf_;
	};
}
