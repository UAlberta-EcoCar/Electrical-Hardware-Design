# Fuel Cell Controller

Designed by: **Nicholas Semmens**
Last edited: 2024-05-08

## Theory of Operation

The purpose of this board is to primarily control the peripherals attached to and working
in tandem with the fuel cell. Some of the main considerations are as follows:

1. Hydrogen supply & purge valves
2. Temperature and Pressure sensors
3. MCU possesses logic for the state machine
4. Basic board level buttons for operation

To operate the fuel cell, we require a state machine that operates with certain parameters
of particular importance in our system. Since the recent move to CANbus, I had to implement
more code that 'handshakes' with different boards to verify data has been transceived (by
this I am referring to bi-directional communication). One of the most important parameters
of the start up state is the super-capacitor voltage. The relay board utilizes passive components,
a resistor and capacitor, to support the Fuel Cell in its power delivery. If the car has been
in the off state for any period of time, it is likely that the super-capacitors will have
discharged to a safe level (approx 0-2V). When they are this low, we must wait for the caps
to charge before switching on the motor relay (this allows the operator to apply acceleration).
So, as the capacitors are charging up in the 'charging' state, the fuel cell controller (FCC)
is requesting data from the relay board (RB) of the current voltage of the caps. Once a 
certain threshold is reached, the state machine will move into its 'run' state and send out
the appropriate commands to inform other boards (via CANbus) that the vehicle is now allowed
to have power to the motor.

Between the hydrogen supply and commands being sent to the RB, there is not much else this
board is *required* to do. Obviously, there is more that this board *can* do and *should* do 
for the sake of data telemtry and making cool stuff. For instance, there is an accelerometer
that has not been utilized quite yet, an LCD screen that we could display some information on 
for general testing, and a USB port for real time viewing and communication with the MCU. Besides
this, there is no point in explaining every little thing that appears on the schematic. It is 
rather up to the observer to study and learn how the board was designed. Since I followed some 
generally good practices, it should not be **challenging** for anyone who is comfortable reading
schematics. If you have never seen an electrical schematic it will *obviously* be a challenge
to figure out what is going on, and that is all part of the process of becoming comfortable
doing work in this field.

## The Programming

Once again, explaining every little detail in the code is absolutely **pointless**. Rather, it 
is up to the writer to do a good job organizing and commenting the code. When done right, anybody
who understands C/C++ syntax should be able to understand where the code goes and how it is meant
to work.

Instead, I want to discuss overall design decisions of the structure. Firstly, we used STM32 MCU's 
and as a result we decided to use STM's framework known as HAL (Hardware Abstraction Layer). This 
framework essentially provides one layer of code that prevents you, the user, from needing to dig 
through the datasheet and figure what registers you need to write data and set bits too. This would 
be called bare metal programming. For those interested, give it a shot, otherwise use this wonderful
free tool. You will see a lot of functions with the prefix HAL_ in front of them, this is exactly the 
abstraction layer code I am referring too. Secondly, ***I*** used STM32CubeMX to generate a **Makefile**
version of the code. This is the more classic way of writing and compiling C code when most IDEs did not 
exist. Don't get me wrong, C projects still to this day use Makefiles, and for good reason, it saves
you a hell of a lot of time when you build the project. Furthermore, I used CMake to generate my
Makefile. If you don't know what that is right now, don't worry, I didn't when I started making this 
board. Truth is, its a little complicated and definitely a lot more challenging to figure out than
writing all the code in the STM32CubeIDE (another great tool). I will add a section at the end of 
this document describing exacty how I went about building and flashing the firmware to the MCUs.
To go back to the CubeMX thing, STM provides a base layer of code that is necessary for booting up
and configuring the MCU to do what it needs to do. Then, and only then, can you write your own code 
to get the chip to do what *YOU* want it to do. You will see Init functions in main(), explore what 
they do to get an idea. Thirdly, I am using a software package FreeRTOS (conveniently selectable in 
CubeMX to add to your project) to allow *parallelism*. This just means that I can have multiple threads 
running at the same time. Since the STM32L4xx series MCUs only have one core, it is not truly running 
in parallel, rather, each thread gets to run a few lines of code and a Kernel organizes **context
switches**. Thus, we get a perceived mutli-threading experience. You could argue that this was not 
necessary for this program, but it did have some useful properties that made programming easier and 
much tidier. The big problem is that if you don't understand that these threads are running in 'parallel'
once the scheduler (Kernel) is activated, it is impossible to decipher what is happening in the code.
So read up on FreeRTOS online!

As of writing this document, the code employs five seperate tasks that run in 'parallel'. The titles
of the threads are very descriptive of the overall purpose of the task. CANbus was probably the most
important innovation introduced in Lucy and as such we started from nothing to figure out how this was
all going to work. To put it briefly (and without describing the intimate details of how CAN 2.0 works),
we need to be able to send and receive CAN messages seamlessly. To do this, I employed the use of 
interrupts and two threads. Firstly, whenever a CAN message goes live on the bus, the CAN peripheral 
built into the STM32L4xx has the ability to decipher whether or not that message is important to our 
program. It does this by comparing the Standard ID (StdID) of the CAN message frame to a set of filters 
that we define in the program [see MX\_CAN1\_Init()]. If the StdID matches any of our configured filters,
the CAN controller (peripheral) will insert the message into a Rx (read) first in first out (FIFO) buffer.
When this occurs, the nested vector interrupt controller (NVIC) of the MCU will throw an interrupt 
that says "Hey there is a message in the Rx FIFO buffer, what do you want to do with it?". This interrupt 
calls HAL\_CAN\_RxFifo0MsgPendingCallback() and thus you can read what happens with the message.

If a request for data comes in on the CANbus, the callback mentioned will send the request to the 
StartCanRtrTask() and it will then send the requested data out. Alternatively, if data comes in from 
the bus, it will go to StartCanRxTask() which will put the data in the desired variable.

The I2C task has its own dedicated thread because you cannot try to access the I2C peripheral from two 
different threads at the same time. This is because if one thread is making a request for I2C data from 
some device like the accelerometer, it then proceeds to wait for the response. Thus if during this wait 
time another thread tries to use the same I2C peripheral, we crash the program. Instead of employing the 
use of semaphores (a FreeRTOS tool(s) you should read up on), we have a dedicated thread that handles all 
I2C and thus we avoid unwanted crashes. If you decide I2C requests are necessary elsewhere, you must protect
the operation with semaphores. This is not necessary for the CANbus peripheral because messages get put into 
a queue and the controller manages when the message gets sent. However, it can hold a maximum of three
different messages before further requests will overwrite the old ones that haven't been sent yet. I am 
pretty sure you can customize what happens if you attempt to overwrite.

The purgetask only activates periodically and it is in the blocked state otherwise so it is not running 
very often. It is triggered by a semaphore in the fuel cell task when the car is in the 'run' state. This
could easily be changed to a timer task which would remove the need for the semaphore, you would however,
need to keep track of whether or not the timer is running.



