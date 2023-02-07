#include "global.h"
#include "system.h"
#include "blink.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the blinker
    Blink *blink = new Blink(std::string("blink"), 2.0);
    module_list->push_back(blink);

    // All start-up messages are still just queued in the Logger and USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "build complete.")
    );

}

void FC_destroy_system(
    std::list<Module*> *module_list
)
{
    std::list<Module*>::iterator it;
    for (it = module_list->begin(); it != module_list->end(); it++)
    {
        Module* mod = *it;
        // std::cout << "deleting " << mod->id << std::endl;
        delete mod;
    };
}
