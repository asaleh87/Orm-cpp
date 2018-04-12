#pragma once

#include <tuple>
#include <type_traits>
#include "DataModel.hpp"

template<class Father, class Orm, class Id = unsigned long>
struct OrmWithRefToFather {
	Orm m_orm;
	Id m_ref_to_father{ 0 };
	OrmWithRefToFather(Orm orm, Id ref_to_father)
		: m_orm(std::move(orm)), m_ref_to_father(ref_to_father) {}

	template<class O = Orm, class = std::enable_if_t<std::is_default_constructible_v<O>>>
		OrmWithRefToFather() {}

	Id get_ref_to_father() const { return m_ref_to_father; }
	
	void set_ref_to_father(Id id) { m_ref_to_father = id; }
	
	friend std::ostream& operator<<(std::ostream& stream, const OrmWithRefToFather& o) {
		return stream << o.m_ref_to_father << ',' << o.m_orm;
	}
	OrmWithRefToFather& operator=(OrmWithRefToFather&& that) {
		m_ref_to_father = that.m_ref_to_father;
		m_orm = std::move(that.m_orm);
		return *this;
	}

	OrmWithRefToFather(const OrmWithRefToFather& that) : m_ref_to_father(that.m_ref_to_father), m_orm(that.m_orm) {}

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
auto make_orm_with_ref_to_father(Child& child, Id id, const Father&)
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

template<class Range>
auto make_cref(const Range& range) {
	return range | make_transform([](const auto& e) { return std::cref(extractRealType(e)); });
}
template<int I, class Father, class Tuple>
struct GetResultOf {
	using accessor = typename std::tuple_element<I, Tuple>::type::accessor_t;
	using container_type = std::decay_t<std::result_of_t<accessor(Father)>>;
	using type = typename container_type::value_type;
};
template<int I, class Child, class Father, class Tuple>
constexpr typename std::enable_if<std::is_same<Child, typename GetResultOf<I, Father, Tuple>::type>::value, const std::string&>::type
getRefColName_internal(const Tuple& tuple)
{
	return std::get<I>(tuple).m_refColumn;
}

template<int I, class Child, class Father, class Tuple>
constexpr typename std::enable_if<!std::is_same<Child, typename GetResultOf<I, Father, Tuple>::type>::value, const std::string&>::type
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

		auto operator()(const Orm_t& orm) const {
			return m_fn(orm.m_orm);
		}
	};
	template<class Fn>
	static auto makeIndirectAccessor(Fn fn) {
		return IndirectAccessor<Fn>(fn);
	}
	template<class Tuple, size_t... Is>
	static auto wrap_impl(Tuple&& tuple, std::index_sequence<Is...>) {
		return std::make_tuple(
			createColumn(std::get<Is>(std::forward<Tuple>(tuple)).m_name, 
				makeIndirectAccessor(std::get<Is>(std::forward<Tuple>(tuple)).m_accessor),
				makeIndirectAccessor(std::get<Is>(std::forward<Tuple>(tuple)).m_writer))...);
	}
	static auto wrapWithAccessorToRealClass() {
		return wrap_impl(Datamodel<RealChild>::columns(), std::make_index_sequence<std::tuple_size<decltype(Datamodel<RealChild>::columns())>::value>());
	}
	static constexpr auto columns() {
		return std::tuple_cat(wrapWithAccessorToRealClass(),
			std::make_tuple(createColumn(getRefColName<Father, RealChild>(), std::mem_fun_ref(&Orm_t::get_ref_to_father), std::mem_fun_ref(&Orm_t::set_ref_to_father))));
	}
	static const int father_ref_index = std::tuple_size<decltype(columns())>::value - 1;
};

