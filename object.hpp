#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "slice.hpp"
#include <algorithm>
#include <variant>
#include <vector>

namespace matlang {
class object {
public:
	using flat_impl = double;
	using container_impl = std::vector<object>;
	using impl = std::variant<flat_impl, container_impl>;

	object() = default;
	explicit object(flat_impl data) : storage{std::move(data)} {};
	explicit object(container_impl data) : storage{std::move(data)} {};
	object(std::initializer_list<object> data) : storage{std::move(data)} {};

	bool flat() const { return std::holds_alternative<flat_impl>(storage); }
	size_t size() const { return std::get<container_impl>(storage).size(); }
	template <typename Fn> decltype(auto) visit(const Fn &f) {
		return std::visit(f, storage);
	}
	template <typename Fn> decltype(auto) visit(const Fn &f) const {
		return std::visit(f, storage);
	}

	auto &operator[](size_t id) { return std::get<container_impl>(storage)[id]; }
	auto operator[](slice s);
	auto view();

private:
	impl storage{0.0};
};

class object_view {
public:
	using flat_impl = object::flat_impl;
	using container_impl = slice_array<object::container_impl>;
	using impl = container_impl;

	object_view() = default;
	object_view(object::container_impl &o, slice s) : storage{o, std::move(s)} {};

	bool flat() const { return false; }
	size_t size() const { return storage.size(); }
	template <typename Fn> decltype(auto) visit(const Fn &f) { return f(storage); }
	template <typename Fn> decltype(auto) visit(const Fn &f) const {
		return f(storage);
	}

	object copy() const {
		object::container_impl result;
		for (auto &a : storage)
			result.push_back(a);
		return object(result);
	}

	auto &operator[](size_t id) { return storage[id]; }

private:
	impl storage{};
};
auto object::operator[](slice s) {
	return object_view(std::get<container_impl>(storage), move(s));
}
auto object::view() {
	slice sl;
	for (size_t i = 0; i != size(); i++) {
		sl.push_back(i);
	}
	return (*this)[sl];
}
} // namespace matlang

#include "object_ops.hpp"

#endif /* end of include guard: OBJECT_HPP */
