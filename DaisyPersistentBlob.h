#pragma once
#ifndef DAISY_PERSISTENT_BLOB_H
#define DAISY_PERSISTENT_BLOB_H

#include "DaisySeedHAL.h"

class DaisyPersistentBlobBase
{
	template <typename T>
	friend class DaisyPersistentBlob;

private:
	static uint32 GetAndIncreamentOffset(uint16 Size)
	{
		const uint32 SECTOR_SIZE = 4 KB;

		static uint32 offset = 0;

		uint32 current = offset;

		offset += Size;
		offset = (offset + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);

		return current;
	}
};

template <typename T>
class DaisyPersistentBlob
{
private:
	struct Blob
	{
	public:
		Blob(void)
			: DataSize(sizeof(T))
		{
		}

		bool operator!=(const Blob &Other)
		{
			const uint8 *selfData = reinterpret_cast<const uint8 *>(&Data);
			const uint8 *otherData = reinterpret_cast<const uint8 *>(&Other.Data);

			for (uint16 i = 0; i < sizeof(T); ++i)
				if (selfData[i] != otherData[i])
					return true;

			return false;
		}

		bool IsMatched(void) const
		{
			return DataSize == sizeof(T);
		}

		T Data;

		uint16 DataSize;
	};

public:
	DaisyPersistentBlob(void)
		: m_IsInitialized(false)
	{
	}

	DaisyPersistentBlob(DaisySeedHAL *HAL)
		: m_Storage(HAL),
		  m_IsInitialized(false)
	{
	}

	bool Initialize(DaisySeedHAL *HAL, const T &DefaultData = {})
	{
		ASSERT(!m_IsInitialized, "DaisyPersistentBlob is already initialized");

		Blob defaultBlob;
		defaultBlob.Data = DefaultData;

		m_Storage.Init(HAL->GetQSPI(), defaultBlob, DaisyPersistentBlobBase::GetAndIncreamentOffset(sizeof(T)));

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
	daisy::PersistentStorage<Blob> m_Storage;
};

template <typename T>
struct PersistentBlobDelayedFlush
{
public:
	PersistentBlobDelayedFlush(DaisyPersistentBlob<T> *Blob)
		: m_Blob(Blob),
		  m_Requested(false),
		  m_Time(0)
	{
		ASSERT(Blob != nullptr, "Blob cannot be null");
	}

	void Update(float CurrentTime)
	{
		if (!m_Requested)
			return;

		if (CurrentTime < m_Time)
			return;

		m_Requested = false;
		m_Blob->Flush();
	}

	void Request(float Time)
	{
		m_Time = Time;
		m_Requested = true;
	}

private:
	DaisyPersistentBlob<T> *m_Blob;
	bool m_Requested;
	float m_Time;
};

#endif