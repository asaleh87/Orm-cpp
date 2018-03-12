#pragma once

#include <tuple>
#include <string>

template<class Orm>
struct OneToMany {
	static std::tuple<> relations() { return std::make_tuple(); }
};

template<class Accessor>
struct OneToManyRelation {
	using accessor_t = Accessor;
	std::string m_refColumn;
	Accessor m_accessor;
	OneToManyRelation(std::string refColumn, Accessor accessor)
		: m_refColumn(std::move(refColumn)), m_accessor(accessor) {}
};

template<class Accessor>
OneToManyRelation<Accessor> createOneToManyRelation(std::string refColumn, Accessor accessor) {
	return OneToManyRelation<Accessor>(refColumn, accessor);
}
