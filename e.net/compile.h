#pragma once

#include "efs.h"

using namespace std;

struct EInfo
{
	ESection_SystemInfo SystemInfo;
	ESection_UserInfo UserInfo;
	ESection_Program Program;
	ESection_AuxiliaryInfo2 TagStatus;
};

EInfo* ParseEcode(byte*);