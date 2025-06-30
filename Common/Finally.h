#pragma once

template <class T>
	requires(requires(T& t) 
	{
		{t()} -> std::same_as<void>;
	})
class Finally
{
	T _op;

public:

	Finally(T&& t) : _op(std::forward<T>(t))
	{
	}

	~Finally()
	{
		_op();
	}
};

#define FINALLY Finally _ = [&] noexcept -> void