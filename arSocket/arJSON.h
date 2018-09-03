#pragma once
#include <vector>
#include <string>
#include <map>

#ifndef OUT
#define OUT
#endif

using arJSONKey = std::string;
class arJSONValue;
class arJSON
{
public:
	inline arJSON & operator=(const std::string& jsonStr);

	inline arJSONValue &		operator[](const std::string& key);
	inline const arJSONValue &	operator[](const std::string& key) const;
	inline bool					IsIn(const std::string& key) const;

	inline auto	begin();
	inline auto	end();
	inline auto	begin() const;
	inline auto	end() const;

	bool ToJSON(OUT std::string& jsonStr) const;
	std::string ToJSON() const;

private:
	std::map<arJSONKey, arJSONValue> m_json;
};
bool JSON_To_arJSON(const std::string& jsonStr, OUT arJSON& json);


class arJSONValue
{
public:
	enum class Type_t
	{
		_NULL,
		_int,
		_float,
		_str,
		_sub,
		_arr,
	};

public:
	arJSONValue();

	inline void operator=(int value);
	inline void operator=(float value);
	inline void operator=(const std::string& value);
	inline void operator=(const arJSON& value);
	inline void Append(const arJSON& value);


	inline std::string&	Str();
	inline arJSON&		Sub();


	inline Type_t				Type() const;
	inline int					Int() const;
	inline float				Float() const;
	inline const std::string&	Str() const;

	inline const arJSON&		Sub() const;
	inline const arJSONValue&	operator[](const std::string& key) const;

	inline size_t		ArrSize() const;
	inline arJSON&		operator[](size_t index);
	inline const arJSON&operator[](size_t index) const;
	inline auto			begin();
	inline auto			end();
	inline auto			begin() const;
	inline auto			end() const;

private:
	Type_t m_type;

	int m_int;
	float m_float;
	std::string m_str;
	arJSON m_sub;
	std::vector<arJSON> m_arr;
};
#include "arJSON.inl"
