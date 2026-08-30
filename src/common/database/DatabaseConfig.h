/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include <cstdint>
#include <string>

namespace UnrealCore::Database
{
    enum class DatabaseType
    {
        MariaDB
        // В будущем: PostgreSQL, SQLite, MSSQL и т.д.
    };

    struct DatabaseConfig
    {
        DatabaseType type;

        std::string host;
        std::uint16_t port;

        std::string user;
        std::string password;
        std::string database;

        std::uint32_t poolSize;
        std::uint32_t connectTimeoutSeconds;
    };
}
