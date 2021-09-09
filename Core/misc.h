#pragma once

template<typename list, typename predicate>
bool contains(list&& items, predicate&& _predicate)
{
	for (auto&& item : items)
		if (_predicate(item))
			return true;
	return false;
}

template<typename list, typename predicate>
bool foreach_no_except(list&& items, predicate&& _predicate)
{
	bool result;
	try
	{
		for (auto&& item : items)
			_predicate(item);
		result = true;
	}
	catch (...)
	{
		result = false;
	}
	return result;
}


template<typename T>
class property {
public:
	property(
		std::function<void(T)> setter,
		std::function<T()> getter) :
		Setter(setter), Getter(getter) { }
	operator T() const { return Getter(); }
	property<T>& operator= (const T& value) { Setter(value); return *this; }
	T& Value() { return value; }
private:
	std::function<void(T)> Setter;
	std::function<T()> Getter;
	T value;
};