# Multidim

[![Build Status](https://travis-ci.org/btzy/multidim.svg?branch=master)](https://travis-ci.org/btzy/multidim) [![Build Status](https://btzy.visualstudio.com/Multidim/_apis/build/status/btzy.multidim?branchName=master)](https://btzy.visualstudio.com/Multidim/_build/latest?definitionId=1&branchName=master)

Modern C++ library for multidimensional arrays, dynarrays, and vectors (with provisions for deques and other containers).

Works with C++17, better with C++20.

No dependencies apart from the C++ standard library.

**This is a work in progress, major redesigns and breaking changes might happen at any time.  Furthermore, not everything claimed in this README has been implemented yet.**

## Motivation

Storing a two-dimensional grid as `std::vector<std::vector<int>>` is rather inefficient because each of the inner vectors contain separately allocated heap memory.  If at construction time we know that each of the inner `std::vector<int>`s have a fixed number of elements, then we can store the entire grid in a single contiguous chunk of heap memory.  For example:

Original 2D grid:
```
-----------------
|     row 1     |
-----------------
|     row 2     |
-----------------
|     row 3     |
-----------------
```

Ideal storage in memory:
```
-------------------------------------------------
|     row 1     |     row 2     |     row 3     |
-------------------------------------------------
```

This layout can be extended to higher dimensions as well, as long as we know that all dimensions except the outermost one have the number of elements fixed at construction time.

Multidim provides multidimensional arrays that internally store data linearly in memory, in (at most) one heap allocation.

## Example code

```cpp
// Basic usage - like a normal 2D array
multidim::dynarray<multidim::inner_dynarray<int>> arr2d(3, 4);
for (int i=0; i<3; ++i) {
    for (int j=0; j<4; ++j) {
        arr2d[i][j] = i * 10 + j;
    }
}
assert(arr2d[2][3] == 23);

// Comparing whole rows
multidim::dynarray<int> row(4);
for (int j=0; j<4; ++j) {
    row[j] = 20 + j;
}
assert(arr2d[2] == row);

// Feels like a standard container
for (auto r : arr2d) {
    assert(r[0] + 1 == r[1]);
    assert(r.front() == r[0]);
    assert(r.size() == 4);
    assert(r.at(1) == r[1]);
}
assert(r.size() == 3);

// Iterators work like standard containers
std::reverse(arr2d.begin(), arr2d.end());
assert(arr2d[0] == row);

// Support for compile-time fixed arrays at any dimension
multidim::array<multidim::inner_dynarray<int>, 5> arr1(10); // outer array has 5 elements (compile-time constant), each inner array has 10 elements (fixed at contruction time)
multidim::dynarray<multidim::inner_array<int, 10>> arr2(5); // each inner array has 10 elements (compile-time constant), outer array has 5 elements (fixed at contruction time)
multidim::array<multidim::inner_array<int, 10>, 5> arr3; // stored on the stack if every nesting level uses a fixed-size array, should compile to code equivalent to std::array<std::array<int, 10>, 5> in C++20 (where [[no_unique_address]] is supported)

// Outermost dimension can be a growable vector
multidim::vector<multidim::inner_dynarray<int>> vec1(5); // outer collection is growable, like std::vector; inner array has 5 elements (fixed at construction time)

// Supports arbitrary number of dimensions
multidim::vector<multidim::inner_array<multidim::inner_dynarray<multidim::inner_array<int, 7>>, 42>> complicated; // very nested abomination

// Supports zero-element arrays
multidim::dynarray<multidim::inner_dynarray<int>> arr4(4, 0);
multidim::dynarray<multidim::inner_dynarray<int>> arr5(5, 0);
for (auto r : arr4) {
    // loops for 4 times
}
assert(arr4 != arr5);
```

Note about dynarrays:  A dynarray is an array with size known at _construction_ time.  Unlike `std::array`, its size is not known at _compilation_ time.  Unlike `std::vector`, it is not growable (i.e. you cannot call `push_back()` on a dynarray).  At some point in time, `std::dynarray` was part of the Arrays TS and slated for inclusion into the C++ standard, but the TS has since been abandoned.  Multidim of course also supports one-dimensional dynarrays that behave like those in the TS.

## Benchmarks

TODO

## What will not work

```cpp
multidim::dynarray<multidim::inner_dynarray<int>> arr2d(3, 4);
// std::sort(arr2d.begin(), arr2d.end()); // won't work
multidim::sort(arr2d.begin(), arr2d.end()); // ok
```
It doesn't work because `arr2d[0]` is a concrete type that acts like a 'reference wrapper', instead of a real reference to something.  It can't be a real reference to something when we don't know the size of that something.  Most implementations of `std::sort` will construct temporary elements, but of course we can't actually construct a temporary element of unknown size (see the note below on what copy construction instead does on Multidim's reference wrapper type).  We instead need to use a sorting algorithm that won't construct temporary elements (i.e. only does swapping of pairs of elements).  Some algorithms (including `multidim::sort`, which works on all of Multidim's containers) are provided in `<multidim/algorithm.hpp>`; they are intended to be drop-in replacements for those in the standard algorithms library.

Note:
```cpp
arr2d[1] = arr2d[0]; // Copy assignment will copy the actual data in the array
auto tmp = arr2d[0]; // Copy construction will create a new reference that points to the same data (tmp and arr2d[0] both have type multidim::dynarray_ref<int>)
```
