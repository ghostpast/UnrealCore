/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include "DatabaseFactory.h"

#include "mariadb/MariaDbConnection.h"

namespace UnrealCore::Database
{
    std::unique_ptr<IDatabaseConnection> DatabaseFactory::Create(DatabaseType type)
    {
        switch (type)
        {
        case DatabaseType::MariaDB:
            return std::make_unique<MariaDbConnection>();
        }

        return nullptr;
    }
}
