### Button Debounce

#### 🧭 What Is **Button Debounce**?

**Button debounce** is the process of filtering out *unintended, noisy, or repeated signals* when a mechanical button is pressed or released.

When you press a physical button, the internal **metal contacts** don't make a clean, single connection. Instead, they **"bounce"**—rapidly connecting and disconnecting—for a few milliseconds.

This creates multiple **electrical signals** (spikes) like this:

```c
Raw Button Signal:
      _____           _____
     |     |_______|     |
     |     |       |     |
     |     |       |     |
     |     |       |     |
    --   Bounce   Bounce  --> NOT a clean signal
```

To the microcontroller, it looks like the button was pressed **multiple times**, even though you pressed it just once.

#### 🛠️ Types of Debouncing

| Method                | Description                                                                        |
| --------------------- | ---------------------------------------------------------------------------------- |
| **Software debounce** | Use code to filter noisy signals (e.g., with delay, timer, or integrator logic)    |
| **Hardware debounce** | Use resistors, capacitors, or Schmitt triggers to smooth out the signal physically |

#### Fix #1 Debounce Integrator

Instead of reacting instantly to every change, we say:

I’ll only believe the button is truly pressed or released **if I see the same signal several times in a row.**”

##### 🧮 How does it work?

You use a **counter** called an **integrator**.

Each time you check the button:

- If it's **pressed** (0 for active-low), you **increase** the counter.

- If it's **not pressed**, you **decrease** the counter.

- The counter is kept between `0` and `MAXIMUM_DEBOUNCES`.

Then:

- If the counter reaches the **top** (e.g., 5), we say: “✅ Okay, the button is really **pressed**.”

- If the counter reaches the **bottom** (0), we say: “❌ The button is really **not pressed**.”

### 📊 Example:

Let’s say `MAXIMUM_DEBOUNCES = 3`:

| Raw Input (every 1ms) | 1   | 0   | 1   | 0   | 0   | 0            | 0           | 0           |
| --------------------- | --- | --- | --- | --- | --- | ------------ | ----------- | ----------- |
| Counter               | 0   | 1   | 0   | 1   | 2   | 3            | 3           | 3           |
| Result                | -   | -   | -   | -   | -   | **Pressed!** | **Pressed** | **Pressed** |

```c
/*------------------custom_debounce.c-----------------------------*/
#include "custom_debounce.h"
#include <stdbool.h>

bool debounce_integrator(GPIO_Registers * button,int pin,DebounceState* state){

	int input = (((button->IDR) >> pin)& 0x1); // 0 or 1: raw input signal

	if (input == 0){ // Port is pulled up so ➡️ Port is grounded when button is pressed
		if(state->integrator < MAXIMUM_DEBOUNCES) state->integrator++;
	}
	else{ // button is not pressed [Port is PUlled up]
		if(state->integrator > 0)state->integrator--;
		}
	// check the filtered ButtonCurrState
	state->ButtonCurrState = (state->integrator >= MAXIMUM_DEBOUNCES);

	return state->ButtonCurrState;
}

bool debounce_integrator_rEdge(GPIO_Registers * button,int pin,DebounceState* state){
	bool new_state = debounce_integrator(button, pin, state);
	// state is already a pointer to structure that is why you don't need to do &state [this will be DebounceState **]

	bool pressed_edge = false;
	if (new_state && !state->ButtonPrevState) { // 1 = currently pressed && !(0 = was not pressed earlier) ➡️ Rising Edge
		// Rising edge of a valid press detected
		pressed_edge = true;
	}

	state->ButtonPrevState = new_state; // Save state for next check

	return pressed_edge; // True only once per press
}

bool debounce_integrator_fEdge(GPIO_Registers * button,int pin,DebounceState* state){
	bool new_state = debounce_integrator(button, pin, state);
	// state is already a pointer to structure that is why you don't need to do &state [this will be DebounceState **]

	bool pressed_edge = false;
	if (!new_state && state->ButtonPrevState) { // !(0 = currently pressed) && 1 = was not pressed earlier ➡️ Falling Edge
		// Rising edge of a valid press detected
		pressed_edge = true;
	}

	state->ButtonPrevState = new_state; // Save state for next check

	return pressed_edge; // True only once per press
}

```

```c
/*------------------custom_debounce.h-----------------------------*/

#ifndef CUSTOM_DEBOUNCE_H
#define CUSTOM_DEBOUNCE_H

#include "AHB1_Driver.h" // My our GPIO driver <Not related to this code>
#define DEBOUNCE_TIME       0.3 //s
#define SAMPLE_FREQUENCY    10 //hz
#define MAXIMUM_DEBOUNCES       ((unsigned int)(DEBOUNCE_TIME * SAMPLE_FREQUENCY))

typedef struct {
    int integrator;
    bool ButtonCurrState;
    bool ButtonPrevState;
} DebounceState;

bool debounce_integrator(GPIO_Registers *,int,DebounceState *);
bool debounce_integrator_rEdge(GPIO_Registers *,int,DebounceState *);
bool debounce_integrator_fEdge(GPIO_Registers *,int,DebounceState *);

#endif


```


