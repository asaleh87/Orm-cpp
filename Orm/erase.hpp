#pragma once

#include "OneToMany.hpp"
#include "Query.hpp"
#include "OrmWithRefToFather.hpp"
#include "TypedIndex.hpp"

template<class Orm, class Relation, class Range, class DBHandler>
bool eraseChild(const Relation& relation, Range& father_ids, DBHandler handler)
{
	using Accessor = typename Relation::accessor_t;
	using container_type = std::decay_t<std::result_of_t<Accessor(Orm)>>;
	using child_type = typename container_type::value_type;
	using child_type_with_father_ref = OrmWithRefToFather<Orm, child_type, typename Range::value_type>;
	
	return erase(makeFatherIdQuery<child_type_with_father_ref>(father_ids), handler);
}


template<class Orm, class Range, class DBHandler, class... Relations>
bool erase_children_impl(const std::tuple<Relations...>&, const Range&, DBHandler, TypedIndex<sizeof...(Relations)>) { return true; }

template<class Orm, size_t I, class Range, class DBHandler, class... Relations>
bool erase_children_impl(const std::tuple<Relations...>& relations, const Range& father_ids, DBHandler stream, TypedIndex<I>)
{
	if (eraseChild<Orm>(std::get<I>(relations), father_ids, stream))
		return erase_children_impl<Orm>(relations, father_ids, stream, TypedIndex<I+1>());
	
	return false;
}

template<class Orm, class DBHandler >
bool erase(const Query<Orm>& query, DBHandler handler) {
	return erase_children_impl<orm_undl_type_t<Orm>>(OneToMany<orm_undl_type_t<Orm>>::relations(), handler(query), handler, TypedIndex<0>());
}