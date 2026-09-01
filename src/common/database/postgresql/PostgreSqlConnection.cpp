/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "PostgreSqlConnection.h"

#include "PostgreSqlResult.h"
#include "PostgreSqlStatement.h"

#include <charconv>
#include <string_view>
#include <utility>

namespace UnrealCore::Database
{
    PostgreSqlConnection::~PostgreSqlConnection()
    {
        Disconnect();
    }

    std::string PostgreSqlConnection::EscapeConnectionValue(const std::string& value)
    {
        std::string escaped;
        escaped.reserve(value.size() + 2);

        for (const char symbol : value)
        {
            if (symbol == '\'' || symbol == '\\')
                escaped.push_back('\\');

            escaped.push_back(symbol);
        }

        return escaped;
    }

    std::string PostgreSqlConnection::BuildConnectionString(const DatabaseConfig& config)
    {
        std::string result;
        result.reserve(192);

        const auto append = [&result](const char* key, const std::string& value)
        {
            if (value.empty())
                return;

            if (!result.empty())
                result.push_back(' ');

            result += key;
            result += "='";
            result += EscapeConnectionValue(value);
            result += '\'';
        };

        append("host", config.host);
        append("port", std::to_string(config.port));
        append("user", config.user);
        append("password", config.password);
        append("dbname", config.database);

        if (config.connectTimeoutSeconds > 0)
            append("connect_timeout", std::to_string(config.connectTimeoutSeconds));

        if (!result.empty())
            result.push_back(' ');

        result += "client_encoding='UTF8'";

        return result;
    }

    bool PostgreSqlConnection::Connect(const DatabaseConfig& config)
    {
        Disconnect();

        const std::string connectionString = BuildConnectionString(config);

        _connection = PQconnectdb(connectionString.c_str());
        if (_connection == nullptr)
        {
            _lastError = "PostgreSqlConnection: не удалось выделить дескриптор соединения";
            return false;
        }

        if (PQstatus(_connection) != CONNECTION_OK)
        {
            _lastError = PQerrorMessage(_connection);
            PQfinish(_connection);
            _connection = nullptr;
            return false;
        }

        _lastError.clear();
        _inTransaction = false;
        return true;
    }

    void PostgreSqlConnection::Disconnect()
    {
        if (_connection == nullptr)
            return;

        PQfinish(_connection);
        _connection = nullptr;
        _inTransaction = false;
    }

    bool PostgreSqlConnection::IsConnected() const
    {
        return _connection != nullptr && PQstatus(_connection) == CONNECTION_OK;
    }

    std::uint64_t PostgreSqlConnection::Execute(const std::string& query)
    {
        if (!IsConnected())
        {
            _lastError = "PostgreSqlConnection: соединение не установлено";
            return 0;
        }

        PGresult* result = PQexec(_connection, query.c_str());
        if (result == nullptr)
        {
            _lastError = PQerrorMessage(_connection);
            return 0;
        }

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

    std::unique_ptr<IDatabaseResult> PostgreSqlConnection::Query(const std::string& query)
    {
        if (!IsConnected())
        {
            _lastError = "PostgreSqlConnection: соединение не установлено";
            return nullptr;
        }

        PGresult* result = PQexec(_connection, query.c_str());
        if (result == nullptr)
        {
            _lastError = PQerrorMessage(_connection);
            return nullptr;
        }

        if (PQresultStatus(result) != PGRES_TUPLES_OK)
        {
            _lastError = PQresultErrorMessage(result);
            PQclear(result);
            return nullptr;
        }

        _lastError.clear();
        return std::make_unique<PostgreSqlResult>(result);
    }

    std::unique_ptr<IDatabaseStatement> PostgreSqlConnection::PrepareStatement(const std::string& query)
    {
        if (!IsConnected())
        {
            _lastError = "PostgreSqlConnection: соединение не установлено";
            return nullptr;
        }

        auto statement = std::make_unique<PostgreSqlStatement>(_connection, query);
        if (!statement->Prepare())
        {
            _lastError = statement->GetLastError();
            return nullptr;
        }

        _lastError.clear();
        return statement;
    }

    bool PostgreSqlConnection::ExecuteCommand(const char* sql)
    {
        if (!IsConnected())
        {
            _lastError = "PostgreSqlConnection: соединение не установлено";
            return false;
        }

        PGresult* result = PQexec(_connection, sql);
        if (result == nullptr)
        {
            _lastError = PQerrorMessage(_connection);
            return false;
        }

        const bool ok = PQresultStatus(result) == PGRES_COMMAND_OK;
        if (!ok)
            _lastError = PQresultErrorMessage(result);
        else
            _lastError.clear();

        PQclear(result);
        return ok;
    }

    bool PostgreSqlConnection::BeginTransaction()
    {
        if (_inTransaction)
        {
            _lastError = "PostgreSqlConnection: транзакция уже открыта";
            return false;
        }

        if (!ExecuteCommand("BEGIN"))
            return false;

        _inTransaction = true;
        return true;
    }

    bool PostgreSqlConnection::Commit()
    {
        if (!_inTransaction)
        {
            _lastError = "PostgreSqlConnection: нет активной транзакции";
            return false;
        }

        const bool ok = ExecuteCommand("COMMIT");
        _inTransaction = false;
        return ok;
    }

    bool PostgreSqlConnection::Rollback()
    {
        if (!_inTransaction)
        {
            _lastError = "PostgreSqlConnection: нет активной транзакции";
            return false;
        }

        const bool ok = ExecuteCommand("ROLLBACK");
        _inTransaction = false;
        return ok;
    }

    std::string PostgreSqlConnection::GetLastError() const
    {
        if (!_lastError.empty())
            return _lastError;

        if (_connection != nullptr)
            return PQerrorMessage(_connection);

        return {};
    }
}