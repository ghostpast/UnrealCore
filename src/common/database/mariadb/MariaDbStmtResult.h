/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "database/IDatabaseResult.h"

#include <mysql.h>

#include <vector>

namespace UnrealCore::Database
{
    // Результат выполнения подготовленного запроса (mysql_stmt_*).
    // В отличие от MariaDbResult, работает через mysql_stmt_bind_result/mysql_stmt_fetch.
    class MariaDbStmtResult final : public IDatabaseResult
    {
    public:
        // metaResult — результат mysql_stmt_result_metadata(statement).
        // Перед вызовом ожидается, что для statement установлен
        // STMT_ATTR_UPDATE_MAX_LENGTH и выполнен mysql_stmt_store_result.
        MariaDbStmtResult(MYSQL_STMT* statement, MYSQL_RES* metaResult);
        ~MariaDbStmtResult() override;

        MariaDbStmtResult(const MariaDbStmtResult&) = delete;
        MariaDbStmtResult& operator=(const MariaDbStmtResult&) = delete;

        bool Next() override;

        std::uint32_t GetColumnCount() const override;

        bool IsNull(std::uint32_t columnIndex) const override;

        std::string GetString(std::uint32_t columnIndex) const override;
        std::int32_t GetInt32(std::uint32_t columnIndex) const override;
        std::int64_t GetInt64(std::uint32_t columnIndex) const override;
        std::uint32_t GetUInt32(std::uint32_t columnIndex) const override;
        std::uint64_t GetUInt64(std::uint32_t columnIndex) const override;
        float GetFloat(std::uint32_t columnIndex) const override;
        double GetDouble(std::uint32_t columnIndex) const override;
        bool GetBool(std::uint32_t columnIndex) const override;

    private:
        struct ColumnBuffer
        {
            std::vector<char> data;
            unsigned long length;
            my_bool isNull;
            my_bool error;
        };

        void SetupBindings();

        MYSQL_STMT* m_statement;
        MYSQL_RES* m_metaResult;
        std::vector<MYSQL_BIND> m_binds;
        std::vector<ColumnBuffer> m_columns;
        std::uint32_t m_columnCount;
    };
}
