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
		if (type1->IsArray) type1 = type1->Module->ImportReference(typeof(Array));
		else if (type1->IsByReference) type1 = dynamic_cast<ByReferenceType^>(type1)->ElementType;
		else type1 = type1->Resolve()->BaseType;
		if (type1 == nullptr) return false;
		return IsAssignableFrom(type1, type2);
	}
}

generic<typename T> void AddList(ICollection<T>^ list1, T item)
{
	if (item != nullptr && !list1->Contains(item)) list1->Add(item);
}

generic<typename T> void AddList(ICollection<T>^ list1, ICollection<T>^ list2)
{
	for each (T item in list2) if (!list1->Contains(item)) list1->Add(item);
}

generic<typename T> void DelList(ICollection<T>^ list1, ICollection<T>^ list2)
{
	for each (T item in list2) list1->Remove(item);
}

generic<typename T1, typename T2> T2 GetDictionary(IDictionary<T1, T2>^ dictionary, T1 key)
{
	if (dictionary->ContainsKey(key)) return dictionary[key];
	T2 T;
	return T;
}

generic<typename T1, typename T2> void AddDictionary(IDictionary<T1, IList<T2>^>^ dictionary, T1 key, T2 item)
{
	IList<T2>^ list;
	if (dictionary->ContainsKey(key)) list = dictionary[key];
	else
	{
		list = gcnew List<T2>();
		dictionary->Add(key, list);
	}
	AddList(list, item);
}