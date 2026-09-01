/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "PostgreSqlResult.h"

#include <charconv>
#include <cstdlib>
#include <string_view>
#include <utility>

namespace UnrealCore::Database
{
    namespace
    {
        template <typename T>
        T ParseInteger(const char* value)
        {
            if (value == nullptr)
                return T{};

            const std::string_view text{value};
            T parsed{};

            const auto* begin = text.data();
            const auto* end = text.data() + text.size();

            if (std::from_chars(begin, end, parsed).ec == std::errc{})
                return parsed;

            return T{};
        }

        double ParseDouble(const char* value)
        {
            if (value == nullptr)
                return 0.0;

            // libpq отдаёт числа в C-локали, strtod здесь безопасен.
            return std::strtod(value, nullptr);
        }
    }

    PostgreSqlResult::PostgreSqlResult(PGresult* result) noexcept
        : _result(result)
    {
        if (_result != nullptr)
        {
            _rowCount = PQntuples(_result);
            _columnCount = PQnfields(_result);
        }
    }

    PostgreSqlResult::~PostgreSqlResult()
    {
        if (_result != nullptr)
            PQclear(_result);
    }

    PostgreSqlResult::PostgreSqlResult(PostgreSqlResult&& other) noexcept
        : _result(std::exchange(other._result, nullptr))
        , _rowIndex(std::exchange(other._rowIndex, -1))
        , _rowCount(std::exchange(other._rowCount, 0))
        , _columnCount(std::exchange(other._columnCount, 0))
    {
    }

    PostgreSqlResult& PostgreSqlResult::operator=(PostgreSqlResult&& other) noexcept
    {
        if (this != &other)
        {
            if (_result != nullptr)
                PQclear(_result);

            _result = std::exchange(other._result, nullptr);
            _rowIndex = std::exchange(other._rowIndex, -1);
            _rowCount = std::exchange(other._rowCount, 0);
            _columnCount = std::exchange(other._columnCount, 0);
        }

        return *this;
    }

    bool PostgreSqlResult::Next()
    {
        if (_result == nullptr || _rowIndex + 1 >= _rowCount)
            return false;

        ++_rowIndex;
        return true;
    }

    std::uint32_t PostgreSqlResult::GetColumnCount() const
    {
        return static_cast<std::uint32_t>(_columnCount);
    }

    const char* PostgreSqlResult::GetRawValue(std::uint32_t columnIndex) const
    {
        if (_result == nullptr || _rowIndex < 0 || _rowIndex >= _rowCount)
            return nullptr;

        const auto column = static_cast<int>(columnIndex);
        if (column < 0 || column >= _columnCount)
            return nullptr;

        if (PQgetisnull(_result, _rowIndex, column) == 1)
            return nullptr;

        return PQgetvalue(_result, _rowIndex, column);
    }

    bool PostgreSqlResult::IsNull(std::uint32_t columnIndex) const
    {
        return GetRawValue(columnIndex) == nullptr;
    }

    std::string PostgreSqlResult::GetString(std::uint32_t columnIndex) const
    {
        const char* value = GetRawValue(columnIndex);
        if (value == nullptr)
            return {};

        const auto column = static_cast<int>(columnIndex);
        const auto length = static_cast<std::size_t>(PQgetlength(_result, _rowIndex, column));

        return std::string{value, length};
    }

    std::int32_t PostgreSqlResult::GetInt32(std::uint32_t columnIndex) const
    {
        return ParseInteger<std::int32_t>(GetRawValue(columnIndex));
    }

    std::int64_t PostgreSqlResult::GetInt64(std::uint32_t columnIndex) const
    {
        return ParseInteger<std::int64_t>(GetRawValue(columnIndex));
    }

    std::uint32_t PostgreSqlResult::GetUInt32(std::uint32_t columnIndex) const
    {
        return ParseInteger<std::uint32_t>(GetRawValue(columnIndex));
    }

    std::uint64_t PostgreSqlResult::GetUInt64(std::uint32_t columnIndex) const
    {
        return ParseInteger<std::uint64_t>(GetRawValue(columnIndex));
    }

    float PostgreSqlResult::GetFloat(std::uint32_t columnIndex) const
    {
        return static_cast<float>(ParseDouble(GetRawValue(columnIndex)));
    }

    double PostgreSqlResult::GetDouble(std::uint32_t columnIndex) const
    {
        return ParseDouble(GetRawValue(columnIndex));
    }

    bool PostgreSqlResult::GetBool(std::uint32_t columnIndex) const
    {
        const char* value = GetRawValue(columnIndex);
        if (value == nullptr)
            return false;

        // В текстовом формате PostgreSQL возвращает 't' / 'f'.
        return value[0] == 't' || value[0] == 'T' || value[0] == '1';
    }
}
