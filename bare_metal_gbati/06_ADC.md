### ADC

An **ADC (Analog-to-Digital Converter)** converts a **continuous analog signal** (like voltage) into a **digital binary value** that can be processed by microcontrollers or digital systems.

#### ADC resolution

**ADC Resolution** refers to how finely an ADC can divide the input analog voltage range. It is determined by the **number of bits (n)** used to represent the digital value.

---

### 📌 Formula

```c
Steps (levels)=2^n
Voltage per step (LSB) = Vref/(2^n-1)
```

- **n = resolution in bits**

- **Vref = reference voltage**

- **LSB = Least Significant Bit**, the smallest detectable voltage change



### 📊 Resoluton Example

| Resolution | Levels (2ⁿ) | Voltage per Step (3.3V Ref) |
| ---------- | ----------- | --------------------------- |
| 8-bit      | 256         | ~12.9 mV                    |
| 10-bit     | 1024        | ~3.22 mV                    |
| 12-bit     | 4096        | ~0.805 mV                   |
| 16-bit     | 65536       | ~0.05 mV                    |

| Resolution (bits) | Levels (2ⁿ) | Voltage per Step (Vref = 5V) |
| ----------------- | ----------- | ---------------------------- |
| **8-bit**         | 256         | 5 V / 255 ≈ **19.6 mV**      |
| **10-bit**        | 1024        | 5 V / 1023 ≈ **4.89 mV**     |
| **12-bit**        | 4096        | 5 V / 4095 ≈ **1.22 mV**     |
| **16-bit**        | 65536       | 5 V / 65535 ≈ **0.076 mV**   |

#### ADC independent mode

**ADC Independent Mode** means each ADC module works *separately*, without synchronization or sharing data with other ADC units. This is a commonly used mode in microcontrollers (like STM32) when you want each ADC to measure different signals independently.

### Key Points – Independent Mode

- Each ADC converts its own analog input.

- No triggering or data sharing between ADCs.

- Can use different:
  
  - Input channels
  
  - Sampling times
  
  - Conversion modes (single/continuous)
  
  - Trigger sources

- Ideal when measurements are unrelated.

| Single Channel       | Multi Channel        | Injected       |
| -------------------- | -------------------- | -------------- |
| Single Conversion    | Single Conversion    | Continous mode |
| Continous Conversion | Continous Conversion |                |

Injected modes means the conversion is triggered  by an external event or by software.



Single ADC module does not mean we have only one single channel, channle is different from ADC modules number in your microcontroller. Channels simply refers to how many sensors we can connect to the ADC module of our microcontroller and sample them.

| Feature                       | **ADC Channel**             | **ADC Module**                                        |
| ----------------------------- | --------------------------- | ----------------------------------------------------- |
| Definition                    | Individual analog input pin | Entire ADC hardware unit                              |
| Physical Entity               | Pin (e.g., PA0, PA1…)       | ADC1, ADC2, ADC3 (in MCU)                             |
| Number                        | Many channels per ADC       | Few modules per MCU                                   |
| Function                      | Reads one analog signal     | Performs conversion process                           |
| Configuration                 | Select input signal         | Controls conversion logic                             |
| Can measure multiple signals? | ❌ One per channel           | ✔ Yes (multiple sequentially or via multiple modules) |
| Trigger?                      | Shares ADC timing           | Has its own trigger                                   |
| Example                       | Channel 0 → Temp sensor     | ADC1 → Handles multiple channels                      |
