/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "MariaDbStatement.h"

#include "MariaDbStmtResult.h"

namespace UnrealCore::Database
{
    MariaDbStatement::MariaDbStatement(MYSQL_STMT* statement)
        : m_statement(statement)
    {
    }

    MariaDbStatement::~MariaDbStatement()
    {
        if (m_statement != nullptr)
        {
            mysql_stmt_close(m_statement);
        }
    }

    void MariaDbStatement::EnsureCapacity(std::uint32_t paramIndex)
    {
        const std::size_t requiredSize = static_cast<std::size_t>(paramIndex) + 1;
        if (m_storage.size() < requiredSize)
        {
            m_storage.resize(requiredSize);
            m_binds.resize(requiredSize);
        }
    }

    void MariaDbStatement::BindNull(std::uint32_t paramIndex)
    {
        EnsureCapacity(paramIndex);
        m_storage[paramIndex].isNull = 1;
    }

    void MariaDbStatement::BindString(std::uint32_t paramIndex, const std::string& value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.stringValue = value;
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_STRING;
    }

    void MariaDbStatement::BindInt32(std::uint32_t paramIndex, std::int32_t value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.intValue = value;
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_LONG;
        m_binds[paramIndex].is_unsigned = 0;
    }

    void MariaDbStatement::BindInt64(std::uint32_t paramIndex, std::int64_t value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.intValue = value;
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_LONGLONG;
        m_binds[paramIndex].is_unsigned = 0;
    }

    void MariaDbStatement::BindUInt32(std::uint32_t paramIndex, std::uint32_t value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.intValue = static_cast<std::int64_t>(value);
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_LONG;
        m_binds[paramIndex].is_unsigned = 1;
    }

    void MariaDbStatement::BindUInt64(std::uint32_t paramIndex, std::uint64_t value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.intValue = static_cast<std::int64_t>(value);
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_LONGLONG;
        m_binds[paramIndex].is_unsigned = 1;
    }

    void MariaDbStatement::BindFloat(std::uint32_t paramIndex, float value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.doubleValue = static_cast<double>(value);
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_FLOAT;
    }

    void MariaDbStatement::BindDouble(std::uint32_t paramIndex, double value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.doubleValue = value;
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_DOUBLE;
    }

    void MariaDbStatement::BindBool(std::uint32_t paramIndex, bool value)
    {
        EnsureCapacity(paramIndex);
        ParamStorage& storage = m_storage[paramIndex];
        storage.intValue = value ? 1 : 0;
        storage.isNull = 0;
        m_binds[paramIndex].buffer_type = MYSQL_TYPE_TINY;
        m_binds[paramIndex].is_unsigned = 0;
    }

    void MariaDbStatement::ApplyBindings()
    {
        for (std::size_t i = 0; i < m_binds.size(); ++i)
        {
            MYSQL_BIND& bind = m_binds[i];
            ParamStorage& storage = m_storage[i];

            bind.is_null = &storage.isNull;

            switch (bind.buffer_type)
            {
                case MYSQL_TYPE_STRING:
                    bind.buffer = const_cast<char*>(storage.stringValue.data());
                    bind.buffer_length = static_cast<unsigned long>(storage.stringValue.size());
                    break;

                case MYSQL_TYPE_LONG:
                    bind.buffer = &storage.intValue;
                    bind.buffer_length = sizeof(std::int32_t);
                    break;

                case MYSQL_TYPE_LONGLONG:
                    bind.buffer = &storage.intValue;
                    bind.buffer_length = sizeof(std::int64_t);
                    break;

                case MYSQL_TYPE_TINY:
                    bind.buffer = &storage.intValue;
                    bind.buffer_length = sizeof(std::int64_t);
                    break;

                case MYSQL_TYPE_FLOAT:
                    bind.buffer = &storage.doubleValue;
                    bind.buffer_length = sizeof(float);
                    break;

                case MYSQL_TYPE_DOUBLE:
                    bind.buffer = &storage.doubleValue;
                    bind.buffer_length = sizeof(double);
                    break;

                default:
                    break;
            }
        }

        if (!m_binds.empty())
        {
            mysql_stmt_bind_param(m_statement, m_binds.data());
        }
    }

    std::uint64_t MariaDbStatement::Execute()
    {
        ApplyBindings();

        if (mysql_stmt_execute(m_statement) != 0)
        {
            return 0;
        }

        return static_cast<std::uint64_t>(mysql_stmt_affected_rows(m_statement));
    }

    std::unique_ptr<IDatabaseResult> MariaDbStatement::ExecuteQuery()
    {
        ApplyBindings();

        if (mysql_stmt_execute(m_statement) != 0)
        {
            return nullptr;
        }

        MYSQL_RES* metaResult = mysql_stmt_result_metadata(m_statement);
        if (metaResult == nullptr)
        {
            // Запрос не возвращает набор строк (например, INSERT/UPDATE через ExecuteQuery).
            return nullptr;
        }

        if (mysql_stmt_store_result(m_statement) != 0)
        {
            mysql_free_result(metaResult);
            return nullptr;
        }

        return std::make_unique<MariaDbStmtResult>(m_statement, metaResult);
    }
}
