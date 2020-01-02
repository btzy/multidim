#pragma once

#include <array>

namespace multidim {
	template <typename T, size_t N>
	class fixed_buffer {
	public:
		constexpr T* data() noexcept { return buf_.data(); }
		constexpr const T* data() const noexcept { return buf_.data(); }
		constexpr fixed_buffer() = default;
		constexpr fixed_buffer(const std::array<T, N>& buf) noexcept : buf_(buf) {}
		constexpr fixed_buffer(const fixed_buffer&) noexcept = delete;
		constexpr fixed_buffer(fixed_buffer&&) noexcept = default;
		constexpr fixed_buffer& operator=(const fixed_buffer&) noexcept = delete;
		constexpr fixed_buffer& operator=(fixed_buffer&&) noexcept = default;
		constexpr fixed_buffer clone(size_t) const noexcept { return fixed_buffer(buf_); }
		friend void swap(fixed_buffer& a, fixed_buffer& b) noexcept {
			using std::swap;
			swap(a.buf_, b.buf_);
		}
	private:
		std::array<T, N> buf_;
	};
}
