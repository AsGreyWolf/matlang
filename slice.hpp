#ifndef SLICE_HPP
#define SLICE_HPP

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace matlang {
using slice = std::vector<size_t>;

template <typename Container> struct slice_array {
private:
	Container *container{};
	slice sl;

public:
	using value_type = typename Container::value_type;
	slice_array() = default;
	slice_array(Container &data, slice s) : container{&data}, sl{std::move(s)} {
		if (std::find_if(sl.begin(), sl.end(),
		                 [&](auto &el) { return el >= data.size(); }) != sl.end())
			throw std::invalid_argument{"slice index out of bounds"};
	}

	value_type &operator[](size_t i) { return (*container)[sl[i]]; }
	const value_type &operator[](size_t i) const { return (*container)[sl[i]]; }
	// auto operator[](const slice &s) const {
	// 	return slice_array<const slice_array>{this, s};
	// }
	auto size() const { return sl.size(); }

	struct iterator {
		using value_type = typename Container::iterator::value_type;
		using reference = typename Container::iterator::reference;
		using pointer = typename Container::iterator::reference;
		using difference_type = typename Container::iterator::difference_type;
		using iterator_category = typename Container::iterator::iterator_category;

		Container *container;
		typename slice::const_iterator i;
		iterator() = default;
		iterator(Container *c, typename slice::const_iterator it)
		    : container{c}, i{it} {};
		bool operator!=(const iterator &second) const {
			return container != second.container || i != second.i;
		}
		auto &operator*() { return (*container)[*i]; }
		auto &operator-> () { return (*container)[*i]; }
		iterator &operator++() {
			++i;
			return *this;
		}
		iterator operator++(int) {
			iterator temp = *this;
			++*this;
			return temp;
		}
		iterator &operator--() {
			--i;
			return *this;
		}
		iterator operator--(int) {
			iterator temp = *this;
			--temp;
			return temp;
		}
		iterator &operator+=(difference_type diff) {
			i += diff;
			return *this;
		}
		iterator operator+(difference_type diff) const {
			iterator temp = *this;
			temp += diff;
			return temp;
		}
		iterator &operator-=(difference_type diff) {
			i -= diff;
			return *this;
		}
		iterator operator-(difference_type diff) const {
			iterator temp = *this;
			temp -= diff;
			return temp;
		}
		auto operator-(iterator r) const { return i - r.i; }
		auto operator[](difference_type diff) const { return *(*this + diff); }
		auto operator<(iterator r) const { return i < r.i; }
		auto operator<=(iterator r) const { return i <= r.i; }
		auto operator>(iterator r) const { return i > r.i; }
		auto operator>=(iterator r) const { return i >= r.i; }
	};
	auto begin() { return iterator{container, sl.begin()}; }
	auto begin() const { return iterator{container, sl.begin()}; }
	auto end() { return iterator{container, sl.end()}; }
	auto end() const { return iterator{container, sl.end()}; }
};
} // namespace matlang

#endif /* end of include guard: SLICE_HPP */
