// Orm.cpp : Defines the entry point for the console application.
//



#include "Datamodel_specific.hpp"
#include "save.hpp"
#include "load.hpp"

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
auto A_Table() {
	A a("Yay", 2., { B(1,{ D("D1"), D("D2") }), B(4,{ D("D3"), D("D4") }), B(6,{ D("D9"), D("D10") }) }, { C(1.2), C(0.5) });
	A a2("Boo", 2., { B(2,{ D("D5"), D("D6") }), B(3,{ D("D7"), D("D8") }) }, { C(1.2), C(0.5) });
	return std::vector<A> { a, a2 };
}

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
	auto buildElements(const Range& range, const Query& query) const {
		std::vector<std::pair<unsigned long, Orm>> elements;
		Range filtered_elements;
		std::copy_if(range.begin(), range.end(), std::back_inserter(filtered_elements), createFilter(query.getAllCriterias()));
		std::transform(filtered_elements.begin(), filtered_elements.end(), std::back_inserter(elements), [](const auto& p) {return std::make_pair(p.first, fromTuple<Orm>(p.second)); });
		return elements;
	}
	auto operator()(const Query<A>& query) const
	{
		using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, double>>>;
		tuples As = 
		{
			{1, {"Yay", 2.} },
			{2, {"Boo", 3.} }
		};
		return buildElements<A>(As, query);
	}
	auto operator()(const Query<OrmWithRefToFather<A, B>>& query) {
		using tuples = std::vector<std::pair<unsigned long, std::tuple<int, unsigned long>>>;
		tuples B_with_refs =
		{
			{ 1,{2, 1}},
			{ 2,{1, 1}},
			{ 3,{5, 2}},
			{ 4,{6, 2}}
		};
		return buildElements<OrmWithRefToFather<A, B>>(B_with_refs, query);
	}
	auto operator()(const Query<OrmWithRefToFather<A, C>>& query) {
		using tuples = std::vector<std::pair<unsigned long, std::tuple<double, unsigned long>>>;
		tuples C_with_refs =
		{
			{ 1,{2., 1}},
			{ 2,{1., 1}},
			{ 3,{5., 2}},
			{ 4,{6., 2}}
		};
		return buildElements<OrmWithRefToFather<A, C>>(C_with_refs, query);
	}
	auto operator()(const Query<OrmWithRefToFather<B, D>>& query) {
		using tuples = std::vector<std::pair<unsigned long, std::tuple<std::string, unsigned long>>>;
		tuples D_with_refs =
		{
			{ 1,{"D1", 1}},
			{ 2,{"D2", 2}},
			{ 3,{"D3", 3}},
			{ 4,{"D4", 4}}
		};
		return buildElements<OrmWithRefToFather<B, D>>(D_with_refs, query);
	}
};
struct DBHandler {
	template<class Range>
	void operator()(const Range& range) {
		for (const auto& p : range) {
			std::cout << p.first << ',' << p.second << '\n';
		}
		std::cout << '\n';
	}
};

int main() {
	A a("Yay", 2., { B(1, {D("D1"), D("D2")}), B(4,{ D("D3"), D("D4") }), B(6,{ D("D9"), D("D10") }) }, { C(1.2), C(0.5) });
	A a2("Boo", 2., { B(2,{ D("D5"), D("D6") }), B(3,{ D("D7"), D("D8") }) }, { C(1.2), C(0.5) });
	std::vector<A> as{ a, a2 };
	IdGenerator id_generator;
	DBHandler db_handler;
	auto ids = id_generator(as);
	db_handler(zip(ids, make_cref(as)));
	saveChildren(zip(ids, make_cref(as)), db_handler, id_generator);
	
	Query<A> query;
	DB_Loader loader;
	using fields_A = Datamodel<A>::fields;
	query.withCriteria<fields_A::FIELD>(std::vector<std::string>{ "Yay", "Boo" });
	auto loaded_elements = loadElements(query, loader);
	return 0;
}