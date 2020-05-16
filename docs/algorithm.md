# Multidim algorithms library

Multidim provides equivalent algorithms for most of the algorithms in the [C++ algorithms library](https://en.cppreference.com/w/cpp/algorithm) (`<algorithm>` and `<numeric>`).

Because of the 'fake' references (e.g. `array_ref` and `dynarray_ref`) used in Multidim, many of the functions provided in the C++ algorithms library may not work with Multidim's iterators.  Hence, Multidim provides its own algorithms that mimic those in the standard.  See [this page](TODO) for the differences between Multidim's iterators and a [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator).

Unlike many functions in the C++ algorithms library, functions in Multidim's algorithms library have the following relaxation in requirements:
- `std::iterator_traits<Iter>::reference` need not be a real reference
- `std::iterator_traits<Iter>::value_type` is never used; this means that:
  - no Multidim algorithm demands a functor that takes in a `std::iterator_traits<Iter>::value_type &` or `const std::iterator_traits<Iter>::value_type &`
  - no Multidim algorithm constructs temporary elements (i.e. all modifying operations may only do move/copy assignment or swaps)

As such, whenever an algorithm in `std::` may be used, the equivalent algorithm in `multidim::` (if it exists) will most likely work too.  However, there are a few `std::` algorithms that do not have an equivalent, or require stronger iterator category requirements, in `multidim::` because Multidim algorithms must never construct temporary elements.

Note: The Multidim algorithms library does not provide overloads for execution policies.

For the algorithms listed below, if the remarks column contains "Equivalent", it means that code that use Multidim's references with the `std::` algorithm will probably work (even though it may be undefined behaviour).

### Non-modifying sequence operations

| Standard Algorithm | Multidim Algorithm | Remarks |
| ----- | ----- | ----- |
| `std::all_of` <br/> `std::any_of` <br/> `std::none_of` | `multidim::all_of` <br/> `multidim::any_of` <br/> `multidim::none_of` | Equivalent |
| `std::for_each` <br/> `std::for_each_n` | `multidim::for_each` <br/> `multidim::for_each_n` | Equivalent |
| - | [`multidim::for_eachs` <br/> `multidim::for_eachs_n`](algorithm/for_eachs) | Like `for_each`/`for_each_n`, but allows multiple sequences to be iterated together |
| `std::count_if` <br/> `std::count_if` | `multidim::count_if` <br/> `multidim::count_if` | Equivalent |
| `std::mismatch` | `multidim::mismatch` | Equivalent |
| `std::find` <br/> `std::find_if` <br/> `std::find_if_not` | `multidim::find` <br/> `multidim::find_if` <br/> `multidim::find_if_not` | Equivalent |
| `std::find_end` | `multidim::find_end` | Equivalent |
| `std::find_first_of` | `multidim::find_first_of` | Equivalent |
| `std::adjacent_find` | `multidim::adjacent_find` | Equivalent |
| `std::search` | `multidim::search` | Equivalent, but Multidim does not provide the `Searcher` overload |
| `std::search_n` | `multidim::search_n` | Equivalent |
| - | [`multidim::find_consecutive` <br/> `multidim::find_consecutive_if`](algorithm/find_consecutive) | Finds a range of consecutive elements satisfying specific criteria; similar to `multidim::search_n` |

### Modifying sequence operations

| Standard Algorithm | Multidim Algorithm | Remarks |
| ----- | ----- | ----- |
| `std::copy` <br/> `std::copy_if` <br/> `std::copy_n` <br/> `std::copy_backward` | `multidim::copy` <br/> `multidim::copy_if` <br/> `multidim::copy_n` <br/> `multidim::copy_backward` | Equivalent |
| `std::move` <br/> `std::move_backward` | `multidim::move` <br/> `multidim::move_if` <br/> `multidim::move_n` <br/> `multidim::move_backward` | Equivalent, but Multidim provides the extra versions `move_if` and `move_n` that are like `copy_if` and `copy_n` but moves elements |
| `std::fill` <br/> `std::fill_n` | `multidim::fill` <br/> `multidim::fill_n` | Equivalent |
| `std::transform`| - | Not provided by Multidim as the unary operation returns by value, which would construct a temporary; use `multidim::for_eachs` instead |
| `std::generate` <br/> `std::generate_n` | - | Not provided by Multidim as the generator returns by value, which would construct a temporary |
| `std::remove` <br/> `std::remove_if` | `multidim::remove` <br/> `multidim::remove_if` | Equivalent |
| `std::remove_copy` <br/> `std::remove_copy_if` | `multidim::remove_copy` <br/> `multidim::remove_copy_if` | Equivalent |
| `std::replace` <br/> `std::replace_if` | `multidim::replace` <br/> `multidim::replace_if` | Equivalent |
| `std::replace_copy` <br/> `std::replace_copy_if` | `multidim::replace_copy` <br/> `multidim::replace_copy_if` | Equivalent |
| `std::swap` | - | Use argument dependent lookup (ADL) to swap instead |
| `std::swap_ranges` | `multidim::swap_ranges` | Equivalent |
| `std::iter_swap` | `multidim::iter_swap` | Equivalent |
| `std::reverse` <br/> `std::reverse_copy` | `multidim::reverse` <br/> `multidim::reverse_copy` | Equivalent |
| `std::rotate` <br/> `std::rotate_copy` | `multidim::rotate` <br/> `multidim::rotate_copy` | Equivalent |
| `std::shift_left` <br/> `std::shift_right` | `multidim::shift_left` <br/> `multidim::shift_right` | Equivalent |
| `std::unique` | `multidim::unique` | Equivalent |
| `std::unique_copy` | [`multidim::unique_copy`](algorithm/unique_copy) | Multidim requires either InputIt or OutputIt to satisfy LegacyForwardIterator |

### Randomizing sequence operations

| Standard Algorithm | Multidim Algorithm | Remarks |
| ----- | ----- | ----- |
| `std::shuffle` | `multidim::shuffle` | Equivalent |
| `std::sample` | `multidim::sample` | Equivalent |

### Partitioning operations

| Standard Algorithm | Multidim Algorithm | Remarks |
| ----- | ----- | ----- |
| `std::is_partitioned` | `multidim::is_partitioned` | Equivalent |
| `std::partition` | `multidim::partition` | Equivalent |
| `std::partition_copy` | `multidim::partition_copy` | Equivalent |
| `std::stable_partition` | `multidim::stable_partition` | The O(N) algorithm is not provided by Multidim because it allocates additional memory; an O(N log N) algorithm that does not allocate memory is used instead, and it only requires LegacyForwardIterator but not LegacyBidirectionalIterator |
| `std::partition_point` | `multidim::partition_point` | Equivalent |
