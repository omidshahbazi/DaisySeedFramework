#pragma once
#ifndef DAISY_PERSISTENT_BLOB_H
#define DAISY_PERSISTENT_BLOB_H

#include "PersistentBlobCommon.h"
#include <fstream>
#include <filesystem>

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

		m_FilePath = std::filesystem::current_path() / (std::to_string(PersistentBlobBase::GetAndIncreamentOffset(sizeof(T))) + ".bin");
		m_IsInitialized = true;

		std::ifstream file(m_FilePath, std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			file.read(reinterpret_cast<char*>(&m_Data), sizeof(PersistentBlobData<T>));
			file.close();
		}
		else
		{
			m_Data.Data = DefaultData;

			FlushToFile();
			
			return false;
		}

		if (!m_Data.IsMatched())
		{
			Erase();

			return false;
		}

		return true;
	}

	void Flush(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		FlushToFile();
	}

	void Erase(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		std::filesystem::remove(m_FilePath);
	}

	void Set(const T& Object)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		m_Data.Data = Object;
	}

	T& Get(void)
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		return m_Data.Data;
	}

	const T& Get(void) const
	{
		ASSERT(m_IsInitialized, "WindowsPersistentBlob is not initialized");

		return m_Data.Data;
	}

private:
	void FlushToFile(void)
	{
		std::ofstream file(m_FilePath, std::ios::binary);
		if (file.is_open())
		{
			file.write(reinterpret_cast<const char*>(&m_Data), sizeof(PersistentBlobData<T>));
			file.close();
		}
	}

private:
	std::filesystem::path m_FilePath;
	bool m_IsInitialized;
	PersistentBlobData<T> m_Data;
};

#endif