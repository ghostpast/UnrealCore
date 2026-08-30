/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#pragma once

#include "DatabaseConfig.h"
#include "IDatabaseConnection.h"

#include <memory>

namespace UnrealCore::Database
{
    class DatabaseFactory
    {
    public:
        DatabaseFactory() = delete;

        static std::unique_ptr<IDatabaseConnection> Create(DatabaseType type);
    };
}
