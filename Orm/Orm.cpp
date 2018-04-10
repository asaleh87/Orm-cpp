// Orm.cpp : Defines the entry point for the console application.
//



#include "Datamodel_specific.hpp"
#include "save.hpp"
#include "load.hpp"
#include "erase.hpp"
#include "Query.hpp"

#include <iostream>

#include <vector>
#include <algorithm>

#include <functional>
#include <string>

#include <set>

struct IdGenerator {

	template<class Range>
	std::vector<int> operator()(const Range& range) const {
		int id = 0;
		std::vector<int> res;
		std::transform(range.begin(), range.end(), std::back_inserter(res), [&id](const auto& v) { return id++; });
		return res;
	}
};

template<class T>
struct Table;

template<> struct Table<A> {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, double>>>;
	tuples m_table;
	static auto get_table() {
		tuples As =
		{
			{ 1,{ "Yay", 2. } },
			{ 2,{ "Boo", 3. } }
		};
		return As;
	}
	Table() : m_table(get_table()) {}
};
template<> struct Table<OrmWithRefToFather<A, B>> {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<int, unsigned long>>>;
	
	static auto get_table() {
		tuples B_with_refs =
		{
			{ 1,{ 2, 1 } },
			{ 2,{ 1, 1 } },
			{ 3,{ 5, 2 } },
			{ 4,{ 6, 2 } }
		};
		return B_with_refs;
	}
	tuples m_table;
	Table() : m_table(get_table()) {}
};
template<> struct Table<OrmWithRefToFather<B, D>> {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, unsigned long>>>;
	static auto get_table() {
		tuples D_with_refs =
		{
			{ 1,{ "D1", 1 } },
			{ 2,{ "D2", 2 } },
			{ 3,{ "D3", 3 } },
			{ 4,{ "D4", 4 } }
		};
		return D_with_refs;
	}
	tuples m_table;
	Table() : m_table(get_table()) {}

};
template<> struct Table<OrmWithRefToFather<A, C>> {
	using tuples = std::vector<std::pair<unsigned long, std::tuple<double, unsigned long>>>;
	static auto get_table() {
		tuples C_with_refs =
		{
			{ 1,{ 2., 1 } },
			{ 2,{ 1., 1 } },
			{ 3,{ 5., 2 } },
			{ 4,{ 6., 2 } }
		};
		return C_with_refs;
	}
	tuples m_table;
	Table() : m_table(get_table()) {}
};
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
		std::enable_if_t < I==sizeof...(Filters), bool> filter_impl(const T&) const { return true; }
		
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

	template<class Orm, class Range, class Query>
	static auto buildElements(const Range& range, const Query& query) {
		std::vector<std::pair<unsigned long, Orm>> elements;
		Range filtered_elements;
		std::copy_if(range.begin(), range.end(), std::back_inserter(filtered_elements), createFilter(query.getAllCriterias()));
		std::transform(filtered_elements.begin(), filtered_elements.end(), std::back_inserter(elements), [](const auto& p) {return std::make_pair(p.first, fromTuple<Orm>(p.second)); });
		return elements;
	}
	template<class T>
	auto operator()(const Query<T>& query) const
	{
		return buildElements<T>(Table<T>::get_table(), query);
	}
};
struct DBSaver {
	template<class Range>
	void operator()(const Range& range) {
		for (const auto& p : range) {
			std::cout << p.first << ',' << p.second << '\n';
		}
		std::cout << '\n';
	}
};

struct Tables : Table<A>, Table<OrmWithRefToFather<A, B>>, Table<OrmWithRefToFather<A, C>>, Table<OrmWithRefToFather<B, D>>
{
	template<class T>
	auto& get_table() {
		return Table<T>::m_table;
	}
	template<class T>
	const auto& get_table() const {
		return Table<T>::m_table;
	}
};

struct DB_deleter
{
	Tables& m_tables;
	DB_deleter(Tables& tables) : m_tables(tables) {}

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
int main() {
	//save
	A a("Yay", 2., { B(1, {D("D1"), D("D2")}), B(4,{ D("D3"), D("D4") }), B(6,{ D("D9"), D("D10") }) }, { C(1.2), C(0.5) });
	A a2("Boo", 2., { B(2,{ D("D5"), D("D6") }), B(3,{ D("D7"), D("D8") }) }, { C(1.2), C(0.5) });
	std::vector<A> as{ a, a2 };
	IdGenerator id_generator;
	DBSaver db_handler;
	save(as, db_handler, id_generator);

	//load
	Query<A> query;
	DB_Loader loader;
	using fields_A = Datamodel<A>::fields;
	query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay", "Boo" });
	auto loaded_elements = loadElements(query, loader);

	//erase
	Tables tables;
	DB_deleter deleter(tables);
	erase(query, deleter);
	return 0;
}