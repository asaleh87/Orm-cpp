#pragma once

#include <vector>
#include "OneToMany.hpp"
#include "Query.hpp"

template<class Orm, class Relation, class Range, class DBHandler>
void loadChild(const Relation& relation, Range& father_ids, DBHandler handler)
{
	using Accessor = typename Relation::accessor_t;
	using container_type = typename std::decay_t<std::result_of_t<Accessor(Orm)>>;
	using child_type = typename container_type::value_type;
	using id_type = typename Range::value_type::first_type;

	using child_type_with_father_ref = OrmWithRefToFather<Orm, child_type, id_type >;
	auto childRange = loadElements(makeFatherIdQuery<child_type_with_father_ref>(father_ids | make_transform([](const auto& e) { return e.first; })), handler);

	std::sort(childRange.begin(), childRange.end(), typename child_type_with_father_ref::Compare());

	auto start = childRange.begin();
	for (const auto& father : father_ids) {
		auto p = std::equal_range(start, childRange.end(), father.first, typename child_type_with_father_ref::Compare());
		auto range = make_range(p.first, p.second) | make_update_transform([](auto& e) { return std::move(e.m_orm); });
		relation.m_writer(father.second, container_type(range.begin(), range.end()));
		start = p.second;
	}
}

template<size_t I, class Orm, class Range, class DBHandler, class... Relations>
typename std::enable_if<I == sizeof...(Relations), void>::type
	loadChildren_impl(const std::tuple<Relations...>&, const Range&, DBHandler) 
{}

template<size_t I, class Orm, class Range, class DBHandler, class... Relations>
typename std::enable_if < I < sizeof...(Relations), void>::type
	loadChildren_impl(const std::tuple<Relations...>& relations, const Range& father_ids, DBHandler stream)
{
	loadChild<Orm>(std::get<I>(relations), father_ids, stream);
	loadChildren_impl<I + 1, Orm>(relations, father_ids, stream);
}

template<class Orm, class DBHandler>
std::vector<Orm> loadElements(const Query<Orm>& query, DBHandler handler) {
	auto range_with_ids = handler(query);//get ids of mapped to partially constructed objects based only on columns and not on one2many
	std::sort(range_with_ids.begin(), range_with_ids.end(), [](const auto& lhs, const auto& rhs) {return lhs.first < rhs.first; });
	
	using Id = typename decltype(range_with_ids)::value_type::first_type;
	using undltype = std::remove_reference_t<decltype(extractRealType(range_with_ids.begin()->second))>;
	
	std::vector<std::pair<Id, std::reference_wrapper<undltype>>> refs_with_ids;
	std::transform(range_with_ids.begin(), range_with_ids.end(), std::back_inserter(refs_with_ids), [](std::pair<Id, Orm>& p) { return std::make_pair(p.first, std::ref(extractRealType(p.second))); });

	loadChildren_impl<0, undltype>(OneToMany<undltype>::relations(), refs_with_ids, handler);
	auto range_without_ids = range_with_ids | make_update_transform([](auto& p) {return std::move(p.second); });
	return std::vector<Orm>(range_without_ids.begin(), range_without_ids.end());	
}
