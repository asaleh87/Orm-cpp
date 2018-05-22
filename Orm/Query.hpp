#pragma once
#include <vector>
#include <type_traits>
#include <tuple>
#include <memory>
#include "DataModel.hpp"

template<class Column, class Orm>
struct CriteriaByColumn {};
	
template<class Accessor, class Writer, class Orm, template<class, class> class Col>
struct CriteriaByColumn<Orm, Col<Accessor, Writer>> {
	using type = std::pair<Col<Accessor, Writer>, std::vector<std::decay_t<std::result_of_t<Accessor(Orm)>>>>;
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
auto initialize_impl(const Tuple& tuple, util::index_sequence<Is...>) -> decltype(std::make_tuple(std::make_pair(std::get<Is>(tuple), typename std::tuple_element<Is, columns_to_criterias_t<Orm, Tuple>>::type::second_type())...))
{
	using result_type = columns_to_criterias_t<Orm, Tuple>;
	return std::make_tuple(std::make_pair(std::get<Is>(tuple), typename std::tuple_element<Is, result_type>::type::second_type())...);
}

template<class Orm, class Id = unsigned long>
struct Query {
	using fieldType = typename Datamodel<Orm>::fields;
	using criterias = typename columns_to_criterias < Orm, decltype(Datamodel<Orm>::columns())>::type;

	Query() : m_criterias(initialize_impl<Orm>(Datamodel<Orm>::columns(), util::make_index_sequence<std::tuple_size<decltype(Datamodel<Orm>::columns())>::value>())) {}

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
	auto getCriterias() const -> decltype(std::get<static_cast<int>(field)>(m_criterias).second)
	{
		return std::get<static_cast<int>(field)>(m_criterias).second;
	}
	const std::vector<Id>& getIds() const {
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
};


template<class RefToFather, class Range>
Query<RefToFather> makeFatherIdQuery(const Range& range) {
	Query<RefToFather> query;
	static const int father_ref_index = Datamodel<RefToFather>::father_ref_index;
	query.withCriteria<father_ref_index>(range);
	return query;
}