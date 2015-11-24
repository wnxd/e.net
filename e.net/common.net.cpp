#include "stdafx.h"
#include "common.net.h"

using namespace std;

vector<string> split(string s, string delim)
{
	vector<string> elems;
	size_t pos = 0;
	size_t len = s.length();
	size_t delim_len = delim.length();
	if (delim_len == 0) return elems;
	while (pos < len)
	{
		int find_pos = s.find(delim, pos);
		if (find_pos < 0)
		{
			elems.push_back(s.substr(pos, len - pos));
			break;
		}
		elems.push_back(s.substr(pos, find_pos - pos));
		pos = find_pos + delim_len;
	}
	return elems;
}

generic<typename T> IList<T>^ ToList(...array<T>^ args)
{
	return gcnew List<T>(args);
}

bool operator==(TypeReference^ type1, TypeReference^ type2)
{
	bool b1 = Object::Equals(type1, nullptr);
	bool b2 = Object::Equals(type2, nullptr);
	if (b1 == b2)
	{
		if (b1) return true;
		return type1->FullName == type2->FullName;
	}
	else return false;
}
bool operator!=(TypeReference^ type1, TypeReference^ type2)
{
	return !(type1 == type2);
}

bool IsAssignableFrom(TypeReference^ type1, TypeReference^ type2)
{
	if (Object::Equals(type1, nullptr) || Object::Equals(type2, nullptr)) return false;
	else if (type1 == type2) return true;
	else
	{
		type1 = type1->Resolve()->BaseType;
		if (type1 == nullptr) return false;
		return IsAssignableFrom(type1, type2);
	}
}