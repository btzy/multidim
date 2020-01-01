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
		template <typename TN, typename... TNs>
		constexpr explicit dynamic_extent(TN n, TNs... ns) noexcept : size_(n), element_extent_(ns...) {}
	private:
		size_t size_;
		[[no_unique_address]] E element_extent_;
	};
}
