#ifndef OBJECT_OPS_HPP
#define OBJECT_OPS_HPP

#include "object.hpp"
#include <cassert>
#include <ostream>
#include <stdexcept>

namespace matlang {
namespace sfinae {
template <typename T, typename = void> struct is_container {
	constexpr static bool value = false;
};
template <typename T>
struct is_container<
    T, std::enable_if_t<std::is_same_v<typename T::value_type, object>>> {
	constexpr static bool value = true;
};
template <typename T> constexpr bool is_container_v = is_container<T>::value;

template <typename T, typename = void> struct is_object {
	constexpr static bool value = false;
};
template <typename T>
struct is_object<T,
                 std::enable_if_t<is_container_v<typename T::container_impl>>> {
	constexpr static bool value = true;
};
template <typename T> constexpr bool is_object_v = is_object<T>::value;
} // namespace sfinae

//+=
namespace ops_impl {
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> && sfinae::is_container_v<U>, T> &
operator+=(T &l, const U &r) {
	if (l.size() != r.size())
		throw std::invalid_argument{"size mismatch"};
	auto i = l.begin();
	auto j = r.begin();
	for (; i != l.end(); i++, j++)
		*i += *j;
	return l;
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> != sfinae::is_container_v<U> &&
                     !sfinae::is_object_v<T> && !sfinae::is_object_v<U>,
                 T> &
operator+=(T &l, const U &r) {
	throw std::invalid_argument{"size mismatch"};
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, T> &
operator+=(T &l, const U &r) {
	l.visit([&](auto &underlaying) { underlaying += r; });
	return l;
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_object_v<T> && sfinae::is_object_v<U>, T> &
operator+=(T &l, const U &r) {
	r.visit([&](auto &underlaying) { l += underlaying; });
	return l;
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, T> &
operator+=(T &l, const U &r) {
	using ops_impl::operator+=;
	r.visit([&](auto &underlaying) { l += underlaying; });
	return l;
}

//+
namespace ops_impl {
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> && sfinae::is_container_v<U>, object>
operator+(const T &l, const U &r) {
	object::container_impl result(l.begin(), l.end());
	result += r;
	return object(result);
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> != sfinae::is_container_v<U> &&
                     !sfinae::is_object_v<T> && !sfinae::is_object_v<U>,
                 object>
operator+(const T &l, const U &r) {
	throw std::invalid_argument{"size mismatch"};
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, object>
operator+(const T &l, const U &r) {
	return l.visit([&](auto &underlaying) { return object(underlaying + r); });
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator+(const T &l, const U &r) {
	return r.visit([&](auto &underlaying) { return object(l + underlaying); });
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator+(const T &l, const U &r) {
	using ops_impl::operator+;
	return r.visit([&](auto &underlaying) { return object(l + underlaying); });
}

//-=
namespace ops_impl {
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> && sfinae::is_container_v<U>, T> &
operator-=(T &l, const U &r) {
	if (l.size() != r.size())
		throw std::invalid_argument{"size mismatch"};
	auto i = l.begin();
	auto j = r.begin();
	for (; i != l.end(); i++, j++)
		*i -= *j;
	return l;
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> != sfinae::is_container_v<U> &&
                     !sfinae::is_object_v<T> && !sfinae::is_object_v<U>,
                 T> &
operator-=(T &l, const U &r) {
	throw std::invalid_argument{"size mismatch"};
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, T> &
operator-=(T &l, const U &r) {
	l.visit([&](auto &underlaying) { underlaying -= r; });
	return l;
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_object_v<T> && sfinae::is_object_v<U>, T> &
operator-=(T &l, const U &r) {
	r.visit([&](auto &underlaying) { l -= underlaying; });
	return l;
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, T> &
operator-=(T &l, const U &r) {
	using ops_impl::operator-=;
	r.visit([&](auto &underlaying) { l -= underlaying; });
	return l;
}

//-
namespace ops_impl {
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> && sfinae::is_container_v<U>, object>
operator-(const T &l, const U &r) {
	object::container_impl result(l.begin(), l.end());
	result -= r;
	return object(result);
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> != sfinae::is_container_v<U> &&
                     !sfinae::is_object_v<T> && !sfinae::is_object_v<U>,
                 object>
operator-(const T &l, const U &r) {
	throw std::invalid_argument{"size mismatch"};
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, object>
operator-(const T &l, const U &r) {
	return l.visit([&](auto &underlaying) { return object(underlaying - r); });
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator-(const T &l, const U &r) {
	return r.visit([&](auto &underlaying) { return object(l - underlaying); });
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator-(const T &l, const U &r) {
	using ops_impl::operator-;
	return r.visit([&](auto &underlaying) { return object(l - underlaying); });
}

//*
namespace ops_impl {
template <typename T, typename U>
std::enable_if_t<sfinae::is_container_v<T> && !sfinae::is_object_v<U>, object>
operator*(const T &l, const U &r) {
	object::container_impl result(l.begin(), l.end());
	for (auto &a : result)
		a = a * r;
	return object(result);
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_container_v<T> && sfinae::is_container_v<U> &&
                     !sfinae::is_object_v<T>,
                 object>
operator*(const T &l, const U &r) {
	object::container_impl result(r.begin(), r.end());
	for (auto &a : result)
		a = l * a;
	return object(result);
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, object>
operator*(const T &l, const U &r) {
	return l.visit([&](auto &underlaying) { return object(underlaying * r); });
}
template <typename T, typename U>
std::enable_if_t<!sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator*(const T &l, const U &r) {
	return r.visit([&](auto &underlaying) { return object(l * underlaying); });
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, object>
operator*(const T &l, const U &r) {
	using ops_impl::operator*;
	return r.visit([&](auto &underlaying) { return object(l * underlaying); });
}

//*=
namespace ops_impl {
template <typename T, typename U, typename Unp>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U> &&
                 !sfinae::is_container_v<U> && !sfinae::is_container_v<Unp>>
update(T &l, const U &r, Unp &unpacked) {
	unpacked = r;
}
template <typename T, typename U, typename Unp>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U> &&
                 sfinae::is_container_v<U> != sfinae::is_container_v<Unp> &&
                 !std::is_same_v<T, object>>
update(T &l, const U &r, Unp &unpacked) {}
template <typename T, typename U, typename Unp>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U> &&
                 sfinae::is_container_v<U> != sfinae::is_container_v<Unp> &&
                 std::is_same_v<T, object>>
update(T &l, const U &r, Unp &unpacked) {
	l = object(r);
}
template <typename T, typename U, typename Unp>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U> &&
                 sfinae::is_container_v<U> && sfinae::is_container_v<Unp>>
update(T &l, const U &r, Unp &unpacked) {
	assert(r.size() == unpacked.size());
	std::copy(r.begin(), r.end(), unpacked.begin());
}
template <typename T, typename U, typename Unp>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>>
update(T &l, const U &r, Unp &unpacked) {
	r.visit([&](auto &a) { update(l, a, unpacked); });
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T>> update(T &l, const U &r) {
	l.visit([&](auto &a) { update(l, r, a); });
}
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && !sfinae::is_object_v<U>, T> &
operator*=(T &l, const U &r) {
	auto result = l * r;
	update(l, result);
	return l;
}
} // namespace ops_impl
template <typename T, typename U>
std::enable_if_t<sfinae::is_object_v<T> && sfinae::is_object_v<U>, T> &
operator*=(T &l, const U &r) {
	using ops_impl::operator*=;
	r.visit([&](auto &underlaying) { l *= underlaying; });
	return l;
}

//<<
namespace ops_impl {
template <typename T>
std::enable_if_t<sfinae::is_container_v<T>, std::ostream> &
operator<<(std::ostream &os, const T &o) {
	os << '[';
	for (auto &a : o)
		os << a << ", ";
	os << "\b\b";
	os << ']';
	return os;
}
} // namespace ops_impl
template <typename T>
std::enable_if_t<sfinae::is_object_v<T>, std::ostream> &
operator<<(std::ostream &os, const T &o) {
	using ops_impl::operator<<;
	o.visit([&](auto &underlaying) { os << underlaying; });
	return os;
}
} // namespace matlang

#endif /* end of include guard: OBJECT_OPS_HPP */
