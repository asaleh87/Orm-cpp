#pragma once

#include <tuple>

template<class Father, class Orm, class Id>
struct OrmWithRefToFather {
	const Orm& m_orm;
	Id m_ref_to_father;
	OrmWithRefToFather(const Orm& orm, Id ref_to_father)
		: m_orm(orm), m_ref_to_father(ref_to_father) {}
	Id get_ref_to_father() const { return m_ref_to_father; }
	friend std::ostream& operator<<(std::ostream& stream, const OrmWithRefToFather& o) {
		return stream << o.m_ref_to_father << ',' << o.m_orm;
	}
};

template<class Father, class Child, class Id>
OrmWithRefToFather<Father, Child, Id> make_orm_with_ref_to_father(const Child& child, Id id, const Father&)
{
	return OrmWithRefToFather<Father, Child, Id>(child, id);
}

template<int I, class Tuple>
struct GetResultOf {
	using ith_element = typename std::tuple_element<I, Tuple>::type;
	using type = typename std::result_of<typename ith_element::accessor_t>::type::value_type;
};
template<int I, class Child, class Tuple>
constexpr typename std::enable_if<std::is_same<Child, typename GetResultOf<I, Tuple>::type>::value, const std::string&>::type
getRefColName_internal(const Tuple& tuple)
{
	return std::get<I>(tuple).m_refColumn;
}

template<int I, class Child, class Tuple>
constexpr typename std::enable_if<!std::is_same<Child, typename GetResultOf<I, Tuple>::type>::value, const std::string&>::type
getRefColName_internal(const Tuple& tuple)
{
	return getRefColName_internal<I + 1, Child>(tuple);
}

template<class Father, class Child>
std::string getRefColName() {
	return getRefColName_internal<0, Child>(OneToMany<Father>::relations());
}
template<class Child, class Father, class Id>
struct Datamodel<OrmWithRefToFather<Father, Child, Id>> : Datamodel<Child>
{
	using Orm_t = OrmWithRefToFather<Father, Child, Id>;
	static constexpr auto columns() {
		return std::tuple_cat(Datamodel<Child>::columns,
			std::tie(createColumn(getRefColName<Father, Child>(), std::mem_fun_ref(&Orm_t::get_ref_to_father))));
	}
};

