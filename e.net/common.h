#pragma once

#include "efs.h"

#define NOT -1

using namespace std;

template<typename T> bool arrcmp(const T arr1[], const T arr2[], size_t len)
{
	for (size_t i = 0; i < len; i++) if (arr1[i] != arr2[i]) return false;
	return true;
}

template<typename T> T GetData(byte*& pointer)
{
	T data;
	memcpy(&data, pointer, sizeof(T));
	pointer += sizeof(T);
	return data;
}

template<typename T> T GetData(byte* pointer, long long& offset)
{
	T data;
	memcpy(&data, pointer + offset, sizeof(T));
	offset += sizeof(T);
	return data;
}

template<typename T> T FindInfo(vector<T> list, ETAG tag)
{
	for each (T var in list) if (static_cast<EBase>(var).Tag == tag) return var;
	return NULL;
}

vector<string> split(string s, string delim);
string GetString(byte*& pointer, bool jump = false);