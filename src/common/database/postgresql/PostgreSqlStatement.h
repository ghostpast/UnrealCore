/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "../IDatabaseStatement.h"

#include <libpq-fe.h>

#include <string>
#include <vector>

namespace UnrealCore::Database
{
    // Обёртка над серверным prepared statement libpq.
    // Индексация параметров — с нуля (внутри транслируется в $1, $2, ...).
    class PostgreSqlStatement final : public IDatabaseStatement
    {
    public:
        PostgreSqlStatement(PGconn* connection, std::string query);
        ~PostgreSqlStatement() override;

        PostgreSqlStatement(const PostgreSqlStatement&) = delete;
        PostgreSqlStatement& operator=(const PostgreSqlStatement&) = delete;

        // Выполняет PQprepare. Вызывается владельцем сразу после создания.
        bool Prepare();

        void BindNull(std::uint32_t paramIndex) override;
        void BindString(std::uint32_t paramIndex, const std::string& value) override;
        void BindInt32(std::uint32_t paramIndex, std::int32_t value) override;
        void BindInt64(std::uint32_t paramIndex, std::int64_t value) override;
        void BindUInt32(std::uint32_t paramIndex, std::uint32_t value) override;
        void BindUInt64(std::uint32_t paramIndex, std::uint64_t value) override;
        void BindFloat(std::uint32_t paramIndex, float value) override;
        void BindDouble(std::uint32_t paramIndex, double value) override;
        void BindBool(std::uint32_t paramIndex, bool value) override;

        std::uint64_t Execute() override;
        std::unique_ptr<IDatabaseResult> ExecuteQuery() override;

        const std::string& GetLastError() const noexcept { return _lastError; }

    private:
        void SetValue(std::uint32_t paramIndex, std::string value);
        void EnsureCapacity(std::uint32_t paramIndex);

        // Возвращает nullptr при ошибке, статус проверяется вызывающей стороной.
        PGresult* Run();

        PGconn* _connection{nullptr};
        std::string _query;
        std::string _name;
        bool _prepared{false};

        std::vector<std::string> _values;
        std::vector<bool> _nulls;
        std::string _lastError;
    };
}
