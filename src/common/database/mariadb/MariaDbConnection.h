/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "database/IDatabaseConnection.h"

#include <mysql.h>

namespace UnrealCore::Database
{
    class MariaDbConnection final : public IDatabaseConnection
    {
    public:
        MariaDbConnection();
        ~MariaDbConnection() override;

        MariaDbConnection(const MariaDbConnection&) = delete;
        MariaDbConnection& operator=(const MariaDbConnection&) = delete;

        bool Connect(const DatabaseConfig& config) override;
        void Disconnect() override;
        bool IsConnected() const override;

        std::uint64_t Execute(const std::string& query) override;
        std::unique_ptr<IDatabaseResult> Query(const std::string& query) override;
        std::unique_ptr<IDatabaseStatement> PrepareStatement(const std::string& query) override;

        bool BeginTransaction() override;
        bool Commit() override;
        bool Rollback() override;

        std::string GetLastError() const override;

    private:
        MYSQL* m_handle;
        bool m_connected;
    };
}
