#pragma once

#include <ostream>
#include <type_traits>

template<class Tuple>
struct TuplePrinter {
	const Tuple& m_tuple;
	TuplePrinter(const Tuple& tuple) : m_tuple(tuple) {}

	template<size_t... Is>
	static void print(std::ostream&, const Tuple&, std::index_sequence<Is...>) {}

	template<size_t I, size_t... Is>
	static void print(std::ostream& stream, const Tuple& t, std::index_sequence<I, Is...>) {
		stream << std::get<I>(t);
		if (I != std::tuple_size_v<Tuple> -1)
			stream << ',';

		print(stream, t, std::index_sequence<Is...>());
	}
	friend std::ostream& operator<<(std::ostream& stream, const TuplePrinter& t) {
		stream << '{';
		print(stream, t.m_tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>());
		return stream << '}';
	}
};

template<class Tuple>
TuplePrinter<Tuple> makeTuplePrinter(const Tuple& tuple) {
	return TuplePrinter<Tuple>(tuple);
}
