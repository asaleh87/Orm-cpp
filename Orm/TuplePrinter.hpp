#pragma once

#include <ostream>
#include <type_traits>
#include "TypedIndex.hpp"

template<class Tuple>
struct TuplePrinter {
	const Tuple& m_tuple;
	TuplePrinter(const Tuple& tuple) : m_tuple(tuple) {}

	static void print(std::ostream&, const Tuple&, TypedIndex<std::tuple_size<Tuple>::value>) {}

	template<size_t I>
	static void print(std::ostream& stream, const Tuple& t, TypedIndex<I>) {
		stream << std::get<I>(t);
		if (I != std::tuple_size<Tuple>::value -1)
			stream << ',';

		print(stream, t, TypedIndex<I+1>());
	}
	friend std::ostream& operator<<(std::ostream& stream, const TuplePrinter& t) {
		stream << '{';
		print(stream, t.m_tuple, TypedIndex<0>());
		return stream << '}';
	}
};

template<class Tuple>
TuplePrinter<Tuple> makeTuplePrinter(const Tuple& tuple) {
	return TuplePrinter<Tuple>(tuple);
}
