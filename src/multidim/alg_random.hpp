#pragma once

#include <random>

#include "alg_modify.hpp" // for multidim::iter_swap()

namespace multidim {

    template <typename RandomIt, typename URBG>
    constexpr inline void shuffle(RandomIt first, RandomIt last, URBG&& g) {
        using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
        using dist_t = std::uniform_int_distribution<diff_t>;
        using param_t = typename dist_t::param_type;
        for (diff_t i = last - first - 1; i > 0; --i) {
            multidim::iter_swap(first + i, first + dist_t()(g, param_t(0, i)));
        }
    }

    /**
     * Does reservior sampling using Algorithm R.
     * Requires PopulationIterator to satisfy LegacyInputIterator, and SampleIterator to satisfy LegacyRandomAccessIterator.
     */
    template <typename PopulationIterator, typename SampleIterator, typename Distance, typename URBG>
    constexpr inline SampleIterator reservior_sample_r(PopulationIterator first, PopulationIterator last, SampleIterator out, Distance n, URBG&& g) {
        using diff_t = typename std::iterator_traits<PopulationIterator>::difference_type;
        using dist_t = std::uniform_int_distribution<diff_t>;
        using param_t = typename dist_t::param_type;
        SampleIterator out_end = out;
        diff_t sz = static_cast<diff_t>(n);
        for (; n > 0; --n, ++first, ++out_end) {
            if (first == last) return out_end;
            *out_end = *first;
        }
        assert(out_end - out == n);
        diff_t curr = sz;
        for (; first != last; ++first) {
            const diff_t index = dist_t()(g, param_t(0, curr++));
            if (index < sz) out[index] = *first;
        }
        return out_end;
    }
    /**
     * Does selection sampling.
     * Requires PopulationIterator to satisfy LegacyForwardIterator, and SampleIterator to satisfy LegacyInputIterator.
     */
    template <typename PopulationIterator, typename SampleIterator, typename Distance, typename URBG>
    constexpr inline SampleIterator selection_sample(PopulationIterator first, PopulationIterator last, SampleIterator out, Distance n, URBG&& g) {
        using diff_t = typename std::iterator_traits<PopulationIterator>::difference_type;
        using dist_t = std::uniform_int_distribution<diff_t>;
        using param_t = typename dist_t::param_type;
        diff_t pop_left = std::distance(first, last);
        for (; pop_left > 0; ++first) {
            const diff_t tmp = dist_t()(g, param_t(0, --pop_left));
            if (tmp < n) {
                *out++ = *first;
                --n;
            }
        }
        return out;
    }
    /**
     * Samples some numbers from a given range.
     * If PopulationIterator satisfies LegacyForwardIterator then this algorithm uses selection sampling, which is stable (i.e. relative order of selected elements is preserved).
     * Otherwise, this algorithm uses reservior sampling (Algorithm R), which is not stable.
     */
    template <typename PopulationIterator, typename SampleIterator, typename Distance, typename URBG>
    constexpr inline SampleIterator selection_sample(PopulationIterator first, PopulationIterator last, SampleIterator out, Distance n, URBG&& g) {
        if constexpr (std::is_base_of_v<std::forward_iterator_tag, std::iterator_traits<PopulationIterator>::iterator_category>) {
            multidim::selection_sample(std::move(first), std::move(last), std::move(out), std::move(n), std::forward<URBG>(g));
        }
        else {
            multidim::reservior_sample_r(std::move(first), std::move(last), std::move(out), std::move(n), std::forward<URBG>(g));
        }
    }

}