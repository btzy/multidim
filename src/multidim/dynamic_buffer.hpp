#pragma once

#include <algorithm>
#include <memory>

namespace multidim {
	/**
	 * Class that represents a buffer whose size is known at construction time, like a std::unique_ptr<T[]>.
	 * The buffer is located on the heap.
	 * This class is a simple RAII class that owns its buffer, and will free the memory when it is destructed.  However, for efficiency, it does not know the size of its own buffer.
	 */
	template <typename T>
	class dynamic_buffer {
	public:
		constexpr T* data() noexcept { return buf_.get(); }
		constexpr const T* data() const noexcept { return buf_.get(); }
		constexpr dynamic_buffer() = default;
		constexpr dynamic_buffer(size_t sz) noexcept : buf_(std::make_unique<T[]>(sz)) {}
		constexpr dynamic_buffer(std::unique_ptr<T[]>&& ptr) noexcept : buf_(std::move(ptr)) {}
		constexpr dynamic_buffer(const dynamic_buffer&) noexcept = delete;
		constexpr dynamic_buffer(dynamic_buffer&&) noexcept = default;
		constexpr dynamic_buffer& operator=(const dynamic_buffer&) noexcept = delete;
		constexpr dynamic_buffer& operator=(dynamic_buffer&&) noexcept = default;
		/**
		 * Creates a new buffer with a copy of the data from the current one.
		 * Note: If the size of the current buffer is specified incorrectly, then behaviour is undefined.
		 * @param sz the size of the current buffer
		 */
		constexpr dynamic_buffer clone(size_t sz) const {
			std::unique_ptr<T[]> tmp = std::make_unique<T[]>(sz);
			std::copy_n(data(), sz, tmp.get());
			return dynamic_buffer(std::move(tmp));
		}
		friend void swap(dynamic_buffer& a, dynamic_buffer& b) noexcept {
			using std::swap;
			swap(a.buf_, b.buf_);
		}
	private:
		std::unique_ptr<T[]> buf_;
	};
}
