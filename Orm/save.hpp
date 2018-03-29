#pragma once

#include <type_traits>
#include "zip.hpp"
#include "transform.hpp"
#include "OneToManyRange.hpp"


template<class Relation, class Range, class IdGenerator, class DBStream>
void saveChild(const Relation& relation, const Range& range_with_ids, IdGenerator gen, DBStream stream)
{
	auto childRange = makeOneToManyRange(range_with_ids, relation.m_accessor);
	std::vector<int> ids = gen(make_cref(childRange));
	stream(zip(ids, childRange));
	saveChildren(zip(ids, make_cref(childRange)), stream, gen);
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
	using root_type = typename std::remove_const<typename Range::value_type::second_type::type>::type;
	saveChildren_impl<0>(OneToMany<root_type>::relations(), range_with_ids, gen, stream);
}
