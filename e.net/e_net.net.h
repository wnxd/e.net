#pragma once

#define E_NET LI_LIB_GUID_STR

enum e_net_method : UINT
{
	实例化 = 0x00
};

[LibGuid(E_NET)]
ref class E_Net : Plugin
{
	[LibMethod(e_net_method::实例化)]
	static Object^ 实例化(RuntimeTypeHandle 类型, ...array<Object^>^ 参数);
};