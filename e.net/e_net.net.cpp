#include "stdafx.h"
#include "Plugins.h"
#include "e_net.net.h"

Object^ E_Net::实例化(RuntimeTypeHandle 类型, ...array<Object^>^ 参数)
{
	return Activator::CreateInstance(Type::GetTypeFromHandle(类型), 参数);
}