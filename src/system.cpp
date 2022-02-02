#include "system.h"
#include <Arduino.h>
#include "dummy_gps.h"
#include "dummy_telemetry.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    Serial.println("FC_build_system");
    DummyGPS *gps = new DummyGPS(2.0);
    Serial.print("DummyGPS at ");
    Serial.println((uint64_t)gps);
    module_list->push_back(gps);
}

