/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "IDatabaseResult.h"

#include <cstdint>
#include <memory>
#include <string>

namespace UnrealCore::Database
{
    class IDatabaseStatement
    {
    public:
        virtual ~IDatabaseStatement() = default;

        virtual void BindNull(std::uint32_t paramIndex) = 0;
        virtual void BindString(std::uint32_t paramIndex, const std::string& value) = 0;
        virtual void BindInt32(std::uint32_t paramIndex, std::int32_t value) = 0;
        virtual void BindInt64(std::uint32_t paramIndex, std::int64_t value) = 0;
        virtual void BindUInt32(std::uint32_t paramIndex, std::uint32_t value) = 0;
        virtual void BindUInt64(std::uint32_t paramIndex, std::uint64_t value) = 0;
        virtual void BindFloat(std::uint32_t paramIndex, float value) = 0;
        virtual void BindDouble(std::uint32_t paramIndex, double value) = 0;
        virtual void BindBool(std::uint32_t paramIndex, bool value) = 0;

        // Выполнение запроса без возврата данных (INSERT/UPDATE/DELETE).
        // Возвращает количество затронутых строк.
        virtual std::uint64_t Execute() = 0;

        // Выполнение запроса с возвратом данных (SELECT).
        virtual std::unique_ptr<IDatabaseResult> ExecuteQuery() = 0;
    };
}
