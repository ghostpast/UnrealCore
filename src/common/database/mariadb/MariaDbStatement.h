/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "database/IDatabaseStatement.h"

#include <mysql.h>

#include <memory>
#include <string>
#include <vector>

namespace UnrealCore::Database
{
    class MariaDbStatement final : public IDatabaseStatement
    {
    public:
        explicit MariaDbStatement(MYSQL_STMT* statement);
        ~MariaDbStatement() override;

        MariaDbStatement(const MariaDbStatement&) = delete;
        MariaDbStatement& operator=(const MariaDbStatement&) = delete;

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

    private:
        struct ParamStorage
        {
            std::string stringValue;
            std::int64_t intValue;
            double doubleValue;
            my_bool isNull;
        };

        void EnsureCapacity(std::uint32_t paramIndex);
        void ApplyBindings();

        MYSQL_STMT* m_statement;
        std::vector<MYSQL_BIND> m_binds;
        std::vector<ParamStorage> m_storage;
    };
}
