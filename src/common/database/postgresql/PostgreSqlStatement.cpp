/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "PostgreSqlStatement.h"

#include "PostgreSqlResult.h"

#include <atomic>
#include <charconv>
#include <cstdio>
#include <utility>

namespace UnrealCore::Database
{
    namespace
    {
        std::string MakeStatementName()
        {
            static std::atomic<std::uint64_t> counter{0};
            return "ucstmt_" + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
        }

        template <typename T>
        std::string IntegerToString(T value)
        {
            char buffer[32]{};
            const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
            return std::string{buffer, result.ptr};
        }

        std::string DoubleToString(double value, int precision)
        {
            char buffer[64]{};
            const int written = std::snprintf(buffer, sizeof(buffer), "%.*g", precision, value);
            if (written <= 0)
                return "0";

            return std::string{buffer, static_cast<std::size_t>(written)};
        }
    }

    PostgreSqlStatement::PostgreSqlStatement(PGconn* connection, std::string query)
        : _connection(connection)
        , _query(std::move(query))
        , _name(MakeStatementName())
    {
    }

    PostgreSqlStatement::~PostgreSqlStatement()
    {
        if (!_prepared || _connection == nullptr)
            return;

        if (PQstatus(_connection) != CONNECTION_OK)
            return;

        const std::string sql = "DEALLOCATE " + _name;
        if (PGresult* result = PQexec(_connection, sql.c_str()))
            PQclear(result);
    }

    bool PostgreSqlStatement::Prepare()
    {
        if (_prepared)
            return true;

        if (_connection == nullptr)
        {
            _lastError = "PostgreSqlStatement: соединение отсутствует";
            return false;
        }

        PGresult* result = PQprepare(_connection, _name.c_str(), _query.c_str(), 0, nullptr);
        if (result == nullptr)
        {
            _lastError = PQerrorMessage(_connection);
            return false;
        }

        const bool ok = PQresultStatus(result) == PGRES_COMMAND_OK;
        if (!ok)
            _lastError = PQresultErrorMessage(result);

        PQclear(result);

        if (!ok)
            return false;

        // Узнаём фактическое количество параметров, чтобы заранее выделить слоты.
        if (PGresult* description = PQdescribePrepared(_connection, _name.c_str()))
        {
            if (PQresultStatus(description) == PGRES_COMMAND_OK)
            {
                const int paramCount = PQnparams(description);
                if (paramCount > 0)
                {
                    _values.resize(static_cast<std::size_t>(paramCount));
                    _nulls.assign(static_cast<std::size_t>(paramCount), true);
                }
            }

            PQclear(description);
        }

        _prepared = true;
        return true;
    }

    void PostgreSqlStatement::EnsureCapacity(std::uint32_t paramIndex)
    {
        const std::size_t required = static_cast<std::size_t>(paramIndex) + 1;
        if (_values.size() < required)
        {
            _values.resize(required);
            _nulls.resize(required, true);
        }
    }

    void PostgreSqlStatement::SetValue(std::uint32_t paramIndex, std::string value)
    {
        EnsureCapacity(paramIndex);
        _values[paramIndex] = std::move(value);
        _nulls[paramIndex] = false;
    }

    void PostgreSqlStatement::BindNull(std::uint32_t paramIndex)
    {
        EnsureCapacity(paramIndex);
        _values[paramIndex].clear();
        _nulls[paramIndex] = true;
    }

    void PostgreSqlStatement::BindString(std::uint32_t paramIndex, const std::string& value)
    {
        SetValue(paramIndex, value);
    }

    void PostgreSqlStatement::BindInt32(std::uint32_t paramIndex, std::int32_t value)
    {
        SetValue(paramIndex, IntegerToString(value));
    }

    void PostgreSqlStatement::BindInt64(std::uint32_t paramIndex, std::int64_t value)
    {
        SetValue(paramIndex, IntegerToString(value));
    }

    void PostgreSqlStatement::BindUInt32(std::uint32_t paramIndex, std::uint32_t value)
    {
        SetValue(paramIndex, IntegerToString(value));
    }

    void PostgreSqlStatement::BindUInt64(std::uint32_t paramIndex, std::uint64_t value)
    {
        SetValue(paramIndex, IntegerToString(value));
    }

    void PostgreSqlStatement::BindFloat(std::uint32_t paramIndex, float value)
    {
        SetValue(paramIndex, DoubleToString(static_cast<double>(value), 9));
    }

    void PostgreSqlStatement::BindDouble(std::uint32_t paramIndex, double value)
    {
        SetValue(paramIndex, DoubleToString(value, 17));
    }

    void PostgreSqlStatement::BindBool(std::uint32_t paramIndex, bool value)
    {
        SetValue(paramIndex, value ? "t" : "f");
    }

    PGresult* PostgreSqlStatement::Run()
    {
        if (_connection == nullptr || !_prepared)
        {
            _lastError = "PostgreSqlStatement: запрос не подготовлен";
            return nullptr;
        }

        const int paramCount = static_cast<int>(_values.size());

        std::vector<const char*> params;
        params.reserve(_values.size());

        for (std::size_t i = 0; i < _values.size(); ++i)
            params.push_back(_nulls[i] ? nullptr : _values[i].c_str());

        PGresult* result = PQexecPrepared(
            _connection,
            _name.c_str(),
            paramCount,
            paramCount > 0 ? params.data() : nullptr,
            nullptr,  // длины не нужны для текстового формата
            nullptr,  // все параметры текстовые
            0);       // результат в текстовом формате

        if (result == nullptr)
            _lastError = PQerrorMessage(_connection);

        return result;
    }

    std::uint64_t PostgreSqlStatement::Execute()
    {
        PGresult* result = Run();
        if (result == nullptr)
            return 0;

        const ExecStatusType status = PQresultStatus(result);
        if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            _lastError = PQresultErrorMessage(result);
            PQclear(result);
            return 0;
        }

        _lastError.clear();

        std::uint64_t affected = 0;

        if (const char* tuples = PQcmdTuples(result); tuples != nullptr && tuples[0] != '\0')
        {
            const std::string_view text{tuples};
            std::from_chars(text.data(), text.data() + text.size(), affected);
        }
        else if (status == PGRES_TUPLES_OK)
        {
            affected = static_cast<std::uint64_t>(PQntuples(result));
        }

        PQclear(result);
        return affected;
    }

    std::unique_ptr<IDatabaseResult> PostgreSqlStatement::ExecuteQuery()
    {
        PGresult* result = Run();
        if (result == nullptr)
            return nullptr;

        const ExecStatusType status = PQresultStatus(result);
        if (status != PGRES_TUPLES_OK && status != PGRES_SINGLE_TUPLE)
        {
            _lastError = PQresultErrorMessage(result);
            PQclear(result);
            return nullptr;
        }

        _lastError.clear();
        return std::make_unique<PostgreSqlResult>(result);
    }
}
