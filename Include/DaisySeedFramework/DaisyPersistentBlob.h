#pragma once
#ifndef DAISY_PERSISTENT_BLOB_H
#define DAISY_PERSISTENT_BLOB_H

#include "DaisySeedHAL.h"
#include "PersistentBlobCommon.h"

template <typename T>
class DaisyPersistentBlob
{
public:
	DaisyPersistentBlob(void)
		: m_IsInitialized(false)
	{
	}

	bool Initialize(DaisySeedHAL *HAL, const T &DefaultData = {})
	{
		ASSERT(!m_IsInitialized, "DaisyPersistentBlob is already initialized");

		PersistentBlobData<T> defaultBlob;
		defaultBlob.Data = DefaultData;

		m_Storage.Init(HAL->GetQSPI(), defaultBlob, PersistentBlobBase::GetAndIncrementOffset(sizeof(T)));

		m_IsInitialized = true;

		if (!m_Storage.GetSettings().IsMatched())
		{
			Erase();

			return false;
		}

		return true;
	}

	void Flush(void)
	{
		ASSERT(m_IsInitialized, "DaisyPersistentBlob is not initialized");

		m_Storage.Save();
	}

	void Erase(void)
	{
		ASSERT(m_IsInitialized, "DaisyPersistentBlob is not initialized");

		m_Storage.RestoreDefaults(false);
	}

	void Set(const T &Object)
	{
		ASSERT(m_IsInitialized, "DaisyPersistentBlob is not initialized");

		m_Storage.GetSettings().Data = Object;
	}

	T &Get(void)
	{
		ASSERT(m_IsInitialized, "DaisyPersistentBlob is not initialized");

		return m_Storage.GetSettings().Data;
	}

	const T &Get(void) const
	{
		ASSERT(m_IsInitialized, "DaisyPersistentBlob is not initialized");

		return m_Storage.GetSettings().Data;
	}

private:
	bool m_IsInitialized;
	daisy::PersistentStorage<PersistentBlobData<T>> m_Storage;
};

#endif