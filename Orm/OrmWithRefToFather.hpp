#pragma once

#include <tuple>
#include <type_traits>
#include "DataModel.hpp"
#include "index_sequence.hpp"

template<class Father, class Orm, class Id = unsigned long>
struct OrmWithRefToFather {
	Orm m_orm;
	Id m_ref_to_father{ 0 };
	OrmWithRefToFather(Orm orm, Id ref_to_father)
		: m_orm(std::move(orm)), m_ref_to_father(ref_to_father) {}

	OrmWithRefToFather() {}

	friend std::ostream& operator<<(std::ostream& stream, const OrmWithRefToFather& o) {
		return stream << o.m_ref_to_father << ',' << o.m_orm;
	}

	struct Compare {
		bool operator()(Id id, const OrmWithRefToFather& o) const { return id < o.m_ref_to_father; }
		bool operator()(const OrmWithRefToFather& o, Id id) const { return o.m_ref_to_father < id; }
		bool operator()(const OrmWithRefToFather& lhs, const OrmWithRefToFather& rhs) const { return lhs.m_ref_to_father < rhs.m_ref_to_father; }
	};
};

template<class T>
struct orm_with_ref_decay {
	using type = T;
};
template<class Father, class Orm, class Id>
struct orm_with_ref_decay<OrmWithRefToFather<Father, Orm, Id>>
{
	using type = OrmWithRefToFather<Father, std::decay_t<Orm>, Id>;
};

template<class T>
using orm_with_ref_decay_t = typename orm_with_ref_decay<T>::type;

template<class T>
struct orm_undl_type {
	using type = T;
};
template<class F, class Son, class Id>
struct orm_undl_type<OrmWithRefToFather<F, Son, Id>> {
	using type = Son;
};
template<class T>
using orm_undl_type_t = typename orm_undl_type<T>::type;

template<class Father, class Child, class Id>
OrmWithRefToFather<Father, Child&, Id> make_orm_with_ref_to_father(Child& child, Id id, const Father&)
{
	return OrmWithRefToFather<Father, Child&, Id>(child, id);
}

template<class E>
E& extractRealType(E& e) { return e; }

template<class E, class Father, class Id>
std::remove_reference_t<E>& extractRealType(OrmWithRefToFather<Father, E, Id>& e) { return e.m_orm; }

template<class E, class Father, class Id>
E& extractRealType(const OrmWithRefToFather<Father, E&, Id>& e) { return e.m_orm; }

template<class E, class Father, class Id>
const E& extractRealType(const OrmWithRefToFather<Father, E, Id>& e) { return e.m_orm; }

template<class V>
auto extractType(const V& v) -> decltype(std::cref(extractRealType(v))) 
{
	return std::cref(extractRealType(v)); 
}

template<class Range>
auto make_cref(const Range& range) -> decltype(range | make_transform(extractType<typename Range::value_type>))
{
	return range | make_transform(extractType<typename Range::value_type>);
}
template<int I, class Father, class Tuple>
struct GetResultOf {
	using accessor = typename std::tuple_element<I, Tuple>::type::accessor_t;
	using container_type = std::decay_t<std::result_of_t<accessor(Father)>>;
	using type = typename container_type::value_type;
};
template<int I, class Child, class Father, class Tuple>
/*constexpr */typename std::enable_if<std::is_same<Child, typename GetResultOf<I, Father, Tuple>::type>::value, const std::string&>::type
getRefColName_internal(const Tuple& tuple)
{
	return std::get<I>(tuple).m_refColumn;
}

template<int I, class Child, class Father, class Tuple>
/*constexpr */typename std::enable_if<!std::is_same<Child, typename GetResultOf<I, Father, Tuple>::type>::value, const std::string&>::type
getRefColName_internal(const Tuple& tuple)
{
	return getRefColName_internal<I + 1, Child, Father>(tuple);
}

template<class Father, class Child>
std::string getRefColName() {
	return getRefColName_internal<0, Child, Father>(OneToMany<Father>::relations());
}

template<class Child, class Father, class Id>
struct Datamodel<OrmWithRefToFather<Father, Child, Id>> : Datamodel<Child>
{
	using Orm_t = OrmWithRefToFather<Father, Child, Id>;
	using fields = int;
	using RealChild = std::decay_t<Child>;

	template<class Fn>
	struct IndirectAccessor {
		Fn m_fn;
		explicit IndirectAccessor(Fn fn) : m_fn(fn) {}
		
		template<class Field>
		void operator()(Orm_t& orm, Field&& field) const {
			m_fn(orm.m_orm, std::forward<Field>(field));
		}

		auto operator()(const Orm_t& orm) const -> decltype(m_fn(orm.m_orm))
		{
			return m_fn(orm.m_orm);
		}
	};
	template<class Fn>
	static IndirectAccessor<Fn> makeIndirectAccessor(Fn fn) {
		return IndirectAccessor<Fn>(fn);
	}

	template<class Accessor, class Writer, template<class, class> class Col>
	static auto createWrapperColumn(const Col<Accessor, Writer>& col) 
		-> decltype(createColumn(col.m_name, makeIndirectAccessor(col.m_accessor), makeIndirectAccessor(col.m_writer), col.m_length)) 
	{
		return createColumn(col.m_name, makeIndirectAccessor(col.m_accessor), makeIndirectAccessor(col.m_writer), col.m_length);
	}

	template<class Accessor, class Writer>
	static auto createWrapperColumn(const FloatingPointColumn<Accessor, Writer>& col) 
		-> decltype(createNumberColumn(col.m_name, makeIndirectAccessor(col.m_accessor), makeIndirectAccessor(col.m_writer), col.m_length, col.m_nbDecimals))
	{
		return createNumberColumn(col.m_name, makeIndirectAccessor(col.m_accessor), makeIndirectAccessor(col.m_writer), col.m_length, col.m_nbDecimals);
	}

	template<class Tuple, size_t... Is>
	static auto wrap_impl(const Tuple& tuple, util::index_sequence<Is...>) 
		-> decltype(std::make_tuple(createWrapperColumn(std::get<Is>(tuple))...))
	{
		return std::make_tuple(createWrapperColumn(std::get<Is>(tuple))...);
	}
	
	static auto wrapWithAccessorToRealClass() -> decltype(wrap_impl(Datamodel<RealChild>::columns(), util::make_index_sequence<std::tuple_size<decltype(Datamodel<RealChild>::columns())>::value>()))
	{
		return wrap_impl(Datamodel<RealChild>::columns(), util::make_index_sequence<std::tuple_size<decltype(Datamodel<RealChild>::columns())>::value>());
	}

	static /*constexpr*/ auto columns() -> decltype(std::tuple_cat(wrapWithAccessorToRealClass(),
		std::make_tuple(createColumn(getRefColName<Father, RealChild>(), &Orm_t::m_ref_to_father, Datamodel<Father>::ref_col_size))))
	{
		return std::tuple_cat(wrapWithAccessorToRealClass(),
			std::make_tuple(createColumn(getRefColName<Father, RealChild>(), &Orm_t::m_ref_to_father, Datamodel<Father>::ref_col_size)));
	}
	static const int father_ref_index = std::tuple_size<decltype(columns())>::value - 1;
};

