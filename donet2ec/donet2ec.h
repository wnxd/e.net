#pragma once

using namespace System;
using namespace System::Collections::Generic;

enum class TypeOffsetInfoMode
{
	Method,
	Param
};

ref struct TypeOffsetInfo
{
	Type^ type;
	int offset;
	TypeOffsetInfoMode mode;
};

enum class TreeType
{
	NameSpace,
	ClassName,
	MethodName
};

ref struct TreeInfo
{
	TreeType Type;
	Dictionary<String^, TreeInfo^>^ Member;
	UINT Tag;
	TreeInfo();
	TreeInfo(TreeType type);
};