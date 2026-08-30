/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "DatabaseConfig.h"
#include "IDatabaseResult.h"
#include "IDatabaseStatement.h"

#include <cstdint>
#include <memory>
#include <string>

namespace UnrealCore::Database
{
    class IDatabaseConnection
    {
    public:
        virtual ~IDatabaseConnection() = default;

        virtual bool Connect(const DatabaseConfig& config) = 0;
        virtual void Disconnect() = 0;
        virtual bool IsConnected() const = 0;

        // Выполнение запроса без возврата данных (INSERT/UPDATE/DELETE/DDL).
        // Возвращает количество затронутых строк.
        virtual std::uint64_t Execute(const std::string& query) = 0;

        // Выполнение запроса с возвратом данных (SELECT).
        virtual std::unique_ptr<IDatabaseResult> Query(const std::string& query) = 0;

        // Подготовка параметризированного запроса.
        virtual std::unique_ptr<IDatabaseStatement> PrepareStatement(const std::string& query) = 0;

        virtual bool BeginTransaction() = 0;
        virtual bool Commit() = 0;
        virtual bool Rollback() = 0;

        virtual std::string GetLastError() const = 0;
    };
}
