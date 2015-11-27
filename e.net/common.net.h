#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace Mono::Cecil;
using namespace Mono::Cecil::Cil;

const System::Reflection::BindingFlags BINDING_ALL = System::Reflection::BindingFlags::Public | System::Reflection::BindingFlags::NonPublic | System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::Static;
const System::Reflection::BindingFlags BINDING_ALLSTATIC = System::Reflection::BindingFlags::Public | System::Reflection::BindingFlags::NonPublic | System::Reflection::BindingFlags::Static;
const System::Reflection::BindingFlags BINDING_ALLINSTANCE = System::Reflection::BindingFlags::Public | System::Reflection::BindingFlags::NonPublic | System::Reflection::BindingFlags::Instance;

#define STRUCT TypeAttributes::SequentialLayout | TypeAttributes::Sealed
#define STATICCLASS TypeAttributes::Abstract | TypeAttributes::Sealed
#define CTOR MethodAttributes::SpecialName | MethodAttributes::RTSpecialName | MethodAttributes::HideBySig
#define STATICMETHOD MethodAttributes::Static | MethodAttributes::HideBySig
#define PROPERTYMETHOD MethodAttributes::SpecialName | MethodAttributes::HideBySig
#define EXTERNMETHOD STATICMETHOD | MethodAttributes::PInvokeImpl

#define typeof(type) type::typeid
#define String2LPSTR(str) (char*)(void*)Marshal::StringToHGlobalAnsi(str)
#define LPSTR2String(lpstr) Marshal::PtrToStringAnsi((IntPtr)(void*)lpstr)
#define CStr2String(cstr) Marshal::PtrToStringAnsi((IntPtr)(void*)cstr.c_str())
#define Ptr2LPBYTE(ptr) (LPBYTE)(void*)ptr
#define Ptr2IntPtr(ptr) (IntPtr)(void*)ptr
#define Ptr2Type(type, ptr) *((type*)ptr)
#define Obj2Ptr(type, obj) (type*)(void*)(IntPtr)obj

#define AddILCode(ILProcessor, ...) ILProcessor->Append(ILProcessor->Create(__VA_ARGS__))
#define GetStaticMethod_D(type, name) typeof(type)->GetMethod(name, BINDING_ALLSTATIC)
#define GetStaticMethod(type, name, ...) typeof(type)->GetMethod(name, BINDING_ALLSTATIC, nullptr, (gcnew array<Type^>{__VA_ARGS__}), nullptr)
#define GetInstanceMethod_D(type, name) typeof(type)->GetMethod(name, BINDING_ALLINSTANCE)
#define GetInstanceMethod(type, name, ...) typeof(type)->GetMethod(name, BINDING_ALLINSTANCE, nullptr, (gcnew array<Type^>{__VA_ARGS__}), nullptr)
#define GetConstructor(type, ...) typeof(type)->GetConstructor(BINDING_ALLINSTANCE, nullptr, (gcnew array<Type^>{__VA_ARGS__}), nullptr)
#define GetStaticField(type, name) typeof(type)->GetField(name, BINDING_ALLSTATIC)
#define GetInstanceField(type, name) typeof(type)->GetField(name, BINDING_ALLINSTANCE)

generic<typename T> IList<T>^ ToList(...array<T>^ args);
bool operator==(TypeReference^ type1, TypeReference^ type2);
bool operator!=(TypeReference^ type1, TypeReference^ type2);
bool IsAssignableFrom(TypeReference^ type1, TypeReference^ type2);