#pragma once
#include <vector>
#include <type_traits>
#include <tuple>
#include <memory>
#include "DataModel.hpp"

template<class Column, class Orm>
struct CriteriaByColumn {};
	
template<class Accessor, class Writer, class Orm>
struct CriteriaByColumn<Orm, Column<Accessor, Writer>> {
	using type = std::pair<Column<Accessor, Writer>, std::vector<std::decay_t<std::result_of_t<Accessor(Orm)>>>>;
};

template<class Orm, class Tuple>
struct columns_to_criterias{};

template<class Orm, class... Args>
struct columns_to_criterias <Orm, std::tuple<Args...>>
{
	using type = std::tuple<typename CriteriaByColumn<Orm, Args>::type...>;
};

template<class Orm, class Tuple>
using columns_to_criterias_t = typename columns_to_criterias<Orm, Tuple>::type;

template<class Orm, class Tuple, size_t... Is>
auto initialize_impl(const Tuple& tuple, std::index_sequence<Is...>) {
	using result_type = columns_to_criterias_t<Orm, Tuple>;
	return std::make_tuple(std::make_pair(std::get<Is>(tuple), typename std::tuple_element_t<Is, result_type>::second_type())...);
}

template<class Orm, class Id = unsigned long>
struct Query {
	using fieldType = typename Datamodel<Orm>::fields;
	using criterias = typename columns_to_criterias < Orm, decltype(Datamodel<Orm>::columns())>::type;

	Query() : m_criterias(initializeEmptyCriterias()) {}

	template<fieldType field, class Range>
	Query& withCriteria(const Range& range) {
		auto& values_for_field = std::get<static_cast<int>(field)>(m_criterias).second;
		std::copy(range.begin(), range.end(), std::back_inserter(values_for_field));
		return *this;
	}
	template<class Range>
	Query& withIds(const Range& range) {
		std::copy(range.begin(), range.end(), std::back_inserter(m_queryByIds));
		return *this;
	}
	template<fieldType field>
	const auto& getCriterias() const {
		return std::get<static_cast<int>(field)>(m_criterias).second;
	}
	const auto& getIds() const {
		return m_queryByIds;
	}
	template<fieldType field>
	bool hasCriteria() const {
		return !std::get<static_cast<int>(field)>(m_criterias).second.empty();
	}

	const criterias& getAllCriterias() const { return m_criterias; }
private:
	std::vector<Id> m_queryByIds;
	criterias m_criterias;
	auto initializeEmptyCriterias() {
		return initialize_impl<Orm>(Datamodel<Orm>::columns(), std::make_index_sequence<std::tuple_size<decltype(Datamodel<Orm>::columns())>::value>());
	}
};
