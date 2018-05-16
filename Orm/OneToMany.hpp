#pragma once
#include "FieldAccessor.hpp"

#include <tuple>
#include <string>

template<class Orm>
struct OneToMany {
	static std::tuple<> relations() { return std::make_tuple(); }
};

template<class Accessor, class Writer>
struct OneToManyRelation {
	using accessor_t = Accessor;
	using writer_t = Writer;
	std::string m_refColumn;
	Accessor m_accessor;
	Writer m_writer;
	OneToManyRelation(std::string refColumn, Accessor accessor, Writer writer)
		: m_refColumn(std::move(refColumn)), m_accessor(accessor), m_writer(writer) {}
};

template<class Accessor, class Writer>
OneToManyRelation<Accessor, Writer> createOneToManyRelation(std::string refColumn, Accessor accessor, Writer writer) {
	return OneToManyRelation<Accessor, Writer>(refColumn, accessor, writer);
}

template<class T, class FieldType>
auto createOneToManyRelation(std::string refColumn, FieldType T::* field) -> decltype(createOneToManyRelation(refColumn, makeFieldAccessor(field), makeFieldAccessor(field)))
{
	auto accessor = makeFieldAccessor(field);
	return createOneToManyRelation(refColumn, accessor, accessor);
}

#define DECLARE_ONETOMANY(Type, ...)\
template<>\
struct OneToMany<Type> {\
	static auto relations() -> decltype(std::make_tuple(__VA_ARGS__)) {\
		return std::make_tuple(__VA_ARGS__);\
	}\
};
