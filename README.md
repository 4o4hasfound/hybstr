
# 🪡 hybstr — Hybrid Compile-Time & Runtime String Library

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-only](https://img.shields.io/badge/library-header--only-lightgrey.svg)](#)
[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blueviolet.svg)](https://4o4hasfound.github.io/hybstr/)

## Overview

`hybstr` provides a **hybrid string type** (`hybstr::string_impl`) that seamlessly bridges **compile-time** (`constexpr`) and **runtime** string handling.

Unlike `std::string` (purely runtime) or `std::string_view` (non-owning),
`hybstr::string_impl` owns its data and can operate in both environments.

In both compile-time and runtime enviroments, all data lives in a fixed buffer known to the compiler.

`hybstr` lets you treat strings as **first-class compile-time objects**
— useful for metaprogramming, code generation, embedded DSLs, or constexpr evaluation —
while still functioning as a normal string type at runtime.

## Features

- **Hybrid compile-time / runtime**
  Works in both `constexpr` and regular runtime expressions.
- **Concatenation and modification**
  Supports operations like `+`, `append, push_back`, and `set<N>()` in both compile time and runtime.
- **NTTP**
  Can be used as a Non-Type Template Parameter (NTTP) in C++20 and above.

## Examples

### Construction

```cpp
#include "hybstr.hpp"
using namespace hybstr;

// compile time
constexpr auto s1 = string("Hello");
constexpr auto s2 = string("World");
constexpr auto s3 = s1 + ", " + s2 + "!";

static_assert(s3 == string("Hello, World!"));
```

### Literals

```cpp
using namespace hybstr::literals;

constexpr auto greet = "Hi"_hyb + ", "_hyb + "there"_hyb;
static_assert(greet == hybstr::string("Hi, there"));
```

### From Runtime Inputs

```cpp
std::string name;
std::cin >> name;

constexpr auto s1 = string("Hello");

auto msg = s1 + name;
std::cout << msg.str() << '\n'; // outupt str
std::cout << msg.view() << '\n'; // output view
```

### Compile time utils

#### C++20

```cpp
constexpr auto s = hybstr::string("CompileTimeText");
constexpr auto fitted = hybstr::fit_string<s>();
static_assert(fitted.capacity() == fitted.size());

// Or combine them if s have a big capacity with little size, this removes the extra memory at compile time
constexpr auto fitted2 = hybstr::fit_string<hybstr::string<100000>("CompileTimeText")>();
```

#### C++17 fallback macro

```cpp
constexpr auto s = hybstr::string("CompileTimeText");
constexpr auto fitted = HYBSTR_FIT_STRING(s);
static_assert(fitted.capacity() == fitted.size());

// Or combine them if s have a big capacity with little size, this removes the extra memory at compile time
constexpr auto fitted2 = HYBSTR_FIT_STRING(hybstr::string<100000>("CompileTimeText"));
```

### Comparison Operators

``hybstr::string_impl`` supports all standard comparison operations:

```cpp
constexpr auto a = hybstr::string("abc");
constexpr auto b = hybstr::string("abd");

static_assert(a < b);
static_assert(a != b);
```

## Factory Functions

| Function                           | Description             |
| ---------------------------------- | ----------------------- |
| `string()`                       | Create an empty string. |
| `string(const char(&)[N])`       | From string literal.    |
| `string(std::string_view)`       | From string view.       |
| `string(_Iter start, _Iter end)` | From iterator range.    |

## Mixed Usage

```cpp
#include "hybstr.hpp"
#include <iostream>

using namespace hybstr;

int main() {
    constexpr auto prefix = string("Hello, ");
    std::string name;
    std::cin >> name;

    // Mix compile-time and runtime strings
    auto result = prefix + string(name.begin(), name.end()) + "!";
    std::cout << result.view() << '\n';
}
```

