#pragma once

#include "OrmWithRefToFather.hpp"


template<class Range, class Accessor>
struct OneToManyRange {
	using root_iterator = typename Range::const_iterator;
	using root_type = typename root_iterator::value_type::second_type::type;
	using child_iterator = typename std::decay<typename std::result_of<Accessor(root_type)>::type>::type::const_iterator;
	using value_type = OrmWithRefToFather<typename std::decay<root_type>::type, typename child_iterator::value_type, typename root_iterator::value_type::first_type>;

	struct const_iterator : public std::iterator<std::input_iterator_tag, value_type>
	{
		root_iterator m_root_it;
		root_iterator m_end_it;
		Accessor m_accessor;
		child_iterator m_child_it;
		const_iterator(root_iterator root_it, Accessor accessor, child_iterator child_it, root_iterator end_it)
			: m_root_it(root_it), m_child_it(child_it), m_accessor(accessor), m_end_it(end_it) {}

		const_iterator& operator++() {
			const auto& p = *m_root_it;
			if (m_accessor(p.second).end() != m_child_it)//corner case if child range is empty
				++m_child_it;

			if (m_accessor(p.second).end() == m_child_it) {
				//std::cout << "end reached\n";
				++m_root_it;
				if (m_root_it != m_end_it)
					m_child_it = m_accessor((*m_root_it).second).begin();
			}
			return *this;
		}
		const auto operator*() const {
			return make_orm_with_ref_to_father(*m_child_it, (*m_root_it).first, extractRealType((*m_root_it).second));
		}

		friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs)
		{
			//both reached end
			if (lhs.m_root_it == lhs.m_end_it && rhs.m_root_it == rhs.m_end_it)
				return false;
			return lhs.m_root_it != rhs.m_root_it || lhs.m_child_it != rhs.m_child_it;
		}
		friend bool operator==(const const_iterator& lhs, const const_iterator& rhs)
		{
			return !(lhs != rhs);
		}
	};
	const_iterator m_begin;
	const_iterator m_end;

	OneToManyRange(const Range& range, Accessor accessor)
		: m_begin(range.begin(), accessor, accessor((*range.begin()).second).begin(), range.end())
		, m_end(range.end(), accessor, accessor((*range.begin()).second).begin(), range.end())
	{}
	const_iterator begin() const { return m_begin; }
	const_iterator end() const { return m_end; }
};

template<class Range, class Accessor>
auto makeOneToManyRange(const Range& range, Accessor accessor) {
	return OneToManyRange<Range, Accessor>(range, accessor);
}
