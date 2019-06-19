# Porting process
This section contains details regarding porting the OS/Software from the existing hardware to one with a new low-power chipset. Those porting process list here:
- Architecture porting
- Board Porting
- Application porting
    - seed project
    - adding sensor
    - adding Bluetooth Low Energy
    - adding algorithms (to be continued)
- Algorithm fine tuning (addressing #5 issue)
- Test Framework
- Continuous Integration (CI)

All the example code available on https://github.com/overheat/mPdM
## First, RTOS or not?
Yes, we need RTOS. There are many reasons, the most important one is that it will help you to migrate to different boards,chipsets, architectures quickly. So, why not?

There are lots of choice of RTOS, like FreeRTOS, Threadx, Zephyr and so on. We use Zephyr here, which is a LFC Project, is a small, scalable RTOS for connected, resource constrained devices.

<div STYLE="page-break-after: always;"></div>

## Architecture porting
can be divided into several parts:
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

<div STYLE="page-break-after: always;"></div>

## Board Porting

Our target board is a customized nRF52832 board, with Arduino UNO hardware interface and proprietary DIP-24 hardware interface.

We are connecting a 3-axis MEMS accelerometer daughterboard, STEVAL-MKI151V1
with LIS2DH12 on it, to the main board. Later we can replace this sensor to an industry level accelerometer ISM330DLC.

![board](images/nrf5_mesh.png)
<center>Fig. 7</center>

<div STYLE="page-break-after: always;"></div>

```
# Enter working folder
cd $(WORKING_FOLDER) 

# copy nrf52_pca10040 board and modify
cp -r $(ZEPHYR_DIR)/boards/arm/nrf52_pca10040/ ./
mv boards/arm/nrf52_pca10040/ boards/arm/nrf5_meshdk/
# TODO: sed | 
...
```
<div STYLE="page-break-after: always;"></div>

## Application porting
### 1. seed project
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


You will see this on the console:
```
***** Booting Zephyr OS zephyr-v1.14.0-1525-g591b0e1c7af2 *****
Hello World! nrf5_meshdk
```

### 2. adding sensor
First, adding sensor Device tree description, it is a way of describing hardware and configuration information for boards.

```
diff --git a/nrf5_meshdk.overlay b/nrf5_meshdk.overlay
new file mode 100644
index 0000000..b9c602f
--- /dev/null
+++ b/nrf5_meshdk.overlay
@@ -0,0 +1,16 @@
+&spi2 {
+
+       lis2dh@0 {
+               compatible = "st,lis2dh";
+               reg = <0>;
+               spi-max-frequency = <8000000>;
+               label = "LIS2DH";
+               int1-gpios = <&gpio0 13 0>;
+       };
+};
```
*Device tree was adopted for use in the **Linux kernel** for the PowerPC architecture. However, it is now in use for ARM and other architectures.*

Adding project configuration.
```
diff --git a/prj.conf b/prj.conf
index b2a4ba5..8ce791b 100644
--- a/prj.conf
+++ b/prj.conf
@@ -1 +1,5 @@
-# nothing here
+CONFIG_SPI=y
+CONFIG_SENSOR=y
+CONFIG_LIS2DH=y
```

Finally, adding application code.
```
diff --git a/src/main.c b/src/main.c
index 2d4a7f0..6447431 100644
--- a/src/main.c
+++ b/src/main.c
@@ -5,9 +5,58 @@
 void main(void)
 {
-       printk("Hello World! %s\n", CONFIG_BOARD);
+       struct device *dev= device_get_binding(
+                               DT_INST_0_ST_LIS2DH_LABEL);
+
+       while(1){
+
+               printf("Accelerometer data:\n");
+               if (read_sensor(dev, SENSOR_CHAN_ACCEL_XYZ) < 0) {
+                       printf("Failed to read accelerometer data\n");
+               }
+               k_sleep(SLEEP_TIME);
+       }
 }
```

### 3. adding Bluetooth Low Energy
Zephyr's bluetooth demo, which is under samples/bluetooth/peripheral, is a perfect start point. This application demonstrating the BLE Peripheral role. It has several well-known services, like DIS (Device Information) GATT Service, and **vendor-specific GATT services** that it exposes.

We can extend vendor-specific GATT services for our case, adding those characteristics:

| Characteristic Name | Requirement | Mandatory | Optional | Security Permissions |
| ------------------- | ----------- | --------- | -------- | -------- |
| First FFT peak      |     M       | Indicate  |          | None.      |
| CCC descriptor      |     M       | Read, Write |          | None.      |
| Second FFT peak     |     O       | Indicate  |          | None.      |
| CCC descriptor      |     C.1     | Read, Write |          | None.      |
| Valid Range descriptor  |  C.2    | Read |          | None.      |
<center>Table. 5</center>

*M: Mandatory* 
*O: Optional*
*C.1: Mandatory if Intermediate second FFT peak characteristic is supported, otherwise excluded.*
*C.2: Mandatory if Measurement Interval is supported and Writable, otherwise excluded.*

For the implementation detail, please check the Github.

### 4. adding algorithms
To be continued...

<div STYLE="page-break-after: always;"></div>

## Algorithm fine tuning

Once enable Segger’s J-Link supports Real-Time Tracing (RTT), there will be a log file under project folder on your PC. It contains all the date send out by RTT channel. We can load those data to other software, like **Matlab**, for advanced algorithm fine tuning.

After algorithm fine tuning on PC, we need to port back those algorithms back to an embedded device. For example, MATLAB Coder™ is an add-on product that allows you to generate portable and readable C or C++ code from your MATLAB code. This code can then be integrated directly into C/C++ development environment.


![Matlab](images/Matlab.png)
<center>Fig. 8</center>

<div STYLE="page-break-after: always;"></div>

## Test Framework

Almost every popular RTOS has a Test Framework, which provides a simple testing framework intended to be used during development. It provides basic assertion macros and a generic test structure.

The framework can be used in two ways, either as a generic framework for integration testing, or for unit testing specific modules. However, the implementation is different depending on a different kind of RTOS. 

Typical **integration testing** case project may consist of multiple sub-tests or smaller tests that either can be testing functionality or APIs. Functions of implementing a test should follow the guidelines below:

- Test cases function
- Test cases should be documented using doxygen
- Test function names should be unique within the section or component being tested

After finish an integration testing case project, It can then be tested with a simple Unix Bash command or script, then it will be automatically built and run by the sanitycheck script.

**Unit testing**, This means that rather than including the entire Zephyr OS for testing a single function, you can focus the testing efforts into the specific module in question. This will speed up testing since only the module will have to be compiled in, and the tested functions will be called directly.

Since you won’t be including basic kernel data structures that most code depends on, you have to provide function stubs in the test.

<div STYLE="page-break-after: always;"></div>

## Continuous Integration (CI)

A Continuous Integration (CI) system that runs on every Pull Request (PR) in order to verify several aspects of the PR:

- Git commit formatting
- Coding Style
- Sanity Check builds for multiple architectures and boards
- Documentation build to verify any doc changes

This part should be already integrated to project version control hosting. For example, continuous integration work using github + travis-ci.

![CI](images/continuous-integration-in-github.jpg)
<center>Fig. 9</center>

<div STYLE="page-break-after: always;"></div>