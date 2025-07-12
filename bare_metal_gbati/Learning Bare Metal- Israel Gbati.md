# Learning Bare Metal- Israel Gbati

## Documents needed before statring

### 1. Microcontroller Board user manual

It contains information such as 

* Microcontroller pin mapping in the controller board
  
  * Example Connector 8 pin 1 in board = Port A pin 0 of the STM32: These pin mapping can be found in "**<u>connectors</u>**" table of the Board user manual

### 2. Microncontroller Data Sheet

It is one the most important documents

* Memory mapping is one of the major advantage of this document as it tell about all the addresses location "**<u>Memory map</u>**" diagram

* It also tell about the offsets of various buses and the Major registers connected to it such as GPIO etc. "**<u>register boundary addresses</u>**".

* You can also find the "**<u>block diagram</u>**" of the microcontroller in this document which will give the detailed connected to each peripherals and buses

### 3. Micontroller Refrence Manual [lengthiest Document]

This douments help in deeper understanding the microcontoller and how to actually use all the freature that it has

* It contains details on each bit of the registers that a programmer needs to make a driver.

* It also contain contains the base address and offset value of each registers just like the Data sheet "**<u>register boundary addresses</u>**"

* It has instructions on how to initialize and use various peripherals, including example code snippets.

*  Datasheets are often aimed at hardware designers, while **reference manuals cater to software developers and engineers needing detailed operational guidance**.

## GPIO Basic

The basic structure of any GPIO port is simply two buffers [Input and output buffers conenected to enable line] as shown in the image below

<img src="https://miro.medium.com/v2/resize:fit:445/1*9CmxaRlXT9vgUvDOMcUNRA.jpeg" title="" alt="" width="325"> <img src="file:///E:/Embedded-Systems/images/2025-07-12-15-01-45-image.png" title="" alt="" width="293">

<img src="https://miro.medium.com/v2/resize:fit:875/1*zBn2FjlVSHE9R6syQOOXig.jpeg" title="" alt="" width="620">

Enable line is controlled based on various modes of the registers [Input,Output etc.]

#### What's inside an input and output buffer? How they work?

Each input/output buffer has a PMOS and NMOS transistor and a NOT gate. These two transistors are connected in such a way that their Gate and Drain pins are shorted as shown below.

<img src="https://miro.medium.com/v2/resize:fit:875/1*M1LY-ghv4g4_b5kQXOAw5w.jpeg" title="" alt="" width="602">

<img title="" src="https://miro.medium.com/v2/resize:fit:501/1*XspPMyrZBIFckMj7y-_vEw.jpeg" alt="" width="306"><img src="https://miro.medium.com/v2/resize:fit:501/1*4YM5ooT2dDZiis5VRw4mDw.jpeg" title="" alt="" width="306">

### GPIO: I/P mode High Impedacne State (Hi-Z State)

Hi-Z state is when an i/p pinis kept into floating by not connecting it either to high or low voltage levels

* The pin behaves like it is **disconnected** from the circuit internally.

* It allows external devices or circuits to **drive the voltage level** on that pin.

#### What happens if the pin is in floating or Hi-Z state?

Due to circuit noise, there could be some voltage on the pin which could potentially turn ON both the transistors and can provide a direct path from Vcc to Gnd to produce leakage current.

### **Push-Pull Configuration**

In this mode, the GPIO pin can actively drive the output high (to Vcc) or low (to GND) using two transistors—one sourcing current and the other sinking it. This allows for faster switching and stronger drive capabilities, making it suitable for driving loads directly.

* This configurtion simply means GPIO can switch between High-Low state

![](https://miro.medium.com/v2/resize:fit:875/1*cQzHigx0F6bMvdHWrKD-MA.jpeg)

### **Pull-Up Configuration**

A pull-up resistor is connected to the GPIO pin, ensuring that it defaults to a high state (Vcc) when not actively driven low. This is crucial for preventing floating states, which can lead to unpredictable behavior. When the pin is connected to ground, it reads low (0).

<img title="" src="https://i.ytimg.com/vi/aPVMKyZpaPA/maxresdefault.jpg" alt="Tinkercad + Arduino Lesson 6: Pull-up and Pull-down Resistors" width="340">  

### **Pull-Down Configuration**

A pull-down configuration in GPIO connects a resistor between the input pin and ground, ensuring that the pin reads a low logic level (0) when not actively driven high. This prevents the pin from floating, providing a defined default state and improving circuit stability

### **Open Drain Configuration**

* An open-drain output can only sink current, meaning it can pull the line to ground (low) but cannot drive it high.

<img src="file:///E:/Embedded-Systems/images/2025-07-12-10-39-00-image.png" title="" alt="" width="348">

* Open Drain with PULL up [External / Internal]
  
  * An open-drain configuration in GPIO allows multiple devices to share a single line by connecting their outputs to a common point, typically through a pull-up resistor. This setup is commonly used in applications like I2C communication, where devices can signal low without interfering with others, enabling a "wired-AND" logic

### Why we need Schmitt triggered in input?

A Schmitt trigger is a voltage comparator that turns ON only when the input voltage gets pass an upper threshold and once running it will turn OFF only when the input voltage goes below the lower threshold. This property exhibited by the Schmitt trigger is called hysteresis.

<img src="https://miro.medium.com/v2/resize:fit:875/1*Myici2RTS6KAL504kLgfog.jpeg" title="" alt="" width="549">

As you might have observed, the output of the normal input buffer keeps fluctuating in accordance with noise. But if you observe the output of the Schmitt trigger, it has suppressed a lot of noise and gives a steady output.


