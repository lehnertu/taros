#include "global.h"
#include "system.h"

Commander *commander;
Watchdog *watchdog;
Modem *modem;

void FC_init_system()
{
    // create a watchdog generating health analyzes every 5 seconds
    watchdog = new Watchdog(std::string("WATCHDOG"), 5000);
    watchdog->status_out.set_receiver(&(system_log->in));
    
    commander = new Commander(std::string("COMMAND"));
    commander->status_out.set_receiver(&(system_log->in));
    
    // create a modem for communication with a ground station
    modem = new Modem(std::string("MODEM_1"));
    modem->status_out.set_receiver(&(system_log->in));

    // All start-up messages are just queued in the Logger
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "init() complete.")
    );
}

void FC_setup_system(
    std::list<Module*> *module_list
)
{
    watchdog->setup();
    if (watchdog->state() >= MODULE_RUNLEVEL_SETUP_OK)
        module_list->push_back(watchdog);

    commander->setup();
    if (commander->state() >= MODULE_RUNLEVEL_SETUP_OK)
        module_list->push_back(commander);
    
    modem->setup();
    if (modem->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(modem);

    // All start-up messages are just queued in the Logger
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "all setup() complete.")
    );
	
}

void FC_build_system()
{

    // wire the syslog output to the modem for communication with a ground station
    system_log->system_out.set_receiver(&(modem->downlink));

    // wire the modem uplink to the commander
    // modem->uplink.set_receiver(&(commander->command_in));
    
    // All start-up messages are still just queued in the Logger.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "build() complete.")
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

