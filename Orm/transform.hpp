#pragma once

#include "iterator_range.hpp"

template<class It, bool update>
using undl_value_type = std::conditional_t < update, typename It::value_type&, typename It::value_type>;

template<class It, class Fn, bool update = false>
struct transform_iterator : public std::iterator<std::input_iterator_tag, std::result_of_t<Fn(undl_value_type<It, update>)>>
{
	using value_type = std::result_of_t<Fn(undl_value_type<It, update>)>;
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
auto make_transform_iterator(It it, Fn fn) {
	return transform_iterator<It, Fn>(it, fn);
}

template<class It, class Fn>
auto make_update_transform_iterator(It it, Fn fn) {
	return transform_iterator<It, Fn, true>(it, fn);
}

template<class Fn, bool update = false>
struct transform_wrapper {
	Fn m_fn;
	explicit transform_wrapper(Fn fn) : m_fn(fn) {}
};

template<class Fn>
auto make_transform(Fn fn) {
	return transform_wrapper<Fn>(fn);
}
template<class Fn>
auto make_update_transform(Fn fn) {
	return transform_wrapper<Fn, true>(fn);
}
template<class Range, class Fn>
auto operator|(const Range& range, transform_wrapper<Fn> transformer) {
	return make_range(make_transform_iterator(range.begin(), transformer.m_fn),
					  make_transform_iterator(range.end(), transformer.m_fn));
}

template<class Range, class Fn>
auto operator|(const Range& range, transform_wrapper<Fn, true> transformer) {
	return make_range(make_update_transform_iterator(range.begin(), transformer.m_fn),
				      make_update_transform_iterator(range.end(), transformer.m_fn));
}