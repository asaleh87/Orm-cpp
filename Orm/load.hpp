#pragma once

#include <vector>
#include "OneToMany.hpp"
#include "Query.hpp"
#include "transform.hpp"
#include "TypedIndex.hpp"

template<class Orm, class Relation, class Range, class DBHandler>
void loadChild(const Relation& relation, Range& father_ids, DBHandler handler)
{
	using Accessor = typename Relation::accessor_t;
	using container_type = typename std::decay_t<std::result_of_t<Accessor(Orm)>>;
	using child_type = typename container_type::value_type;
	using id_type = typename Range::value_type::first_type;

	using child_type_with_father_ref = OrmWithRefToFather<Orm, child_type, id_type >;
	auto childRange = loadElements(makeFatherIdQuery<child_type_with_father_ref>(father_ids | make_transform([](const typename Range::value_type& e) { return e.first; })), handler);

	std::sort(childRange.begin(), childRange.end(), typename child_type_with_father_ref::Compare());

	auto start = childRange.begin();
	for (const auto& father : father_ids) {
		auto p = std::equal_range(start, childRange.end(), father.first, typename child_type_with_father_ref::Compare());
		auto child_range_for_father = make_range(p.first, p.second);
		auto range = child_range_for_father | make_update_transform([](child_type_with_father_ref& e) {
			return std::move(e.m_orm);
		});
		relation.m_writer(father.second, container_type(range.begin(), range.end()));
		start = p.second;
	}
}

template<class Orm, class Range, class DBHandler, class... Relations>
void loadChildren_impl(const std::tuple<Relations...>&, const Range&, DBHandler, TypedIndex<sizeof...(Relations)>) 
{}

template<class Orm, size_t I, class Range, class DBHandler, class... Relations>
void loadChildren_impl(const std::tuple<Relations...>& relations, const Range& father_ids, DBHandler stream, TypedIndex<I>)
{
	loadChild<Orm>(std::get<I>(relations), father_ids, stream);
	loadChildren_impl<Orm>(relations, father_ids, stream, TypedIndex<I+1>());
}

template<class Orm, class DBHandler>
std::vector<Orm> loadElements(const Query<Orm>& query, DBHandler handler) {
	auto range_with_ids = handler(query);//get ids of mapped to partially constructed objects based only on columns and not on one2many
	using range_type = decltype(range_with_ids);
	using value_type = typename range_type::value_type;
	std::sort(range_with_ids.begin(), range_with_ids.end(), [](const value_type& lhs, const value_type& rhs) {return lhs.first < rhs.first; });
	
	using Id = typename decltype(range_with_ids)::value_type::first_type;
	using undltype = std::remove_reference_t<decltype(extractRealType(range_with_ids.begin()->second))>;
	
	std::vector<std::pair<Id, std::reference_wrapper<undltype>>> refs_with_ids;
	std::transform(range_with_ids.begin(), range_with_ids.end(), std::back_inserter(refs_with_ids), [](std::pair<Id, Orm>& p) { return std::make_pair(p.first, std::ref(extractRealType(p.second))); });

	auto relations = OneToMany<undltype>::relations();
	loadChildren_impl<undltype>(relations, refs_with_ids, handler, TypedIndex<0>());
	auto range_without_ids = range_with_ids | make_update_transform([](std::pair<Id, Orm>& p) {
		return std::move(p.second);
	});
	return std::vector<Orm>(range_without_ids.begin(), range_without_ids.end());	
}
