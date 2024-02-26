#include "global.h"
#include "system.h"

Commander *commander;
DisplaySSD1331 *display;
StreamFileWriter* fast_log_file_writer;
DummyGPS *gps;
MotionSensor *imu;
Modem *modem;

void FC_init_system()
{
    // create the USB serial output channel
    // usb = new USB_Serial(std::string("USB_1"), 115200);
	// system_log->text_out.set_receiver(&(usb->in));

    commander = new Commander(std::string("CMD"));
    commander->status_out.set_receiver(&(system_log->in));
    
    // create a display with 2Hz update
    display = new DisplaySSD1331(std::string("DISPLAY"), 2.0);
    display->status_out.set_receiver(&(system_log->in));

    // create a logfile writer for streaming data
    // char log_filename[40];
    // sprintf(log_filename, "taros.%05d.fast.log", SD_file_No);
    // fast_log_file_writer = new StreamFileWriter("FASTLOG",std::string(log_filename));

    // create a modem for communication with a ground station
    modem = new Modem(std::string("MODEM_1"), 9600);
    modem->status_out.set_receiver(&(system_log->in));

    // create a simulated GPS module
    gps = new DummyGPS(std::string("GPS_1"), 5.0, 0.0);
    gps->status_out.set_receiver(&(system_log->in));

    // create a motion controller
    imu = new MotionSensor(std::string("IMU_1"));
    imu->status_out.set_receiver(&(system_log->in));

    // creste a servo controller
    // Servo8chDriver *servo = new Servo8chDriver(std::string("SERVO_1"));

    // create a logger capturing telemetry data at specified rate
    // Requester *req = new Requester(std::string("LOG_5S"), 0.2);

    // All start-up messages are just queued in the Logger
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "init() complete.")
    );
}

void FC_setup_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    // usb->setup();
    // if (usb->state() >= MODULE_RUNLEVEL_SETUP_OK)
    //  	module_list->push_back(usb);
    
    commander->setup();
    if (commander->state() >= MODULE_RUNLEVEL_SETUP_OK)
        module_list->push_back(commander);
    
    // create a display with 2Hz update
    display->setup();
    if (display->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(display);

    // create a logfile writer for streaming data
    // fast_log_file_writer->setup();
    // if (fast_log_file_writer->state() >= MODULE_RUNLEVEL_SETUP_OK)
    // 	module_list->push_back(fast_log_file_writer);
    
    // create a modem for communication with a ground station
    modem->setup();
    if (modem->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(modem);

    // create a simulated GPS module
    gps->setup();
    if (gps->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(gps);

    // create a motion controller
    imu->setup();
    if (imu->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(imu);

    // creste a servo controller
    /*
    servo->setup();
    if (servo->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(servo);
	*/
	
    // create a logger capturing telemetry data at specified rate
    /*
    req->setup();
    if (req->state() >= MODULE_RUNLEVEL_SETUP_OK)
    	module_list->push_back(req);
	*/
	
    // All start-up messages are just queued in the Logger
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "all setup() complete.")
    );
	
}

void FC_build_system()
{

    // wire the syslog output to the modem for communication with a ground station
    // TODO : this leads to lots of systick overruns
    system_log->system_out.set_receiver(&(modem->downlink));

    // wire the modem uplink to the commander
    // modem->uplink.set_receiver(&(commander->command_in));
    
    // wire the simulated GPS module
    gps->tm_out.set_receiver(&(system_log->in));
    
    // wire the motion controller
    imu->AHRS_out.set_receiver(&(display->ahrs_in));
    imu->GYRO_out.set_receiver(&(display->gyro_in));
    imu->AHRS_out.set_receiver(&(fast_log_file_writer->ahrs_in));
    imu->GYRO_out.set_receiver(&(fast_log_file_writer->gyro_in));
    
    
    // create a logger capturing telemetry data at specified rate
    /*
    req->out.set_receiver(&(usb->in));
    req->out.set_receiver(&(modem->downlink));
    auto callback = std::bind(&DummyGPS::get_position, gps); 
    req->register_server_callback(callback,"GPS_1");
    */
    
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
