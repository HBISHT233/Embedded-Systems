# GPIOs

### why we need to use BSSR when we can directly change ODR?

While you *can* directly modify the **ODR (Output Data Register)** to change GPIO pin states, the **BSRR** (Bit Set/Reset Register) offers key advantages — especially in embedded systems where precision and safety matter.

* Hardware optimization
  
  * The microcontroller's hardware **executes `BSRR` instructions more efficiently**, as it's designed for fast pin manipulation.
  
  * Many ARM-based MCUs map `BSRR` directly to **single-instruction GPIO operations**, which saves CPU cycles.

* Atomic Opeations
  
  * **BSRR allows atomic bit-level changes**: This means you can **set or reset a specific pin without affecting others**, even in interrupt-driven code.    
  
  * If you write directly to `ODR`, you usually perform a **read-modify-write** sequence:

```c
GPIOx->ODR |= (1 << pin); // Set
GPIOx->ODR &= ~(1 << pin); // Clear
/* This is not atomic, so if an interrupt occurs 
between the read and the write, it could corrupt the result.*/

GPIOA->BSRR = (1 << 5);       // Set PA5
GPIOA->BSRR = (1 << (5 + 16)); // Reset PA5
/* Because BSRR can perform set and reset simultaneously,
you avoid race conditions*/
```

Note: `ODR` is mostly for reading back current output levels or when atomicity isn’t a concern (e.g., during initialization).

Note: If both BSx and BRx are set, BSx has priority. 

### 🚫 Misconception: "Set overrides reset" — Not in `BSRR`

It's true that in **some MCU architectures**, the **set operation can take priority** over reset *when both are attempted simultaneously on the same pin*.

```c
GPIOA->BSRR = (1 << 5);        // Set PA5
GPIOA->BSRR = (1 << (5 + 16)); // Reset PA5
/* Each line takes one cycle and does exactly what it's told,
one after the other. They do not cancel each other unless
you're writing to both in a single write, which you can do like this: */

GPIOA->BSRR = (1 << 5) | (1 << (5 + 16)); 
/* Set and reset PA5 simultaneously
the pin will be set, because set takes priority in a simultaneous write.*/    
```


