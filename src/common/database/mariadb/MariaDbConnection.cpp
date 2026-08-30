/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "MariaDbConnection.h"

#include "MariaDbResult.h"
#include "MariaDbStatement.h"

namespace UnrealCore::Database
{
    MariaDbConnection::MariaDbConnection()
        : m_handle(nullptr)
        , m_connected(false)
    {
    }

    MariaDbConnection::~MariaDbConnection()
    {
        Disconnect();
    }

    bool MariaDbConnection::Connect(const DatabaseConfig& config)
    {
        if (m_connected)
        {
            Disconnect();
        }

        m_handle = mysql_init(nullptr);
        if (m_handle == nullptr)
        {
            return false;
        }

        const unsigned int timeout = config.connectTimeoutSeconds;
        mysql_options(m_handle, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

        MYSQL* result = mysql_real_connect(
            m_handle,
            config.host.c_str(),
            config.user.c_str(),
            config.password.c_str(),
            config.database.c_str(),
            config.port,
            nullptr,
            0);

        if (result == nullptr)
        {
            mysql_close(m_handle);
            m_handle = nullptr;
            m_connected = false;
            return false;
        }

        m_connected = true;
        return true;
    }

    void MariaDbConnection::Disconnect()
    {
        if (m_handle != nullptr)
        {
            mysql_close(m_handle);
            m_handle = nullptr;
        }

        m_connected = false;
    }

    bool MariaDbConnection::IsConnected() const
    {
        return m_connected;
    }

    std::uint64_t MariaDbConnection::Execute(const std::string& query)
    {
        if (!m_connected)
        {
            return 0;
        }

        if (mysql_real_query(m_handle, query.c_str(), query.size()) != 0)
        {
            return 0;
        }

        return static_cast<std::uint64_t>(mysql_affected_rows(m_handle));
    }

    std::unique_ptr<IDatabaseResult> MariaDbConnection::Query(const std::string& query)
    {
        if (!m_connected)
        {
            return nullptr;
        }

        if (mysql_real_query(m_handle, query.c_str(), query.size()) != 0)
        {
            return nullptr;
        }

        MYSQL_RES* rawResult = mysql_store_result(m_handle);
        if (rawResult == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<MariaDbResult>(rawResult);
    }

    std::unique_ptr<IDatabaseStatement> MariaDbConnection::PrepareStatement(const std::string& query)
    {
        if (!m_connected)
        {
            return nullptr;
        }

        MYSQL_STMT* stmt = mysql_stmt_init(m_handle);
        if (stmt == nullptr)
        {
            return nullptr;
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.size()) != 0)
        {
            mysql_stmt_close(stmt);
            return nullptr;
        }

        return std::make_unique<MariaDbStatement>(stmt);
    }

    bool MariaDbConnection::BeginTransaction()
    {
        if (!m_connected)
        {
            return false;
        }

        static const std::string query = "START TRANSACTION";
        return mysql_real_query(m_handle, query.c_str(), query.size()) == 0;
    }

    bool MariaDbConnection::Commit()
    {
        if (!m_connected)
        {
            return false;
        }

        return mysql_commit(m_handle) == 0;
    }

    bool MariaDbConnection::Rollback()
    {
        if (!m_connected)
        {
            return false;
        }

        return mysql_rollback(m_handle) == 0;
    }

    std::string MariaDbConnection::GetLastError() const
    {
        if (m_handle == nullptr)
        {
            return "Нет соединения";
        }

        return mysql_error(m_handle);
    }
}
