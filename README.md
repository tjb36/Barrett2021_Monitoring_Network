# "An Environmental Monitoring Network for Quantum Gas Experiments and Devices"

The purpose of this repository is to make available the data and code used in the study *"An Environmental Monitoring Network for Quantum Gas Experiments and Devices"*, by T. Barrett, et al. A preprint of the publication is available [here](https://arxiv.org/abs/2101.12726).

The system was designed for monitoring a suite of ultracold atom experiments, together with a variety of relevant environmental conditions to which the experiments are extremely sensitive. Approximately ~100 sensors are installed onto the network, and the data is consolidated into a standardised format in a time-series database. All data is available to inspect remotely for end users through custom dashboards. A high level schematic of the system looks like the following:

![Fig_2](https://user-images.githubusercontent.com/43373700/132879224-3bfe4dbd-2922-46b8-aaa3-1cfb489ff3e8.png)

The code in this repository is organsied according to several main blocks in the system:

1) Environmental measurements are collected using various transducers (photodiodes, thermocouples, current sensors, etc). There are multiple of these "nodes" located around the buliding, connected to an isolated internal local area network using ethernet cables. These microcontrollers are usually Arduino-based, because they are small, cheap, and quick to set up and can be learned by any users in the lab with little programming experience. Examples of firmware deployed on these devices can be found at /Microprocessors Firmware/Sensor Node Devices/. Several transducers will typically be interfaced using the same microcontroller - for example, the board detailed at /Electronics/Photodiode and Thermocouple Amplifier/ allows x12 photodiodes and x8 thermocouples to be measured in succession by a single microcontroller node (gerbers/drill manufacture files are provided).

2) After a set time interval (typically every 20 seconds), a "Collector" microprocessor will poll all the node microcontrollers around the building, requesting that they take sensor readings and send back the values via the network. The firmware for this device can be found at /Microprocessors Firmware/Firmware_Collector.ino. This device keeps track of whether a reply was received from each device (to allow alerts to be sent out if a device becomes inactive), and sends the data as they arrive via serial USB connection to a "Parser" device, based on a Raspberry Pi.

3) The python script running on the Raspberry Pi can be found at /Parser Script/MapCollectorValuesToInfluxDB.py. The Raspberry Pi acts as the single points of contact with the outside internet, and the USB serial acts as a bridge between the internal LAN and the external internet (to avoid connecting many individual microcontrollers to the internet, and allow clearer separation). This script receives the data from the Collector via USB, performs a series of checks on the integrity of the data, and then extracts the relevant values from the payload. The script adds tags, and performs conversion from ADC counts to sensor values, for example (using calibration constants), and finally sends the data via HTTP to the InfluxDB database for storage.

4) An InfluxDB instance needs to be set up at a location that is accessible by the Raspberry Pi, and by default listens on port 8086. InfluxDB can be installed according to the developer's website https://portal.influxdata.com/downloads/ (a docker image is available to make installation quick and simple). In our setup, a virtual machine (VM) provisioned by the university hosts the InfluxDB instance, with 1TB of backed-up disk space (although the amount of data accumulated is typically only several GB per year).

5) For data visualisation, a Grafana instance must be set up, which can be done according the developer's website https://grafana.com/grafana/download?pg=get&plcmt=selfmanaged-box1-cta1 (similarly to InfluxDB, a docker image is available for easy installation). The Grafana instance must then be pointed to the InfluxDB location (in our setup, both Grafana and InfluxDB run on the same virtual machine). Custom dashboards should then be created in Grafana, and data be accessed by individual users as needed.

Typical examples of dashboards are shown below:

![Dashboards](https://user-images.githubusercontent.com/43373700/128684529-48d822db-970a-4d26-8ab9-03cd3e18db1c.png)

which are displayed on various screens around the labs:

![Picture2](https://user-images.githubusercontent.com/43373700/128685035-876fbe2e-174b-4f74-a1f1-e65c0c72b016.png)














