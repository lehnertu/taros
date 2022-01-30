#pragma once

#include <cstdint>
#include <list>

#include "module.h"
#include "message.h"

/*
    This is the system definition. All modules are created and
    inserted into the list. The connections between modules are defined
    and registered along with the message type.
*/
void FC_build_system(
    std::list<Module*> *module_list
);
