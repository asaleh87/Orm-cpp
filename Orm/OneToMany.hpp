#pragma once

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

#define DECLARE_ONETOMANY_1(Type, COL1)\
template<>\
struct OneToMany<Type> {\
	static auto relations() {																							   \
		return std::make_tuple(COL1);					   \
	}																												   \
};																													   \

#define DECLARE_ONETOMANY_2(Type, COL1, COL2)\
 template<>\
struct OneToMany<Type> {\
	static auto relations() {\
		return std::make_tuple(COL1, COL2);\
	}\
};\

#define DECLARE_ONETOMANY_3(Type, COL1, COL2, COL3)																   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_4(Type, COL1, COL2, COL3, COL4)																   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_5(Type, COL1, COL2, COL3, COL4, COL5)																   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_6(Type, COL1, COL2, COL3, COL4, COL5, COL6)																   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6);					   \
	}																												   \
};		

#define DECLARE_ONETOMANY_7(Type, COL1, COL2, COL3, COL4, COL5, COL6, COL7)																   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_8(Type, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8)																   \
 template<>																											   \
struct OneToMany<Type> {																							   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_9(Type, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9)				   \
 template<>																											   \
struct OneToMany<Type> {																							   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9);					   \
	}																												   \
};		
#define DECLARE_ONETOMANY_10(Type, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10)		   \
 template<>																											   \
struct OneToMany<Type> {																								   \
	static auto relations() {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10);						\
	}																												   \
};	