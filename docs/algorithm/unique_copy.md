# multidim::unique_copy


```cpp
template <typename InputIt, typename OutputIt>
constexpr inline OutputIt unique_copy(InputIt first, InputIt last, OutputIt d_first);
```
```cpp
template <typename InputIt, typename OutputIt, typename BinaryPredicate>
constexpr inline OutputIt unique_copy(InputIt first, InputIt last, OutputIt d_first, BinaryPredicate p);
```

These functions are similar to `std::unique_copy`, but they are guaranteed not to construct any temporary elements.  Because of this, `multidim::unique_copy` has the following requirements:
- `InputIt` must meet the requirements of LegacyInputIterator
- `OutputIt` must meet the requirements of LegacyOutputIterator
- `*std::declval<OutputIt>() = *std::declval<InputIt>()` must be valid
- If `InputIt` does not meet the requirements of LegacyForwardIterator, then `OutputIt` must meet the requirements of LegacyForwardIterator and `*std::declval<OutputIt>() == *std::declval<InputIt>()` (or `p(*std::declval<OutputIt>(), *std::declval<InputIt>())` for predicate version) must be valid and convertible to `bool` <br/> Otherwise `*std::declval<InputIt>() == *std::declval<InputIt>()` (or `p(*std::declval<InputIt>(), *std::declval<InputIt>())` for predicate version) must be valid and convertible to `bool`
