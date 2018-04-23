#pragma once

#include "../Orm/DataModel.hpp"
#include "../Orm/OneToMany.hpp"
#include "../Orm/TuplePrinter.hpp"

#include <vector>
#include <set>
#include <functional>

template<class Range>
struct RangePrinter {
	const Range& m_range;
	explicit RangePrinter(const Range& range) : m_range(range) {}

	friend std::ostream& operator<<(std::ostream& stream, const RangePrinter& range) {
		stream << '{';
		for (const auto& e : range.m_range)
			stream << e << ", ";
		return stream << '}';
	}
};

template<class Range>
auto makeRangePrinter(const Range& range) {
	return RangePrinter<Range>(range);
}


template<class Impl>
struct CopyCounter {
	static int m_copy_count;
	CopyCounter() noexcept {}
	CopyCounter(const CopyCounter&) noexcept {
		++m_copy_count;
	}
	CopyCounter& operator=(const CopyCounter&) noexcept {
		++m_copy_count;
		return *this;
	}
	CopyCounter(CopyCounter&&) noexcept {}
	CopyCounter& operator=(CopyCounter&&) noexcept { return *this; }
};

struct D : CopyCounter<D> {
	template<class That>
	void init(That&& that) {
		static_cast<CopyCounter<D>&>(*this) = std::forward<That>(that);
		m_s = that.m_s;
	}
	D& operator=(D&& that) noexcept {
		init(std::move(that));
		return *this;
	}
	D& operator=(const D& that) noexcept {
		init(that);
		return *this;
	}
	D(D&& that) noexcept { init(std::move(that)); }
	D(const D& that) noexcept { init(that); }

	D() {}
	const std::string& getS() const { return m_s; }
	void setS(std::string val) { m_s = std::move(val); }
	D(std::string s) : m_s(std::move(s)) {}

	friend std::ostream& operator<<(std::ostream& stream, const D& d) {
		return stream << d.m_s;
	}
	friend bool operator==(const D& lhs, const D& rhs) {
		return lhs.m_s == rhs.m_s;
	}
private:
	std::string m_s;
};

struct B : CopyCounter<B> {
	B() {}
	int getI() const { return m_i; }
	void setI(int val) { m_i = val; }
	explicit B(int i, std::vector<D> ds) : m_i(i), m_ds(ds) {}

	template<class That>
	void init(That&& that) {
		static_cast<CopyCounter<B>&>(*this) = std::forward<That>(that);
		m_i = that.m_i;
		m_ds = std::move(that.m_ds);
	}
	B& operator=(B&& that) noexcept {
		init(std::move(that));
		return *this;
	}
	B& operator=(const B& that) noexcept {
		init(that);
		return *this;
	}
	B(B&& that) noexcept { init(std::move(that)); }
	B(const B& that) noexcept { init(that); }

	friend std::ostream& operator<<(std::ostream& stream, const B& b) {
		return stream << makeTuplePrinter(std::make_tuple(b.m_i, makeRangePrinter(b.m_ds)));

	}
	void setDs(std::vector<D> val) { m_ds = std::move(val); }
	const std::vector<D>& getDs() const { return m_ds; }

	friend bool operator==(const B& lhs, const B& rhs) {
		return lhs.m_i == rhs.m_i && lhs.m_ds == rhs.m_ds;
	}

private:
	int m_i{ 0 };
	std::vector<D> m_ds;
};

struct C : CopyCounter<C> {

	double getD() const { return m_d; }
	void setD(double val) { m_d = val; }
	explicit C(double d) : m_d(d) {}
	template<class That>
	void init(That&& that) {
		static_cast<CopyCounter<C>&>(*this) = std::forward<That>(that);
		m_d = that.m_d;
	}
	C& operator=(C&& that) noexcept {
		init(std::move(that));
		return *this;
	}
	C& operator=(const C& that) noexcept {
		init(that);
		return *this;
	}
	C(C&& that) noexcept { init(std::move(that)); }
	C(const C& that) noexcept { init(that); }

	C() {}
	friend bool operator<(const C& lhs, const C& rhs) { return lhs.m_d < rhs.m_d; }
	friend std::ostream& operator<<(std::ostream& stream, const C& c) {
		return stream << c.m_d;
	}
	friend bool operator==(const C& lhs, const C& rhs) {
		return lhs.m_d == rhs.m_d;
	}

private:
	double m_d{ 0 };
};

struct A : CopyCounter<A> {
	template<class That>
	void init(That&& that) {
		static_cast<CopyCounter<A>&>(*this) = std::forward<That>(that);
		m_field = std::move(that.m_field);
		m_value = that.m_value;
		m_bs = std::move(that.m_bs);
		m_cs = std::move(that.m_cs);
	}

	static int m_copy_count;
	A(const A& that) { init(that); }
	A(A&& that) noexcept { init(std::move(that)); }

	A& operator=(const A& that) noexcept { 
		init(that);
		return *this;
	}
	A& operator=(A&& that) noexcept {
		init(std::move(that));
		return *this;
	}

	A(std::string field, double value, std::vector<B> bs, std::set<C> cs)
		: m_field(std::move(field)), m_bs(std::move(bs)), m_value(value), m_cs(std::move(cs)) {}

	A() {}
	void setField(std::string val) { m_field = std::move(val); }
	void setValue(double val) { m_value = val; }
	void setBs(std::vector<B> val) { 
		m_bs = std::move(val); 
	}
	void setCs(std::set<C> val) { m_cs = std::move(val); }

	const std::string& getField() const { return m_field; }
	const std::vector<B>& getBs() const { return m_bs; }
	const std::set<C>& getCs() const { return m_cs; }
	double getValue() const { return m_value; }
	friend std::ostream& operator<<(std::ostream& stream, const A& a) {
		return stream << makeTuplePrinter(std::make_tuple(a.m_field, a.m_value, makeRangePrinter(a.m_bs), makeRangePrinter(a.m_cs)));
	}
private:
	std::string m_field;
	std::vector<B> m_bs;
	std::set<C> m_cs;
	double m_value{ 0 };
	friend bool operator==(const A& lhs, const A& rhs) {
		return lhs.m_field == rhs.m_field &&
				lhs.m_value == rhs.m_value &&
				lhs.m_bs == rhs.m_bs &&
				lhs.m_cs == rhs.m_cs;
	}
};

//re-implement std::mem_fun_ref for setters to propagate move of parameter
template<class Res, class T, class P>
struct Writer {
	Res(T::*m_fn)(P);
	explicit Writer(Res(T::*fn)(P)) : m_fn(fn) {}
	Res operator()(T& t, P arg) const {
		return (t.*m_fn)(std::move(arg));
	}
};

template<class Res, class T, class P>
Writer<Res, T, P> writer(Res(T::*fn)(P)) { return Writer<Res, T, P>(fn); }

// DECLARE DATAMODELS
template<> struct FieldIndex<A> { enum class type { FIELD = 0, VALUE }; };
DECLARE_DATAMODEL(A, "A_Table", "REF",
					createColumn("FIELD", std::mem_fun_ref(&A::getField), writer(&A::setField)),
					createColumn("VALUE", std::mem_fun_ref(&A::getValue), writer(&A::setValue)));

DECLARE_ONETOMANY(A,
					createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getBs), writer(&A::setBs)),
					createOneToManyRelation("A_REF", std::mem_fun_ref(&A::getCs), writer(&A::setCs)));

DECLARE_DATAMODEL(B, "B_Table", "REF", createColumn("INT", std::mem_fun_ref(&B::getI), writer(&B::setI)));

DECLARE_ONETOMANY(B, createOneToManyRelation("B_REF", std::mem_fun_ref(&B::getDs), writer(&B::setDs)));

DECLARE_DATAMODEL(C, "C_Table", "REF", createColumn("DBL", std::mem_fun_ref(&C::getD), writer(&C::setD)));

DECLARE_DATAMODEL(D, "D_Table", "REF", createColumn("STR", std::mem_fun_ref(&D::getS), writer(&D::setS)));

