# 🪡 hybstr — Hybrid Compile-Time & Runtime String Library

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-only](https://img.shields.io/badge/library-header--only-lightgrey.svg)](#)
[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blueviolet.svg)](https://4o4hasfound.github.io/hybstr/)

**hybstr** (Hybrid String) is a lightweight, single-header **constexpr-friendly string library** for modern C++.
It allows you to use strings seamlessly across both **compile-time** and **runtime** contexts — ideal for **metaprogramming**, **reflection**, and **code generation**.

## Features

- **Hybrid compile-time / runtime**
  Works in both `constexpr` and regular runtime expressions.
- **Concatenation and modification**
  Supports operations like `+`, `append, push_back`, and `set<N>()` in both compile time and runtime.
- **NTTP**
  Can be used as NTTP in metaprogramming.

## Example Usage

```cpp
#include "hybstr.hpp"
#include <iostream>
#include <cassert>

int main() {
    constexpr auto s1 = std::string_view("Greetings");

    // Compile-time concatenation
    constexpr auto s2 = hybstr::string("Name");
    constexpr auto s3 = hybstr::string(s1) + ", " + s2;
    static_assert(s3 == hybstr::string("Greetings, Name"));

    // Runtime concatenation
    std::string input;
    std::cin >> input;
    auto s4 = hybstr::string(s1) + ", " + input;
    assert(s4.view() == std::string(s1.begin(), s1.end()) + ", " + input);
    std::cout << s4.view() << std::endl;

    // String literal
    auto s5 = "abcde"_hyb;
    static_assert(s5 == hybstr::string("abcde"));
}
```

