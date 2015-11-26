#pragma once
#include "efs.h"
#include "Plugins.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;
using namespace Mono::Cecil::Cil;
using namespace wnxd::E_NET;

#define KRNLN "d09f2340818511d396f6aaf844c7e325"

enum krnln_method : UINT
{
	如果 = 0x00,
	如果真 = 0x01,
	判断 = 0x02,
	判断循环首 = 0x03,
	判断循环尾 = 0x04,
	循环判断首 = 0x05,
	循环判断尾 = 0x06,
	计次循环首 = 0x07,
	计次循环尾 = 0x08,
	变量循环首 = 0x09,
	变量循环尾 = 0x0A,
	返回 = 0x0D,
	结束 = 0x0E,
	求余数 = 0x12,
	相加 = 0x13,
	相减 = 0x14,
	负 = 0x15,
	相乘 = 0x0F,
	相除 = 0x10,
	整除 = 0x11,
	等于 = 0x26,
	不等于 = 0x27,
	小于 = 0x28,
	大于 = 0x29,
	小于或等于 = 0x2A,
	大于或等于 = 0x2B,
	并且 = 0x2D,
	或者 = 0x2E,
	取反 = 0x2F,
	位取反 = 0x30,
	位与 = 0x31,
	位或 = 0x32,
	位异或 = 0x33,
	赋值 = 0x34,
	重定义数组 = 0x37,
	取数组成员数 = 0x38,
	取数组下标 = 0x39,
	复制数组 = 0x3A,
	加入成员 = 0x3B,
	插入成员 = 0x3C,
	到数值 = 0x59,
	到文本 = 0x5A,
	到字节 = 0x0275,
	到短整数 = 0x0276,
	到整数 = 0x0277,
	到长整数 = 0x0278,
	到小数 = 0x0279,
	左移 = 0x027E,
	右移 = 0x027F
};

[LibGuidAttribute(KRNLN)]
ref class Krnln : Plugin, MonoPlugin
{
public:
	property PluginType Type
	{
		virtual PluginType get() override;
	}
	virtual IList<MonoInfo^>^ GetMethods(ModuleDefinition^ module) override;
};