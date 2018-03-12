#pragma once

#include <string>

template<class Orm>

struct Datamodel {};
template<class Accessor>
struct Column {
	explicit Column(std::string fieldName, Accessor accessor)
		: m_name(std::move(fieldName)), m_accessor(accessor) {}

	std::string m_name;
	Accessor m_accessor;
};

template<class Accessor>
Column<Accessor> createColumn(std::string fieldName, Accessor accessor) {
	return Column<Accessor>(fieldName, accessor);
}
