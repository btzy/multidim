#pragma once

#include "core.hpp"

namespace multidim {

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
