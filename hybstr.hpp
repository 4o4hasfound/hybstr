/**
 * @file hybstr.hpp
 * @brief Hybrid string implementation that can be used in both compile time and runtime
 *
 * @details
 * This header defines a **hybrid constexpr string** type that supports operations
 * in both compile-time and runtime environments.
 * It is primarily designed for **constexpr metaprogramming** and **string literal manipulation**, so runtime functionality will be limited.
 * 
 * @note
 * Requires **C++17 or later** - C++20 is recommended, with C++17 is supported with limited constexpr metaprogramming
 * 
 * @author
 * 404hasfound (GitHub: [4o4hasfound](https://github.com/4o4hasfound))
 *
 * @copyright
 * MIT License
 * 
 * @date    2025-10-05
 * @version 1.0
 * @cpp     C++20+
 */

#pragma once

#include <array>
#include <string>
#include <cassert>
#include <cstddef>
#include <utility>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#ifndef HYBSTR_DYNAMIC_EXPAND_CAPACITY
#define HYBSTR_DYNAMIC_EXPAND_CAPACITY 1000
#endif

#if defined(_MSC_VER)
#  define HYBSTR_CPP_STD _MSVC_LANG
#else
#  define HYBSTR_CPP_STD __cplusplus
#endif

#if HYBSTR_CPP_STD >= 202002L
	#define HYBSTR_CPP_20_OR_ABOVE true
#else
	#define HYBSTR_CPP_20_OR_ABOVE false
#endif

#if !HYBSTR_CPP_20_OR_ABOVE
	#ifndef consteval
		#define consteval constexpr
	#endif
#endif

#if HYBSTR_CPP_20_OR_ABOVE
	#include <type_traits>
	#define HYBSTR_IS_CONSTANT_EVALUATED std::is_constant_evaluated()
#elif defined(__clang__) || defined(__GNUC__)
	#define HYBSTR_IS_CONSTANT_EVALUATED __builtin_is_constant_evaluated()
	#elif defined(_MSC_VER) && _MSC_VER >= 1928
	#define HYBSTR_IS_CONSTANT_EVALUATED __builtin_is_constant_evaluated()
#elif defined(__INTEL_LLVM_COMPILER)
	#define HYBSTR_IS_CONSTANT_EVALUATED __builtin_is_constant_evaluated()
#else
	#define HYBSTR_IS_CONSTANT_EVALUATED false
#endif

/**
 * @namespace hybstr
 * @brief Main interface
 */
namespace hybstr
{
	template<std::size_t DynamicExpandCapacity = HYBSTR_DYNAMIC_EXPAND_CAPACITY>
	[[nodiscard]] constexpr auto string();
	template<std::size_t DynamicExpandCapacity = HYBSTR_DYNAMIC_EXPAND_CAPACITY, 
		std::size_t N>
	[[nodiscard]] constexpr auto string(const char(&str)[N]);
	template<std::size_t ViewSize = HYBSTR_DYNAMIC_EXPAND_CAPACITY, 
		std::size_t DynamicExpandCapacity = HYBSTR_DYNAMIC_EXPAND_CAPACITY>
	[[nodiscard]] constexpr auto string(std::string_view sv) noexcept;
	template<std::size_t RangeSize = HYBSTR_DYNAMIC_EXPAND_CAPACITY, 
		std::size_t DynamicExpandCapacity = HYBSTR_DYNAMIC_EXPAND_CAPACITY, 
		typename _Iter>
	[[nodiscard]] constexpr auto string(_Iter start, _Iter end) noexcept;

	/**
	 * @brief Fixed-capacity string that will expand on demand.
	 *
	 * @tparam BufferCapacity Number of characters stored in the compile-time buffer (excluding the null terminator).
	 * @tparam DynamicExpandCapacity Size of the dynamic buffer allocated for runtime operations.
	 *
	 * @details
	 * Combines a compile-time buffer for constexpr evaluation with a dynamic buffer
	 * for runtime use.
	 * 'BufferCapacity' defines how many characters are stored in the fixed buffer,
	 * and 'DynamicExpandCapacity' specifies the size of the dynamic region used when
	 * handling inputs such as 'std::string_view' or iterator ranges whose lengths
	 * cannot be determined at compile time.
	 * The dynamic buffer size can also be customized through template arguments.
	 *
	 * This class is not intended to be used directly.
	 * Prefer using the factory functions:
	 * @code
	 * auto s11 = hybstr::string();               // empty string
	 * auto s12 = hybstr::string<10>();           // empty string with custom dynamic expand capacity
	 *
	 * auto s21 = hybstr::string("abc");          // from string literal
	 * auto s22 = hybstr::string<10>("abc");      // from literal with custom dynamic expand capacity
	 *
	 * constexpr std::string_view sv = "abc";     // non-constexpr string views also work
	 * auto s31 = hybstr::string<3>(sv);          // from string_view (specify length, default to HYBSTR_DYNAMIC_EXPAND_CAPACITY)
	 * auto s32 = hybstr::string<3, 10>(sv);      // from string_view with custom expand capacity
	 *
	 * std::string str = "Hello world";
	 * auto s41 = hybstr::string<11>(str.begin(), str.end());        // from iterator range (specify length, default to HYBSTR_DYNAMIC_EXPAND_CAPACITY)
	 * auto s42 = hybstr::string<11, 10>(str.begin(), str.end());    // from iterator range with custom expand capacity
	 * @endcode
	 *
	 * Example:
	 * @code
	 * constexpr auto s1 = std::string_view("Greetings");
	 * 
	 * // Constexpr
	 * constexpr auto s2 = hybstr::string("Name");
	 * constexpr auto s3 = hybstr::string(s1) + ", " + s2;
	 * static_assert(s3 == hybstr::string("Greetings, Name"));
	 * 
	 * // Run time
	 * std::string s4;
	 * std::cin >> s4;
	 * auto s5 = hybstr::string(s1) + ", " + s4;
	 * assert(s5.view() == std::string(s1.begin(), s1.end()) + ", " + s4);
	 * @endcode
	 */
	template <std::size_t BufferCapacity = 0, std::size_t DynamicExpandCapacity = HYBSTR_DYNAMIC_EXPAND_CAPACITY>
	class string_impl
	{
		template <std::size_t, std::size_t>
		friend class string_impl;
	public:
		using value_type = char;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = std::size_t;
		using view_type = std::string_view;
		using pointer = char*;
		using const_pointer = const char*;
		using iterator = typename std::array<char, BufferCapacity + 1>::iterator;
		using const_iterator = typename std::array<char, BufferCapacity + 1>::const_iterator;
		using reverse_iterator = typename std::array<char, BufferCapacity + 1>::reverse_iterator;
		using const_reverse_iterator = typename std::array<char, BufferCapacity + 1>::const_reverse_iterator;

		/** @brief Default constructor. Initializes an empty string. */
		constexpr string_impl() noexcept
			: _size(0)
		{
			_data[0] = '\0';
		}

		/**
		 * @brief Constructs from a string literal.
		 * @tparam N Number of characters including null terminator.
		 * @param str Null-terminated C-style string.
		 */
		template <std::size_t N>
		constexpr string_impl(const char(&str)[N]) noexcept
		{
			static_assert(N > 0 && N - 1 <= BufferCapacity, "string literal too long");
			for (std::size_t i = 0; i < N - 1; ++i)
			{
				_data[i] = str[i];
			}
			_data[N - 1] = '\0';
			_size = N - 1;
		}

		/**
		 * @brief Fills the buffer with a repeated character.
		 * @param n Number of characters to fill.
		 * @param c Fill character.
		 */
		constexpr string_impl(const std::size_t& n, char c) noexcept
		{
			const std::size_t len = std::min(n, BufferCapacity);
			for (std::size_t i = 0; i < len; ++i)
			{
				_data[i] = c;
			}
			_data[len] = '\0';
			_size = len;
		}

		/**
		 * @brief Constructs from a std::string_view.
		 * Copies up to BufferCapacity characters.
		 */
		constexpr string_impl(std::string_view sv) noexcept
		{
			const std::size_t len = std::min(sv.size(), BufferCapacity);
			for (std::size_t i = 0; i < len; ++i)
			{
				_data[i] = sv[i];
			}
			_data[len] = '\0';
			_size = len;
		}

		/**
		 * @brief Constructs from an iterator range.
		 * Copies until 'end' or until BufferCapacity is reached.
		 */
		template<typename _Iter>
		constexpr string_impl(_Iter start, _Iter end) noexcept
		{
			std::size_t i = 0;

			while (i < BufferCapacity && start != end)
			{
				_data[i++] = *(start++);
			}

			_data[i] = '\0';
			_size = i;
		}

		constexpr string_impl(const string_impl&) noexcept = default;
		constexpr string_impl(string_impl&&) noexcept = default;
		constexpr string_impl& operator=(const string_impl&) noexcept = default;
		constexpr string_impl& operator=(string_impl&&) noexcept = default;

		/** @return Number of characters currently stored. */
		[[nodiscard]] constexpr auto size() const noexcept -> size_type
		{
			return _size;
		}
		/** @return Maximum number of characters in the fixed buffer. */
		[[nodiscard]] constexpr auto capacity() const noexcept -> size_type
		{
			return BufferCapacity;
		}
		/** @return True if the string is empty. */
		[[nodiscard]] constexpr auto empty() const noexcept -> bool
		{
			return _size == 0;
		}

		/** @return Mutable pointer to internal buffer. */
		[[nodiscard]] constexpr auto data() noexcept -> pointer
		{
			return _data.data();
		}
		/** @return Const pointer to internal buffer. */
		[[nodiscard]] constexpr auto data() const noexcept -> const_pointer
		{
			return _data.data();
		}
		/** @return Null-terminated C-string. */
		[[nodiscard]] constexpr auto c_str() const noexcept -> const_pointer
		{
			return _data.data();
		}
		/** @return Read-only view of the string. */
		[[nodiscard]] constexpr auto view() const noexcept -> view_type
		{
			return std::string_view(_data.data(), _size);
		}
		/** @return A new std::string copy of the contents (runtime). */
		[[nodiscard]] auto str() const -> std::string
		{
			return std::string(begin(), end());
		}


		/** @brief Accesses a character by index (mutable). */
		[[nodiscard]] constexpr auto operator[](size_type i) noexcept -> reference
		{
			return _data[i];
		}
		/** @brief Accesses a character by index (const). */
		[[nodiscard]] constexpr auto operator[](size_type i) const noexcept -> const_reference
		{
			return _data[i];
		}

		[[nodiscard]] constexpr auto begin() noexcept -> iterator
		{
			return _data.begin();
		}
		[[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
		{
			return _data.begin();
		}
		[[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator
		{
			return _data.cbegin();
		}
		[[nodiscard]] constexpr auto end() noexcept -> iterator
		{
			return _data.begin() + _size;
		}
		[[nodiscard]] constexpr auto end() const noexcept -> const_iterator
		{
			return _data.begin() + _size;
		}
		[[nodiscard]] constexpr auto cend() const noexcept -> const_iterator
		{
			return _data.begin() + _size;
		}
		[[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator
		{
			return _data.rbegin() + 1;
		}
		[[nodiscard]] constexpr auto rbegin() const noexcept -> const_reverse_iterator
		{
			return _data.rbegin() + 1;
		}
		[[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator
		{
			return _data.crbegin() + 1;
		}
		[[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator
		{
			return _data.rend();
		}
		[[nodiscard]] constexpr auto rend() const noexcept -> const_reverse_iterator
		{
			return _data.rend();
		}
		[[nodiscard]] constexpr auto crend() const noexcept -> const_reverse_iterator
		{
			return _data.crend();
		}

		/**
		 * @brief Replaces a character at compile time.
		 * @tparam N Index to replace (must be less than BufferCapacity).
		 * @param c New character value.
		 * @return A new string_impl with the modified character.
		 */
		template <std::size_t N>
		[[nodiscard]] constexpr auto set(char c) const noexcept
		{
			static_assert(N >= 0 && N < BufferCapacity, "index out of bound");
			string_impl<BufferCapacity, DynamicExpandCapacity> result{};
			for (std::size_t i = 0; i < _size; ++i)
			{
				result._data[i] = _data[i];
			}
			result._size = _size;
			result._data[N] = c;
			result._data[result._size] = '\0';

			return result;
		}

		/**
		 * @brief Appends a C-style string literal.
		 * @tparam N Number of characters including null terminator.
		 * @return A new string_impl containing the concatenated result.
		 */
		template <std::size_t N>
		[[nodiscard]] constexpr auto append(const char(&str)[N]) const noexcept
		{
			string_impl<BufferCapacity + N - 1, DynamicExpandCapacity> result{};

			for (std::size_t i = 0; i < _size; ++i)
			{
				result._data[i] = _data[i];
			}
			for (std::size_t i = 0; i < N - 1; ++i)
			{
				result._data[_size + i] = str[i];
			}

			result._size = _size + (N - 1);
			result._data[result._size] = '\0';

			return result;
		}

		/**
		 * @brief Appends another hybrid string.
		 * @tparam TargetSize Buffer expansion size for the result.
		 * @tparam N BufferCapacity of the other string.
		 * @tparam ExpandCapacity Dynamic capacity of the other string.
		 * @return A new string_impl combining both contents.
		 */
		template<std::size_t TargetSize = DynamicExpandCapacity, std::size_t N, std::size_t ExpandCapacity>
		[[nodiscard]] constexpr auto append(const string_impl<N, ExpandCapacity>& other) const noexcept
		{
			string_impl<
				BufferCapacity + TargetSize, 
				std::max(DynamicExpandCapacity, ExpandCapacity)
			> result{};

			for (std::size_t i = 0; i < _size; ++i)
			{
				result._data[i] = _data[i];
			}
			for (std::size_t i = 0; i < other.size(); ++i)
			{
				result._data[_size + i] = other._data[i];
			}

			result._size = _size + other.size();
			result._data[result._size] = '\0';
			return result;
		}
		/**
		 * @brief Appends a std::string_view.
		 * @tparam TargetSize Buffer expansion size for the result.
		 */
		template<std::size_t TargetSize = DynamicExpandCapacity>
		[[nodiscard]] constexpr auto append(std::string_view sv) const noexcept
		{
			if (HYBSTR_IS_CONSTANT_EVALUATED)
			{
				string_impl<BufferCapacity + TargetSize, DynamicExpandCapacity> result{};
				for (std::size_t i = 0; i < _size; ++i)
				{
					result._data[i] = _data[i];
				}
				for (std::size_t i = 0; i < sv.size(); ++i)
				{
					result._data[_size + i] = sv[i];
				}
				result._size = _size + sv.size();
				result._data[result._size] = '\0';
				return result;
			}
			else
			{
				// "string_impl overflow; increase the dynamic buffer size"
				assert(sv.size() < TargetSize);

				string_impl<BufferCapacity + TargetSize, DynamicExpandCapacity> result{};

				std::copy_n(_data.data(), _size, result._data.data());
				std::copy_n(sv.data(), sv.size(), result._data.data() + _size);
				result._size = _size + sv.size();
				result._data[result._size] = '\0';
				return result;
			}
		}
		/**
		 * @brief Appends one or more repeated characters.
		 * @tparam N Number of characters to add.
		 */
		template<std::size_t N = 1>
		[[nodiscard]] constexpr auto append(char c) const noexcept
		{
			string_impl<BufferCapacity + N, DynamicExpandCapacity> result{};
			for (std::size_t i = 0; i < _size; ++i)
			{
				result._data[i] = _data[i];
			}
			for (std::size_t i = 0; i < N; ++i)
			{
				result._data[_size + i] = c;
			}
			result._size = _size + N;
			result._data[result._size] = '\0';
			return result;
		}

		/** @brief Appends a single character. */
		[[nodiscard]] constexpr auto push_back(char c) const noexcept
		{
			return append<1>(c);
		}

		/**
		 * @brief Resizes the string to a new compile-time capacity.
		 * @tparam N New capacity.
		 * @param c Fill character if expanded.
		 * @return A new string_impl with resized storage.
		 */
		template<std::size_t N>
		[[nodiscard]] constexpr auto resize(char c = ' ') const noexcept
		{
			string_impl<N, DynamicExpandCapacity> result{};

			const std::size_t len = std::min(_size, N);

			for (std::size_t i = 0; i < len; ++i)
			{
				result._data[i] = _data[i];
			}
			for (std::size_t i = len; i < N; ++i)
			{
				result._data[i] = c;
			}

			result._size = N;
			result._data[result._size] = '\0';

			return result;
		}
		/**
		 * @brief Ensures compile-time capacity is at least N.
		 * If current capacity is smaller, a new string_impl is created.
		 */
		template<std::size_t N>
		[[nodiscard]] constexpr auto reserve() const noexcept
		{
			if constexpr (BufferCapacity < N)
			{
				string_impl<N, DynamicExpandCapacity> result{};
				result._size = std::min(_size, N);

				for (std::size_t i = 0; i < result._size; ++i)
				{
					result._data[i] = _data[i];
				}

				result._data[result._size] = '\0';
				return result;
			}
			else
			{
				return string_impl<N, DynamicExpandCapacity> { *this };
			}
		}

		std::array<char, BufferCapacity + 1> _data{}; ///< Internal fixed buffer (+1 for null terminator).
		size_type _size{}; ///< Number of characters currently stored.
	};

	template<std::size_t N>
	string_impl(const char(&)[N]) -> string_impl<N>;

	// ======================================================================
	//                           Operators
	// ======================================================================

	/**
	 * @brief Concatenates two hybrid strings.
	 *
	 * @tparam B1 Capacity of the first string's compile-time buffer.
	 * @tparam DynamicExpandCapacity1 Dynamic capacity of the first string.
	 * @tparam B2 Capacity of the second string's compile-time buffer.
	 * @tparam DynamicExpandCapacity2 Dynamic capacity of the second string.
	 *
	 * @return A new string_impl with combined compile-time and dynamic capacities.
	 */
	template<
		std::size_t B1, std::size_t DynamicExpandCapacity1,
		std::size_t B2, std::size_t DynamicExpandCapacity2
	>
	[[nodiscard]] constexpr auto operator+(
		const string_impl<B1, DynamicExpandCapacity1>& s1,
		const string_impl<B2, DynamicExpandCapacity2>& s2
		) noexcept
	{
		string_impl<
			B1 + B2,
			std::max(DynamicExpandCapacity1, DynamicExpandCapacity2)
		> result{};

		for (std::size_t i = 0; i < s1.size(); ++i)
		{
			result._data[i] = s1._data[i];
		}
		for (std::size_t i = 0; i < s2.size(); ++i)
		{
			result._data[s1.size() + i] = s2._data[i];
		}

		result._size = s1._size + s2._size;
		result._data[result._size] = '\0';
		return result;
	}

	/**
	 * @brief Concatenates a hybrid string with a C-style string literal.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity,
		std::size_t N
	>
	[[nodiscard]] constexpr auto operator+(
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s,
		const char(&str)[N]
		) noexcept
	{
		string_impl<BufferCapacity + N, DynamicExpandCapacity> result{};

		for (std::size_t i = 0; i < s.size(); ++i)
		{
			result._data[i] = s._data[i];
		}
		for (std::size_t i = 0; i < N - 1; ++i)
		{
			result._data[s.size() + i] = str[i];
		}

		result._size = s.size() + N - 1;
		result._data[result._size] = '\0';

		return result;
	}


	/**
	 * @brief Concatenates a C-style string literal with a hybrid string.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity,
		std::size_t N
	>
	[[nodiscard]] constexpr auto operator+(
		const char(&str)[N],
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s
		) noexcept
	{
		string_impl<BufferCapacity + N, DynamicExpandCapacity> result{};

		for (std::size_t i = 0; i < N - 1; ++i)
		{
			result._data[i] = str[i];
		}
		for (std::size_t i = 0; i < s.size(); ++i)
		{
			result._data[N + i - 1] = s._data[i];
		}

		result._size = s.size() + N - 1;
		result._data[result._size] = '\0';

		return result;
	}


	/**
	 * @brief Concatenates a hybrid string with a char.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity
	>
	[[nodiscard]] constexpr auto operator+(
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s,
		char c
		) noexcept
	{
		string_impl<BufferCapacity + 1, DynamicExpandCapacity> result{};

		for (std::size_t i = 0; i < s.size(); ++i)
		{
			result._data[i] = s._data[i];
		}
		result._size = s.size() + 1;
		result._data[result._size - 1] = c;
		result._data[result._size] = '\0';

		return result;
	}

	/**
	 * @brief Concatenates a hybrid string with a char.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity
	>
	[[nodiscard]] constexpr auto operator+(
		char c,
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s
		) noexcept
	{
		string_impl<BufferCapacity + 1, DynamicExpandCapacity> result{};

		result._data[0] = c;
		for (std::size_t i = 0; i < s.size(); ++i)
		{
			result._data[i + 1] = s._data[i];
		}
		result._size = s.size() + 1;
		result._data[result._size] = '\0';

		return result;
	}

	/**
	 * @brief Concatenates a hybrid string with a std::string_view.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity
	>
	[[nodiscard]] constexpr auto operator+(
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s,
		std::string_view sv
		) noexcept
	{
		string_impl<BufferCapacity + DynamicExpandCapacity, DynamicExpandCapacity> result{};

		for (std::size_t i = 0; i < s.size(); ++i)
		{
			result._data[i] = s._data[i];
		}
		for (std::size_t i = 0; i < sv.size(); ++i)
		{
			result._data[s.size() + i] = sv[i];
		}

		result._size = s.size() + sv.size();
		result._data[result._size] = '\0';

		return result;
	}

	/**
	 * @brief Concatenates a std::string_view with a hybrid string.
	 */
	template<
		std::size_t BufferCapacity,
		std::size_t DynamicExpandCapacity
	>
	[[nodiscard]] constexpr auto operator+(
		std::string_view sv,
		const string_impl<BufferCapacity, DynamicExpandCapacity>& s
		) noexcept
	{
		return s + sv;
	}


	// ======================================================================
	//                           Comparisons
	// =====================================================================

	/**
	 * @brief Equality comparison for two hybrid strings.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator==(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		if (s1._size != s2._size)
		{
			return false;
		}

		for (std::size_t i = 0; i < s1._size; ++i)
		{
			if (s1._data[i] != s2._data[i])
			{
				return false;
			}
		}
		return true;
	}


	/**
	 * @brief Inequality comparison for two hybrid strings.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator!=(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		return !(s1 == s2);
	}

#if HYBSTR_CPP_20_OR_ABOVE
	/**
	 * @brief Lexicographical three-way comparison for hybrid strings. C++20 or above.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr auto operator<=>(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		const std::size_t min_size = std::min(s1._size, s2._size);

		for (std::size_t i = 0; i < min_size; ++i)
		{
			if (auto cmp = s1._data[i] <=> s2._data[i]; cmp != 0)
			{
				return cmp;
			}
		}
		return s1._size <=> s2._size;
	}

#else
	/**
	 * @brief Less than comparison for two hybrid strings. C++17.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator<(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		const std::size_t min_size = std::min(s1._size, s2._size);

		for (std::size_t i = 0; i < min_size; ++i)
		{
			if (s1._data[i] < s2._data[i]) return true;
			else return false;
		}
		return s1._size < s2._size;
	}

	/**
	 * @brief Less than or equal to comparison for two hybrid strings.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator<=(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		return !(s2 < s1);
	}

	/**
	 * @brief Greater than comparison for two hybrid strings.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator>(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
		) noexcept
	{
		return s2 < s1;
	}

	/**
	 * @brief Greater than or equal to comparison for two hybrid strings.
	 */
	template<
		std::size_t B1, std::size_t D1,
		std::size_t B2, std::size_t D2
	>
	[[nodiscard]] constexpr bool operator>=(
		const string_impl<B1, D1>& s1,
		const string_impl<B2, D2>& s2
	) noexcept
	{
		return !(s1 < s2);
	}
#endif

	// ======================================================================
	//                           Traits and Utilities
	// ======================================================================

	/**
	 * @brief Type trait for detecting hybstr::string_impl.
	 */
	template<typename T>
	struct is_string_impl : std::false_type {};
	template <std::size_t BufferCapacity, std::size_t DynamicExpandCapacity>
	struct is_string_impl<string_impl<BufferCapacity, DynamicExpandCapacity>> : std::true_type {};

	/// @brief Shorthand boolean variable for is_string_impl.
	template <typename T>
	inline constexpr bool is_string_impl_v = is_string_impl<T>::value;

#if HYBSTR_CPP_20_OR_ABOVE
	/**
	 * @brief Adjusts the compile-time buffer size to exactly fit the string content. C++20 or above.
	 *
	 * @tparam str A hybstr::string_impl instance (must be known at compile time).
	 * @return A new string_impl whose BufferCapacity equals its current size.
	 */
	template <auto str>
	[[nodiscard]] consteval auto fit_string() noexcept
	{
		static_assert(is_string_impl_v<std::remove_cvref_t<decltype(str)>>, "Expect a vre::string as input");
		return str.template resize<str.size()>();
	}
#endif

#if HYBSTR_CPP_20_OR_ABOVE
	/**
	 * @brief Adjusts the compile-time buffer size to exactly fit the string content. C++20 or above.
	 *
	 * @details
	 * Returns a resized copy of @p str whose buffer capacity equals its current size.
	 * The expression is 'constexpr' if @p str supports compile-time evaluation.
	 *
	 * @param str A 'hybstr::string_impl' instance.
	 * @return A new 'string_impl' whose buffer capacity matches its current size.
	 */
	#define HYBSTR_FIT_STRING(str)\
		hybstr::fit_string<(str)>()
#else
	/**
	 * @brief Adjusts the compile-time buffer size to exactly fit the string content. C++17.
	 *
	 * @details
	 * C++17 fallback macro version of 'hybstr::fit_string'.
	 * Returns a resized copy of @p str whose buffer capacity equals its current size.
	 * The expression is 'constexpr' if @p str supports compile-time evaluation.
	 *
	 * @param str A 'hybstr::string_impl' instance.
	 * @return A new 'string_impl' whose buffer capacity matches its current size.
	 */
	#define HYBSTR_FIT_STRING(str) \
		([&]() constexpr noexcept { return (str).template resize<(str).size()>(); })()
#endif

	// ======================================================================
	//                           Factory Functions
	// ======================================================================

	/**
	 * @brief Creates an empty hybrid string.
	 */
	template<std::size_t DynamicExpandCapacity>
	[[nodiscard]] constexpr auto string()
	{
		return string_impl<0, DynamicExpandCapacity>{};
	}

	/**
	 * @brief Creates a hybrid string from a string literal.
	 */
	template<std::size_t DynamicExpandCapacity, std::size_t N>
	[[nodiscard]] constexpr auto string(const char(&str)[N])
	{
		return string_impl<N, DynamicExpandCapacity>{str};
	}

	/**
	 * @brief Creates a hybrid string from a std::string_view.
	 *
	 * @tparam ViewSize The known or estimated size of the string view.
	 * @tparam DynamicExpandCapacity Dynamic buffer size for runtime usage.
	 */
	template<std::size_t ViewSize, std::size_t DynamicExpandCapacity>
	[[nodiscard]] constexpr auto string(std::string_view sv) noexcept
	{
		return string_impl<ViewSize, DynamicExpandCapacity>{sv};
	}

	/**
	 * @brief Creates a hybrid string from an iterator range.
	 *
	 * @tparam RangeSize The known or estimated number of elements in the range.
	 * @tparam DynamicExpandCapacity Dynamic buffer size for runtime usage.
	 * @tparam _Iter Iterator type.
	 */
	template<std::size_t RangeSize, std::size_t DynamicExpandCapacity, typename _Iter>
	[[nodiscard]] constexpr auto string(_Iter start, _Iter end) noexcept
	{
		return string_impl<RangeSize, DynamicExpandCapacity>{start, end};
	}
} // namespace hybstr

/**
 * @namespace hybstr::literals
 * @brief Contains user defined literal operators for creating 'hybstr::string_impl' objects
 */
namespace hybstr::literals
{
#if HYBSTR_CPP_20_OR_ABOVE
	/**
	 * @brief User-defined literal for creating a 'hybstr::string_impl'. C++20 or above.
	 * @code{.cpp}
	 * using namespace hybstr::literals;
	 * constexpr auto s = "Hello"_hyb;
	 * static_assert(s == hybstr::string("Hello"));
	 * @endcode
	 */
	template<hybstr::string_impl str>
	[[nodiscard]] consteval auto operator""_hyb() noexcept
	{
		return str;
	}
#else
	/**
	 * @brief User-defined literal for creating a 'hybstr::string_impl'. C++17.
	 * @code{.cpp}
	 * using namespace hybstr::literals;
	 * constexpr auto s = "Hello"_hyb;
	 * static_assert(s == hybstr::string("Hello"));
	 * @endcode
	 */
	[[nodiscard]] consteval auto operator""_hyb(const char* str, std::size_t len) noexcept
	{
		return hybstr::string(str, str + len);
	}
	#endif
}