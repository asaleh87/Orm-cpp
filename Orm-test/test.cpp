#include "pch.h"
#include "DBHandling.hpp"

#include "../Orm/save.hpp"
#include "../Orm/load.hpp"
#include "../Orm/erase.hpp"
#include "../Orm/Query.hpp"

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