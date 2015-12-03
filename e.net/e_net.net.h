#pragma once

#define E_NET LI_LIB_GUID_STR

using namespace System;
using namespace Mono::Cecil;

enum e_net_method : UINT
{
	实例化 = 0x00,
	取子程序指针 = 0x01,
	取类型 = 0x02
};

[LibGuid(E_NET)]
ref class E_Net : Plugin
{
	[LibMethod(e_net_method::取类型, EMethodMode::Embed)]
	MethodDefinition^ GetType(ModuleDefinition^ module);
	[LibMethod(e_net_method::实例化)]
	static Object^ 实例化(RuntimeTypeHandle 类型, ...array<Object^>^ 参数);
	[LibMethod(e_net_method::取子程序指针)]
	static Delegate^ 取子程序指针(Object^ 对象, String^ 名称, ...array<Object^>^ 参数类型);
};