#pragma once
#ifndef DAISY_PERSISTENT_BLOB_H
#define DAISY_PERSISTENT_BLOB_H

#include "PersistentBlobCommon.h"

template <typename T>
class WindowsPersistentBlob
{
public:
	WindowsPersistentBlob(void)
		: m_IsInitialized(false)
	{}

	bool Initialize(void*, const T& DefaultData = {})
	{
		ASSERT(!m_IsInitialized, "WindowsPersistentBlob is already initialized");

		//Blob defaultBlob;
		//defaultBlob.Data = DefaultData;

		//m_Storage.Init(HAL->GetQSPI(), defaultBlob, DaisyPersistentBlobBase::GetAndIncreamentOffset(sizeof(T)));

		m_IsInitialized = true;

		//if (!m_Storage.GetSettings().IsMatched())
		//{
		//	Erase();

		return false;
		//}

		//return true;
	}

	void Flush(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		//m_Storage.Save();
	}

	void Erase(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		//m_Storage.RestoreDefaults(false);
	}

	void Set(const T& Object)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		//m_Storage.GetSettings().Data = Object;
	}

	T& Get(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		return m_Data;
	}

	const T& Get(void) const
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		return m_Data;
	}

private:
	bool m_IsInitialized;
	T m_Data;
	//daisy::PersistentStorage<PersistentBlobData<T>> m_Storage;
};

#endif