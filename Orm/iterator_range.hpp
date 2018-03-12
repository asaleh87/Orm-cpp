#pragma once

template<class It>
struct iterator_range {
	using value_type = typename It::value_type;
	using const_iterator = It;
	It m_begin;
	It m_end;
	iterator_range(It begin, It end) : m_begin(begin), m_end(end) {}
	It begin() const { return m_begin; }
	It end() const { return m_end; }
};

template<class It>
iterator_range<It> make_range(It begin, It end) {
	return iterator_range<It>(begin, end);
}

