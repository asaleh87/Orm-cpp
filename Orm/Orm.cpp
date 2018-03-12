// Orm.cpp : Defines the entry point for the console application.
//



#include "Datamodel_specific.hpp"
#include "save.hpp"

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
		using value_type = typename Range::value_type;
		std::vector<int> res;
		std::transform(range.begin(), range.end(), std::back_inserter(res), [&id](const value_type& v) { return id++; });
		return res;
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
	db_handler(zip(ids, make_ref(as)));
	saveChildren(zip(ids, make_ref(as)), db_handler, id_generator);
	return 0;
}