#pragma once

#include <string>

template<class Orm>

struct Datamodel {};
template<class Accessor, class Writer>
struct Column {
	explicit Column(std::string fieldName, Accessor accessor, Writer writer)
		: m_name(std::move(fieldName)), m_accessor(accessor), m_writer(writer) {}

	std::string m_name;
	Accessor m_accessor;
	Writer m_writer;
};

template<class Accessor, class Writer>
Column<Accessor, Writer> createColumn(std::string fieldName, Accessor accessor, Writer writer) {
	return Column<Accessor, Writer>(fieldName, accessor, writer);
}
