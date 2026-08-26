#pragma once
#ifndef PERSISTENT_BLOB_COMMON_H
#define PERSISTENT_BLOB_COMMON_H

class PersistentBlobBase
{
	template <typename T>
	friend class DaisyPersistentBlob;

	template <typename T>
	friend class WindowsPersistentBlob;

private:
	static uint32 GetAndIncrementOffset(uint16 Size)
	{
		const uint32 SECTOR_SIZE = QSPI_PAGE_SIZE;

		static uint32 offset = QSPI_START_ADDRESS;

		uint32 current = offset;

		ASSERT(current + Size < QSPI_END_ADDRESS, "We're running out of QSPI bound");

		offset += Size;
		offset = (offset + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);

		return current;
	}
};

template <typename T>
struct PersistentBlobData
{
public:
	PersistentBlobData(void)
		: DataSize(sizeof(T))
	{}

	bool operator!=(const PersistentBlobData& Other)
	{
		const uint8* selfData = reinterpret_cast<const uint8*>(&Data);
		const uint8* otherData = reinterpret_cast<const uint8*>(&Other.Data);

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

	uint32 DataSize;
};

template <typename BlobType>
struct PersistentBlobDelayedFlush
{
public:
	PersistentBlobDelayedFlush(BlobType* Blob)
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
	BlobType* m_Blob;
	bool m_Requested;
	float m_Time;
};

#endif