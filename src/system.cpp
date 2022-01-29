#include "system.h"

#include "dummy_gps.h"

void build_system(
    std::list<Module*> *module_list
)
{
    DummyGPS *gps = new DummyGPS();
    module_list->push_back(gps);
}

