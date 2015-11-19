#pragma once

using namespace System;

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