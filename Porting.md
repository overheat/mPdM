# Porting 

## RTOS or not?
Yes, we need a RTOS. There are many reasons, the most important one for me is that it will help you to migrate to different board or chipset, even architecture quickly. So, why not?

Also, there are lots of choice of RTOS, like FreeRTOS, Threadx, Zephyr and so on. We use Zephyr here, which is a LFC Project, is a small, scalable RTOS for connected, resource constrained devices.

### An architecture porting, like x86 to ARM
can be divided in several parts:
- The early boot sequence:  
    each architecture has different steps it must take when the CPU comes out of reset  
- Interrupt and exception handling:  
    each architecture handles asynchronous and unrequested events in a specific manner  
- Thread context switching:  
    the Zephyr context switch is dependent on the ABI and each ISA has a different set of registers to save  
- Thread creation and termination:  
    A thread’s initial stack frame is ABI and architecture-dependent, and thread abortion possibly as well  
- Device drivers:  
    most often, the system clock timer and the interrupt controller are tied to the architecture  
    (some required, some optional).
- Utility libraries:  
    some common kernel APIs rely on a architecture-specific implementation for performance reasons  
- CPU idling/power management:  
    most architectures implement instructions for putting the CPU to sleep  
    (partly optional, most likely very desired).
- Fault management:  
    for implementing architecture-specific debug help and handling of fatal error in threads  
    (partly optional).
- Linker scripts and toolchains:  
    architecture-specific details will most likely be needed in the build system and when linking the image  

Fortunately, we already have the same architecture(ARM) and same chipset(nRF52832) even similar board(nrf52_pca10040) in Zephyr project RTOS, so we only need to port on the board level.

### Board Porting

Our target board is a customized nRF52832 board, with Arduino UNO hardware interface and proprietary DIP-24 hardware interface.

Connecting a 3-axis MEMS accelerometer daughterboard, STEVAL-MKI105V1
with LIS3DH on it, to the main board. Later we can replace this sensor to an industry level accelerometer ISM330DLC.

![board](images/nrf5_mesh.png)

```
# Enter working folder
cd $(WORKING_FOLDER) 

# copy nrf52_pca10040 board and modify
cp -r $(ZEPHYR_DIR)/boards/arm/nrf52_pca10040/ ./
mv boards/arm/nrf52_pca10040/ boards/arm/nrf5_meshdk/
# TODO: sed | 
...
```
You also can find the example code in https://github.com/overheat/mPdM


## Application porting
### seed
```
# Enter working folder
cd $(WORKING_FOLDER) 
# copy hello_world sample as seed
cp -r $(ZEPHYR_DIR)/samples/hello_world/* ./

# verify project
mkdir build && cd build
# Use cmake to configure a Ninja-based build system:# build system
cmake -GNinja -DBOARD=nrf5_meshdk -DBOARD_ROOT=$(WORKING_FOLDER)/ ..
# Now run ninja on the generated build system:
ninja
# Flash if everything goes well
ninja flash
```

You will see this:
```
***** Booting Zephyr OS zephyr-v1.14.0-1525-g591b0e1c7af2 *****
Hello World! nrf5_meshdk
```

### Tree





## Issues

- performance
### HW
### SW

## Testing
### HW
- power consumption
### SW
- unit testing(zephyr)
- integrate testing(zephyr)
- system testing
- Regression testing(for bugs)


## Not today
- high throughput
- Microphone
- Environmental sensor