/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "database/IDatabaseResult.h"

#include <mysql.h>

namespace UnrealCore::Database
{
    class MariaDbResult final : public IDatabaseResult
    {
    public:
        explicit MariaDbResult(MYSQL_RES* result);
        ~MariaDbResult() override;

        MariaDbResult(const MariaDbResult&) = delete;
        MariaDbResult& operator=(const MariaDbResult&) = delete;

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
        MYSQL_RES* m_result;
        MYSQL_ROW m_currentRow;
        unsigned long* m_columnLengths;
        std::uint32_t m_columnCount;
    };
}
