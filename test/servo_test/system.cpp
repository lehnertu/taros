#include "global.h"
#include "system.h"
#include "blink.h"
#include "servo.h"
#include "servo_tester.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{

    // create the blinker
    Blink *blink = new Blink(std::string("blink"), 5.0);
    module_list->push_back(blink);

    // create the servo driver
    Servo8chDriver *servo = new Servo8chDriver(std::string("SERVO"));
    module_list->push_back(servo);
    // set sane defaults
    short int defaults[] = {0,200,0,0,-1000,-800,0,0};
    servo->set_pos(defaults);
    // activate channels 1,2,5,6
    servo->activate(0x33);

    // create the servo tester
    // only first channel is influenced
    ServoTester *tester = new ServoTester(std::string("TESTER"),0x01);
    module_list->push_back(tester);
    tester->servo_out.set_receiver(&(servo->in));

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
