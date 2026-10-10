#pragma once
#ifndef FACTORY_H
#define FACTORY_H

#include "StaticVector.h"
#include <DigitalSignalProcessing/FixedFunction.h>
#include <DigitalSignalProcessing/Allocator.h>

template<typename IdentifierType, typename Type, uint8_t Alignment = 16, bool IsAllocatorResettable = true>
class Factory
{
public:
	typedef Allocator<uint8_t, Alignment, IsAllocatorResettable> FactoryAllocator;
	typedef FixedFunction<Type* (IdentifierType Identifier, FactoryAllocator& Allocator)> FactoryCallback;
	typedef StaticVector<FactoryCallback, 2> FactoryCallbackList;

public:
	static void AddFactoryCallback(FactoryCallback Callback)
	{
		GetFactoryCallbacks().Push(Callback);
	}

protected:
	static Type* Create(FactoryAllocator& Allocator, IdentifierType Identifier)
	{
		Type* ptr = nullptr;
		for (uint8_t i = 0; i < GetFactoryCallbacks().GetSize(); ++i)
		{
			ptr = GetFactoryCallbacks()[i](Identifier, Allocator);
			if (ptr == nullptr)
				continue;

			break;
		}

		ASSERT(ptr != nullptr, "Failed to Create Object [%u]", Identifier);

		return ptr;
	}

private:
	static FactoryCallbackList& GetFactoryCallbacks(void)
	{
		static FactoryCallbackList factoryCallbacks;

		return factoryCallbacks;
	}
};

#endif