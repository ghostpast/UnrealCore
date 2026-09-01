/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "../IDatabaseResult.h"

#include <libpq-fe.h>

namespace UnrealCore::Database
{
    // Владеет PGresult и освобождает его в деструкторе.
    class PostgreSqlResult final : public IDatabaseResult
    {
    public:
        explicit PostgreSqlResult(PGresult* result) noexcept;
        ~PostgreSqlResult() override;

        PostgreSqlResult(const PostgreSqlResult&) = delete;
        PostgreSqlResult& operator=(const PostgreSqlResult&) = delete;

        PostgreSqlResult(PostgreSqlResult&& other) noexcept;
        PostgreSqlResult& operator=(PostgreSqlResult&& other) noexcept;

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
        // Возвращает nullptr, если строка/колонка вне диапазона или значение NULL.
        const char* GetRawValue(std::uint32_t columnIndex) const;

        PGresult* _result{nullptr};
        int _rowIndex{-1};
        int _rowCount{0};
        int _columnCount{0};
    };
}
