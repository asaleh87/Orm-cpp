#pragma once

#include "DataModel.hpp"
#include "OneToMany.hpp"

#include <vector>
#include <set>
#include <functional>

struct D {
	std::string m_s;
	D(std::string s) : m_s(std::move(s)) {}

	const std::string& getS() const { return m_s; }
	friend std::ostream& operator<<(std::ostream& stream, const D& d) {
		return stream << d.m_s;
	}
};
struct B {
	int m_i;
	explicit B(int i, std::vector<D> ds) : m_i(i), m_ds(ds) {}
	int getI() const { return m_i; }
	friend std::ostream& operator<<(std::ostream& stream, const B& b) {
		return stream << b.m_i;
	}
	std::vector<D> m_ds;
	const std::vector<D>& getDs() const { return m_ds; }
};

struct C {
	double m_d;
	explicit C(double d) : m_d(d) {}
	double getD() const { return m_d; }
	friend bool operator<(const C& lhs, const C& rhs) { return lhs.m_d < rhs.m_d; }
	friend std::ostream& operator<<(std::ostream& stream, const C& c) {
		return stream << c.m_d;
	}
};

struct A {
	A(std::string field, double value, std::vector<B> bs, std::set<C> cs)
		: m_field(std::move(field)), m_bs(std::move(bs)), m_value(value), m_cs(std::move(cs)) {}

	std::string m_field;
	double m_value;
	std::vector<B> m_bs;
	std::set<C> m_cs;
	const std::string& getField() const { return m_field; }
	const std::vector<B>& getBs() const { return m_bs; }
	const std::set<C>& getCs() const { return m_cs; }
	double getValue() const { return m_value; }
	friend std::ostream& operator<<(std::ostream& stream, const A& a) {
		return stream << a.m_field << ',' << a.m_value;
	}
};

template<>
struct Datamodel<A> {
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "A_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("FIELD", std::mem_fun_ref(&A::getField)),
							   createColumn("VALUE", std::mem_fun_ref(&A::getValue)));
	}
};

template<>
struct OneToMany<A> {
	static auto relations() {
		return std::make_tuple(createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getBs)),
							   createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getCs)));
	}
};


template<>
struct Datamodel<B> {
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "B_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("INT", std::mem_fun_ref(&B::getI)));
	}
};

template<>
struct OneToMany<B> {
	static auto relations() {
		return std::make_tuple(createOneToManyRelation("B_REF", std::mem_fun_ref(&B::getDs)));
	}
};
template<>
struct Datamodel<C> {
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "C_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("DBL", std::mem_fun_ref(&C::getD)));
	}
};

template<>
struct Datamodel<D> {
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "D_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("STR", std::mem_fun_ref(&D::getS)));
	}
};