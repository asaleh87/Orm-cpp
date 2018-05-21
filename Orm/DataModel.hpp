#pragma once
#include "FieldAccessor.hpp"

#include <string>

template<class Orm>
struct Datamodel {};

template<class Orm>
struct FieldIndex { using type = size_t; };

#define DECLARE_DATAMODEL_ALL(Type, Table, REFCOLUMN, REFCOL_LENGTH, REFCOL_TYPE, ...) \
template<> \
struct Datamodel<Type> { \
	using fields = FieldIndex<Type>::type;	\
	static const int ref_col_size = REFCOL_LENGTH; \
	using ref_col_type = REFCOL_TYPE; \
	static const std::string ref_label() { return Table; } \
	static const std::string table_name() { return REFCOLUMN; } \
	static auto columns() -> decltype(std::make_tuple(__VA_ARGS__)) \
	{ \
		return std::make_tuple(__VA_ARGS__); \
	} \
};					

#define DECLARE_DATAMODEL_WITH_LENGTH(Type, Table, REFCOLUMN, REFCOL_LENGTH,...) DECLARE_DATAMODEL_ALL(Type, Table, REFCOLUMN, REFCOL_LENGTH, unsigned long, __VA_ARGS__)

#define DECLARE_DATAMODEL(Type, Table, REFCOLUMN,...) DECLARE_DATAMODEL_WITH_LENGTH(Type, Table, REFCOLUMN, 19, unsigned long, __VA_ARGS__))

template<class Accessor, class Writer, class Impl>
struct AbstractColumn {
	explicit AbstractColumn(std::string fieldName, Accessor accessor, Writer writer, int length)
		: m_name(std::move(fieldName)), m_accessor(accessor), m_writer(writer), m_length(length) {}

	std::string m_name;
	Accessor m_accessor;
	Writer m_writer;
	int m_length;
};


template<class Accessor, class Writer>
struct Column : public AbstractColumn<Accessor, Writer, Column<Accessor, Writer>> {
	explicit Column(std::string fieldName, Accessor accessor, Writer writer, int length)
		: AbstractColumn<Accessor, Writer, Column>(fieldName, accessor, writer, length) {}
};
template<class Accessor, class Writer>
struct NumberColumn : AbstractColumn<Accessor, Writer, NumberColumn<Accessor, Writer>>
{
	explicit NumberColumn(std::string fieldName, Accessor accessor, Writer writer, int length, int nbDecimals)
		: AbstractColumn<Accessor, Writer, NumberColumn>(fieldName, accessor, writer, length), m_nbDecimals(nbDecimals) {}

	int m_nbDecimals;
};
template<class Accessor, class Writer>
auto createColumn(std::string fieldName, Accessor accessor, Writer writer, int length) {
	return Column<Accessor, Writer>(fieldName, accessor, writer, length);
}
template<class T, class FieldType>
auto createColumn(std::string fieldName, FieldType T::* field, int length) {
	static_assert(!std::is_floating_point_v<FieldType>, "you have to specify number of decimals for floating type field. Use createNumberColumn instead");
	auto accessor = makeFieldAccessor(field);
	return createColumn(fieldName, accessor, accessor, length);
}

template<class T, class FieldType>
auto createNumberColumn(std::string fieldName, FieldType T::* field, int length, int decimals) {
	static_assert(std::is_floating_point_v<FieldType>, "use createColumn for non-floating fields");
	auto accessor = makeFieldAccessor(field);
	return NumberColumn<decltype(accessor), decltype(accessor)>(fieldName, accessor, accessor, length, decimals);
}


