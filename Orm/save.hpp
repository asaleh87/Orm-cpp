#pragma once

#include <type_traits>
#include <numeric>

#include "zip.hpp"
#include "transform.hpp"
#include "OneToManyRange.hpp"


template<class Relation, class Range, class IdGenerator, class DBStream>
void saveChild(const Relation& relation, const Range& range_with_ids, IdGenerator gen, DBStream stream)
{
	using Id = typename Range::value_type::first_type;
	using root_type = typename Range::value_type::second_type::type;
	using child_type = typename std::decay_t<std::result_of_t<typename Relation::accessor_t(root_type)>>::value_type;
	using value_type = OrmWithRefToFather<std::decay_t<root_type>, const child_type&, Id>;
	std::vector<value_type> sons_refs;
	const size_t total_sons =
		std::accumulate(range_with_ids.begin(), range_with_ids.end(), 0, [&relation](size_t accum, const auto& father) { return accum + relation.m_accessor(father.second).size(); });

	sons_refs.reserve(total_sons);
	for (const auto& father : range_with_ids) {
		const auto& father_sons = relation.m_accessor(father.second);
		std::transform(father_sons.begin(), father_sons.end(), std::back_inserter(sons_refs), [&](const auto& son) {return value_type(son, father.first); });
	}
	save_impl(sons_refs, stream, gen);
}

template<size_t I, class Range, class IdGenerator, class DBStream, class... Relations>
typename std::enable_if<I == sizeof...(Relations), void>::type
saveChildren_impl(const std::tuple<Relations...>&, const Range&, IdGenerator, DBStream) {}

template<size_t I, class Range, class IdGenerator, class DBStream, class... Relations>
typename std::enable_if < I < sizeof...(Relations), void>::type
	saveChildren_impl(const std::tuple<Relations...>& relations, const Range& range_with_ids, IdGenerator gen, DBStream stream)
{
	saveChild(std::get<I>(relations), range_with_ids, gen, stream);
	saveChildren_impl<I + 1>(relations, range_with_ids, gen, stream);
}

template<class Range, class IdGenerator, class DBStream>
void saveChildren(const Range& range_with_ids, DBStream stream, IdGenerator gen) {
	using undl_element_type = std::decay_t<typename Range::value_type::second_type::type>;
	saveChildren_impl<0>(OneToMany<undl_element_type >::relations(), range_with_ids, gen, stream);
}
template<class Range, class IdGenerator, class DBStream>
void save_impl(const Range& range, DBStream stream, IdGenerator gen) {
	auto ids = gen(make_cref(range));
	stream(zip(ids, range | make_transform([](const auto& e) { return &e; })));
	saveChildren(zip(ids, make_cref(range)), stream, gen);
}
template<class Range, class IdGenerator, class DBStream>
void save(const Range& range, DBStream stream, IdGenerator gen) {
	return save_impl(range, stream, gen);
}

