## USART

The universal synchronous asynchronous receiver transmitter  (USART) offers a flexible
means of full-duplex data exchange with external equipment requiring an industry standard NRZ asynchronous serial data format.

### NRZ (Non-Return-To-Zero)

* A digital **line coding scheme** for representing binary data on a serial line.

* The signal **does not return to zero between bits** (unlike RZ coding)

* Characteristic of NRZ coding is that consecutive bits of the same polarity exhibit no level changes.

### **Data length is configured in UART registers**

- Before communication starts, both **transmitter and receiver must be configured with the same parameters**:
  
  - **Number of data bits** (5, 6, 7, 8, or 9 bits)
  
  - **Parity bit** (enabled/disabled)
  
  - **Number of stop bits** (0.5, 1, 1.5, 2)

- There’s no “auto-detect” of data length; **both sides must agree**.

#### Mark and Space (UART terminology)

UART/USART was designed to be compatible with **telegraphy** and **RS-232 electrical standards**, where signal states were described as **"marking" (idle/high)** and **"spacing" (active/low)**.

* So instead of "1" and "0," old serial standards used **Mark** and **Space**.

* Mark = Logic **1** in UART language ➡️ Idle line State where nothing is tramitted
  
  * In **RS-232 voltage levels** → line held at **negative voltage** (e.g. –12 V)
  
  * In **TTL/CMOS logic UARTs** → line held **high (Vcc, e.g. 3.3 V or 5 V)**

* Space = Logic **0** in UART language ➡️ Active line state
  
  * In **RS-232** → line goes to **positive voltage** (e.g. +12 V).
  
  * In **TTL/CMOS** → line goes **low (0 V)**.

#### Why idle = Mark (1) and start bit = Space (0)?

- UART needs a clear way to **detect the start of transmission**.

- Since idle is always "Mark" (1), a **falling edge to Space (0)** indicates a **Start bit** → receiver synchronizes here.This ensures the receiver can always find the beginning of the frame.

```c
Idle        Start    D0    D1    D2    D3    D4    D5    D6    D7   Stop   Idle
(Mark=1)   (Space=0) (1)   (0)   (0)   (0)   (0)   (0)   (1)   (0)  (Mark=1)
```

### Fractional Baud Rate Generator

The baud rate generator needs to divide the peripheral clock (`fCK`) to produce the desired baud rate.  But often, the required baud rate (e.g., 9600, 115200) doesn’t divide evenly into the system clock. So, instead of using just an integer divisor, STM32 USART uses a **fractional divider**

Note: ***Baud rate is a unit of signals/second***

```c
BaudRate = Fck/(16*DIV)  // DIV is the Clock Divivder
                         // 16 is the oversampling
let the Fck = 16Mhz
Required Baud Rate = 9600 baud
9600 = 16000000 / (16 * DIV)
DIV = 16000000/(16*9600) = 104.1666

As we can see the clock Divison is not an Integere so it means 
we need to take a fractional divisor.
```

STM32 uses "USART_BRR" USART Baud Rate Register to load the baud rate divisor value (mantissa + fraction): The USART then uses this value to generate the timing for TX/RX serial data.

```c
If the DIV = 104.166

Bits [15:4] store the mantissa (integer part)
Bits [3:0] store the fraction (fractional part × 16)

Mantissa = 104
Fraction = 0.166 * 16 =     2.66 ≈ 3

USART_BRR=(104<<4)+3 = 1667 (decimal)   //left shift <<n = multiply by 2^n
That sets the USART close to 9600 baud.

Note: USART_BRR is a 16 bit register
12 bits = Matinssa
4  bits = Fraction
```

### What is oversampling in USART?

When receiving asynchronous serial data, the USART does not have an external clock line.  So it has to **reconstruct the timing** of each bit using its own internal clock.

To make this reliable, the USART samples each bit **multiple times per bit period**. That’s what we mean by *oversampling*.

```c
Let us understand the oversampling a bit more !!!

Receiving at 9600 baud  [9600 baud Rate = 9600 bits per seconds]
Bit period = 1/9600 ≈ 104𝜇𝑠 [Time period of each bit] 
// This is the duration of one data bit on the UART line.


OverSampling = 16 [It sample 16 times the Same bit]
USART samples 16 times per bit → sampling clock = 9600 * 16 =153.6Khz

9600 bits/sec * 16 samples/bit = 153.6K samples/sec = sampling freq

Sampling Time = 1/153.6Khz = 6.5 µs
That means every 6.5 µs it takes a sample.[6.5𝜇𝑠/sample*16 sample = 104𝜇𝑠]


Bit:   |-----------------------1-----------------------|
Time: 0µs                                             104µs
Samples: ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^  ^
                  (16 samples across one bit)
```

The USART then uses the **middle sample** (around 52 µs) as the actual value of that bit. The other samples help in detecting transitions and verifying start/stop bits.

If there’s **noise** or small timing errors (like mismatch between transmitter & receiver clocks),

- With **16x oversampling**, the USART has more resolution to correctly identify the real bit.

- With **8x oversampling**, it’s faster but less tolerant to errors.

✅ So oversampling is like **taking multiple photos per bit** → the more photos you take, the better your chance of catching the true signal in the middle, even if the edges are noisy.

### What is 0.5,1, 1.5, 2 Stop bits indicating?

Stop bits are **extra bit periods after the last data bit** that signal the **end of a frame**. They **ensure the receiver has time to recognize the frame boundary** and prepare for the next start bit

| Stop bits | Meaning                                                                      |
| --------- | ---------------------------------------------------------------------------- |
| 0.5       | Half a bit period of high (Mark) = Half the duration of a normal bit period. |
| 1         | One full bit period of high (Mark)                                           |
| 1.5       | One and a half bit periods of high                                           |
| 2         | Two full bit periods of high                                                 |

Note: **Mark** = logic 1 = idle/high line.

### How UART differentiates stop bits from continuous zeros?

* UART communication is a **continuous stream of framed bits**: **Start → Data → Stop → Start → Data → Stop →...**

* UART does **not** have a separate clock line. It detects **start, data, and stop bits purely by timing**.

**Start bit detection**:

- Receiver watches the idle line (Mark = 1). When it sees a **falling edge** (1 → 0), it assumes this is a **start bit**, and begins timing.

**Data sampling**:

- The receiver samples at the **middle of each bit period**.

- Data bits are read one by one according to the configured **data length** (5–9 bits usually).

**Stop bit(s) detection**:

- After the last data bit, the line **must return to high (Mark = 1)**.

- The receiver samples the stop bit at the **expected middle of the stop bit period**.

- If the line is high at that moment, frame is **valid**.

- If the line is low during the stop bit sample → **framing error**.

### Common regiters Names used in MicroControllers

| Register                        | Purpose                                                                       | Commonly Used?                         |
| ------------------------------- | ----------------------------------------------------------------------------- | -------------------------------------- |
| Status Register `USART_SR`      | Status flags (TX ready, RX ready, errors)                                     | ✅ Yes, always                          |
| Data Register`USART_DR`         | Transmit/Receive data                                                         | ✅ Yes, always                          |
| Baud Rate Register`USART_BRR`   | Configure baud rate                                                           | ✅ Yes, always                          |
| Guard Time Register`USART_GTPR` | extra delay between bytes to allow card processing or insertion/removal time. | ⚠ Rare, only in smartcard/ISO7816 mode |

### Break Tranmission in UART

A **break is essentially a “special long zero”** that tells the receiver:

- “Pay attention, frame is starting”

- Or “Something abnormal happened”

- Or “Wake up from sleep”

| Use Case                      | How Break Helps                                   |
| ----------------------------- | ------------------------------------------------- |
| Synchronization               | Resets receiver timing, aligns start of frame     |
| Error signaling               | Indicates framing or communication errors         |
| Wake-up from sleep            | MCU wakes from low-power mode when break detected |
| Protocol start (LIN, ISO7816) | Indicates start of a new frame to all nodes       |

```c
Break:  10 low bits + stop bits (1 stop bit)
TX:     _ _ _ _ _ _ _ _ _ _ ─
Bits #:  1 2 3 4 5 6 7 8 9 10  Stop

high (Mark = 1) → stop bit
```

### Procedure to send data from USART

#### Configure USART

* Enable USART

* Define Word Length

* Define Stop Bit

* Select Bauda Rate

#### TX of Data

* Write the Data to DR (TDR) register ➡️ TXE  Will become Zero "0"
  
  * a write instruction to the USART_DR register stores the data in the TDR register and which is copied in the shift register at the end of the current transmission.

* Wait for the TXE (Transmit Data Register Empty) bit to be "1" again TXE bit is set by hardware and it indicates:
  
  * Data has been moved from TDR → Shift Register  [Communication has been started]
  
  * TDR will ne now empty [since data is moved to Shift resgiter]
  
  * Now the next data frame can be written in the USART_DR register without overwriting the previous data.

* After writing the last data into the USART_DR register, it is mandatory to wait for TC=1 before disabling the USART or causing the microcontroller to enter the low-power mode
  
  * If you **disable the USART or enter sleep mode now**, the transmission will be **cut off midway** — the stop bit may not be fully sent, causing:
    
    - **Framing error** on the receiver
    
    - **Corrupted or incomplete data**
    
    - Or the receiver **still waiting for stop bit**

#### RX of Data: Introduction

In **asynchronous** communication (like UART), the receiver has no shared clock.  
So it must **detect when a new frame starts** by watching the RX line — which normally stays **HIGH (idle)**.

When the line **goes LOW**, it means a **start bit** has begun. But — small noise or glitches can also cause short LOW pulses. So the receiver doesn’t trigger immediately; it needs to **confirm** that this LOW is a real satrt bit

USART Samples each bit multiple times that means, for start bit detection, the USART looks at a **stream of samples**, not just one.

#### 🧩 The Detection Sequence

```c
1 1 1 0 X 0 X 0 X 0 0 0 0

This sequence represents what the receiver expects to see 
on the RX line before declaring "Start bit detected !"
```

| Sample Index | Expected Value    | Meaning                                      |
| ------------ | ----------------- | -------------------------------------------- |
| 1–3          | 1 1 1             | Line was idle (high) for a while             |
| 4            | 0                 | Start of potential start bit (line went low) |
| 5–10         | Alternating 0 / X | Line stays mostly low (X = don’t care)       |
| 11–14        | 0 0 0 0           | Confirm stable low level — not a glitch      |

* The line must stay low for **at least 7–8 sample periods** continuously  
  → ensures it’s not just a small spike.

* Then, the USART synchronizes its internal clock and starts counting bit periods from this point.

So only **after** seeing this pattern, the USART says:  
✅ “This is a valid start bit — begin sampling for data bits.”

#### 🧠 3️⃣ How the USART Validates the Start Bit

The hardware now samples the RX line at specific points:

```c
Bit period (16 samples):
  |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
        ^     ^     ^  ^  ^  ^
       |-----|-----|   |--------|
       Group 1 (3,5,7)  Group 2 (8,9,10)
```

| Sampling Group | Sample Points (out of 16) | Expected        |                                                        |
| -------------- | ------------------------- | --------------- | ------------------------------------------------------ |
| 1st sampling   | 3rd, 5th, 7th             | All should be 0 | First group (3,5,7): Detects beginning of stable low   |
| 2nd sampling   | 8th, 9th, 10th            | All should be 0 | Second group (8,9,10): Confirms it remains low mid-bit |

✅ Case 1: All 3 bits in both groups are `0`: Start Bit is confirmed

⚠️ Case 2: In both samplings, *2 out of 3 bits* are `0` : Start bit is *accepted*, but **Noise Error (NE flag)** is set.

🚫 Case 3: If the condition above is not met :  Start bit is **rejected** 

### RX of Data: Process

* Set the RE bit USART_CR1. This enables the receiver which begins searching for a
  start bit

* Data is **copied from Shift Register → RDR** . The RXNE =1 (Receive Not Empty flag) bit is set. It indicates that the content of the shift register is transferred to the RDR. In other words, data has been received and can be read (as well as its associated error flags).
  
  * When CPU reads `USART_RDR`:
    
    - **RXNE flag is cleared**  [RXNE = 0]
    
    - Receiver ready for next byte

Note: When a break character is received, the USART handles it as a framing error.

### What is a overrun Error?

An **overrun error** occurs when **new data arrives before the previous data is read** from the **Receive Data Register (RDR)**.  The receiver can’t move data from the **Shift Register → RDR** until the **RXNE flag** (Receive Not Empty) is cleared.

* New data arrives while **RXNE = 1** (i.e., the previous byte wasn’t read yet)

##### What happens during an overrun Error

* **ORE bit** is set in the **Status Register (USART_SR)**.

* **RDR content remains valid** (you can still read the previous data).

* **New incoming data is lost** (the one that caused the overrun).

##### Clearing the ORE Flag

* Read **USART_SR**.

* Then read **USART_DR**.

#### Bit Sampling Method

| Method            | Setting      | Description                                                                                                                  | Best For                                     |
| ----------------- | ------------ | ---------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| **Majority Vote** | `ONEBIT = 0` | Takes **3 samples** (center of bit) and decides the bit value by majority. If samples disagree → **NF (Noise Flag)** is set. | **Noisy environments**                       |
| **Single Sample** | `ONEBIT = 1` | Takes **only 1 sample** (center of bit). No noise detection (NF never set).                                                  | **Noise-free systems** with tight clock sync |

when noise is detected

* The NF bit is set at the rising edge of the RXNE bit

* The invalid data is transferred from the Shift register to the USART_DR register

The NF bit is reset by a USART_SR register read operation followed by a USART_DR
register read operation.

| Sampled Values | NE Status | Received Bit Value | Meaning                                               |
| -------------- | --------- | ------------------ | ----------------------------------------------------- |
| **000**        | 0         | 0                  | All samples low → clean logic 0                       |
| **001**        | 1         | 0                  | One high, two low → noisy but still recognized as 0   |
| **010**        | 1         | 0                  | Middle bit flipped → noise detected, still 0          |
| **011**        | 1         | 1                  | Two highs, one low → noise detected, logic 1 accepted |
| **100**        | 1         | 0                  | Two lows, one high → noise detected, logic 0 accepted |
| **101**        | 1         | 1                  | Mixed pattern → noise detected, logic 1 accepted      |
| **110**        | 1         | 1                  | Two highs, one low → noise detected, logic 1 accepted |
| **111**        | 0         | 1                  | All highs → clean logic 1                             |

#### Framing Error

A framing error is detected when: The stop bit is not recognized on reception at the expected time, following either a de-synchronization or excessive noise.

| Mode               | Stop Bits         | Description                              | Sampling / Error Handling                                                                                                                                                |
| ------------------ | ----------------- | ---------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Normal mode**    | **1 stop bit**    | Default for standard UART                | Samples at **8th, 9th, and 10th** sample positions                                                                                                                       |
| **Normal mode**    | **2 stop bits**   | Adds extra idle time for synchronization | Samples **first stop bit only** (8th–10th samples); if framing error occurs here, **FE flag set**; second stop bit ignored for FE check                                  |
| **Smartcard mode** | **0.5 stop bit**  | Shorter stop bit (half duration)         | ⚠️ **No sampling** → cannot detect framing or break errors                                                                                                               |
| **Smartcard mode** | **1.5 stop bits** | Required for Smartcard protocol          | Receiver **enabled during TX** to detect NACK (low level on stop bit if parity error). Sampling on **16th–18th samples**. **FE flag** set if stop bit pulled low (NACK). |

Note: **The number of stop bits is set using **Control Register 2 (USART_CR2)**.
