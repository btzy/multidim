#pragma once

#include <array>

namespace multidim {
	/**
	 * Class that represents a buffer whose size is known at compilation time, like a std::array.
	 * The buffer is stored on the stack.
	 * This class is a simple RAII class that owns its buffer.
	 */
	template <typename T, size_t N>
	class fixed_buffer {
	public:
		constexpr T* data() noexcept { return buf_.data(); }
		constexpr const T* data() const noexcept { return buf_.data(); }
		constexpr fixed_buffer() = default;
		constexpr fixed_buffer(size_t sz) : fixed_buffer() { assert(sz == N); (void)sz; }
		constexpr fixed_buffer(const std::array<T, N>& buf) noexcept : buf_(buf) {}
		constexpr fixed_buffer(const fixed_buffer&) noexcept = delete;
		constexpr fixed_buffer(fixed_buffer&&) noexcept = default;
		constexpr fixed_buffer& operator=(const fixed_buffer&) noexcept = delete;
		constexpr fixed_buffer& operator=(fixed_buffer&&) noexcept = default;
		/**
		 * Creates a new buffer with a copy of the data from the current one.  The parameter is ignored.
		 */
		constexpr fixed_buffer clone(size_t) const noexcept(std::is_nothrow_copy_constructible_v<T>) { return fixed_buffer(buf_); }
		friend void swap(fixed_buffer& a, fixed_buffer& b) noexcept(std::is_nothrow_swappable_v<T>) {
			using std::swap;
			swap(a.buf_, b.buf_);
		}
	private:
		std::array<T, N> buf_;
	};
}
