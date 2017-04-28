SerialDV
========

**Warning**

Since kernel 4.4.52 the default for FTDI devices (that is in the ftdi_sio kernel module) is not to set it as low latency. This results in the ThumbDV dongle not working anymore because its response is too slow to sustain the normal AMBE packets flow. The solution is to force low latency by changing the variable for your device (ex: /dev/ttyUSB0) as follows:

`echo 1 | sudo tee /sys/bus/usb-serial/devices/ttyUSB0/latency_timer`

**SerialDV** is inspired from the [OpenDV project](https://github.com/dl5di/OpenDV.git) but retains only the minimal interface to encode and decode audio with AMBE3000 based devices in packet mode over a serial link.

For details on the packet interface of AMBE3000 chip download the documentation on [DVSI website](http://www.dvsinc.com/products/a3000.htm)

<h1>Introduction</h1>

**SerialDV** is designed with the following assumptions

  - One object controls one device in one thread. It is up to you to control the device in a separate thread or create a pool of threads for a pool of devices with load balancing. No fancy stuff here because fancy stuff depends too much on the environment.
  - The library manages the atomic operations of decoding one AMBE frame or encoding one audio frame in query/reply pairs or transactions. Each query is returned a complete reply or an error. There is no queuing mechanism whatsoever. 
  - AMBE3000 chip has many modes and features the scope of this library is to provide an easy to use interface for the most popular digital voice modes i.e. D-Star and the DMR likes (DMR, YSF, P25, ...). Some more may be added in the future if the need arises.
  - It will work for both encoding and decoding
  
<h1>Hardware</h1>

This library can control the serial interface to the AMBE3000 chip in packet mode. There are several devices or hardware blocks that implement it. A popular and easy to use one because it works with the well known serial oved UDP using a FTDI chip is the [ThumbDV dongle](http://nwdigitalradio.com/thumbdv-and-dv3000-resource-page/). It can be purchased in the US or from several UK resellers.  In Linux systems the FTDI driver will create a TTY device like `/dev/ttyUSB0` that you will use as the serial device name.
  
<h1>Build and install</h1>

As with all cmake based projects you have to create a build directory in your cloned copy of this repository and cd into it.

Then to install in your own path say `/opt/install/serialdv`use the following cmake comamnd: `cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=/opt/install/serialdv ..`

Then to `make` and `make install`

That's it!

<h1>Usage</h1>

<h2>Test program</h2>

A test program `dvtest` is created in the `bin` subdirectory of the install directory. This program takes a raw audio samples file as input (S16LE 8 kS/s) encodes it then decodes it and writes the result to an output file with the same format (S16LE 8 kS/s). Standard input and/or standard output can be used for piped commands with the `-` special filename.

Ex: `dvtest -D /dev/ttyUSB0 -f 1 -i ../samples/hts1a.raw -o test.raw`

Then you can play back the file with sox package installed: `play -r 8k -e signed-integer -b 16 test.raw`

The full list of parameters can be accessed with the on-line help: `dvtest -h`

In the `samples` subdirectory of the source tree some sample audio files taken from the Codec2 project are provided:

  - `forig.raw`: female voice
  - `morig.raw`: male voice
  - `hts1a.raw`: another male voice
  - `vk5qi.raw`: amateur radio call test (VK5QI). This is a slightly longer sample with a male voice. 