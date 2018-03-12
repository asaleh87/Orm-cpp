#pragma once

template<class It1, class It2>
struct zip_iterator : public std::iterator<std::input_iterator_tag, std::pair<typename It1::value_type, typename It2::value_type>> {
	using value_type = std::pair<typename It1::value_type, typename It2::value_type>;
	It1 m_it1;
	It2 m_it2;
	zip_iterator(It1 it1, It2 it2) : m_it1(it1), m_it2(it2) {}
	zip_iterator& operator++() {
		++m_it1; ++m_it2;
		return *this;
	}
	friend bool operator!=(const zip_iterator& lhs, const zip_iterator& rhs) {
		return lhs.m_it1 != rhs.m_it1;
	}

	friend bool operator==(const zip_iterator& lhs, const zip_iterator& rhs) {
		return lhs.m_it1 == rhs.m_it1;
	}

	auto operator*() const {
		return std::make_pair(*m_it1, *m_it2);
	}
};

template<class It1, class It2>
zip_iterator<It1, It2> make_zip_iterator(It1 it1, It2 it2) {
	return zip_iterator<It1, It2>(it1, it2);
}

template<class Range1, class Range2>
auto zip(const Range1& range1, const Range2& range2) {
	return make_range(make_zip_iterator(range1.begin(), range2.begin()), make_zip_iterator(range1.end(), range2.end()));
}