#pragma once

#include "Datamodel_specific.hpp"
#include "../Orm/TuplePrinter.hpp"
#include "../Orm/OrmWithRefToFather.hpp"
#include "../Orm/Query.hpp"
#include "../Orm/TypedIndex.hpp"

#include <algorithm>
#include <functional>

struct ToTuple {
	template<class Orm, class Columns, size_t... Is>
	static auto toTuple_impl(const Orm& orm, const Columns& cols, util::index_sequence<Is...>) -> decltype(std::make_tuple(std::get<Is>(cols).m_accessor(orm)...))
	{
		return std::make_tuple(std::get<Is>(cols).m_accessor(orm)...);
	}

	template<class Orm>
	auto operator()(const Orm& orm) const -> decltype(toTuple_impl(orm, Datamodel<Orm>::columns(), util::make_index_sequence<std::tuple_size<decltype(Datamodel<Orm>::columns())>::value>()))
	{
		auto columns = Datamodel<Orm>::columns();
		return toTuple_impl(orm, columns, util::make_index_sequence<std::tuple_size<decltype(columns)>::value>());
	}
};

template<class Orm>
struct Tag {};

std::vector<std::pair<unsigned long, std::tuple<std::string, double>>> get_table(Tag<A>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, double>>>;
	tuples As =
	{
		std::make_pair(0, std::make_tuple("Yay", 2.)),
		std::make_pair(1, std::make_tuple("Boo", 3.))
	};
	return As;
}
std::vector<std::pair<unsigned long, std::tuple<int, unsigned long>>> get_table(Tag<OrmWithRefToFather<A, B>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<int, unsigned long>>>;

	tuples B_with_refs =
	{
		std::make_pair(0, std::make_tuple(1, 0)),
		std::make_pair(1, std::make_tuple(4, 0)),
		std::make_pair(2, std::make_tuple(6, 0)),
		std::make_pair(3, std::make_tuple(2, 1)),
		std::make_pair(4, std::make_tuple(3, 1))
	};
	return B_with_refs;
}
std::vector<std::pair<unsigned long, std::tuple<std::string, unsigned long>>> get_table(Tag<OrmWithRefToFather<B, D>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, unsigned long>>>;
	tuples D_with_refs =
	{
		std::make_pair(0, std::make_tuple("D1", 0)),
		std::make_pair(1, std::make_tuple("D2", 0)),
		std::make_pair(2, std::make_tuple("D3", 1)),
		std::make_pair(3, std::make_tuple("D4", 1)),
		std::make_pair(4, std::make_tuple("D5", 2)),
		std::make_pair(5, std::make_tuple("D6", 2)),
		std::make_pair(6, std::make_tuple("D7", 3)),
		std::make_pair(7, std::make_tuple("D8", 3)),
		std::make_pair(8, std::make_tuple("D9", 4)),
		std::make_pair(9, std::make_tuple("D10",4)),
	};
	return D_with_refs;
}
std::vector<std::pair<unsigned long, std::tuple<double, unsigned long>>> get_table(Tag<OrmWithRefToFather<A, C>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<double, unsigned long>>>;
	tuples C_with_refs =
	{
		std::make_pair(0, std::make_tuple(0.5, 0)),
	   std::make_pair(1, std::make_tuple(1.2, 0)),
	   std::make_pair(2, std::make_tuple(0.5, 1)),
	   std::make_pair(3, std::make_tuple(1.2, 1))
	};
	return C_with_refs;
}


template<class Orm>
struct Table {
	using tuples = std::vector <std::pair<unsigned long, typename std::result_of<ToTuple(Orm)>::type>>;
	tuples m_table;

	explicit Table(bool preinitialize = false) {
		if (preinitialize)
			m_table = get_table(Tag<Orm>());
	}

	friend std::ostream& operator<<(std::ostream& stream, const Table& table) {
		for (auto t : table.m_table)
			stream << '[' << t.first << ',' << makeTuplePrinter(t.second) << "]\n";

		return stream;
	}
};

template<size_t I, class Orm, class T>
static std::enable_if_t < I == std::tuple_size<T>::value> fill_impl(Orm&, const T&) { }

template<size_t I, class Orm, class T>
static std::enable_if_t < I<std::tuple_size<T>::value> fill_impl(Orm& res, const T& t)
{
	(std::get<I>(Datamodel<Orm>::columns()).m_writer)(res, std::get<I>(t));
	fill_impl<I + 1>(res, t);
}

template<class Orm, class Tuple>
static Orm fromTuple(const Tuple& t) {
	Orm res;
	fill_impl<0>(res, t);
	return res;
}

template<class... Orms>
struct Tables {
	Tables(bool) {}
	bool empty() const { return true; }
};
template<class First, class... Orms>
struct Tables<First, Orms...> : Table<First>, Tables<Orms...>
{
	explicit Tables(bool preinitialize = false)
		: Table<First>(preinitialize)
		, Tables<Orms...>(preinitialize) {}

	template<class T>
	typename Table<T>::tuples& get_table() {
		return Table<T>::m_table;
	}
	template<class T>
	const typename Table<T>::tuples& get_table() const {
		return Table<T>::m_table;
	}
	friend bool operator==(const Tables& lhs, const Tables& rhs) {
		return equal_impl(lhs, rhs);
	}
	bool empty() const {
		return Table<First>::m_table.empty() && Tables<Orms...>::empty();
	}

};
using Test_tables = Tables<A, OrmWithRefToFather<A, B>, OrmWithRefToFather<A, C>, OrmWithRefToFather<B, D>>;

struct DB_Loader {

	template<size_t I, class Range, class Tuple>
	static bool find(const Tuple& t, const Range& range) {
		return range.empty() || std::find_if(range.begin(), range.end(), [&t](const typename Range::value_type& c) { return std::get<I>(t) == c; }) != range.end();
	}

	template<size_t I, class Tuple>
	struct FindFilter {
		const Tuple& m_criterias;
		explicit FindFilter(const Tuple& criterias) : m_criterias(criterias) {}
		
		template<class T>
		bool operator()(const T& t) const { return find<I>(t, std::get<I>(m_criterias).second); }
	};

	template<size_t I, class Tuple>
	static FindFilter<I, Tuple> makeFilter(const Tuple& criterias) {
		return FindFilter<I, Tuple>(criterias);
	}

	template<class... Filters>
	struct CompositeFilters {
		std::tuple<Filters...> m_filters;
		explicit CompositeFilters(Filters... filters) : m_filters(filters...) {}

		template<size_t I, class Tuple, class T>
		static void apply_filter_impl(const Tuple& tuple, const T& t, bool& good) { good = good && std::get<I>(tuple)(t); }

		template<class T>
		bool filter_impl(const T&, TypedIndex<sizeof...(Filters)>) const { return true; }

		template<size_t I, class T>
		bool filter_impl(const T& t, TypedIndex<I>) const
		{
			if (!std::get<I>(m_filters)(t))
				return false;
			return filter_impl(t, TypedIndex<I + 1>());
		}

		template<class T>
		bool operator()(const std::pair<unsigned long, T>& t) const {
			return filter_impl(t.second, TypedIndex<0>());
		}
	};
	template<class... Filters>
	static CompositeFilters<Filters...> createCompositeFilter(Filters... filters) {
		return CompositeFilters<Filters...>(filters...);
	}
	template<class Tuple, size_t... Is>
	static auto createCriteria_impl(const Tuple& criterias, util::index_sequence<Is...>) -> decltype(createCompositeFilter(makeFilter<Is>(criterias)...))
	{
		return createCompositeFilter(makeFilter<Is>(criterias)...);
	}
	template<class Tuple>
	static auto createFilter(const Tuple& criterias) -> decltype(createCriteria_impl(criterias, util::make_index_sequence<std::tuple_size<Tuple>::value>())) 
	{
		return createCriteria_impl(criterias, util::make_index_sequence<std::tuple_size<Tuple>::value>());
	}

	template<class Orm, class Range, class Query>
	static std::vector<std::pair<unsigned long, Orm>> buildElements(const Range& range, const Query& query) 
	{
		std::vector<std::pair<unsigned long, Orm>> elements;
		Range filtered_elements;
		std::copy_if(range.begin(), range.end(), std::back_inserter(filtered_elements), createFilter(query.getAllCriterias()));
		std::transform(filtered_elements.begin(), filtered_elements.end(), std::back_inserter(elements), [](const typename Range::value_type& p) {
			return std::make_pair(p.first, fromTuple<Orm>(p.second));
		});
		return elements;
	}
	template<class T>
	auto operator()(const Query<T>& query) const -> decltype(buildElements<T>(m_tables.get_table<T>(), query))
	{
		return buildElements<T>(m_tables.get_table<T>(), query);
	}

	explicit DB_Loader(const Test_tables& tables) : m_tables(tables) {}

	Test_tables m_tables;

};

template<class... Orms>
std::ostream& operator<<(std::ostream& stream, const Tables<Orms...>& tables) { return stream; }

template<class First, class... Orms>
std::ostream& operator<<(std::ostream& stream, const Tables<First, Orms...>& tables) {
	return stream << static_cast<const Table<First>&>(tables) << '\n'
		<< static_cast<const Tables<Orms...>&>(tables);
}

template<class... Args>
bool equal_impl(const Tables<Args...>&, const Tables<Args...>&) { return true; }

template<class First, class... Rest>
bool equal_impl(const Tables<First, Rest...>& lhs, const Tables<First, Rest...>& rhs) {
	if (lhs.get_table<First>() != rhs.get_table<First>())
		return false;
	return equal_impl(static_cast<const Tables<Rest...>&>(lhs), static_cast<const Tables<Rest...>&>(rhs));
}


struct DBSaver {
	Test_tables& m_tables;
	explicit DBSaver(Test_tables& tables) : m_tables(tables) {}

	template<class Range>
	void operator()(const Range& range) {
		for (const auto& p : range) {
			using orm_type = std::decay_t<std::remove_pointer_t<typename Range::value_type::second_type>>;
			m_tables.get_table<orm_with_ref_decay_t<orm_type>>().emplace_back(p.first, ToTuple()(*p.second));
		}
	}
};

struct DB_deleter
{
	Test_tables& m_tables;
	DB_deleter(Test_tables& tables) : m_tables(tables) {}

	template<class T>
	std::vector<unsigned long> operator()(const Query<T>& query)
	{
		auto items_to_delete = DB_Loader::buildElements<T>(m_tables.get_table<T>(), query);
		using items_to_delete_type = decltype(items_to_delete);
		using items_to_delete_value_type = typename items_to_delete_type::value_type;

		std::vector<unsigned long> ids_to_delete;
		std::transform(items_to_delete.begin(), items_to_delete.end(), std::back_inserter(ids_to_delete), [](const items_to_delete_value_type& p) { return p.first; });
		
		using table_t = typename std::decay<decltype(m_tables.get_table<T>())>::type;
		using table_t_value_type = typename table_t::value_type;

		auto deleted_it = std::remove_if(m_tables.get_table<T>().begin(), m_tables.get_table<T>().end(), [&](const table_t_value_type & p) {
			return std::find(ids_to_delete.begin(), ids_to_delete.end(), p.first) != ids_to_delete.end();
		});
		m_tables.get_table<T>().erase(deleted_it, m_tables.get_table<T>().end());
		return ids_to_delete;
	}
};