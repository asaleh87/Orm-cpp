#pragma once
#include <type_traits>
	
template<class T, class FieldType>
struct FieldAccessorWrapper {
	explicit FieldAccessorWrapper(FieldType T::* field) : m_field(field) {}
	typedef const typename std::decay<FieldType>::type& result_type;

	result_type operator()(const T& t) const { return t.*m_field; }
	void operator()(T& t, FieldType field) const { t.*m_field = std::move(field); }
private:
	FieldType T::* m_field;
};

template<class T, class FieldType>
FieldAccessorWrapper<T, FieldType> makeFieldAccessor(FieldType T::* field) {
	return FieldAccessorWrapper<T, FieldType>(field);
}
