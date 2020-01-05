# Multidim algorithms library

Multidim provides equivalent algorithms for most of the algorithms in the [C++ algorithms library](https://en.cppreference.com/w/cpp/algorithm) (`<algorithm>` and `<numeric>`).

Because of the 'fake' references (e.g. `array_ref` and `dynarray_ref`) used in Multidim, many of the functions provided in the C++ algorithms library may not work with Multidim's iterators.  Hence, Multidim provides its own algorithms that mimic those in the standard.  See [this page](/) for the differences between Multidim's iterators and a [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator).

Unlike many functions in the C++ algorithms library, functions in Multidim's algorithms library have the following relaxation in requirements:
- `std::iterator_traits<Iter>::reference` need not be a real reference
- `std::iterator_traits<Iter>::value_type` is never used; this means that:
  - no Multidim algorithm demands a functor that takes in a `std::iterator_traits<Iter>::value_type &` or `const std::iterator_traits<Iter>::value_type &`
  - no Multidim algorithm constructs temporary elements (i.e. all modifying operations may only do move/copy assignment or swaps)

Note that the above is a relaxation of requirements &mdash; whenever an algorithm in `std::` may be used, the equivalent algorithm in `multidim::` (if it exists) will work too.

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
