#pragma once

#include <string>

template<class Orm>
struct Datamodel {};

template<class Orm>
struct FieldIndex { using type = size_t; };
#define DECLARE_DATAMODEL(Type, Table, REFCOLUMN, TUPLE)\
template<>\
struct Datamodel<Type> {\
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(TUPLE) \
	{ \
		return TUPLE;					   \
	}																												   \
};					

#define DECLARE_DATAMODEL_1(Type, Table, REFCOLUMN, COL1)\
template<>\
struct Datamodel<Type> {\
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1)) \
	{ \
		return std::make_tuple(COL1);					   \
	}																												   \
};																													   \

#define DECLARE_DATAMODEL_2(Type, Table, REFCOLUMN, COL1, COL2)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2))\
    {\
		return std::make_tuple(COL1, COL2);\
	}\
};\

#define DECLARE_DATAMODEL_3(Type, Table, REFCOLUMN, COL1, COL2, COL3)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3))\
	{\																							   \
		return std::make_tuple(COL1, COL2, COL3);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_4(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(td::make_tuple(COL1, COL2, COL3, COL4))\
	{\																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_5(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5))\
	{\																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_6(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5, COL6)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6)) {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6);					   \
	}																												   \
};		

#define DECLARE_DATAMODEL_7(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5, COL6, COL7)																   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7)) {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_8(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8)																   \
 template<>																											   \
struct Datamodel<Type> {																							   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8)) {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_9(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9)				   \
 template<>																											   \
struct Datamodel<Type> {																							   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9)) {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9);					   \
	}																												   \
};		
#define DECLARE_DATAMODEL_10(Type, Table, REFCOLUMN, COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10))		   \
 template<>																											   \
struct Datamodel<Type> {																								   \
	using fields = typename FieldIndex<Type>::type;																	   \
	static const std::string ref_label() { return Table; }															   \
	static const std::string table_name() { return REFCOLUMN; }														   \
	static auto columns() -> decltype(std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10)) {																							   \
		return std::make_tuple(COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10);						\
	}																												   \
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
