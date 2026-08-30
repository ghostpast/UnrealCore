/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include <cstdint>
#include <string>

namespace UnrealCore::Database
{
    class IDatabaseResult
    {
    public:
        virtual ~IDatabaseResult() = default;

        // Переход к следующей строке результата.
        // Возвращает false, если строк больше нет.
        virtual bool Next() = 0;

        virtual std::uint32_t GetColumnCount() const = 0;

        virtual bool IsNull(std::uint32_t columnIndex) const = 0;

        virtual std::string GetString(std::uint32_t columnIndex) const = 0;
        virtual std::int32_t GetInt32(std::uint32_t columnIndex) const = 0;
        virtual std::int64_t GetInt64(std::uint32_t columnIndex) const = 0;
        virtual std::uint32_t GetUInt32(std::uint32_t columnIndex) const = 0;
        virtual std::uint64_t GetUInt64(std::uint32_t columnIndex) const = 0;
        virtual float GetFloat(std::uint32_t columnIndex) const = 0;
        virtual double GetDouble(std::uint32_t columnIndex) const = 0;
        virtual bool GetBool(std::uint32_t columnIndex) const = 0;
    };
}
