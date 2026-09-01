/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "../IDatabaseConnection.h"

#include <libpq-fe.h>

namespace UnrealCore::Database
{
    class PostgreSqlConnection final : public IDatabaseConnection
    {
    public:
        PostgreSqlConnection() = default;
        ~PostgreSqlConnection() override;

        PostgreSqlConnection(const PostgreSqlConnection&) = delete;
        PostgreSqlConnection& operator=(const PostgreSqlConnection&) = delete;

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
        // Выполняет служебную команду (BEGIN/COMMIT/ROLLBACK) и проверяет статус.
        bool ExecuteCommand(const char* sql);

        // Экранирует значение для строки подключения (формат key='value').
        static std::string EscapeConnectionValue(const std::string& value);

        static std::string BuildConnectionString(const DatabaseConfig& config);

        PGconn* _connection{nullptr};
        mutable std::string _lastError;
        bool _inTransaction{false};
    };
}
