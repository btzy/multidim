# multidim::find_consecutive / multidim::find_consecutive_if


```cpp
template <typename ForwardIt, typename Size, typename T>
constexpr inline ForwardIt find_consecutive(ForwardIt first, ForwardIt last, Size count, const T& val);
```
```cpp
template <typename ForwardIt, typename Size, typename UnaryPredicate>
constexpr inline ForwardIt find_consecutive_if(ForwardIt first, ForwardIt last, Size count, UnaryPredicate p);
```

`multidim::find_consecutive` is equivalent to `multidim::search_n` (`operator==` version).

`multidim::find_consecutive_if` takes a unary predicate instead of using `operator==` to compare to `val`.  It is similar to `std::search_n` (binary predicate version), but `multidim::find_consecutive_if` uses a unary predicate instead.
