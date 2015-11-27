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
	删除成员 = 0x3D,
	清除数组 = 0x3E,
	数组排序 = 0x3F,
	取文本长度 = 0x4C,
	取文本左边 = 0x4D,
	取文本右边 = 0x4E,
	取文本中间 = 0x4F,
	字符 = 0x50,
	取代码 = 0x51,
	寻找文本 = 0x52,
	倒找文本 = 0x53,
	到大写 = 0x54,
	到小写 = 0x55,
	到数值 = 0x59,
	到文本 = 0x5A,
	数组清零 = 0x0231,
	到字节 = 0x0275,
	到短整数 = 0x0276,
	到整数 = 0x0277,
	到长整数 = 0x0278,
	到小数 = 0x0279,
	左移 = 0x027E,
	右移 = 0x027F
};

[LibGuid(KRNLN)]
ref class Krnln : Plugin, MonoPlugin
{
public:
	property PluginType Type
	{
		virtual PluginType get() override;
	}
	virtual IList<MonoInfo^>^ GetMethods(ModuleDefinition^ module) override;
private:
	[LibMethod(krnln_method::删除成员)]
	static int 删除成员([Out]Array^% 欲删除成员的数组变量, int 欲删除的位置, [Optional][DefaultValue(1)]int 欲删除的成员数目);
	[LibMethod(krnln_method::清除数组)]
	static void 清除数组([Out]Array^% 欲删除成员的数组变量);
	[LibMethod(krnln_method::数组排序)]
	static void 数组排序([Out]Array^% 数值数组变量, [Optional][DefaultValue(true)]bool 排序方向是否为从小到大);
	[LibMethod(krnln_method::数组清零)]
	static void 数组清零([Out]Array^% 数值数组变量);
	[LibMethod(krnln_method::取文本长度)]
	static int 取文本长度(String^ 文本数据);
	[LibMethod(krnln_method::取文本左边)]
	static String^ 取文本左边(String^ 欲取其部分的文本, int 欲取出字符的数目);
	[LibMethod(krnln_method::取文本右边)]
	static String^ 取文本右边(String^ 欲取其部分的文本, int 欲取出字符的数目);
	[LibMethod(krnln_method::取文本中间)]
	static String^ 取文本中间(String^ 欲取其部分的文本, int 起始取出位置, int 欲取出字符的数目);
	[LibMethod(krnln_method::取代码)]
	static int 取代码(String^ 欲取字符代码的文本, [Optional][DefaultValue(1)]int 欲取其代码的字符位置);
	[LibMethod(krnln_method::寻找文本)]
	static int 寻找文本(String^ 被搜寻的文本, String^ 欲寻找的文本, [Optional][DefaultValue(1)]int 起始搜寻位置, bool 是否不区分大小写);
	[LibMethod(krnln_method::倒找文本)]
	static int 倒找文本(String^ 被搜寻的文本, String^ 欲寻找的文本, [Optional][DefaultValue(1)]int 起始搜寻位置, bool 是否不区分大小写);
};