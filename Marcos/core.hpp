#ifndef _core_hpp_include_
#define _core_hpp_include_

// Core: A simple single header library to enhance C++ /////////////////////////
// See end of file for License information.
//
// This is a single header full of utilities that I consider to be either
// essential or extremely important for a programming language like C++.
//
// Main features include:
// - No usage of `virtual`
// - No exceptions, errors are just values.
// - No implict memory allocations
// - Defer
// - Custom Allocator support without "polymorphic resource" madness
// - Vector arithmetic built in
// - Track calls with the `Caller_Location` macro
// - dynamic_array, stack, queue, hash_map (TODO) that use the Allocator interface
//   to get resources
// - Memory Arena, convenience functions such as make() and make_slice()
// - Slice-centric design to prevent bounds checking problems
// - Environment constexpr variables and macros for conditional compilation
// - Still be able to take advantage of newer C++20 features
// - String type that is UTF-8 centric
//
// Requires C++20 or above, you can make this work with C++17 if you get rid of
// the Caller_Location feature.

// Primitive Types: Built-ins //////////////////////////////////////////////////
#ifndef _types_hpp_include_
#define _types_hpp_include_

#include <cstddef>
#include <cstdint>

namespace x {
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using usize = size_t;
using isize = ptrdiff_t;

using uintptr = uintptr_t;

using byte = unsigned char;
using rune = u32;

// Some machines don't have access to float 16
#ifndef DISABLE_FLOAT16
using f16 = _Float16;
#endif
using f32 = float;
using f64 = double;

#ifndef __STDC_NO_COMPLEX_
using complex64  = _Complex float;
using complex128 = _Complex double;
#endif

namespace prefix {
constexpr auto kilo = isize(1'000);
constexpr auto mega = isize(1'000) * kilo;
constexpr auto giga = isize(1'000) * mega;
constexpr auto tera = isize(1'000) * giga;
constexpr auto peta = isize(1'000) * tera;
constexpr auto exa  = isize(1'000) * peta;

constexpr auto kibi = isize(1024);
constexpr auto mebi = isize(1024) * kibi;
constexpr auto gibi = isize(1024) * mebi;
constexpr auto tebi = isize(1024) * gibi;
constexpr auto pebi = isize(1024) * tebi;
constexpr auto exbi = isize(1024) * pebi;
}
// Make sure this isn't some weird platform
static_assert(sizeof(usize) == sizeof(isize), "Mismatched isize and usize");
static_assert(sizeof(byte) == sizeof(char), "Literally impossible");
#ifndef DISABLE_FLOAT16
static_assert(sizeof(f16) == 2, "Wrong size for f16");
#endif
static_assert(sizeof(f32) == 4, "Wrong size for f32");
static_assert(sizeof(f64) == 8, "Wrong size for f64");
static_assert(sizeof(complex64) == 8, "Wrong size for complex64");
static_assert(sizeof(complex128) == 16, "Wrong size for complex128");
}

#ifdef USE_CORE_BUILTIN_TYPES
using x::i8;
using x::i16;
using x::i32;
using x::i64;
using x::u8;
using x::u16;
using x::u32;
using x::u64;
using x::usize;
using x::isize;
using x::uintptr;
using x::byte;
using x::rune;
#ifndef DISABLE_FLOAT16
using x::f16;
#endif
using x::f32;
using x::f64;
#ifndef __STDC_NO_COMPLEX_
using x::complex64;
using x::complex128;
#endif
#endif

#endif /* Include guard */
// Environment and Standard Includes ///////////////////////////////////////////
#ifndef _environment_hpp_include_
#define _environment_hpp_include_

#include <source_location>
#include <bit>
// NOTE: Do **NOT** change the order of the ifdefs
#if defined(__clang__)
	#define COMPILER_VENDOR_CLANG 1
#elif defined(__GNUC__) && !defined(__clang__)
	#define COMPILER_VENDOR_GCC 1
#elif defined(_MSVC_VER)
	#define COMPILER_VENDOR_MSVC 1
#else
	#define COMPILER_VENDOR_UNKNOWN 1
#endif

#if COMPILER_VENDOR_CLANG || COMPILER_VENDOR_GCC
	#define USE_BUILTIN_MEM_FUNCTIONS 1
#endif

namespace x {
using std::bit_cast;
using source_location = std::source_location;

namespace env {
enum struct CompilerVendor : u8 {
	Unknown = 0,
	Clang, GCC, MSVC,
};

constexpr auto compiler_vendor =
#if COMPILER_VENDOR_CLANG
	CompilerVendor::Clang
#elif COMPILER_VENDOR_GCC
	CompilerVendor::GCC
#elif COMPILER_VENDOR_MSVC
	CompilerVendor::MSVC
#else
	#warning "Unknown Compiler Vendor: This may result in degraded performance."
	CompilerVendor::Unknown
#endif
;

constexpr char const* const compiler_vendor_name =
	compiler_vendor == CompilerVendor::Clang ? "clang"
	: compiler_vendor == CompilerVendor::GCC ? "gcc"
	: compiler_vendor == CompilerVendor::MSVC ? "msvc"
	: "<unknown>"
;

constexpr long cplusplus_version = __cplusplus;

constexpr bool use_builtin_mem_functions =
	bool(USE_BUILTIN_MEM_FUNCTIONS)
;

constexpr bool assert_enabled =
#ifdef DISABLE_ASSERT
	false
#else
	true
#endif
;

constexpr bool debug_build =
#if defined(NDEBUG) || defined(RELEASE_BUILD)
	false
#else
	true
#endif
;

constexpr bool bounds_checking_enabled =
#ifdef DISABLE_BOUNDS_CHECKING
	false
#else
	true
#endif
;
}

// To use as a default parameter to functions that represent the
// caller's location
#define Caller_Location \
	[[maybe_unused]] \
	x::source_location const& caller_location = x::source_location::current()

}

#endif /* Include guard */
// General Utility /////////////////////////////////////////////////////////////
#ifndef _utility_hpp_include_
#define _utility_hpp_include_
namespace x {
// Absolute value
template<typename T>
constexpr
T abs(T const& x){
	if(x < 0){
		return -x;
	}
	return x;
}

// Maximum
template<typename T>
constexpr
T max(T const& a, T const& b){
	if(b > a){ return b; }
	return a;
}

// Minimum
template<typename T>
constexpr
T min(T const& a, T const& b){
	if(b < a){ return b; }
	return a;
}

// Maximum
template<typename T, typename ...Rest>
constexpr
T max(T const& a, T const& b, Rest&& ...rest){
	if(b > a){
		return max(b, rest...);
	}
	return max(a, rest...);
}

// Minimum
template<typename T, typename ...Rest>
constexpr
T min(T const& a, T const& b, Rest&& ...rest){
	if(b < a){
		return min(b, rest...);
	}
	return min(a, rest...);
}

// Make x fit the rage mini..maxi (inclusive)
template<typename T>
constexpr
T clamp(T const& mini, T const& x, T const& maxi){
	return min(max(x, mini), maxi);
}

namespace typing {
template<typename T>
struct remove_reference_type {typedef T type; };
template<typename T>
struct remove_reference_type<T&> {typedef T type; };
template<typename T>
struct remove_reference_type<T&&> {typedef T type; };

template<typename T, T v>
struct integral_constant {
	static constexpr T value = v;
	typedef T value_type;
	constexpr operator value_type() { return value; }
};

using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template<typename A, typename B>
struct same_type : false_type {};

template<typename T>
struct same_type<T, T> : true_type {};

template<typename A, typename B>
constexpr auto same_as = same_type<A, B>::value;

template<typename T>
struct is_lvalue_reference_type : false_type {};
template<typename T>
struct is_lvalue_reference_type<T&> : true_type {};
template<typename T>
struct is_lvalue_reference_type<T&&> : false_type {};

template<typename T>
struct is_rvalue_reference_type : false_type {};
template<typename T>
struct is_rvalue_reference_type<T&> : false_type {};
template<typename T>
struct is_rvalue_reference_type<T&&> : true_type {};

template<typename T>
using remove_reference = typename remove_reference_type<T>::type;

template<typename T>
constexpr bool is_lvalue_ref = is_lvalue_reference_type<T>::value;

template<typename T>
constexpr bool is_rvalue_ref = is_rvalue_reference_type<T>::value;
}

// Cast x to rvalue reference
template<typename T>
constexpr
typing::remove_reference<T>&& move(T&& x) noexcept {
	using rv = typing::remove_reference<T>&&;
	return static_cast<rv>(x);
}

// Contitionally moves x, if and only if, x is an rvalue reference.
// Requires passing template type explicitly. This is used to implement
// "perfect forwarding"
template<typename T>
constexpr
T&& forward(typing::remove_reference<T>& x) noexcept {
	return static_cast<T&&>(x);
}

// Contitionally moves x, if and only if, x is an rvalue reference.
// Requires passing template type explicitly. This is used to implement
// "perfect forwarding"
template<typename T>
constexpr
T&& forward(typing::remove_reference<T>&& x) noexcept {
	static_assert(
		!typing::is_lvalue_ref<T>,
		"Cannot use forward() to convert an rvalue to an lvalue"
	);
	return static_cast<T&&>(x);
}

// Swap values of a and b
template<typename T>
constexpr
void swap(T& a, T& b) noexcept {
	T t = x::move(b);
	b   = x::move(a);
	a   = x::move(t);
}

// Replaces x with val and returns the old value of x
template<typename T, typename U = T>
constexpr
T exchange(T& x, U&& val) noexcept {
	T t = x::move(x);
	x   = x::forward<U>(val);
	return t;
}

template<typename A, typename B = A>
struct pair {
	A a;
	B b;
};

// Check if an error enum is zero.
// If it is a boolean, check if it's true (ok)
template<typename EnumType>
constexpr
bool error_ok(EnumType e){
	return (isize)(e) == 0;
}

template<>
constexpr
bool error_ok<bool>(bool e){
	return e;
}
template<typename T, typename ... Args>
constexpr
void construct(T* ptr, Args&& ... args){
	new (ptr) T(x::forward<Args>(args)...);
}

template<typename T, typename ... Args>
constexpr
void destruct(T& val){
	val.~T();
}

#define Or_Return(ERR_) if(!error_ok((ERR_))){ return ERR_; }

}

#endif /* Include guard */
// Defer ///////////////////////////////////////////////////////////////////////
#ifndef _defer_hpp_include_
#define _defer_hpp_include_
namespace x::defer_impl {
	template <typename Fn>
	struct DeferredCall {
		Fn f;

		DeferredCall(Fn&& f)
			: f(x::move(f)) {}

		~DeferredCall(){
			f();
		}
	};
}

#define DEFER_GLUE_0(X_, Y_) X_##Y_
#define DEFER_GLUE_1(X_, Y_) DEFER_GLUE_0(X_, Y_)
#define DEFER_VAR_NAME(NAME_) DEFER_GLUE_1(NAME_, __COUNTER__)
#define Defer(BLK_) \
	x::defer_impl::DeferredCall DEFER_VAR_NAME(_deferred_call_) = [&](){ \
		do { \
			BLK_ \
			; \
		} while(0); \
	}
#endif /* Include guard */
// Assertions and Bounds checking //////////////////////////////////////////////
#ifndef _assert_hpp_include_
#define _assert_hpp_include_

#include <cstdio>
#include <cstdlib>

namespace x {
namespace impl {
[[maybe_unused]] static
void report_location(source_location const& loc){
	std::fprintf(stderr, "%s:%d ", loc.file_name(), loc.line());
}

[[maybe_unused]] static
void report_location_and_function(source_location const& loc){
	std::fprintf(stderr, "%s:%d %s ",
		loc.file_name(),
		loc.line(),
		loc.function_name());
}
}

// Causes a fatal error that cannot be recovered from
[[maybe_unused]] static
void panic(char const* const msg){
	do {
		std::fprintf(stderr, "Panic: %s\n", msg);
		std::abort();
	} while(1);
}

// Checks a predicate and panics if it is false
[[maybe_unused]] static constexpr
void panic_assert(
	bool predicate,
	const char* msg = "",
	Caller_Location
){
	if constexpr(env::assert_enabled){
		[[unlikely]]
		if(!predicate){
			impl::report_location(caller_location);
			std::fprintf(stderr, "%s ", msg);
			panic("Assertion failure");
		}
	}
}

// Checks a predicate and panics if it is false, enabled only in debug builds
[[maybe_unused]] static
void debug_assert(
	bool predicate,
	const char* msg = "",
	Caller_Location
){
	if constexpr(env::assert_enabled && !env::debug_build){
		[[unlikely]]
		if(!predicate){
			impl::report_location(caller_location);
			std::fprintf(stderr, "%s ", msg);
			panic("Assertion failure");
		}
	}
}

// Same as assert_expr, but used specifically for bounds checking
[[maybe_unused]] static
void bounds_check(
	bool predicate,
	Caller_Location
){
	if constexpr(env::bounds_checking_enabled){
		[[unlikely]]
		if(!predicate){
			impl::report_location_and_function(caller_location);
			panic("Bounds checking error.");
		}
	}
}

[[maybe_unused]] static
void unreachable(Caller_Location){
	if constexpr(env::assert_enabled){
		impl::report_location(caller_location);
		panic("Unreachable code");
	}
}

[[maybe_unused]] static
void unimplemented(Caller_Location){
	if constexpr(env::assert_enabled){
		impl::report_location_and_function(caller_location);
		panic("Unimplemented code");
	}
}

#ifndef DISABLE_ASSERT
#define Assert(PRED_) x::assert_expr(PRED_, #PRED_)
#else
#define Assert(PRED_, MSG_)
#endif
}
#endif /* Include guard */
// Primitive Types: Array //////////////////////////////////////////////////////
#ifndef _array_hpp_include_
#define _array_hpp_include_
namespace x {
// An array is a fixed sized homogenous collection of values, usually used for
// math. Element wise operations are implemented for this struct to allow for
// vector arithmetic.
template<typename T, usize N>
struct array {
	constexpr
	usize size() const {
		return N;
	}

	constexpr
	T* raw_data() {
		return &data[0];
	}

	constexpr
	T& operator[](usize idx){
		return data[idx];
	}

	constexpr
	T const& operator[](usize idx) const {
		return data[idx];
	}

	struct Iterator {
		Iterator(T* data)
			: ptr_val(data) {}
		T& operator*() const { return *ptr_val; }
		T* operator->() const { return ptr_val; }
		constexpr
		auto& operator++(){ ptr_val += 1; return *this; }
		constexpr
		bool operator==(Iterator const& it){ return ptr_val == it.ptr_val; }
		constexpr
		bool operator!=(Iterator const& it){ return ptr_val != it.ptr_val; }
	private:
		T* ptr_val;
	};

	struct ConstIterator {
		ConstIterator(T const* data)
			: ptr_val(data) {}
		T const& operator*() const { return *ptr_val; }
		T* operator->() const { return ptr_val; }
		constexpr
		auto& operator++(){ ptr_val += 1; return *this; }
		constexpr
		bool operator==(ConstIterator const& it){ return ptr_val == it.ptr_val; }
		constexpr
		bool operator!=(ConstIterator const& it){ return ptr_val != it.ptr_val; }

	private:
		T const* ptr_val;
	};

	constexpr
	auto begin(){
		return Iterator(&data[0]);
	}

	constexpr
	auto end(){
		return Iterator(&data[N]);
	}

	constexpr
	auto begin() const {
		return ConstIterator(&data[0]);
	}

	constexpr
	auto end() const {
		return ConstIterator(&data[N]);
	}

	T data[N];
};

template<typename T, usize N, typename Func>
constexpr
auto map(array<T, N> const& arr, Func&& fn){
	array<decltype(fn(arr[0])), N> res;
	for(usize i = 0; i < N; i += 1){
		res[i] = fn(arr[i]);
	}
	return res;
}

template<typename T, usize N, typename Func, typename U>
constexpr
T fold(array<T, N> const& arr, Func&& fn, U&& initial){
	T acc = initial;
	for(usize i = 1; i < N; i += 1){
		acc = fn(acc, arr[i]);
	}
	return acc;
}

template<typename T, usize N, usize I, typename U>
constexpr
void fill_arr_with_param_pack(array<T, N>& v, U&& elem){
	static_assert(I < N, "Out of bounds");
	v[I] = static_cast<T>(elem);
}

template<typename T, usize N, usize I = 0, typename U,typename... Args>
constexpr
void fill_arr_with_param_pack(array<T, N>& v, U&& elem, Args&& ...indices){
	static_assert(I < N, "Out of bounds");
	v[I] = static_cast<T>(elem);
	fill_arr_with_param_pack<T, N, I+1>(v, x::forward<Args>(indices)...);
}

template<typename T, usize N>
constexpr
array<T, N> abs(array<T, N> const& a){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){
		res[i] = abs(a[i]);
	}
	return res;
}

template<typename T, usize N>
constexpr
bool array_equal(array<T, N> const& a, array<T, N> const& b){
	for(usize i = 0; i < N; i += 1){
		if(a[i] != b[i]){ return false; }
	}
	return true;
}

template<typename T, usize N, typename U>
constexpr
auto swizzle(const array<T, N>& v, U&& idx){
	array<T, 1> res;
	res[0] = v[idx];
	return res;
}

template<typename T, usize N, typename... Index>
constexpr
auto swizzle(const array<T, N>& v, Index&& ...indices){
	constexpr usize L  = sizeof...(indices);
	array<usize, L> idxv;
	array<T, L> res;

	fill_arr_with_param_pack(idxv, x::forward<Index>(indices)...);
	for(usize i = 0; i < L; i += 1){
		res[i] = v[idxv[i]];
	}

	return res;
}

// Note: All operator overloads were auto generated.
template<typename T, usize N>
constexpr static inline
array<T, N> operator+(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] + b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator-(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] - b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator*(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] * b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator/(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] / b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator%(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] % b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator&(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] & b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator|(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] | b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator^(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] ^ b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator<<(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] << b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator>>(array<T, N> const& a, array<T, N> const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >> b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator+(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] + b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator-(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] - b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator*(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] * b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator/(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] / b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator%(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] % b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator&(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] & b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator|(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] | b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator^(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] ^ b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator<<(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] << b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator>>(array<T, N> const& a, T const& b){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >> b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator==(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] == b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator!=(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] != b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator>(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] > b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator<(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] < b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator>=(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >= b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator<=(array<T, N> const& a, array<T, N> const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] <= b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator==(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] == b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator!=(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] != b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator>(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] > b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator<(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] < b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator>=(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >= b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<bool, N> operator<=(array<T, N> const& a, T const& b){
	array<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] <= b; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator-(array<T, N> const& a){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = - a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator~(array<T, N> const& a){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = ~ a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator!(array<T, N> const& a){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = ! a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static inline
array<T, N> operator+(array<T, N> const& a){
	array<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = + a[i]; }
	return res;
}

}
#endif /* Include guard */
// Primitive Types: Slice //////////////////////////////////////////////////////
#ifndef _slice_hpp_include_
#define _slice_hpp_include_

namespace x {
// A Slice is simply a pointer to contigous memory and a length, this is a much
// safer and useful construct than C's usual pointer decay.
template<typename T>
struct slice {
	constexpr
	usize size() const {
		return length;
	}

	constexpr
	T* raw_data() const {
		return &data[0];
	}

	constexpr
	T& operator[](usize idx){
		bounds_check(idx < length);
		return data[idx];
	}

	constexpr
	T const& operator[](usize idx) const {
		bounds_check(idx < length);
		return data[idx];
	}

	constexpr
	bool empty() const {
		return (data == nullptr) || (length == 0);
	}

	// Implicit comparison is not allowed.
	bool operator==(slice<T> const&) = delete;

	constexpr
	slice(T* ptr, usize length)
		: data{ptr}, length{length} {}

	constexpr
	slice()
		: data{nullptr}, length{0} {}

	template<usize N>
	constexpr
	slice(array<T, N>& arr)
		: data{arr.raw_data()}, length{N} {}

	constexpr
	slice(slice const& s)
		: data{s.data}, length{s.length} {}

	constexpr
	void operator=(slice const& s){
		data = s.data;
		length = s.length;
	}

	constexpr
	slice(slice&& s){
		data   = x::exchange(s.data, nullptr);
		length = x::exchange(s.length, 0);
	}

	constexpr
	void operator=(slice&& s){
		data   = x::exchange(s.data, nullptr);
		length = x::exchange(s.length, 0);
	}

	constexpr
	slice<T> sub(usize start, usize end) const {
		bounds_check(end <= length);
		bounds_check(start <= length);
		if(start >= end){ return slice<T>(); }

		return slice<T>(&data[start], end - start);
	}

	struct Iterator {
		constexpr
		Iterator(T* data)
			: ptr_val(data) {}
		constexpr
		T& operator*() const { return *ptr_val; }
		constexpr
		T* operator->() const { return ptr_val; }
		constexpr
		auto& operator++(){ ptr_val += 1; return *this; }
		constexpr
		bool operator==(Iterator const& it){ return ptr_val == it.ptr_val; }
		constexpr
		bool operator!=(Iterator const& it){ return ptr_val != it.ptr_val; }
	private:
		T* ptr_val;
	};

	struct ConstIterator {
		constexpr
		ConstIterator(T const* data)
			: ptr_val(data) {}
		T const& operator*() const { return *ptr_val; }
		T* operator->() const { return ptr_val; }
		constexpr
		auto& operator++(){ ptr_val += 1; return *this; }
		constexpr
		bool operator==(ConstIterator const& it){ return ptr_val == it.ptr_val; }
		constexpr
		bool operator!=(ConstIterator const& it){ return ptr_val != it.ptr_val; }

	private:
		T const* ptr_val;
	};
	constexpr
	auto begin(){
		return Iterator(data);
	};

	constexpr
	auto end(){
		return Iterator(&data[length]);
	};

	constexpr
	auto begin() const {
		return ConstIterator(data);
	};

	constexpr
	auto end() const {
		return ConstIterator(&data[length]);
	};

	T* data;
	usize length;
};

// A view is slice whose memory is read-only.
template<typename T>
struct view {
	constexpr
	usize size() const {
		return length;
	}

	constexpr
	T const* raw_data() const {
		return &data[0];
	}

	constexpr
	T const& operator[](usize idx) const {
		bounds_check(idx < length);
		return data[idx];
	}

	constexpr
	bool empty() const {
		return (data == nullptr) || (length == 0);
	}

	// Implicit comparison is not allowed.
	bool operator==(slice<T> const&) = delete;

	constexpr
	view(T const* ptr, usize length)
		: data{ptr}, length{length} {}

	constexpr
	view()
		: data{nullptr}, length{0} {}

	template<usize N>
	constexpr
	view(array<T, N>& arr)
		: data{arr.raw_data()}, length{N} {}

	constexpr
	view(slice<T> const& s)
		: data{s.data}, length{s.length} {}

	constexpr
	view(view const& v)
		: data{v.data}, length{v.length} {}

	constexpr
	void operator=(view const& v){
		data = v.data;
		length = v.length;
	}

	constexpr
	view(view&& v){
		data   = x::exchange(v.data, nullptr);
		length = x::exchange(v.length, 0);
	}

	constexpr
	void operator=(view&& v){
		data   = x::exchange(v.data, nullptr);
		length = x::exchange(v.length, 0);
	}

	constexpr
	view<T> sub(usize start, usize end) const {
		bounds_check(end <= length);
		bounds_check(start <= length);
		if(start >= end){ return view<T>(); }

		return view<T>(&data[start], end - start);
	}

	struct ConstIterator {
		constexpr
		ConstIterator(T const* data)
			: ptr_val(data) {}
		T const& operator*() const { return *ptr_val; }
		T* operator->() const { return ptr_val; }
		constexpr
		auto& operator++(){ ptr_val += 1; return *this; }
		constexpr
		bool operator==(ConstIterator const& it){ return ptr_val == it.ptr_val; }
		constexpr
		bool operator!=(ConstIterator const& it){ return ptr_val != it.ptr_val; }

	private:
		T const* ptr_val;
	};

	constexpr
	auto begin() const {
		return ConstIterator(data);
	};

	constexpr
	auto end() const {
		return ConstIterator(&data[length]);
	};

	T const* data;
	usize length;
};

template<typename T>
constexpr inline
bool slice_equal(slice<T> const& l, slice<T> const& r){
	return slice_equal(view(l), view(r));
}

template<typename T>
constexpr
bool slice_equal(view<T> const& l, view<T> const& r){
	if(l.size() != r.size()){
		return false;
	}

	auto ld = l.raw_data();
	auto rd = r.raw_data();
	for(usize i = 0; i < l.size(); i += 1){
		if(ld[i] != rd[i]){
			return false;
		}
	}
	return true;
}

}

#endif /* Include guard */
// Primitive Types: Matrix /////////////////////////////////////////////////////
#ifndef _matrix_hpp_include_
#define _matrix_hpp_include_
namespace x {
// Primitive NxN matrix, stored column-wise
template<typename T, usize N>
struct matrix {
	static constexpr usize max_size = 8;
	using row_vector = array<T, N * N>;
	using column = array<T, N>;

	array<column, N> data;

	constexpr
	matrix()
		: data{} {}

	constexpr
	matrix(row_vector const& rows){
		column buf;
		for(usize i = 0; i < N; i += 1){
			for(usize j = 0; j < N; j += 1){
				buf[j] = rows[i + j * N];
			}
			data[i] = buf;
		}
	}

	constexpr
	column col(usize idx) const {
		return data[idx];
	}

	constexpr
	column row(usize idx) const {
		column c;
		for(usize i = 0; i < N; i += 1){
			c[i] = data[i][idx];
		}
		return c;
	}

	constexpr
	column& operator[](usize idx){
		return data[idx];
	}

	constexpr
	T& at(usize row, usize col){
		return (data[col])[row];
	}

	constexpr
	T const& at(usize row, usize col) const {
		return (data[col])[row];
	}

	constexpr
	auto begin(){ return data.begin(); }

	constexpr
	auto end(){ return data.end(); }

	constexpr
	auto begin() const { return data.begin(); }

	constexpr
	auto end() const { return data.end(); }

	static_assert(N <= max_size, "Matrix is too big");
};

template<typename T, usize N>
constexpr
bool matrix_equal(matrix<T, N> const& a, matrix<T, N> b){
	for(usize i = 0; i < N; i += 1){
		if(!array_equal(a.col(i), b.col(i))){ return false; }
	}
	return true;
}

template<typename T, usize N>
constexpr
matrix<T, N> transpose(matrix<T, N> const& mat){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){
		for(usize j = 0; j < N; j += 1){
			res.at(j, i) = mat.at(i, j);
		}
	}
	return res;
}

template<typename T, usize N>
constexpr static
matrix<T, N> hadamard_product(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){
		res[i] = a[i] * b[i];
	}
	return res;
}

template<typename T, usize N>
constexpr static
matrix<T, N> operator*(matrix<T, N> const& a, matrix<T, N> const& b){
	constexpr
	auto sum = [](array<T, N> const& a) -> T {
		auto acc = T{0};
		for(usize i = 0; i < N; i += 1){
			acc += a[i];
		}
		return acc;
	};

	matrix<T, N> res;

	for(usize i = 0; i < N; i += 1){
		for(usize j = 0; j < N; j += 1){
			res.at(i, j) = sum(a.row(i) * b.col(j));
		}
	}

	return res;
}

// Column vector
template<typename T, usize N>
constexpr static
array<T, N> operator*(matrix<T, N> const& a, array<T, N> const& b){
	constexpr
	auto sum = [](array<T, N> const& a) -> T {
		auto acc = T{0};
		for(usize i = 0; i < N; i += 1){
			acc += a[i];
		}
		return acc;
	};

	array<T, N> res;

	for(usize i = 0; i < N; i += 1){
		for(usize j = 0; j < N; j += 1){
			res[j] = sum(a.row(i) * b[j]);
		}
	}

	return res;
}

// All operators below were auto-generated.

template<typename T, usize N>
constexpr static
matrix<T, N> operator+(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] + b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator-(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] - b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator/(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] / b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator%(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] % b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator&(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] & b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator|(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] | b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator^(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] ^ b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator<<(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] << b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator>>(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >> b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator+(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] + b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator-(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] - b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator/(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] / b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator%(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] % b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator&(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] & b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator|(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] | b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator^(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] ^ b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator<<(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] << b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator>>(matrix<T, N> const& a, T const& b){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >> b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator==(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] == b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator!=(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] != b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator>(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] > b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator<(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] < b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator>=(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >= b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator<=(matrix<T, N> const& a, matrix<T, N> const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] <= b[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator==(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] == b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator!=(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] != b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator>(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] > b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator<(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] < b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator>=(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] >= b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<bool, N> operator<=(matrix<T, N> const& a, T const& b){
	matrix<bool, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = a[i] <= b; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator-(matrix<T, N> const& a){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = - a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator~(matrix<T, N> const& a){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = ~ a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator!(matrix<T, N> const& a){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = ! a[i]; }
	return res;
}
template<typename T, usize N>
constexpr static
matrix<T, N> operator+(matrix<T, N> const& a){
	matrix<T, N> res;
	for(usize i = 0; i < N; i += 1){ res[i] = + a[i]; }
	return res;
}
}

#endif /* Include guard */
// Optional Type ///////////////////////////////////////////////////////////////
#ifndef _optional_hpp_include_
#define _optional_hpp_include_
namespace x {

template<typename T>
struct option {
	constexpr
	bool ok() const {
		return _has_value;
	}

	constexpr
	pair<T*, bool> raw_data() const& {
		return {&_data, _has_value};
	}

	constexpr
	option()
		: _has_value{false} {}

	constexpr
	option(T const& val)
		: _data(val), _has_value{true} {}

	constexpr
	option(T && val)
		: _data(x::move(val)), _has_value{true} {}

	constexpr
	void operator=(T const& val){
		if(!_has_value){
			construct(&_data, val);
		}
		else {
			_data = val;
		}
	}

	constexpr
	void operator=(T&& val){
		if(!_has_value){
			construct(_data, x::move(val));
		}
		else {
			_data = val;
		}
	}

	// Regular get
	constexpr
	T get() const& {
		panic_assert(_has_value, "get() from empty Option");
		return _data;
	}

	// Move get
	constexpr
	T get() && {
		panic_assert(_has_value, "get() from empty Option");
		auto tmp = x::move(_data);
		reset();
		return x::move(tmp);
	}

	constexpr
	void reset(){
		if(_has_value){
			destruct(_data);
		}
		_has_value = false;
	}

	constexpr
	~option(){
		reset();
	}

private:
	union {
		T _data;
	};
	bool _has_value;
};
}

#endif /* Include guard */
// Memory //////////////////////////////////////////////////////////////////////
#ifndef _memory_hpp_include_
#define _memory_hpp_include_
#if !USE_BUILTIN_MEM_FUNCTIONS
#include <cstring>
#endif

namespace x {
// Set nbytes of ptr to val
[[maybe_unused]] static
void* mem_set(void* ptr, u8 val, usize nbytes){
	#if USE_BUILTIN_MEM_FUNCTIONS
	__builtin_memset(ptr, val, nbytes);
	#else
	memset(ptr, val, nbytes);
	#endif
	return ptr;
}

template<typename T>
constexpr
bool valid_alignment(T n){
	return (n > 0) && ((n & (n - 1)) == 0);
}

// Copy nbytes from source to dest
[[maybe_unused]] static
void* mem_copy(void* dest, const void* source, usize nbytes){
	#if USE_BUILTIN_MEM_FUNCTIONS
	__builtin_memcpy(dest, source, nbytes);
	#else
	std::memcpy(dest, source, nbytes);
	#endif
	return dest;
}

// Copies slice using mem_copy, will not run copy/move assignment
template<typename T>
constexpr
slice<T> slice_raw_copy(slice<T> dest, slice<T> source, usize n){
	bounds_check((n <= dest.size()) && (n <= source.size()));
	mem_copy(dest.raw_data(), source.raw_data(), n * sizeof(T));
	return dest;
}

// Copies slice using mem_copy, will not run copy/move assignment
template<typename T>
constexpr
slice<T> slice_raw_copy(slice<T> dest, view<T> source, usize n){
	bounds_check((n <= dest.size()) && (n <= source.size()));
	mem_copy(dest.raw_data(), source.raw_data(), n * sizeof(T));
	return dest;
}

// Copies slice regularly, will run copy assignment
template<typename T>
constexpr
slice<T> slice_copy(slice<T> dest, slice<T> source){
	return slice_copy(dest, view(source));
}

// Copies slice regularly, will run copy assignment
template<typename T>
constexpr
slice<T> slice_copy(slice<T> dest, view<T> source){
	auto n = min(dest.size(), source.size());
	auto d = dest.raw_data();
	auto s = source.raw_data();

	for(usize i = 0; i < n; i += 1){
		d[i] = s[i];
	}
	return dest;
}

// Moves n elements of source to dest individually
template<typename T>
slice<T> slice_move(slice<T> dest, slice<T> source){
	auto n = min(dest.size(), source.size());
	auto d = dest.raw_data();
	auto s = source.raw_data();

	for(usize i = 0; i < n; i += 1){
		d[i] = x::move(s[i]);
	}

	return dest;
}

// Align an integer number to a particular alignment
template<typename T>
constexpr
T align_forward(T value, T align){
	T mod = value % align;

	if(mod > 0){
		value += (align - mod);
	}

	return value;
}

// Memory Allocator Interface
struct allocator {
	enum struct operation : u8 {
		Alloc,   // Allocate a zero-initialized block of memory
		Resize,  // Try to resize allocation in place
		Free,    // Deallocate a block of memory owned by allocator
		FreeAll, // Deallocate all blocks owned by allocator
	};

	enum struct error : u8 {
		None = 0,
		OutOfMemory,
		NotOwnedPointer,
		AlignmentError,
		CannotResize,
		CannotFree,
		UnsupportedOperation,
		CustomError, // Cast to implementation's error type to get value.
	};

	using proc = pair<void*, error> (*) (
		void* impl,
		operation operation,
		usize new_size,
		usize align,
		void* old_ptr,
		usize old_size,
		source_location const& caller_location
	);

	constexpr
	auto procedure_ptr() const {
		return proc_;
	}

	constexpr
	auto impl_data() const {
		return impl_data_;
	}

	// Raw allocator procedure
	pair<void*, error> procedure(
		operation operation,
		usize new_size,
		usize align,
		void * old_ptr,
		usize old_size,
		Caller_Location
	){
		return proc_(
			impl_data_,
			operation,
			new_size,
			align,
			old_ptr,
			old_size,
			caller_location
		);
	}

	auto alloc(usize size, usize align, Caller_Location){
		return procedure(operation::Alloc, size, align, nullptr, 0, caller_location);
	}

	auto resize(
		void* old_ptr,
		usize new_size,
		usize old_size,
		Caller_Location
	){
		return procedure(
			operation::Resize,
			new_size,
			0,
			old_ptr,
			old_size,
			caller_location);
	}

	auto free(void* ptr, Caller_Location){
		auto [_, err] = procedure(operation::Free, 0, 0, ptr, 0, caller_location);
		return err;
	}

	bool free_all(Caller_Location){
		auto [_, err] = procedure(
			operation::FreeAll,
			0,
			0,
			nullptr,
			0,
			caller_location);
		return error_ok(err);
	}

	constexpr
	allocator(){}

	constexpr
	allocator(void* impl, proc proc) : impl_data_{impl}, proc_{proc} {}

private:
	void* impl_data_ = nullptr;
	proc proc_ = nullptr;
};

// Allocate a particular type and run its constructor with provided
// arguments (forced error check)
template<typename T, typename ...Args>
[[nodiscard]]
pair<T*, allocator::error> make_checked(
	allocator allocator,
	Args&& ...args,
	Caller_Location
){
	auto [raw, err] = allocator.alloc(sizeof(T), alignof(T), caller_location);
	auto ptr = static_cast<T*>(raw);

	if(error_ok(err)){
		new (ptr) T(x::forward<Args>(args)...);
	}
	else {
		ptr = nullptr;
	}

	return {ptr, err};
}

// Allocate a particular type and run its constructor with provided
// arguments
template<typename T, typename ...Args>
[[nodiscard]]
T* make(allocator allocator, Args&& ...args, Caller_Location){
	auto [ptr, _] = make_checked<T>(
		allocator,
		x::forward<Args>(args)...,
		caller_location);
	return ptr;
}

// Allocator a slice of a particular type and run constructor on each element
// with provided arguments (forced error checking)
template<typename T, typename ...Args>
[[nodiscard]]
pair<slice<T>, allocator::error> make_slice_checked(
	allocator allocator,
	usize count,
	Args&&... args,
	Caller_Location
){
	[[unlikely]] if(count == 0){
		return {slice<T>(), allocator::error::None};
	}

	auto [raw, err] = allocator.alloc(
		sizeof(T) * count,
		alignof(T),
		caller_location);
	T* ptr = static_cast<T*>(raw);

	if(error_ok(err)){
		for(usize i = 0; i < count; i += 1){
			new (&ptr[i]) T(args...);
		}
	}
	else {
		ptr = nullptr;
		count = 0;
	}

	auto sl = slice<T>(ptr, count);
	return {sl, err};
}

// Allocator a slice of a particular type and run constructor on each element
// with provided arguments
template<typename T, typename ...Args>
[[nodiscard]] constexpr
slice<T> make_slice(
	allocator allocator,
	usize count,
	Args&& ...args,
	Caller_Location
){
	auto [ptr, _] = make_slice_checked<T>(
		allocator,
		count,
		x::forward<Args>(args)...,
		caller_location
	);
	return ptr;
}

// Allocate a particular type, don't run constructor.
template<typename T>
[[nodiscard]]
pair<T*, allocator::error> make_raw(allocator allocator, Caller_Location){
	auto [raw, err] = allocator.alloc(sizeof(T), alignof(T), caller_location);
	T* ptr = static_cast<T*>(raw);
	return {ptr, err};
}

template<typename T>
[[nodiscard]]
pair<slice<T>, allocator::error> make_slice_raw(
	allocator allocator,
	usize count,
	Caller_Location
){
	[[unlikely]] if(count == 0){
		return {slice<T>(), allocator::error::None};
	}

	auto [raw, err] = allocator.alloc(
		sizeof(T) * count,
		alignof(T),
		caller_location);
	T* ptr = static_cast<T*>(raw);

	if(!error_ok(err)){
		ptr = nullptr;
		count = 0;
	}

	auto sl = slice<T>(ptr, count);
	return {sl, err};
}

// Run object's destructor and call allocator.free() on it.
template<typename T>
void destroy(allocator allocator, T* obj){
	if(obj != nullptr){
		obj->~T();
		allocator.free(obj);
	}
}

// Run each item's destructor and call allocator.free() on it.
template<typename T>
void destroy(allocator allocator, slice<T> slice){
	bool ok = !slice.empty();
	if(ok){
		T* raw = slice.raw_data();
		auto n = slice.size();
		for(usize i = 0; i < n; i += 1){
			raw[i].~T();
		}
		allocator.free(raw);
	}
}

#undef USE_BUILTIN_MEM_FUNCTIONS
}

#endif /* Include guard */
// String //////////////////////////////////////////////////////////////////////
#ifndef _string_hpp_include_
#define _string_hpp_include_
namespace x {
// Limit cstrings to be around 4 billion chars
constexpr usize MAX_CSTR_LEN = 0xffff'ffff;

constexpr
usize cstr_len(const char* const s){
	usize n = 0;
	for(usize i = 0; i < MAX_CSTR_LEN; i += 1){
		if(s[i] == 0){ break; }
		n += 1;
	}
	return n;
}

constexpr rune RUNE_ERROR = 0xfffd;

// NOTE: Inclusive ranges.
constexpr auto RANGE_1 = array<u32, 2>{0x0000, 0x00007f};
constexpr auto RANGE_2 = array<u32, 2>{0x0080, 0x0007ff};
constexpr auto RANGE_3 = array<u32, 2>{0x0800, 0x00ffff};
constexpr auto RANGE_4 = array<u32, 2>{0x1000, 0x10ffff};

constexpr auto MASK_X = u8(0b11'000000);
constexpr auto MASK_2 = u8(0b111'00000);
constexpr auto MASK_3 = u8(0b1111'0000);
constexpr auto MASK_4 = u8(0b11111'000);

constexpr static
u32 utf8_size_category(byte b){
	constexpr auto mask_ascii = u8(0b1000'0000);

	// ASCII
	if((b & mask_ascii) == 0){
		return 1;
	}

	if     ((b & MASK_2) == 0b110'00000){ return 2; }
	else if((b & MASK_3) == 0b1110'0000){ return 3; }
	else if((b & MASK_4) == 0b11110'000){ return 4; }
	else   { return 0; } // Continuation or invalid byte
}
// Decodes the first utf8 sequence from a slice of bytes. Returns codepoint and size.
static constexpr
pair<rune, u32> utf8_decode(view<byte> bytes){
	if(bytes.size() < 1){
		return {RUNE_ERROR, 0};
	}

	auto size = utf8_size_category(bytes[0]);
	if(bytes.size() < size){
		return {RUNE_ERROR, 0};
	}

	auto data = bytes.sub(0, size);
	rune r = RUNE_ERROR;

	switch(size){
		case 1: {
			r = rune(bytes[0]);
		} break;

		case 2: {
			auto b0 = rune(data[0] & ~MASK_2) << 6;
			auto b1 = rune(data[1] & ~MASK_X);
			r = b0 | b1;
		} break;

		case 3: {
			auto b0 = rune(data[0] & ~MASK_3) << 12;
			auto b1 = rune(data[1] & ~MASK_X) << 6;
			auto b2 = rune(data[2] & ~MASK_X);
			r = b0 | b1 | b2;
		} break;

		case 4: {
			auto b0 = rune(data[0] & ~MASK_4) << 18;
			auto b1 = rune(data[1] & ~MASK_X) << 12;
			auto b2 = rune(data[2] & ~MASK_X) << 6;
			auto b3 = rune(data[3] & ~MASK_X);
			r = b0 | b1 | b2 | b3;
		} break;
	}

	return {r, size};
}

static constexpr
pair<array<byte, 4>, u32> utf8_encode(rune r){
	array<byte, 4> bytes = {0};
	u32 size = 0;

	auto r1 = r >> 6;
	auto r2 = r >> 12;
	auto r3 = r >> 18;

	if(r <= RANGE_1[1]){
		size = 1;
		bytes[0] = u8(r);
	}
	else if(r <= RANGE_2[1]){
		size = 2;
		bytes[1] = byte(0b10'000000 | (r  & 0x3f));
		bytes[0] = byte(0b110'00000 | (r1 & 0x1f));
	}
	else if(r <= RANGE_3[1]){
		size = 3;
		bytes[2] = byte(0b10'000000 | (r  & 0x3f));
		bytes[1] = byte(0b10'000000 | (r1 & 0x3f));
		bytes[0] = byte(0b1110'0000 | (r2 & 0x0f));
	}
	else if(r <= RANGE_4[1]){
		size = 4;
		bytes[3] = byte(0b10'000000 | (r  & 0x3f));
		bytes[2] = byte(0b10'000000 | (r1 & 0x3f));
		bytes[1] = byte(0b10'000000 | (r2 & 0x3f));
		bytes[0] = byte(0b11110'000 | (r3 & 0x07));
	}

	return {bytes, size};
}

struct utf8_iterator {
	constexpr
	utf8_iterator(view<byte> data)
		: data{data}{}

	constexpr
	rune operator*() const {
		auto [r, _] = utf8_decode(data);
		return r;
	}

	constexpr
	auto& operator++(){
		auto size = utf8_size_category(data.raw_data()[0]);
		data = view<byte>(data.raw_data() + size, data.size() - size);
		return *this;
	}

	constexpr
	bool operator==(utf8_iterator const& it){ return data.raw_data() == it.data.raw_data(); }

	constexpr
	bool operator!=(utf8_iterator const& it){ return data.raw_data() != it.data.raw_data(); }

private:
	view<byte> data;
};

struct string {
public:
	constexpr
	usize size() const {
		return data.size();
	}

	constexpr
	usize rune_count() const {
		usize n = 0;
		for([[maybe_unused]] auto _ : *this){
			n += 1;
		}
		return n;
	}

	constexpr
	auto raw_data() const {
		return data.raw_data();
	}

	string clone(allocator allocator) const {
		auto buf = make_slice<byte>(allocator, size());
		mem_copy(buf.raw_data(), data.raw_data(), size());
		auto s = string(view(buf));
		return s;
	}

	constexpr
	string(char const* cstr){
		auto size = cstr_len(cstr);
		auto ptr = bit_cast<byte const*>(cstr);
		// NOTE: bit_cast is required to bypass the stupid C++ type
		// system because according to the ISO idiots casting an
		// unsigned char* to a char* is "unsafe".
		data = view(ptr, size);
	}

	constexpr
	string()
		: data{} {}

	constexpr
	string(string const& s)
		: data(s.data) {}

	constexpr explicit
	string(view<byte> raw_bytes)
		: data(raw_bytes) {}

	constexpr explicit
	string(view<char> raw_bytes)
		: data(x::bit_cast<const byte*>(raw_bytes.raw_data()), raw_bytes.size()) {}

	constexpr
	string(string && s)
		: data(x::move(s.data)) {}

	constexpr
	utf8_iterator begin() const {
		return utf8_iterator(data);
	}

	constexpr
	utf8_iterator end() const {
		return utf8_iterator(view(data.raw_data() + data.size(), 0));
	}
private:
	view<byte> data;
};

constexpr
bool operator==(string const& a, string const& b) {
	if(a.size() != b.size()){ return false; }
	for(usize i = 0; i < a.size(); i += 1){
		if(a.raw_data()[i] != b.raw_data()[i]){
			return false;
		}
	}
	return true;
}

constexpr
bool operator!=(string const& a, string const& b) {
	return !(a == b);
}
static inline
void destroy(allocator allocator, string s){
	void* ptr = (void*)s.raw_data();
	allocator.free(ptr);
}

// Quick conversion to cstring, use a temporary allocator for easy cleanup. This
// is mostly to make printing and interoping with C-libraries easier.
[[maybe_unused]] static 
char const* temp_cstring(string s, allocator temp_alloc){
	auto buf = make_slice<char>(temp_alloc, s.size() + 1);
	mem_copy(buf.raw_data(), s.raw_data(), s.size());
	buf[buf.size() - 1] = 0;
	return buf.raw_data();
}

struct string_builder {
};

}

#endif /* Include guard */
// Dynamic Array ///////////////////////////////////////////////////////////////
#ifndef _dynamic_array_hpp_include_
#define _dynamic_array_hpp_include_

#include <new>

namespace x {
template<typename T>
struct dynamic_array {
	static constexpr usize default_initial_capacity = 16;

	constexpr
	T& operator[](usize idx) {
		bounds_check(idx < length);
		return data.raw_data()[idx];
	}

	constexpr
	T const& operator[](usize idx) const {
		bounds_check(idx < length);
		return data.raw_data()[idx];
	}

	constexpr
	T* raw_data() const {
		return data.raw_data();
	}

	constexpr
	auto get_allocator() & {
		return backing_allocator;
	}

	constexpr
	usize size() const {
		return length;
	}

	constexpr
	usize capacity() const {
		return data.size();
	}

	constexpr
	bool empty() const {
		return (data.empty()) || (length == 0);
	}

	// Clear all elements, but retain capacity, this destroys all objects
	// in the array.
	constexpr
	void clear(){
		auto s = data.sub(0, length);
		for(auto& e : s){
			e.~T();
		}
		length = 0;
	}

	// Append item to end of array, returns updated length
	constexpr
	usize append(T const& e){
		if((length + 1) >= data.size()){
			auto err = resize_capacity((data.size() + 1) * 2);
			if(!error_ok(err)){ return length; }
		}
		new (&data[length]) T(e);
		length += 1;
		return length;
	}

	// Append item to end of array, returns updated length
	constexpr
	usize append(T&& e){
		if((length + 1) >= data.size()){
			auto err = resize_capacity((data.size() + 1) * 2);
			debug_assert(error_ok(err), "Failed to append");
			if(!error_ok(err)){ return length; }
		}
		new (&data[length]) T(x::move(e));
		length += 1;
		return length;
	}

	// Remove last item
	constexpr
	void pop(){
		if(length < 1){ return; }
		length -= 1;
		data[length].~T();
	}
	// Remove element at idx by swapping it with last element and deleting
	// it, this operation does not preserve the order of the array
	constexpr
	void remove_unordered(usize idx){
		bounds_check(idx < length);
		x::swap(data[idx], data[length - 1]);
		pop();
	}

	// Insert element at index, appends it and swaps last element with index,
	// this operation does not preserve the order of the array
	template<typename U>
	constexpr
	usize insert_unordered(U&& e, usize idx){
		bounds_check(idx < (length + 1));
		auto old = length;
		append(x::forward<U>(e));

		// Failed to append
		[[unlikely]] if (length == old){ return old; }

		swap(data[idx], data[length - 1]);
		return length;
	}

	template<typename U>
	constexpr
	usize insert_ordered(U&& e, usize idx){
		bounds_check(idx <= data.size());

		auto old = length;
		append(x::forward<U>(e));

		// Failed to append
		[[unlikely]] if (length == old){ return old; }

		auto raw = data.raw_data();
		for(usize i = idx; i < length - 1; i += 1){
			x::swap(raw[i], raw[length - 1]);
		}

		return length;
	}
	// Remove element at idx by shifiting later elements and deleting
	// it, this operation preserves the order of the array
	constexpr
	void remove_ordered(usize idx){
		bounds_check(idx < length);

		if(idx != (length - 1)){
			auto raw = data.raw_data();
			for(usize i = idx; i < (length - 1); i += 1){
				x::swap(raw[i], raw[i+1]);
			}
		}

		pop();
	}

	constexpr
	allocator::error resize_capacity(usize new_size){
		if(new_size == 0){
			clear();
			backing_allocator.free(data.raw_data());
			data = slice<T>{};
			return allocator::error::None;
		}

		// TODO: Use Allocator.resize() when possible
		auto [new_data, err] = make_slice_raw<T>(backing_allocator, new_size);
		Or_Return(err);

		if(new_size >= length){
			slice_raw_copy(new_data, data, length);
			destroy(backing_allocator, data);
			data = new_data;
		}
		else {
			// Remove excess elements
			auto diff = length - new_size;
			auto old_data = data.raw_data();
			for(usize i = 0; i < diff; i += 1){
				old_data[length - (i+1)].~T();
			}
			length = new_size;

			slice_raw_copy(new_data, data, new_size);
			destroy(backing_allocator, data);
			data = new_data;
		}

		return err;
	}

	constexpr
	dynamic_array<T> clone(x::allocator allocator){
		auto arr = dynamic_array<T>(allocator);
		auto err = arr.resize_capacity(length);
		if(!error_ok(err)){
			return arr;
		}
		slice_copy(arr.data, data);
		return arr;
	}

	constexpr
	dynamic_array(x::allocator allocator, usize cap = default_initial_capacity)
		: data{slice<T>()}, length{0}, backing_allocator{allocator}
	{
		resize_capacity(cap);
	}

	// Move storage slice out of dynamic_array, resetting its capacity
	[[nodiscard]]
	slice<T> extract_data(){
		resize_capacity(length);
		auto s = x::exchange(data, slice<T>());
		length = 0;
		return s;
	}

	dynamic_array(dynamic_array const&) = delete;
	void operator=(dynamic_array const&) = delete;

	dynamic_array(dynamic_array&& arr){
		data      = x::exchange(arr.data, slice<T>{});
		length    = x::exchange(arr.length, 0);
		backing_allocator = arr.backing_allocator;
	}

	void operator=(dynamic_array&& arr){
		destroy(backing_allocator, data);
		data      = x::exchange(arr.data, slice<T>{});
		length    = x::exchange(arr.length, 0);
		backing_allocator = arr.backing_allocator;
	}

	~dynamic_array(){
		clear();
		backing_allocator.free(data.raw_data());

		data = slice<T>{};
		length = 0;
	}

	constexpr
	auto begin(){
		return data.sub(0, length).begin();
	}

	constexpr
	auto end(){
		return data.sub(0, length).end();
	}

	constexpr
	auto begin() const {
		const auto s = data.sub(0, length);
		return s.begin();
	}

	constexpr
	auto end() const {
		const auto s = data.sub(0, length);
		return s.end();
	}

private:
	slice<T> data;
	usize length;
	x::allocator backing_allocator;
};

static_assert(dynamic_array<int>::default_initial_capacity > 1);
}

#endif /* Include guard */
// Stack ///////////////////////////////////////////////////////////////////////
#ifndef _stack_hpp_include_
#define _stack_hpp_include_
namespace x {

template<typename T>
struct stack {
	constexpr
	usize push(T const& val){
		return items.append(val);
	}

	constexpr
	usize push(T&& val){
		return items.append(x::move(val));
	}

	constexpr
	bool pop(){
		bool ok = items.size() > 0;
		items.pop();
		return ok;
	}

	constexpr
	bool empty() const {
		return items.empty();
	}

	constexpr
	bool size() const {
		return items.size();
	}

	constexpr
	T& top(){
		return items[items.size() - 1];
	}

	constexpr
	T const& top() const {
		return items[items.size() - 1];
	}

	stack(allocator al)
		: items(al) {}

private:
	dynamic_array<T> items;
};

}

#endif /* Include guard */
// Queue ///////////////////////////////////////////////////////////////////////
#ifndef _queue_hpp_include_
#define _queue_hpp_include_
namespace x {

template<typename T>
struct queue {
	static constexpr usize default_initial_capacity = 16;

	constexpr
	usize push(T const& val){
		if((length + 1) >= items.size()){
			bool ok = resize_capacity(max(length * 2, default_initial_capacity));
			if(!ok){ return length; }
		}

		usize idx = (first + length) % items.size();
		items[idx] = val;
		length += 1;
	}

	constexpr
	T& front(){
		bounds_check(length > 0);
		return items[first];
	}

	constexpr
	T const& front() const {
		bounds_check(length > 0);
		return items[first];
	}

	constexpr
	bool pop(){
		bool ok = length > 0;
		if(ok){
			destruct(front());
			first = (first + 1) % items.size();
		}
		return ok;
	}

	constexpr
	bool empty() const {
		return length == 0;
	}

	constexpr
	usize size() const {
		return length;
	}

	constexpr
	void clear(){}

	queue(allocator al, usize initial_cap = default_initial_capacity)
		: backing_allocator{al}, first{0}, length{0}, items{} {}

	bool resize_capacity(usize new_cap){
		unimplemented();
		debug_assert(new_cap > items.size(), "Queue does not support down-sizing");
	}

private:
	usize first;
	usize length;
	slice<T> items;
	x::allocator backing_allocator;
};

static_assert(queue<int>::default_initial_capacity > 1);
}

#endif /* Include guard */
// Bump Allocator //////////////////////////////////////////////////////////////
#ifndef _bump_allocator_hpp_include_
#define _bump_allocator_hpp_include_
namespace x {

// A bump allocator is a simple allocator that manages a contigous region
// of memory, it cannot free individual allocations, but it can release
// all of its memory at once.
struct bump_allocator {
	using error = allocator::error;

	constexpr
	usize available_space() const {
		return data_.size() - usize(offset_);
	}

	constexpr
	usize size() const {
		return data_.size();
	}

	bool owns_ptr(void const * ptr) const {
		auto p = uintptr(ptr);
		auto base = uintptr(data_.raw_data());
		auto limit = base + size();
		return (p >= base) && (p < limit);
	}

	pair<void*, error> alloc(usize size, usize align){
		auto [ptr, err] = alloc_undef(size, align);
		if(error_ok(err)){
			mem_set(ptr, 0, size);
		}

		return {ptr, err};
	}

	pair<void*, error> alloc_undef(usize size, usize align){
		panic_assert(valid_alignment(align), "Invalid memory aligment");
		auto err = error::None;
		uintptr base = uintptr(data_.raw_data());
		uintptr start = base + offset_;
		uintptr pad = align_forward(start, align) - start;
		uintptr required = offset_ + pad + size;

		if(required > available_space()){
			err = error::OutOfMemory;
			return {nullptr, err};
		}

		auto allocation = (void*)(start + pad);

		offset_ += pad + size;
		last_allocation_ = allocation;
		last_size_ = size;

		return {allocation, err};
	}

	error free(void* ptr){
		if(ptr != last_allocation_){
			return error::CannotFree;
		}
		auto last = uintptr(last_allocation_);
		auto base = uintptr(data_.raw_data());

		offset_ = last - base;
		last_allocation_ = nullptr;
		last_size_ = 0;
		return error::None;
	}

	pair<void*, error> resize(void* ptr, usize new_size){
		if(ptr != last_allocation_){
			return {nullptr, error::CannotResize};
		}

		auto base     = uintptr(data_.raw_data());
		auto last     = uintptr(last_allocation_);
		auto required = (last - base) + new_size;

		auto last_offset = base - last;
		if(required > (data_.size() - last_offset)){
			return {nullptr, error::OutOfMemory};
		}

		// Fill extra memory with 0's
		if(new_size > last_size_){
			auto diff = new_size - last_size_;
			void* start = (void*)(last + last_size_);
			mem_set(start, 0, diff);
		}

		offset_ = last - base + new_size;
		last_size_ = new_size;

		return {ptr, error::None};
	}

	void reset(){
		offset_ = 0;
		last_allocation_ = nullptr;
		last_size_ = 0;
	}

	// Change allocator's storage slice.
	void set_storage(slice<u8> data){
		reset();
		data_ = data;
	}

	// Expose BumpAllocator as an Allocator
	constexpr
	x::allocator as_allocator() &;

	constexpr
	bump_allocator(){}

	constexpr
	bump_allocator(bump_allocator const& bp) = default;

	constexpr
	bump_allocator(slice<u8> data)
		: data_(data) {}

	constexpr
	bump_allocator(bump_allocator&& bp)
		: data_(x::move(bp.data_))
	{
		offset_          = x::exchange(bp.offset_, 0);
		last_allocation_ = x::exchange(bp.last_allocation_, nullptr);
		last_size_       = x::exchange(bp.last_size_, 0);
	}

	constexpr
	void operator=(bump_allocator && bp){
		data_            = x::exchange(bp.data_, slice<u8>{});
		offset_          = x::exchange(bp.offset_, 0);
		last_allocation_ = x::exchange(bp.last_allocation_, nullptr);
		last_size_       = x::exchange(bp.last_size_, 0);
	}

	slice<u8> data_ = {};
	uintptr offset_ = 0;
	void* last_allocation_ = nullptr;
	usize last_size_ = 0;
};

[[maybe_unused]] static
pair<void*, allocator::error> _bump_allocator_proc (
	void* impl,
	allocator::operation operation,
	usize new_size,
	usize align,
	void* old_ptr,
	usize,
	Caller_Location
){
	using Op = allocator::operation;

	auto bump_alloc = static_cast<bump_allocator*>(impl);
	auto err = allocator::error::None;

	switch (operation) {
		case Op::Alloc: {
			auto [ptr, err] = bump_alloc->alloc(new_size, align);
			return {ptr, err};
		} break;

		case Op::Resize: {
			auto [ptr, err] = bump_alloc->resize(old_ptr, new_size);
			return {ptr, err};
		} break;

		case Op::Free: {
			err = bump_alloc->free(old_ptr);
		} break;

		case Op::FreeAll: {
			bump_alloc->reset();
		} break;

		default: {
			err = allocator::error::UnsupportedOperation;
			unreachable();
		} break;
	}

	return {nullptr, err};
}

constexpr inline 
x::allocator bump_allocator::as_allocator() & {
	return x::allocator((void*)(this), _bump_allocator_proc);
}
}
#endif /* Include guard */
// Standard Heap Allocator /////////////////////////////////////////////////////
#ifndef _std_heap_allocator_hpp_include_
#define _std_heap_allocator_hpp_include_
namespace x {
[[maybe_unused]] static
pair<void*, allocator::error> _std_heap_alloc_proc(
	void*,
	allocator::operation op,
	usize size,
	usize align,
	void* ptr,
	usize,
	Caller_Location
){
	using O = allocator::operation;

	auto err = allocator::error::None;

	// Note: Allocating with a particular alignment in C++ is very annoying, so we over-allocate a bit to make sure
	switch(op){
		case O::Alloc: {
			void* raw      = new u8[align_forward(size, align)];
			auto unaligned = uintptr(raw);
			auto aligned   = align_forward(unaligned, uintptr(align));
			if(raw == nullptr){
				err = allocator::error::OutOfMemory;
			}
			// if(aligned != unaligned){
			// 	err = allocator::error::AlignmentError;
			// 	delete[] (u8*)raw;
			// 	raw = nullptr;
			// }
			return {(void*)(aligned), err};
		} break;

		case O::Free: {
			delete[] (u8*)ptr;
			return {nullptr, err};
		} break;

		case O::Resize: {
			err = allocator::error::UnsupportedOperation;
			return {nullptr, err};
		} break;

		case O::FreeAll: {
			err = allocator::error::UnsupportedOperation;
			return {nullptr, err};
		} break;

		default: {
			err = allocator::error::UnsupportedOperation;
			unreachable();
		} break;
	}

	return {nullptr, err};
}

[[maybe_unused]] static
allocator std_heap_allocator(){
	return allocator(nullptr, _std_heap_alloc_proc);
}
}
#endif /* Include guard */
// Arena Allocator /////////////////////////////////////////////////////////////
#ifndef _arena_allocator_hpp_include_
#define _arena_allocator_hpp_include_
namespace x {
// An arena is a series of chained bump_allocator's, it uses another allocator
// for requesting extra memory (usually the heap allocator). An arena can free
// all of its contents at once, and grow dynamically.
struct arena_allocator {
	using error = allocator::error;
	static constexpr usize default_min_pool_size = 128;
	enum struct reset_mode {
		RetainCapacity,
		DeallocAll,
	};

	allocator backing_alloc = {};
	dynamic_array<bump_allocator> mem_pools;

	pair<void*, error> alloc(usize size, usize align, Caller_Location){
		panic_assert(valid_alignment(align), "Bad alignment", caller_location);
		if(size == 0){ return {0, error::None }; }

		for(auto& pool : mem_pools){
			auto [ptr, err] = pool.alloc(size, align);
			if(error_ok(err)){
				return {ptr, error::None};
			}
		}

		// No pool available, create new one
		auto err = create_new_pool(size, align);
		if(!error_ok(err)){
			return {nullptr, err};
		}

		return mem_pools[mem_pools.size() - 1].alloc(size, align);
	}

	pair<void*, error> resize(void* ptr, usize new_size, Caller_Location){
		auto i = find_ptr_pool(ptr);
		if(i < 0){
			return {nullptr, error::NotOwnedPointer};
		}
		return mem_pools[i].resize(ptr, new_size);
	}

	isize find_ptr_pool(void* ptr) const {
		isize i = 0;
		for(auto const& pool : mem_pools){
			if(pool.owns_ptr(ptr)){
				return i;
			}
			i += 1;
		}
		return -1;
	}

	void reset(reset_mode policy = reset_mode::RetainCapacity){
		switch (policy) {
			case reset_mode::RetainCapacity: {
				for(auto& pool : mem_pools){
					pool.reset();
				}
			} break;

			case reset_mode::DeallocAll: {
				for(auto& pool : mem_pools){
					auto storage = x::move(pool.data_);
					backing_alloc.free(storage.raw_data());
				}
				mem_pools.clear();
			} break;
		}

	}

	error create_new_pool(usize size, usize align, Caller_Location) {
		// Make damn sure there's enough size.
		auto size_aligned      = align_forward(size, align);
		auto min_pool_aligned  = align_forward(default_min_pool_size, align);
		auto real_size         = max(size_aligned, min_pool_aligned);

		auto [data, err] = make_slice_checked<u8>(backing_alloc, real_size);

		if(!error_ok(err)){
			return err;
		}

		auto bp = bump_allocator(data);
		mem_pools.append(bp);

		return error::None;
	}

	constexpr
	x::allocator as_allocator() &;

	arena_allocator(x::allocator backing)
		: backing_alloc{backing},
		mem_pools(backing) {}

	~arena_allocator(){
		reset(reset_mode::DeallocAll);
	}
};

[[maybe_unused]] static
pair<void*, allocator::error> _arena_allocator_proc (
	void* impl,
	allocator::operation operation,
	usize new_size,
	usize align,
	void* old_ptr,
	usize,
	Caller_Location
){
	using Op = allocator::operation;
	auto arena = static_cast<arena_allocator*>(impl);
	auto err = arena_allocator::error::None;

	switch (operation) {
		case Op::Alloc: {
			return arena->alloc(new_size, align, caller_location);
		} break;

		case Op::Resize: {
			return arena->resize(old_ptr, new_size, caller_location);
		} break;

		case Op::Free: {
			err = arena_allocator::error::UnsupportedOperation;
		} break;

		case Op::FreeAll: {
			arena->reset();
		} break;

		default:{
			err = arena_allocator::error::UnsupportedOperation;
			unreachable();
		} break;
	}

	return {nullptr, err};
}

inline constexpr
x::allocator arena_allocator::as_allocator() & {
	return x::allocator(this, _arena_allocator_proc);
}

}
#endif /* Include guard */
// Tracking Allocator //////////////////////////////////////////////////////////
#ifndef _tracking_allocator_hpp_include_
#define _tracking_allocator_hpp_include_
namespace x {
struct tracking_allocator {
	using error = allocator::error;

	struct allocation_entry {
		void* ptr;
		usize size;
		source_location where;
	};

	dynamic_array<allocation_entry> allocations;
	x::allocator backing_alloc;
	source_location created_at;

	tracking_allocator(x::allocator alloc, Caller_Location)
		: allocations(alloc),
		backing_alloc(alloc),
		created_at(caller_location)
	{
			debug_assert(
				alloc.procedure_ptr() != nullptr,
				"Null procedure is not allowed.");
	}

	constexpr
	x::allocator as_allocator() &;

	bool report(){
		std::printf("Testing allocator report (created at %s:%d):\n",
			  created_at.file_name(),
			  created_at.line());

		if(allocations.size() > 0){
			std::printf("Leaked allocations:\n");
			for(auto const& a : allocations){
				std::printf("  %p: %zuB from %s:%d\n",
					a.ptr,
					a.size,
					a.where.file_name(),
					a.where.line());
			}
			return false;
		}
		else {
			std::printf("  No leaks found.\n");
			return true;
		}
	}

	pair<void*, error> alloc(usize new_size, usize align, Caller_Location){
		auto [ptr, err] = backing_alloc.alloc(
			new_size,
			align,
			caller_location);

		if(!error_ok(err)){
			ptr = nullptr;
		}
		else {
			allocations.append(allocation_entry{
				.ptr = ptr,
				.size = new_size,
				.where = caller_location,
			});
		}
		return {ptr, err};
	}

	pair<void*, error> resize(
		usize new_size,
		void* old_ptr,
		usize old_size,
		Caller_Location
	){
		auto [ptr, err] = backing_alloc.resize(old_ptr, new_size, old_size);
		if(!error_ok(err)){
			return {nullptr, err};
		}
		else {
			for(auto& alloc : allocations){
				if(alloc.ptr == old_ptr){
					alloc.size = new_size;
					break;
				}
			}
		}
		return {ptr, err};
	}

	error free(void* old_ptr, Caller_Location){
		auto err = backing_alloc.free(old_ptr, caller_location);
		// Even if the operation is not supported or couldn't be
		// completed, the allocation is still marked as freed, this is
		// mostly for consistency with arena-style allocators.
		bool ok = (err == error::None) ||
			(err == error::UnsupportedOperation) ||
			(err == error::CannotFree);

		if(ok) {
			for(usize i = 0; i < allocations.size(); i += 1){
				auto const& alloc = allocations[i];
				if(alloc.ptr == old_ptr){
					allocations.remove_unordered(i);
					return err;
				}
			}
		}
		return err;
	}

	error free_all(Caller_Location){
		auto ok = backing_alloc.free_all(caller_location);
		if(error_ok(ok)){
			allocations.clear();
		}
		return ok ? error::None : error::UnsupportedOperation;
	}
	~tracking_allocator(){}
};

[[maybe_unused]] static
pair<void*, allocator::error> _tracking_allocator_proc(
	void* impl,
	allocator::operation op,
	usize new_size,
	usize align,
	void* old_ptr,
	usize old_size,
	Caller_Location
){
	using O = allocator::operation;
	using error = allocator::error;

	auto talloc = (tracking_allocator*)(impl);

	switch (op) {
		case O::Alloc: {
			return talloc->alloc(new_size, align, caller_location);
		} break;

		case O::Resize: {
			return talloc->resize(
				new_size,
				old_ptr,
				old_size,
				caller_location);
		} break;

		case O::Free: {
			return {nullptr, talloc->free(old_ptr, caller_location) };
		};

		case O::FreeAll: {
			return {nullptr, talloc->free_all(caller_location)};
		} break;
	}

	return {nullptr, error::UnsupportedOperation};
}

constexpr inline
x::allocator tracking_allocator::as_allocator() & {
	return x::allocator(this, _tracking_allocator_proc);
}
}
#endif /* Include guard */
/*
Copyright marcs-feh 2024

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#endif /* Include guard */

