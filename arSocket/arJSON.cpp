#include "arJSON.h"


bool JSON_To_arJSON_SupportModule(std::string::const_iterator strEnd, std::string::const_iterator& jsonStr, OUT arJSON& json)
{
	using Type_t = arJSONValue::Type_t;

	std::string key, data;
	Type_t type = Type_t::_NULL;
	bool keyLoaded = false;
	bool dataEnd = false; // _str, _arr
	bool commaFound = false; // _arr
	auto ClearData = [&]()
	{
		key = "";
		data = "";
		type = Type_t::_NULL;
		keyLoaded = false;
		dataEnd = false;
		commaFound = false;
	};
	
	while (jsonStr != strEnd)
	{
		char _char = *(jsonStr++);

		if (!keyLoaded)
		{
			if (type != Type_t::_str)
			{
				if (_char == '"')
					type = Type_t::_str;
				else if (_char == ':' || _char == '=')
					keyLoaded = true;
				else if (_char > 32)
					return true;
			}
			else
			{
				if (_char == '"')
				{
					if (key.empty())
						return true;

					type = Type_t::_NULL;
					json[key];
				}
				else
					key += _char;
			}
		}
		else
		{
			switch (type)
			{
			case Type_t::_NULL:
				if (_char <= 32)
					continue;

				if (_char == '"')
					type = Type_t::_str;
				else if (_char == '{')
				{
					type = Type_t::_sub;
					arJSON subJson;
					if (JSON_To_arJSON_SupportModule(strEnd, jsonStr, subJson))
						return true;
					json[key] = subJson;
				}
				else if (_char == '[')
					type = Type_t::_arr;
				else if ('0' <= _char && _char <= '9')
				{
					data += _char;
					type = Type_t::_int;
				}
				else
					return true;
				break;

			case Type_t::_str:
				if (!dataEnd)
				{
					if (_char == '"')
						dataEnd = true;
					else
						data += _char;
				}
				else
				{
					if (_char <= 32)
						continue;

					if (_char == ',')
					{
						json[key] = data;
						ClearData();
					}
					else if (_char == '}')
					{
						json[key] = data;
						return false;
					}
					else
						return true;
				}
				break;

			case Type_t::_int:
				if (_char <= 32)
					continue;

				if ('0' <= _char && _char <= '9')
					data += _char;
				else if (_char == ',')
				{
					json[key] = atoi(data.data());
					ClearData();
				}
				else if (_char == '}')
				{
					json[key] = atoi(data.data());
					return false;
				}
				else if (_char == '.')
				{
					data += '.';
					type = Type_t::_float;
				}
				else
					return true;
				break;

			case Type_t::_float:
				if (_char <= 32)
					continue;

				if ('0' <= _char && _char <= '9')
					data += _char;
				else if (_char == ',')
				{
					json[key] = (float)atof(data.data());
					ClearData();
				}
				else if (_char == '}')
				{
					json[key] = (float)atof(data.data());
					return false;
				}
				else
					return true;
				break;

			case Type_t::_sub:
				if (_char == ',')		ClearData();
				else if (_char == '}')	return false;
				else					return true;
				break;

			case Type_t::_arr:
				if (!dataEnd)
				{
					if (_char == ']')
						dataEnd = true;
					else if (_char == '{')
					{
						arJSON subJson;
						if (JSON_To_arJSON_SupportModule(strEnd, jsonStr, subJson))
							return true;
						json[key].Append(subJson);
						commaFound = false;
					}
					else if (_char == ',')
					{
						if (!commaFound)
							commaFound = true;
						else
							return true;
					}
					else
						return true;
				}
				else
				{
					if (_char == ',')
						ClearData();
					else if (_char == '}')
						return false;
					else
						return true;
				}
				break;
			}
		}
	}
	return true;
}

bool JSON_To_arJSON(const std::string& jsonStr, OUT arJSON& json)
{
	bool loaded = false;
	for (auto iter = jsonStr.begin(); iter != jsonStr.end();)
	{
		if (*(iter++) == '{')
		{
			if (loaded)
				return true;
			else
			{
				if (JSON_To_arJSON_SupportModule(jsonStr.end(), iter, json))
					return true;
				loaded = true;
			}
		}
	}
	return false;
}



bool arJSON::ToJSON(OUT std::string & jsonStr) const
{
	jsonStr.clear();
	jsonStr += '{';
	for (auto jsonIter = m_json.cbegin(); jsonIter != m_json.cend(); )
	{
		const arJSONKey& jsonKey = jsonIter->first;
		const arJSONValue& jsonValue = jsonIter->second;

		(((jsonStr += '"') += jsonKey) += '"') += ':';
		
		using Type_t = arJSONValue::Type_t;
		switch (jsonValue.Type())
		{
		case Type_t::_int:
			jsonStr += std::to_string(jsonValue.Int());
			break;

		case Type_t::_float:
			jsonStr += std::to_string(jsonValue.Float());
			break;

		case Type_t::_str:
			((jsonStr += '"') += jsonValue.Str()) += '"';
			break;

		case Type_t::_sub:{
			std::string subJsonStr;
			if (jsonValue.Sub().ToJSON(subJsonStr))
				return true;
			jsonStr += subJsonStr;
			break;}

		case Type_t::_arr: {
			std::string subJsonStr;
			jsonStr += '[';
			for (size_t subJsonIndex = 0; subJsonIndex < jsonValue.ArrSize(); )
			{
				if (jsonValue[subJsonIndex].ToJSON(subJsonStr))
					return true;
				jsonStr += subJsonStr;

				if (++subJsonIndex < jsonValue.ArrSize())
					jsonStr += ',';
			}
			jsonStr += ']';
			break;}

		default:
			return true;
		}

		if (++jsonIter != m_json.cend())
			jsonStr += ',';
	}
	jsonStr += '}';
	return false;
}

std::string arJSON::ToJSON() const
{
	std::string str;
	if (ToJSON(str))
		return std::string();
	return str;
}





arJSONValue::arJSONValue()
	: m_type(Type_t::_NULL)

	, m_int(NULL)
	, m_float(NULL)
	, m_str()
{
}
