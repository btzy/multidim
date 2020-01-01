#pragma once

#include <array>

namespace multidim {
	template <typename T, size_t N>
	using fixed_buffer = std::array<T, N>;
}
