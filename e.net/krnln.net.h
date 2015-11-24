#pragma once
#include "efs.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace Mono::Cecil;
using namespace Mono::Cecil::Cil;

enum krnln_method : UINT
{
	如果 = 0x00,
	如果真 = 0x01,
	判断 = 0x02,
	返回 = 0x0D,
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
	到数值 = 0x59,
	到文本 = 0x5A,
	到字节 = 0x0275,
	到短整数 = 0x0276,
	到整数 = 0x0277,
	到长整数 = 0x0278,
	到小数 = 0x0279,
	左移 = 0x027E,
	右移 = 0x027F,
	重定义数组
};

MethodDefinition^ CreateMethod(String^ name, TypeReference^ returntype, IList<ParameterDefinition^>^ params = nullptr, MethodAttributes attr = MethodAttributes::HideBySig);
ParameterDefinition^ CreateParameter(String^ name, TypeReference^ type, ParameterAttributes attr = ParameterAttributes::None);
MethodDefinition^ CreateReturn(ModuleDefinition^ module);
MethodDefinition^ CreateMod(ModuleDefinition^ module);
MethodDefinition^ CreateAdd(ModuleDefinition^ module);
MethodDefinition^ CreateEvenAdd(ModuleDefinition^ module);
MethodDefinition^ CreateIntAdd(ModuleDefinition^ module);
MethodDefinition^ CreateEvenIntAdd(ModuleDefinition^ module);
MethodDefinition^ CreateLongAdd(ModuleDefinition^ module);
MethodDefinition^ CreateEvenLongAdd(ModuleDefinition^ module);
MethodDefinition^ CreateDoubleAdd(ModuleDefinition^ module);
MethodDefinition^ CreateEvenDoubleAdd(ModuleDefinition^ module);
MethodDefinition^ CreateEvenBinAdd(ModuleDefinition^ module);
MethodDefinition^ CreateSub(ModuleDefinition^ module);
MethodDefinition^ CreateIntSub(ModuleDefinition^ module);
MethodDefinition^ CreateNeg(ModuleDefinition^ module);
MethodDefinition^ CreateMul(ModuleDefinition^ module);
MethodDefinition^ CreateDiv(ModuleDefinition^ module);
MethodDefinition^ CreateIDiv(ModuleDefinition^ module);
MethodDefinition^ CreateEqual(ModuleDefinition^ module);
MethodDefinition^ CreateNotEqual(ModuleDefinition^ module);
MethodDefinition^ CreateLess(ModuleDefinition^ module);
MethodDefinition^ CreateMore(ModuleDefinition^ module);
MethodDefinition^ CreateLessOrEqual(ModuleDefinition^ module);
MethodDefinition^ CreateMoreOrEqual(ModuleDefinition^ module);
MethodDefinition^ CreateAnd(ModuleDefinition^ module);
MethodDefinition^ CreateOr(ModuleDefinition^ module);
MethodDefinition^ CreateNot(ModuleDefinition^ module);
MethodDefinition^ CreateBnot(ModuleDefinition^ module);
MethodDefinition^ CreateBand(ModuleDefinition^ module);
MethodDefinition^ CreateBor(ModuleDefinition^ module);
MethodDefinition^ CreateBxor(ModuleDefinition^ module);
MethodDefinition^ CreateSet(ModuleDefinition^ module);
MethodDefinition^ CreateToDouble(ModuleDefinition^ module);
MethodDefinition^ CreateToStr(ModuleDefinition^ module);
MethodDefinition^ CreateToByte(ModuleDefinition^ module);
MethodDefinition^ CreateToShort(ModuleDefinition^ module);
MethodDefinition^ CreateToInt(ModuleDefinition^ module);
MethodDefinition^ CreateToLong(ModuleDefinition^ module);
MethodDefinition^ CreateToFloat(ModuleDefinition^ module);
MethodDefinition^ CreateShl(ModuleDefinition^ module);
MethodDefinition^ CreateShr(ModuleDefinition^ module);
MethodDefinition^ CreateIfe(ModuleDefinition^ module);
MethodDefinition^ CreateIf(ModuleDefinition^ module);
MethodDefinition^ CreateSwitch(ModuleDefinition^ module);
MethodDefinition^ CreateWhile(ModuleDefinition^ module);
MethodDefinition^ CreateWend(ModuleDefinition^ module);
MethodDefinition^ CreateDoWhile(ModuleDefinition^ module);
MethodDefinition^ CreateLoop(ModuleDefinition^ module);
MethodDefinition^ CreateCounter(ModuleDefinition^ module);
MethodDefinition^ CreateCounterLoop(ModuleDefinition^ module);
MethodDefinition^ CreateFor(ModuleDefinition^ module);
MethodDefinition^ CreateNext(ModuleDefinition^ module);