#include "pch.h"
#include "Datamodel_specific.hpp"

#include "../Orm/save.hpp"
#include "../Orm/load.hpp"
#include "../Orm/erase.hpp"
#include "../Orm/Query.hpp"
#include "../Orm/TuplePrinter.hpp"

#include <iostream>

#include <vector>
#include <algorithm>

#include <functional>
#include <string>

struct IdGenerator {

	template<class Range>
	std::vector<unsigned long> operator()(const Range& range) const {
		int id = 0;
		std::vector<unsigned long> res;
		std::transform(range.begin(), range.end(), std::back_inserter(res), [&id](const auto& v) { return id++; });
		return res;
	}
};

struct ToTuple {
	template<class Orm, class Columns, size_t... Is>
	static auto toTuple_impl(const Orm& orm, const Columns& cols, std::index_sequence<Is...>) {
		return std::make_tuple(std::get<Is>(cols).m_accessor(orm)...);
	}

	template<class Orm>
	auto operator()(const Orm& orm) const {
		auto columns = Datamodel<Orm>::columns();
		return toTuple_impl(orm, columns, std::make_index_sequence<std::tuple_size_v<decltype(columns)>>());
	}
};

template<class Orm>
struct Tag {};

auto get_table(Tag<A>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, double>>>;
	tuples As =
	{
		{ 0,{ "Yay", 2. } },
		{ 1,{ "Boo", 3. } }
	};
	return As;
}
auto get_table(Tag<OrmWithRefToFather<A, B>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<int, unsigned long>>>;

	tuples B_with_refs =
	{
		{ 0,{ 1, 0 } },
		{ 1,{ 4, 0 } },
		{ 2,{ 6, 0 } },
		{ 3,{ 2, 1 } },
		{ 4,{ 3, 1 } }
	};
	return B_with_refs;
}
auto get_table(Tag<OrmWithRefToFather<B, D>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, unsigned long>>>;
	tuples D_with_refs =
	{
		{ 0,{ "D1", 0 } },
		{ 1,{ "D2", 0 } },
		{ 2,{ "D3", 1 } },
		{ 3,{ "D4", 1 } },
		{ 4,{ "D5", 2 } },
		{ 5,{ "D6", 2 } },
		{ 6,{ "D7", 3 } },
		{ 7,{ "D8", 3 } },
		{ 8,{ "D9", 4 } },
		{ 9,{ "D10",4 } },
	};
	return D_with_refs;
}
auto get_table(Tag<OrmWithRefToFather<A, C>>) {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<double, unsigned long>>>;
	tuples C_with_refs =
	{
		{ 0,{ 0.5, 0 } },
		{ 1,{ 1.2, 0 } },
		{ 2,{ 0.5, 1 } },
		{ 3,{ 1.2, 1 } }
	};
	return C_with_refs;
}



template<class Orm>
struct Table {
	using tuples = std::vector <std::pair<unsigned long, std::result_of_t<ToTuple(Orm)>>>;
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
static auto fromTuple(const Tuple& t) {
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
	auto& get_table() {
		return Table<T>::m_table;
	}
	template<class T>
	const auto& get_table() const {
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
		return range.empty() || std::find_if(range.begin(), range.end(), [&t](const auto& c) { return std::get<I>(t) == c; }) != range.end();
	}

	template<size_t I, class Tuple>
	static auto makeFilter(const Tuple& criterias) {
		return [&](const auto& t) { return find<I>(t, std::get<I>(criterias).second); };
	}


	template<class... Filters>
	struct CompositeFilters {
		std::tuple<Filters...> m_filters;
		explicit CompositeFilters(Filters... filters) : m_filters(filters...) {}

		template<size_t I, class Tuple, class T>
		static void apply_filter_impl(const Tuple& tuple, const T& t, bool& good) { good = good && std::get<I>(tuple)(t); }

		template<size_t I, class T>
		std::enable_if_t < I == sizeof...(Filters), bool> filter_impl(const T&) const { return true; }

		template<size_t I, class T>
		std::enable_if_t < I<sizeof...(Filters), bool> filter_impl(const T& t) const
		{
			if (!std::get<I>(m_filters)(t))
				return false;
			return filter_impl<I + 1>(t);
		}

		template<class T>
		bool operator()(const std::pair<unsigned long, T>& t) const {
			return filter_impl<0>(t.second);
		}
	};
	template<class... Filters>
	static auto createCompositeFilter(Filters... filters) {
		return CompositeFilters<Filters...>(filters...);
	}
	template<class Tuple, size_t... Is>
	static auto createCriteria_impl(const Tuple& criterias, std::index_sequence<Is...>) {
		return createCompositeFilter(makeFilter<Is>(criterias)...);
	}
	template<class Tuple>
	static auto createFilter(const Tuple& criterias) {
		return createCriteria_impl(criterias, std::make_index_sequence<std::tuple_size<Tuple>::value>());
	}

	template<class Orm, class Range, class Query>
	static auto buildElements(const Range& range, const Query& query) {
		std::vector<std::pair<unsigned long, Orm>> elements;
		Range filtered_elements;
		std::copy_if(range.begin(), range.end(), std::back_inserter(filtered_elements), createFilter(query.getAllCriterias()));
		std::transform(filtered_elements.begin(), filtered_elements.end(), std::back_inserter(elements), [](const auto& p) {
			return std::make_pair(p.first, fromTuple<Orm>(p.second)); 
		});
		return elements;
	}
	template<class T>
	auto operator()(const Query<T>& query) const
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
	auto operator()(const Query<T>& query)
	{
		auto items_to_delete = DB_Loader::buildElements<T>(m_tables.get_table<T>(), query);
		std::vector<unsigned long> ids_to_delete;
		std::transform(items_to_delete.begin(), items_to_delete.end(), std::back_inserter(ids_to_delete), [](const auto& p) { return p.first; });
		auto deleted_it = std::remove_if(m_tables.get_table<T>().begin(), m_tables.get_table<T>().end(), [&](const auto& p) {
			return std::find(ids_to_delete.begin(), ids_to_delete.end(), p.first) != ids_to_delete.end();
		});
		m_tables.get_table<T>().erase(deleted_it, m_tables.get_table<T>().end());
		return ids_to_delete;
	}
};
TEST(OrmTest, should_save_one_to_many_range) {
	//save
	A a("Yay", 2., { B(1,{ D("D1"), D("D2") }), B(4,{ D("D3"), D("D4") }), B(6,{ D("D5"), D("D6") }) }, { C(1.2), C(0.5) });
	A a2("Boo", 3., { B(2,{ D("D7"), D("D8") }), B(3,{ D("D9"), D("D10") }) }, { C(1.2), C(0.5) });
	std::vector<A> as{ a, a2 };

	IdGenerator id_generator;

	Test_tables reached_tables(false);
	DBSaver db_handler(reached_tables);

	save(as, db_handler, id_generator);	

	Test_tables expected_tables(true);
	EXPECT_EQ(expected_tables, reached_tables);
}
int CopyCounter<A>::m_copy_count = 0;
int CopyCounter<B>::m_copy_count = 0;
int CopyCounter<C>::m_copy_count = 0;
int CopyCounter<D>::m_copy_count = 0;

void reset_copy_counters() {
	CopyCounter<A>::m_copy_count = 0;
	CopyCounter<B>::m_copy_count = 0;
	CopyCounter<C>::m_copy_count = 0;
	CopyCounter<D>::m_copy_count = 0;
}

void expect_zero_copy_count() {
	EXPECT_EQ(0, CopyCounter<A>::m_copy_count);
	EXPECT_EQ(0, CopyCounter<B>::m_copy_count);
	EXPECT_EQ(0, CopyCounter<C>::m_copy_count);
	EXPECT_EQ(0, CopyCounter<D>::m_copy_count);
}

TEST(OrmTest, should_load_correctly) {
	Test_tables initial_tables(true);
	DB_Loader loader(initial_tables);
	A a("Yay", 2., { B(1,{ D("D1"), D("D2") }), B(4,{ D("D3"), D("D4") }), B(6,{ D("D5"), D("D6") }) }, { C(1.2), C(0.5) });
	A a2("Boo", 3., { B(2,{ D("D7"), D("D8") }), B(3,{ D("D9"), D("D10") }) }, { C(1.2), C(0.5) });
	{
		Query<A> query;
		using fields_A = Datamodel<A>::fields;
		query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay", "Boo" });

		reset_copy_counters();
		auto loaded_elements = loadElements(query, loader);
		expect_zero_copy_count();

		std::vector<A> expected_loaded{ a, a2 };

		EXPECT_EQ(expected_loaded, loaded_elements);
	}
	{
		Query<A> query;
		using fields_A = Datamodel<A>::fields;
		query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay"});

		reset_copy_counters();
		auto loaded_elements = loadElements(query, loader);
		expect_zero_copy_count();

		std::vector<A> expected_loaded{ a };
		EXPECT_EQ(expected_loaded, loaded_elements);
	}

}
TEST(OrmTest, should_erase_correctly) {
	{
		Test_tables initial_tables(true);

		Query<A> query;
		using fields_A = Datamodel<A>::fields;
		query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay", "Boo" });

		DB_deleter deleter(initial_tables);
		erase(query, deleter);
		EXPECT_TRUE(initial_tables.empty());
	}
	{
		Test_tables initial_tables(true);
		Query<A> query;
		using fields_A = Datamodel<A>::fields;
		query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay" });

		DB_deleter deleter(initial_tables);
		erase(query, deleter);
		
		DB_Loader loader(initial_tables);
		auto loadedElements = loadElements(Query<A>(), loader);
		std::vector<A> expected_loaded{ A("Boo", 3.,{ B(2,{ D("D7"), D("D8") }), B(3,{ D("D9"), D("D10") }) },{ C(1.2), C(0.5) }) };
		EXPECT_EQ(expected_loaded, loadedElements);
	}
}