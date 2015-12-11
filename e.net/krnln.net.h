#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
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
	到全角 = 0x56,
	到半角 = 0x57,
	到时间 = 0x58,
	到数值 = 0x59,
	到文本 = 0x5A,
	删首空 = 0x5B,
	删尾空 = 0x5C,
	删首尾空 = 0x5D,
	删全部空 = 0x5E,
	文本替换 = 0x5F,
	子文本替换 = 0x60,
	取空白文本 = 0x61,
	取重复文本 = 0x62,
	文本比较 = 0x63,
	分割文本 = 0x64,
	取字节集长度 = 0x65,
	到字节集 = 0x66,
	取字节集数据 = 0x67,
	取字节集左边 = 0x68,
	取字节集右边 = 0x69,
	取字节集中间 = 0x6A,
	寻找字节集 = 0x6B,
	倒找字节集 = 0x6C,
	字节集替换 = 0x6D,
	子字节集替换 = 0x6E,
	取空白字节集 = 0x6F,
	取重复字节集 = 0x70,
	分割字节集 = 0x71,
	数值到大写 = 0x72,
	数值到金额 = 0x73,
	数值到格式文本 = 0x74,
	取十六进制文本 = 0x75,
	取八进制文本 = 0x76,
	增减时间 = 0x77,
	取时间间隔 = 0x78,
	取某月天数 = 0x79,
	时间到文本 = 0x7A,
	取时间部分 = 0x7B,
	取年份 = 0x7C,
	取月份 = 0x7D,
	取日 = 0x7E,
	取星期几 = 0x7F,
	取小时 = 0x80,
	取分钟 = 0x81,
	取秒 = 0x82,
	指定时间 = 0x83,
	取现行时间 = 0x84,
	置现行时间 = 0x85,
	信息框 = 0xC0,
	取启动时间 = 0xC2,
	载入 = 0xC8,
	延时 = 0x019F,
	取日期 = 0x022F,
	取时间 = 0x0230,
	数组清零 = 0x0231,
	标准输出 = 0x0241,
	标准输入 = 0x0242,
	到字节 = 0x0275,
	到短整数 = 0x0276,
	到整数 = 0x0277,
	到长整数 = 0x0278,
	到小数 = 0x0279,
	左移 = 0x027E,
	右移 = 0x027F,
	十六进制 = 0x029A,
	二进制 = 0x029B,
};

[LibGuid(KRNLN)]
ref class Krnln : Plugin
{
	[LibMethod(krnln_method::返回, EMethodMode::Embed)]
	MethodDefinition^ CreateReturn(ModuleDefinition^ module);
	[LibMethod(krnln_method::求余数, EMethodMode::Embed)]
	MethodDefinition^ CreateMod(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Embed)]
	MethodDefinition^ CreateIntAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Call)]
	MethodDefinition^ CreateEvenIntAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Embed)]
	MethodDefinition^ CreateLongAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Call)]
	MethodDefinition^ CreateEvenLongAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Embed)]
	MethodDefinition^ CreateDoubleAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Call)]
	MethodDefinition^ CreateEvenDoubleAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Embed)]
	MethodDefinition^ CreateAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加, EMethodMode::Call)]
	MethodDefinition^ CreateEvenAdd(ModuleDefinition^ module);
	[LibMethod(krnln_method::相减, EMethodMode::Embed)]
	MethodDefinition^ CreateSub(ModuleDefinition^ module);
	[LibMethod(krnln_method::相减, EMethodMode::Embed)]
	MethodDefinition^ CreateIntSub(ModuleDefinition^ module);
	[LibMethod(krnln_method::负, EMethodMode::Embed)]
	MethodDefinition^ CreateNeg(ModuleDefinition^ module);
	[LibMethod(krnln_method::相乘, EMethodMode::Embed)]
	MethodDefinition^ CreateMul(ModuleDefinition^ module);
	[LibMethod(krnln_method::相除, EMethodMode::Embed)]
	MethodDefinition^ CreateDiv(ModuleDefinition^ module);
	[LibMethod(krnln_method::整除, EMethodMode::Embed)]
	MethodDefinition^ CreateIDiv(ModuleDefinition^ module);
	[LibMethod(krnln_method::等于, EMethodMode::Embed)]
	MethodDefinition^ CreateEqualNull1(ModuleDefinition^ module);
	[LibMethod(krnln_method::等于, EMethodMode::Embed)]
	MethodDefinition^ CreateEqualNull2(ModuleDefinition^ module);
	[LibMethod(krnln_method::等于, EMethodMode::Embed)]
	MethodDefinition^ CreateEqual(ModuleDefinition^ module);
	[LibMethod(krnln_method::不等于, EMethodMode::Embed)]
	MethodDefinition^ CreateNotEqualNull1(ModuleDefinition^ module);
	[LibMethod(krnln_method::不等于, EMethodMode::Embed)]
	MethodDefinition^ CreateNotEqualNull2(ModuleDefinition^ module);
	[LibMethod(krnln_method::不等于, EMethodMode::Embed)]
	MethodDefinition^ CreateNotEqual(ModuleDefinition^ module);
	[LibMethod(krnln_method::小于, EMethodMode::Embed)]
	MethodDefinition^ CreateLess(ModuleDefinition^ module);
	[LibMethod(krnln_method::大于, EMethodMode::Embed)]
	MethodDefinition^ CreateMore(ModuleDefinition^ module);
	[LibMethod(krnln_method::小于或等于, EMethodMode::Embed)]
	MethodDefinition^ CreateLessOrEqual(ModuleDefinition^ module);
	[LibMethod(krnln_method::大于或等于, EMethodMode::Embed)]
	MethodDefinition^ CreateMoreOrEqual(ModuleDefinition^ module);
	[LibMethod(krnln_method::并且, EMethodMode::Embed)]
	MethodDefinition^ CreateAnd(ModuleDefinition^ module);
	[LibMethod(krnln_method::或者, EMethodMode::Embed)]
	MethodDefinition^ CreateOr(ModuleDefinition^ module);
	[LibMethod(krnln_method::取反, EMethodMode::Embed)]
	MethodDefinition^ CreateNot(ModuleDefinition^ module);
	[LibMethod(krnln_method::位取反, EMethodMode::Embed)]
	MethodDefinition^ CreateBnot(ModuleDefinition^ module);
	[LibMethod(krnln_method::位与, EMethodMode::Embed)]
	MethodDefinition^ CreateBand(ModuleDefinition^ module);
	[LibMethod(krnln_method::位或, EMethodMode::Embed)]
	MethodDefinition^ CreateBor(ModuleDefinition^ module);
	[LibMethod(krnln_method::位异或, EMethodMode::Embed)]
	MethodDefinition^ CreateBxor(ModuleDefinition^ module);
	[LibMethod(krnln_method::赋值, EMethodMode::Embed)]
	MethodDefinition^ CreateSet(ModuleDefinition^ module);
	[LibMethod(krnln_method::到数值, EMethodMode::Call)]
	MethodDefinition^ CreateToDouble(ModuleDefinition^ module);
	[LibMethod(krnln_method::到文本, EMethodMode::Embed)]
	MethodDefinition^ CreateToStr(ModuleDefinition^ module);
	[LibMethod(krnln_method::到字节, EMethodMode::Call)]
	MethodDefinition^ CreateToByte(ModuleDefinition^ module);
	[LibMethod(krnln_method::到短整数, EMethodMode::Call)]
	MethodDefinition^ CreateToShort(ModuleDefinition^ module);
	[LibMethod(krnln_method::到整数, EMethodMode::Call)]
	MethodDefinition^ CreateToInt(ModuleDefinition^ module);
	[LibMethod(krnln_method::到长整数, EMethodMode::Call)]
	MethodDefinition^ CreateToLong(ModuleDefinition^ module);
	[LibMethod(krnln_method::到小数, EMethodMode::Call)]
	MethodDefinition^ CreateToFloat(ModuleDefinition^ module);
	[LibMethod(krnln_method::左移, EMethodMode::Embed)]
	MethodDefinition^ CreateShl(ModuleDefinition^ module);
	[LibMethod(krnln_method::右移, EMethodMode::Embed)]
	MethodDefinition^ CreateShr(ModuleDefinition^ module);
	[LibMethod(krnln_method::如果, EMethodMode::Embed)]
	MethodDefinition^ CreateIfe(ModuleDefinition^ module);
	[LibMethod(krnln_method::如果真, EMethodMode::Embed)]
	MethodDefinition^ CreateIf(ModuleDefinition^ module);
	[LibMethod(krnln_method::判断, EMethodMode::Embed)]
	MethodDefinition^ CreateSwitch(ModuleDefinition^ module);
	[LibMethod(krnln_method::判断循环首, EMethodMode::Embed)]
	MethodDefinition^ CreateWhile(ModuleDefinition^ module);
	[LibMethod(krnln_method::判断循环尾, EMethodMode::Embed)]
	MethodDefinition^ CreateWend(ModuleDefinition^ module);
	[LibMethod(krnln_method::循环判断首, EMethodMode::Embed)]
	MethodDefinition^ CreateDoWhile(ModuleDefinition^ module);
	[LibMethod(krnln_method::循环判断尾, EMethodMode::Embed)]
	MethodDefinition^ CreateLoop(ModuleDefinition^ module);
	[LibMethod(krnln_method::计次循环首, EMethodMode::Embed)]
	MethodDefinition^ CreateCounter(ModuleDefinition^ module);
	[LibMethod(krnln_method::计次循环尾, EMethodMode::Embed)]
	MethodDefinition^ CreateCounterLoop(ModuleDefinition^ module);
	[LibMethod(krnln_method::变量循环首, EMethodMode::Embed)]
	MethodDefinition^ CreateFor(ModuleDefinition^ module);
	[LibMethod(krnln_method::变量循环尾, EMethodMode::Embed)]
	MethodDefinition^ CreateNext(ModuleDefinition^ module);
	[LibMethod(krnln_method::结束, EMethodMode::Embed)]
	MethodDefinition^ CreateEnd(ModuleDefinition^ module);
	[LibMethod(krnln_method::重定义数组, EMethodMode::Call)]
	MethodDefinition^ CreateReDim(ModuleDefinition^ module);
	[LibMethod(krnln_method::取数组成员数, EMethodMode::Call)]
	MethodDefinition^ CreateGetAryElementCount(ModuleDefinition^ module);
	[LibMethod(krnln_method::取数组下标, EMethodMode::Call)]
	MethodDefinition^ CreateUBound(ModuleDefinition^ module);
	[LibMethod(krnln_method::复制数组, EMethodMode::Call)]
	MethodDefinition^ CreateCopyAry(ModuleDefinition^ module);
	[LibMethod(krnln_method::加入成员, EMethodMode::Call)]
	MethodDefinition^ CreateAddElement(ModuleDefinition^ module);
	[LibMethod(krnln_method::插入成员, EMethodMode::Call)]
	MethodDefinition^ CreateInsElement(ModuleDefinition^ module);
	[LibMethod(krnln_method::字符, EMethodMode::Embed)]
	MethodDefinition^ CreateChr(ModuleDefinition^ module);
	[LibMethod(krnln_method::到大写, EMethodMode::Embed)]
	MethodDefinition^ CreateUCase(ModuleDefinition^ module);
	[LibMethod(krnln_method::到小写, EMethodMode::Embed)]
	MethodDefinition^ CreateLCase(ModuleDefinition^ module);
	[LibMethod(krnln_method::删首空, EMethodMode::Embed)]
	MethodDefinition^ CreateLTrim(ModuleDefinition^ module);
	[LibMethod(krnln_method::删尾空, EMethodMode::Embed)]
	MethodDefinition^ CreateRTrim(ModuleDefinition^ module);
	[LibMethod(krnln_method::删首尾空, EMethodMode::Embed)]
	MethodDefinition^ CreateTrim(ModuleDefinition^ module);
	[LibMethod(krnln_method::删全部空, EMethodMode::Embed)]
	MethodDefinition^ CreateTrimAll(ModuleDefinition^ module);
	[LibMethod(krnln_method::文本比较, EMethodMode::Embed)]
	MethodDefinition^ CreateStrComp(ModuleDefinition^ module);
	[LibMethod(krnln_method::到时间, EMethodMode::Embed)]
	MethodDefinition^ CreateToTime(ModuleDefinition^ module);
	[LibMethod(krnln_method::取某月天数, EMethodMode::Embed)]
	MethodDefinition^ CreateGetDaysOfSpecMonth(ModuleDefinition^ module);
	[LibMethod(krnln_method::取年份, EMethodMode::Embed)]
	MethodDefinition^ CreateYear(ModuleDefinition^ module);
	[LibMethod(krnln_method::取月份, EMethodMode::Embed)]
	MethodDefinition^ CreateMonth(ModuleDefinition^ module);
	[LibMethod(krnln_method::取日, EMethodMode::Embed)]
	MethodDefinition^ CreateDay(ModuleDefinition^ module);
	[LibMethod(krnln_method::取星期几, EMethodMode::Embed)]
	MethodDefinition^ CreateWeekDay(ModuleDefinition^ module);
	[LibMethod(krnln_method::取小时, EMethodMode::Embed)]
	MethodDefinition^ CreateHour(ModuleDefinition^ module);
	[LibMethod(krnln_method::取分钟, EMethodMode::Embed)]
	MethodDefinition^ CreateMinute(ModuleDefinition^ module);
	[LibMethod(krnln_method::取秒, EMethodMode::Embed)]
	MethodDefinition^ CreateSecond(ModuleDefinition^ module);
	[LibMethod(krnln_method::指定时间, EMethodMode::Embed)]
	MethodDefinition^ CreateGetSpecTime(ModuleDefinition^ module);
	[LibMethod(krnln_method::取现行时间, EMethodMode::Embed)]
	MethodDefinition^ CreateNow(ModuleDefinition^ module);
	[LibMethod(krnln_method::取日期, EMethodMode::Embed)]
	MethodDefinition^ CreateGetDatePart(ModuleDefinition^ module);
	[LibMethod(krnln_method::取时间, EMethodMode::Embed)]
	MethodDefinition^ CreateGetTimePart(ModuleDefinition^ module);
	[LibMethod(krnln_method::取十六进制文本, EMethodMode::Embed)]
	MethodDefinition^ CreateGetHexText(ModuleDefinition^ module);
	[LibMethod(krnln_method::取八进制文本, EMethodMode::Embed)]
	MethodDefinition^ CreateGetOctText(ModuleDefinition^ module);
	[LibMethod(krnln_method::十六进制, EMethodMode::Embed)]
	MethodDefinition^ CreateHex(ModuleDefinition^ module);
	[LibMethod(krnln_method::二进制, EMethodMode::Embed)]
	MethodDefinition^ CreateBinary(ModuleDefinition^ module);
	[LibMethod(krnln_method::取空白字节集, EMethodMode::Embed)]
	MethodDefinition^ CreateSpaceBin(ModuleDefinition^ module);
	[LibMethod(krnln_method::取启动时间, EMethodMode::Embed)]
	MethodDefinition^ CreateGetTickCount(ModuleDefinition^ module);
	[LibMethod(krnln_method::延时, EMethodMode::Embed)]
	MethodDefinition^ CreateSleep(ModuleDefinition^ module);
	[LibMethod(krnln_method::相加)]
	static array<byte>^ 相加(array<byte>^ 被加字节集, ...array<array<byte>^>^ 加字节集);
	[LibMethod(krnln_method::相加)]
	static double 相加(Nullable<double> 被加可空数, ...array<Nullable<double>>^ 加可空数);
	[LibMethod(krnln_method::相减)]
	static double 相减(Nullable<double> 被加可空数, ...array<Nullable<double>>^ 加可空数);
	[LibMethod(krnln_method::相乘)]
	static double 相乘(Nullable<double> 被加可空数, ...array<Nullable<double>>^ 加可空数);
	[LibMethod(krnln_method::相除)]
	static double 相除(Nullable<double> 被加可空数, ...array<Nullable<double>>^ 加可空数);
	[LibMethod(krnln_method::到文本)]
	static String^ 到文本(array<byte>^ 待转换的数据);
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
	[LibMethod(krnln_method::到全角)]
	static String^ 到全角(String^ 欲变换的文本);
	[LibMethod(krnln_method::到半角)]
	static String^ 到半角(String^ 欲变换的文本);
	[LibMethod(krnln_method::文本替换)]
	static String^ 文本替换(String^ 欲被替换的文本, int 起始替换位置, int 替换长度, [Optional]String^ 用作替换的文本);
	[LibMethod(krnln_method::子文本替换)]
	static String^ 子文本替换(String^ 欲被替换的文本, String^ 欲被替换的子文本, [Optional]String^ 用作替换的子文本, [Optional][DefaultValue(1)]int 进行替换的起始位置, [Optional]int 替换进行的次数, bool 是否区分大小写);
	[LibMethod(krnln_method::取空白文本)]
	static String^ 取空白文本(int 重复次数);
	[LibMethod(krnln_method::取重复文本)]
	static String^ 取重复文本(int 重复次数, String^ 待重复文本);
	[LibMethod(krnln_method::分割文本)]
	static array<String^>^ 分割文本(String^ 待分割文本, [Optional][DefaultValue(",")]String^ 用作分割的文本, [Optional]int 要返回的子文本数目);
	[LibMethod(krnln_method::增减时间)]
	static DateTime 增减时间(DateTime 时间, int 被增加部分, int 增加值);
	[LibMethod(krnln_method::取时间间隔)]
	static double 取时间间隔(DateTime 时间1, DateTime 时间2, int 取间隔部分);
	[LibMethod(krnln_method::时间到文本)]
	static String^ 时间到文本(DateTime 欲转换到文本的时间, [Optional][DefaultValue(1)]int 转换部分);
	[LibMethod(krnln_method::取时间部分)]
	static int 取时间部分(DateTime 欲取其部分的时间, int 转换部分);
	[LibMethod(krnln_method::置现行时间)]
	static bool 置现行时间(DateTime 欲设置的时间);
	[LibMethod(krnln_method::数值到大写)]
	static String^ 数值到大写(double 欲转换形式的数值, bool 是否转换为简体);
	[LibMethod(krnln_method::数值到金额)]
	static String^ 数值到金额(double 欲转换形式的数值, bool 是否转换为简体);
	[LibMethod(krnln_method::数值到格式文本)]
	static String^ 数值到格式文本(double 欲转换为文本的数值, [Optional]Nullable<int> 小数保留位数, bool 是否进行千分位分隔);
	[LibMethod(krnln_method::取字节集长度)]
	static int 取字节集长度(array<byte>^ 字节集数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(String^ 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(DateTime 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(IntPtr 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(bool 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(byte 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(short 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(int 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(Int64 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(float 欲转换为字节集的数据);
	[LibMethod(krnln_method::到字节集)]
	static array<byte>^ 到字节集(double 欲转换为字节集的数据);
	[LibMethod(krnln_method::取字节集数据)]
	static Object^ 取字节集数据(array<byte>^ 欲取出其中数据的字节集, int 欲取出数据的类型, [Optional][DefaultValue(1)]int 起始索引位置);
	[LibMethod(krnln_method::取字节集左边)]
	static array<byte>^ 取字节集左边(array<byte>^ 欲取其部分的字节集, int 欲取出字节的数目);
	[LibMethod(krnln_method::取字节集右边)]
	static array<byte>^ 取字节集右边(array<byte>^ 欲取其部分的字节集, int 欲取出字节的数目);
	[LibMethod(krnln_method::取字节集中间)]
	static array<byte>^ 取字节集中间(array<byte>^ 欲取其部分的字节集, int 起始取出位置, int 欲取出字节的数目);
	[LibMethod(krnln_method::寻找字节集)]
	static int 寻找字节集(array<byte>^ 被搜寻的字节集, array<byte>^ 欲寻找的字节集, [Optional][DefaultValue(1)]int 起始搜寻位置);
	[LibMethod(krnln_method::倒找字节集)]
	static int 倒找字节集(array<byte>^ 被搜寻的字节集, array<byte>^ 欲寻找的字节集, [Optional][DefaultValue(1)]int 起始搜寻位置);
	[LibMethod(krnln_method::字节集替换)]
	static array<byte>^ 字节集替换(array<byte>^ 欲替换其部分的字节集, int 起始替换位置, int 替换长度, [Optional]array<byte>^ 用作替换的字节集);
	[LibMethod(krnln_method::子字节集替换)]
	static array<byte>^ 子字节集替换(array<byte>^ 欲被替换的字节集, array<byte>^ 欲被替换的子字节集, [Optional]array<byte>^ 用作替换的子字节集, [Optional][DefaultValue(1)]int 进行替换的起始位置, [Optional]int 替换进行的次数);
	[LibMethod(krnln_method::取重复字节集)]
	static array<byte>^ 取重复字节集(int 重复次数, array<byte>^ 待重复的字节集);
	[LibMethod(krnln_method::分割字节集)]
	static array<array<byte>^>^ 分割字节集(array<byte>^ 待分割字节集, [Optional]array<byte>^ 用作分割的字节集, [Optional]int 要返回的子字节集数目);
	[LibMethod(krnln_method::信息框)]
	static int 信息框(Object^ 提示信息, int 按钮, [Optional]String^ 窗口标题, [Optional]IWin32Window^ 父窗口);
	[LibMethod(krnln_method::标准输出)]
	static void 标准输出([Optional][DefaultValue(1)]int 输出方向, ...array<Object^>^ 欲输出内容);
	[LibMethod(krnln_method::标准输入)]
	static String^ 标准输入([Optional][DefaultValue(true)]bool 是否回显);
};