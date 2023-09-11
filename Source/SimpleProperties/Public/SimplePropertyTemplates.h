// Copyright Matt Chapman. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"

// TIsFunction doesn't work for static class functions
template<typename InType>
struct TIsStaticClassFunction
{
	enum { Value = false };
};

template<typename InRetType, typename... InParams>
struct TIsStaticClassFunction<InRetType(*)(InParams...)>
{
	enum { Value = true };
};

// Couldn't find this in the engine... it's probably there.
template<typename InType>
struct TIsInstanceClassFunction
{
	enum { Value = false };
};

template<typename InRetType, typename InClass, typename... InParams>
struct TIsInstanceClassFunction<InRetType(InClass::*)(InParams...)>
{
	enum { Value = true };
};
