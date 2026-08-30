/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include <iostream>
#include "revision.h"

int main() {
    std::cout << "UnrealCore AuthServer" << std::endl;
    std::cout << "Revision: " << REVISION_ID << std::endl;
    std::cout << "Branch:   " << REVISION_BRANCH << std::endl;
    std::cout << "Date:     " << REVISION_DATE << std::endl;
    std::cout << "Built:    " << REVISION_TIME << std::endl;
    
    return 0;
}
