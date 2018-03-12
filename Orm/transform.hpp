#pragma once

#include "iterator_range.hpp"

template<class It, class Fn>
struct transform_iterator : public std::iterator<std::input_iterator_tag, typename std::result_of<Fn(typename It::value_type)>::type>
{
	using value_type = typename std::result_of<Fn(typename It::value_type)>::type;
	It m_it;
	Fn m_fn;
	transform_iterator(It it, Fn fn) : m_it(it), m_fn(fn) {}
	auto operator*() const { return m_fn(*m_it); }

	transform_iterator& operator++() {
		++m_it;
		return *this;
	}

	friend bool operator!=(const transform_iterator& lhs, const transform_iterator& rhs) {
		return lhs.m_it != rhs.m_it;
	}

	friend bool operator==(const transform_iterator& lhs, const transform_iterator& rhs) {
		return lhs.m_it == rhs.m_it;
	}
};

template<class It, class Fn>
transform_iterator<It, Fn> make_transform_iterator(It it, Fn fn) {
	return transform_iterator<It, Fn>(it, fn);
}

template<class Fn>
struct transform_wrapper {
	Fn m_fn;
	explicit transform_wrapper(Fn fn) : m_fn(fn) {}
};

template<class Fn>
transform_wrapper<Fn> make_transform(Fn fn) {
	return transform_wrapper<Fn>(fn);
}

template<class Range, class Fn>
auto operator|(const Range& range, transform_wrapper<Fn> transformer) {
	return make_range(make_transform_iterator(range.begin(), transformer.m_fn),
		make_transform_iterator(range.end(), transformer.m_fn));
}
