# multidim::for_eachs / multidim::for_eachs_n


```cpp
template <typename Function, typename InputIt, typename... InputIts>
constexpr inline Function for_eachs(Function f, InputIt first, InputIt last, InputIts... firsts);
```
```cpp
template <typename Function, typename InputIt, typename Size, typename... InputIts>
constexpr inline InputIt for_eachs_n(Function f, InputIt first, Size n, InputIts... firsts);
```

These functions are similar to `multidim::for_each` and `multidim::for_each_n`, but allow sequences to be iterated together.  For `for_eachs`, `std::distance(first, last)` determines the number of applications of `f`; for `for_eachs_n`, `n` determines the number of applications of `f`.

`f` - function object, which must be callable as `f(*it, (*its)...)`, where `it` has type `InputIt` and `its...` has type `InputIts...`.

### Possible implementation

```cpp
	template <typename Function, typename InputIt, typename... InputIts>
	constexpr inline Function for_eachs(Function f, InputIt first, InputIt last, InputIts... firsts) {
		for (; first != last; ++first, ((++firsts), ...)) {
			f(*first, (*firsts)...);
		}
		return f;
	}
	template <typename Function, typename InputIt, typename Size, typename... InputIts>
	constexpr inline InputIt for_eachs_n(Function f, InputIt first, Size n, InputIts... firsts) {
		for (Size i = 0; i < n; ++i, ++first, ((++firsts), ...)) {
			f(*first, (*firsts)...);
		}
		return first;
	}
```
