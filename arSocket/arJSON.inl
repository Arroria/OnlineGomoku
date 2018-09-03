#pragma once
#include "arJSON.h"

inline arJSON & arJSON::operator=(const std::string& jsonStr) { JSON_To_arJSON(jsonStr, *this); return *this; }

inline arJSONValue &		arJSON::operator[](const std::string& key) { return m_json[key]; }
inline const arJSONValue &	arJSON::operator[](const std::string& key) const { return m_json.at(key); }
inline bool					arJSON::IsIn(const std::string& key) const { return m_json.find(key) != m_json.end(); }

inline auto	arJSON::begin() { return m_json.begin(); }
inline auto	arJSON::end() { return m_json.end(); }
inline auto	arJSON::begin() const { return m_json.cbegin(); }
inline auto	arJSON::end() const { return m_json.cend(); }




inline void arJSONValue::operator=(int value)				{ m_type = Type_t::_int;		m_int	= value; }
inline void arJSONValue::operator=(float value)				{ m_type = Type_t::_float;		m_float = value; }
inline void arJSONValue::operator=(const std::string& value){ m_type = Type_t::_str;		m_str	= value; }
inline void arJSONValue::operator=(const arJSON& value)		{ m_type = Type_t::_sub;		m_sub	= value; }
inline void arJSONValue::Append(const arJSON& value)		{ m_type = Type_t::_arr;		m_arr.push_back(value); }

inline std::string&	arJSONValue::Str()	{ return m_str; }
inline arJSON&		arJSONValue::Sub()	{ return m_sub; }

inline arJSONValue::Type_t	arJSONValue::Type() const	{ return m_type; }
inline int					arJSONValue::Int() const	{ return m_int; }
inline float				arJSONValue::Float() const	{ return m_float; }
inline const std::string&	arJSONValue::Str() const	{ return m_str; }

inline const arJSON&		arJSONValue::Sub() const								{ return m_sub; }
inline const arJSONValue&	arJSONValue::operator[](const std::string& key) const	{ return m_sub[key]; }

inline size_t		arJSONValue::ArrSize() const	{ return m_arr.size(); }
inline arJSON&		arJSONValue::operator[](size_t index) { return m_arr[index]; }
inline const arJSON&arJSONValue::operator[](size_t index) const { return m_arr[index]; }
inline auto			arJSONValue::begin()			{ return m_arr.begin(); }
inline auto			arJSONValue::end()				{ return m_arr.end(); }
inline auto			arJSONValue::begin() const		{ return m_arr.cbegin(); }
inline auto			arJSONValue::end() const		{ return m_arr.cend(); }
