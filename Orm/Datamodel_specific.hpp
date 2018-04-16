#pragma once

#include "DataModel.hpp"
#include "OneToMany.hpp"

#include <vector>
#include <set>
#include <functional>

struct D {
	D() {}
	std::string getS() const { return m_s; }
	void setS(std::string val) { m_s = std::move(val); }
	D(std::string s) : m_s(std::move(s)) {}

	friend std::ostream& operator<<(std::ostream& stream, const D& d) {
		return stream << d.m_s;
	}
private:
	std::string m_s;
};
struct B {
	B() {}
	int getI() const { return m_i; }
	void setI(int val) { m_i = val; }
	explicit B(int i, std::vector<D> ds) : m_i(i), m_ds(ds) {}

	friend std::ostream& operator<<(std::ostream& stream, const B& b) {
		return stream << b.m_i;
	}
	void setDs(std::vector<D> val) { m_ds = std::move(val); }
	const std::vector<D>& getDs() const { return m_ds; }
private:
	int m_i{ 0 };
	std::vector<D> m_ds;
};

struct C {

	double getD() const { return m_d; }
	void setD(double val) { m_d = val; }
	explicit C(double d) : m_d(d) {}
	C() {}
	friend bool operator<(const C& lhs, const C& rhs) { return lhs.m_d < rhs.m_d; }
	friend std::ostream& operator<<(std::ostream& stream, const C& c) {
		return stream << c.m_d;
	}
private:
	double m_d{ 0 };
};

struct A {

	A(std::string field, double value, std::vector<B> bs, std::set<C> cs)
		: m_field(std::move(field)), m_bs(std::move(bs)), m_value(value), m_cs(std::move(cs)) {}

	A() {}


	void setField(std::string val) { m_field = std::move(val); }
	void setValue(double val) { m_value = val; }
	void setBs(std::vector<B> val) { m_bs = std::move(val); }
	void setCs(std::set<C> val) { m_cs = std::move(val); }

	const std::string& getField() const { return m_field; }
	const std::vector<B>& getBs() const { return m_bs; }
	const std::set<C>& getCs() const { return m_cs; }
	double getValue() const { return m_value; }
	friend std::ostream& operator<<(std::ostream& stream, const A& a) {
		return stream << a.m_field << ',' << a.m_value;
	}
private:
	std::string m_field;
	std::vector<B> m_bs;
	std::set<C> m_cs;
	double m_value{ 0 };
};
int A::m_copy_count = 0;

template<>
struct Datamodel<A> {
	enum class fields {FIELD =0, VALUE};
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "A_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("FIELD", std::mem_fun_ref(&A::getField), std::mem_fun_ref(&A::setField)),
							   createColumn("VALUE", std::mem_fun_ref(&A::getValue), std::mem_fun_ref(&A::setValue)));
	}
};

template<>
struct OneToMany<A> {
	static auto relations() {
		return std::make_tuple(createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getBs), std::mem_fun_ref(&A::setBs)),
							   createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getCs), std::mem_fun_ref(&A::setCs)));
	}
};


template<>
struct Datamodel<B> {
	enum class fields {INT=0};
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "B_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("INT", std::mem_fun_ref(&B::getI), std::mem_fun_ref(&B::setI)));
	}
};

template<>
struct OneToMany<B> {
	static auto relations() {
		return std::make_tuple(createOneToManyRelation("B_REF", std::mem_fun_ref(&B::getDs), std::mem_fun_ref(&B::setDs)));
	}
};
template<>
struct Datamodel<C> {
	enum class fields {DBL=0};
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "C_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("DBL", std::mem_fun_ref(&C::getD), std::mem_fun_ref(&C::setD)));
	}
};

template<>
struct Datamodel<D> {
	enum class fields {STR=0};
	static const std::string ref_label() { return "REF"; }
	static const std::string table_name() { return "D_Table"; }
	static auto columns() {
		return std::make_tuple(createColumn("STR", std::mem_fun_ref(&D::getS), std::mem_fun_ref(&D::setS)));
	}
};