#pragma once
#include "FieldAccessor.hpp"

#include <string>

template<class Orm>
struct Datamodel {};

template<class Orm>
struct FieldIndex { using type = size_t; };
#define DECLARE_DATAMODEL(Type, Table, REFCOLUMN, ...)\
template<>\
struct Datamodel<Type> {\
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(__VA_ARGS__)) \
	{ \
		return std::make_tuple(__VA_ARGS__);					   \
	}\
};					

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

template<class T, class FieldType>
auto createColumn(std::string fieldName, FieldType T::* field) {
	auto accessor = makeFieldAccessor(field);
	return createColumn(fieldName, accessor, accessor);
}
