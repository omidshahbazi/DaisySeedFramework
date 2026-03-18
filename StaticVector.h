#pragma once
#ifndef STATIC_VECTOR_H
#define STATIC_VECTOR_H

#include "DSP/Debug.h"

template <typename T, uint8 MaxSize>
struct StaticVector
{
public:
    typedef T ItemType;

public:
    StaticVector(void)
        : m_Buffer{},
          m_Count(0)
    {
    }

    void PushBack(const T &Value)
    {
        ASSERT(m_Count < MaxSize, "Out of Size");

        m_Buffer[m_Count++] = Value;
    }

    void Insert(uint8 Index, const T &Value)
    {
        ASSERT(m_Count < MaxSize, "Out of Size");
        ASSERT(Index <= m_Count, "Index Out of Range");

        for (uint8 i = m_Count; i > Index; --i)
        {
            m_Buffer[i] = m_Buffer[i - 1];
        }

        m_Buffer[Index] = Value;
        ++m_Count;
    }

    void PopBack()
    {
        ASSERT(0 < m_Count, "Out of Size");

        --m_Count;
        m_Buffer[m_Count] = {};
    }

    T &Front(void)
    {
        ASSERT(0 < m_Count, "Out of Size");
        return m_Buffer[0];
    }

    const T &Front(void) const
    {
        ASSERT(0 < m_Count, "Out of Size");
        return m_Buffer[0];
    }

    T &Back(void)
    {
        ASSERT(0 < m_Count, "Out of Size");
        return m_Buffer[m_Count - 1];
    }

    const T &Back(void) const
    {
        ASSERT(0 < m_Count, "Out of Size");
        return m_Buffer[m_Count - 1];
    }

    void RemoveAt(uint8 Index)
    {
        ASSERT(Index < m_Count, "Index Out of Range");

        for (uint8 i = Index; i < m_Count - 1; ++i)
        {
            m_Buffer[i] = m_Buffer[i + 1];
        }

        --m_Count;
        m_Buffer[m_Count] = {};
    }

    void Clear(void)
    {
        for (uint8 i = 0; i < m_Count; ++i)
        {
            m_Buffer[i].~T();
            m_Buffer[i] = {};
        }

        m_Count = 0;
    }

    uint8 GetSize(void) const
    {
        return m_Count;
    }

    uint8 GetCapacity(void) const
    {
        return MaxSize;
    }

    bool HasFree(void) const
    {
        return m_Count < MaxSize;
    }

    bool IsEmpty(void) const
    {
        return m_Count == 0;
    }

    T &operator[](uint8 Index)
    {
        ASSERT(Index < m_Count, "Index Out of Range");
        return m_Buffer[Index];
    }

    const T &operator[](uint8 Index) const
    {
        ASSERT(Index < m_Count, "Index Out of Range");
        return m_Buffer[Index];
    }

private:
    T m_Buffer[MaxSize];
    uint8 m_Count;
};

#endif