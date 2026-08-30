/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "MariaDbResult.h"

#include <cstdlib>

namespace UnrealCore::Database
{
    MariaDbResult::MariaDbResult(MYSQL_RES* result)
        : m_result(result)
        , m_currentRow(nullptr)
        , m_columnLengths(nullptr)
        , m_columnCount(static_cast<std::uint32_t>(mysql_num_fields(result)))
    {
    }

    MariaDbResult::~MariaDbResult()
    {
        if (m_result != nullptr)
        {
            mysql_free_result(m_result);
        }
    }

    bool MariaDbResult::Next()
    {
        m_currentRow = mysql_fetch_row(m_result);
        if (m_currentRow == nullptr)
        {
            m_columnLengths = nullptr;
            return false;
        }

        m_columnLengths = mysql_fetch_lengths(m_result);
        return true;
    }

    std::uint32_t MariaDbResult::GetColumnCount() const
    {
        return m_columnCount;
    }

    bool MariaDbResult::IsNull(std::uint32_t columnIndex) const
    {
        if (m_currentRow == nullptr || columnIndex >= m_columnCount)
        {
            return true;
        }

        return m_currentRow[columnIndex] == nullptr;
    }

    std::string MariaDbResult::GetString(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return std::string();
        }

        return std::string(m_currentRow[columnIndex], m_columnLengths[columnIndex]);
    }

    std::int32_t MariaDbResult::GetInt32(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::int32_t>(std::strtol(m_currentRow[columnIndex], nullptr, 10));
    }

    std::int64_t MariaDbResult::GetInt64(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::int64_t>(std::strtoll(m_currentRow[columnIndex], nullptr, 10));
    }

    std::uint32_t MariaDbResult::GetUInt32(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::uint32_t>(std::strtoul(m_currentRow[columnIndex], nullptr, 10));
    }

    std::uint64_t MariaDbResult::GetUInt64(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::uint64_t>(std::strtoull(m_currentRow[columnIndex], nullptr, 10));
    }

    float MariaDbResult::GetFloat(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0.0f;
        }

        return std::strtof(m_currentRow[columnIndex], nullptr);
    }

    double MariaDbResult::GetDouble(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0.0;
        }

        return std::strtod(m_currentRow[columnIndex], nullptr);
    }

    bool MariaDbResult::GetBool(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return false;
        }

        return std::strtol(m_currentRow[columnIndex], nullptr, 10) != 0;
    }
}
