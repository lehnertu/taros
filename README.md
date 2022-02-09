# TAROS
## Teensy Autonomous Robot Operating System

This project aims at the autonomous operation of robotic vehicles.
The target hardware is the Teensy series or ARM microcontrollers.

The operating system of the robot is built as a network of communicating modules.
Modules are handled with tasks in a cooperative multi-tasking scheme.
They communicate by sending and receiving messages that are handled asynchronously.

Every modules is queried once every millisecond whether it has work to do.
If that is the case (for instance if it has received a message) it gets
scheduled in the list of pending tasks. The tasks are then handled
based on a priority level. All work must be divided into chunks that take
less than a millisecond to run. There is no preemption mechanism that
would stop an overrunning module but such events are detected and recorded.

### Project status

At present only the very basic ingredients of TAROS are implemented:
1) scheduling and execution of tasks
2) messages and ports that send and receive messages
3) setting up a network of modules
4) message printout over the USB serial connection

Some example modules are implmented:
1) a simulated GPS sensor
2) a Blink module operating the on-board LED

## Branches

There are two branches that are maintained in parallel.
Many files are identical between those two branches and every commit
of changes to these files should be applied to both branches.

Main differences are:

- core/ contains platform specific code
- hw/ contains drivers for specific hardware in the master (Teensy) branch
- src/ these files are mostly identical (the kernel code of TAROS) with the
exception of global.h global.cpp (platform references) and system.cpp (robot setup)

### master branch

A makefile is provided which creates and uploads an executable onto a Teensy 4.1 microcontroller

### linux_sim branch

This branch runs parallel to the development in the master branch.
It allows to compile and execute TAROS systems on a standard Linux PC.
It is fully independent of any Teensyduino code making up the core of the Teensy version.
All modules accessing special hardware are omitted.
Some special modules are defined like a console output of messages.
The one millisecond interrupt used on the Teensy is replaced by a timer.

