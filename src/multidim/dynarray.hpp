#pragma once

#include "core.hpp"

namespace multidim {
	template <typename E>
	class dynamic_extent {
	public:
		constexpr size_t stride() const noexcept {
			return size_ * element_extent_.stride();
		}
		constexpr size_t top_extent() const noexcept {
			return size_;
		}
		constexpr const E& inner() const noexcept {
			return element_extent_;
		}
		constexpr static bool is_dynamic = true;
		/**
		 * Constructs a dynamic_extent with a size of n.  Additional parameters are forwarded to the inner extents.
		 */
		template <typename TN, typename... TNs>
		constexpr explicit dynamic_extent(TN n, TNs... ns) noexcept : size_(n), element_extent_(ns...) {}
		/**
		 * Constructs a dynamic_extent with a size of zero.  All inner extents will also be default constructed.
		 */
		constexpr explicit dynamic_extent() noexcept : size_(0), element_extent_() {}
		friend bool operator==(const dynamic_extent& a, const dynamic_extent& b) noexcept { return a.size_ == b.size_ && a.element_extent_ == b.element_extent_; }
		friend bool operator!=(const dynamic_extent& a, const dynamic_extent& b) noexcept { return !(a == b); }

	private:
		size_t size_;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
		[[no_unique_address]] E element_extent_;
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	};
}
