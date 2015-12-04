#pragma once

#define E_NET LI_LIB_GUID_STR

using namespace System;
using namespace Mono::Cecil;

enum e_net_method : UINT
{
	实例化 = 0x00,
	取类函数 = 0x01,
	取类型 = 0x02
};

[LibGuid(E_NET)]
ref class E_Net : Plugin
{
	[LibMethod(e_net_method::取类型, EMethodMode::Embed)]
	MethodDefinition^ GetType(ModuleDefinition^ module);
	[LibMethod(e_net_method::取类函数, EMethodMode::Embed)]
	MethodDefinition^ GetFunction(ModuleDefinition^ module);
	[LibMethod(e_net_method::实例化)]
	static Object^ 实例化(RuntimeTypeHandle 类型, ...array<Object^>^ 参数);
};