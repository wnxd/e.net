#pragma once

using namespace std;

#define NOT -1

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

vector<string> split(string s, string delim);
string GetString(byte*& pointer, bool jump = false);