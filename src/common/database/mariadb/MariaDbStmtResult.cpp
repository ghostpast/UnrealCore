/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "MariaDbStmtResult.h"

#include <cstring>

namespace UnrealCore::Database
{
    namespace
    {
        constexpr std::size_t kDefaultBufferSize = 256;
    }

    MariaDbStmtResult::MariaDbStmtResult(MYSQL_STMT* statement, MYSQL_RES* metaResult)
        : m_statement(statement)
        , m_metaResult(metaResult)
        , m_columnCount(static_cast<std::uint32_t>(mysql_num_fields(metaResult)))
    {
        SetupBindings();
    }

    MariaDbStmtResult::~MariaDbStmtResult()
    {
        if (m_metaResult != nullptr)
        {
            mysql_free_result(m_metaResult);
        }
    }

    void MariaDbStmtResult::SetupBindings()
    {
        m_columns.resize(m_columnCount);
        m_binds.resize(m_columnCount);

        MYSQL_FIELD* fields = mysql_fetch_fields(m_metaResult);

        for (std::uint32_t i = 0; i < m_columnCount; ++i)
        {
            ColumnBuffer& column = m_columns[i];

            std::size_t bufferSize = static_cast<std::size_t>(fields[i].max_length);
            if (bufferSize == 0)
            {
                bufferSize = kDefaultBufferSize;
            }

            column.data.resize(bufferSize);
            column.length = 0;
            column.isNull = 0;
            column.error = 0;

            MYSQL_BIND& bind = m_binds[i];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));

            bind.buffer_type = MYSQL_TYPE_STRING;
            bind.buffer = column.data.data();
            bind.buffer_length = static_cast<unsigned long>(bufferSize);
            bind.length = &column.length;
            bind.is_null = &column.isNull;
            bind.error = &column.error;
        }

        if (m_columnCount > 0)
        {
            mysql_stmt_bind_result(m_statement, m_binds.data());
        }
    }

    bool MariaDbStmtResult::Next()
    {
        const int fetchResult = mysql_stmt_fetch(m_statement);

        if (fetchResult == MYSQL_NO_DATA)
        {
            return false;
        }

        if (fetchResult == MYSQL_DATA_TRUNCATED)
        {
            // Перевыделяем буферы для колонок, которые были обрезаны, и получаем данные повторно.
            for (std::uint32_t i = 0; i < m_columnCount; ++i)
            {
                ColumnBuffer& column = m_columns[i];

                if (column.error != 0 && column.length > column.data.size())
                {
                    column.data.resize(column.length);

                    MYSQL_BIND& bind = m_binds[i];
                    bind.buffer = column.data.data();
                    bind.buffer_length = static_cast<unsigned long>(column.data.size());

                    mysql_stmt_fetch_column(m_statement, &bind, i, 0);
                }
            }

            return true;
        }

        if (fetchResult != 0)
        {
            return false;
        }

        return true;
    }

    std::uint32_t MariaDbStmtResult::GetColumnCount() const
    {
        return m_columnCount;
    }

    bool MariaDbStmtResult::IsNull(std::uint32_t columnIndex) const
    {
        if (columnIndex >= m_columnCount)
        {
            return true;
        }

        return m_columns[columnIndex].isNull != 0;
    }

    std::string MariaDbStmtResult::GetString(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return std::string();
        }

        const ColumnBuffer& column = m_columns[columnIndex];
        return std::string(column.data.data(), column.length);
    }

    std::int32_t MariaDbStmtResult::GetInt32(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::int32_t>(std::strtol(GetString(columnIndex).c_str(), nullptr, 10));
    }

    std::int64_t MariaDbStmtResult::GetInt64(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::int64_t>(std::strtoll(GetString(columnIndex).c_str(), nullptr, 10));
    }

    std::uint32_t MariaDbStmtResult::GetUInt32(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::uint32_t>(std::strtoul(GetString(columnIndex).c_str(), nullptr, 10));
    }

    std::uint64_t MariaDbStmtResult::GetUInt64(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0;
        }

        return static_cast<std::uint64_t>(std::strtoull(GetString(columnIndex).c_str(), nullptr, 10));
    }

    float MariaDbStmtResult::GetFloat(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0.0f;
        }

        return std::strtof(GetString(columnIndex).c_str(), nullptr);
    }

    double MariaDbStmtResult::GetDouble(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return 0.0;
        }

        return std::strtod(GetString(columnIndex).c_str(), nullptr);
    }

    bool MariaDbStmtResult::GetBool(std::uint32_t columnIndex) const
    {
        if (IsNull(columnIndex))
        {
            return false;
        }

        return std::strtol(GetString(columnIndex).c_str(), nullptr, 10) != 0;
    }
}
