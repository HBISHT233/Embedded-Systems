### UART: CODING

#### Initialization Function

```c
void uart2_tx_init(void){
    /* Configure USART GPIO Pin
     * 1. Enable Clock access to GPIOA  (PA2 is USART_TX)
     * 2. Set the mode of the GPIOA Pin 2 mode to alternation function [10] using MODER Register
     * 3. Set the PA2 alternate function type to 0111: AF7 [Check alternate function mapping]
    */

    RCC->AHB1ENR |= GPIOAEN; // Enable clock to GPIOA

    //[10] = Alternate mode
    GPIOA->MODER &= ~(1U<<4); // set bit 4 as zero
    GPIOA->MODER |= (1U<<5);    // Set bit 5 as 1

    // Alternation function has two register High and Low [0111: AF7]
    GPIOA->AFR[0] |= (1U<<8);   // ARF[0] is the ARFL register
    GPIOA->AFR[0] |= (1U<<9);
    GPIOA->AFR[0] |= (1U<<10);
    GPIOA->AFR[0] &= ~(1U<<11);

    /* Configure UART Module
     * 1. Enable Clock Access to USART2
     * 2. Configure Baud Rate
     * 3. Configure direction of Data transfer
     * 4. Enable UART module
    */

    RCC->APB1ENR |= UART2EN;
    uart_set_baudrate(USART2,SYS_FREQ,UART_BaudRate);

    USART2->CR1 = TX_Enable;  // We are Enabling Transmission and resetting all other bits as well
    /*
    USRART2->CR1 &= ~(1<<15);  // 16 Over Sampling
    USRART2->CR1 &= ~(1<<12);  // M Bit: World Length : 1 Start bit, 8 Data bits, n Stop bit
    USRART2->CR1 &= ~(1<<10);  // Parity is Disabled
    USRART2->CR1 &= ~(1<<2);   // Receiver is Disabled
     */

    USART2->CR1 |= UART_Enable;

}
```

#### Baud Rate: Method 1

```c
static uint16_t compute_usart_bd (uint32_t PeriphClk, uint32_t BaudRate){
    return (PeriphClk + (BaudRate/2U))/BaudRate;
}

static void uart_set_baudrate (USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate){
    USARTx->BRR = compute_usart_bd(PeriphClk, BaudRate);
}
```

One thing that you must be curious about is that why we are not setting the Mantissa and fraction and why we are just setting the BRR register directly...Let's Understand this by a some example

```c
-----------------OverSampling = 16 ---------------
PeriphClk = 16MHZ = 1,60,00,000
Expected BaudRate = 115200

#Book Formulas for Setting BRR register
Baudrate=Fclk/(8×(2−OVER8)×USARTDIV)  // Over8 = 0 means 16 oversmapling
115200 = 16000000/(16*div)
div = 8.6805
Mantissa = 8d [0x8 in Hexadecimal]
Fraction = 0.6805 * 16 = 10.88 ≃ 11d  [0xB in hexadecimal]

so USART_BRR = Mantissa << 4 + Fraction  
             = 8*16 + 11   [Right shift by n = multiplying by 2^n]
             = 128 + 11
             = 139   [0x8B in Hexadecimal]

#Coding Shortcut
USART_BRR = (Fclk + (BaudRate/2U))/BaudRate;
            (or) (2*PeriphClk + BaudRate) / (2*BaudRate);    
          = Fclk/BaudRate + 0.5
          = 16000000/115200 +0.5
          = 138.88 + 0.5
          = Decimal(139.38) ≃ 139  [0x8B in Hexadecimal]

So we can see from both the method we are getting similar values 
```

```c
-----------------OverSampling = 8 ---------------
PeriphClk = 16MHZ = 1,60,00,000
Expected BaudRate = 57600


#Book Formulas for Setting BRR register
Baudrate=Fclk/(8×(2−OVER8)×USARTDIV)  // Over8 = 1 means 8 oversmapling

57600 = 16000000/(8*div)
div = 34.722
Mantissa = 34d [0x22 in Hexadecimal]
Fraction = 0.722*8 = 5.766 ≃ 6d [0x6 in Hexadecimal]

so USART_BRR = Mantissa << 3 + Fraction  
             = 34*8 + 6  [Right shift by n = multiplying by 2^n]
             = 272 + 6
             = 278  [0x116 in Hexadecimal]

#Coding Shortcut
USART_BRR = (Fclk + (BaudRate/2U))/BaudRate; 
            (or) (2*PeriphClk + BaudRate) / (2*BaudRate);
          = Fclk/BaudRate + 0.5
          = 16000000/57600 +0.5
          = 277.77 + 0.5
          = Decimal(278.27) ≃ 278  [0x116 in Hexadecimal]
```

```c
Note: (2*PeriphClk + BaudRate) / (2*BaudRate); is better compared to (Fclk + (BaudRate/2U))/BaudRate 
since it is possible that BaudRate/2U could result in fraction in cases where BaudRate is odd values.

(PeriphClk + (BaudRate/2U)) / BaudRate computes BaudRate/2U first using integer division.
If BaudRate is odd, BaudRate/2U truncates (drops 0.5), so you lose that half-step before it ever affects the numerator
```

#### Simple Main.c : Send a single Character

```c
int main(void){
    uart2_tx_init();

    while(1){
        usart2_write('H');
        for (volatile int i = 0; i < 100000; i++); // small delay
    }
}
```

#### 

| Mode          | Mantissa bits | Fraction bits | Oversampling | BRR layout       |
| ------------- | ------------- | ------------- | ------------ | ---------------- |
| **OVER8 = 0** | 12 bits       | 4 bits        | 16×          | `(mantissa << 4) |
| **OVER8 = 1** | 13 bits       | 3 bits        | 8×           | `(mantissa << 3) |

##### Why mantissa << 3 in OverSampling of 8?

```c
For oversmapling of 8 the fraction only uses 3 bits
unlike the OvermSampling of 16 where 4 bits are used for fraction part


Therefore [2:0] = Fraction bits in OverSampling = 8 
and [3:0] = Fraction bits when OverSampling = 16

# OverSamling = 8
| 15 ............. 3 | 2 ... 0 |
|   Mantissa (12b)   | Fraction (3b) |

#OverSmapling = 16
| 15 ............. 4 | 3 ... 0 |
|   Mantissa (12b)   | Fraction (4b) |
```

#### Baud Rate: Method 2 [Standard way -  No Shortcut]

```c
static void uart_set_baudrate (USART_TypeDef *USARTx, uint32_t PeriphClk, uint32_t BaudRate){
    // USARTx->BRR = compute_usart_bd(PeriphClk, BaudRate);
    uint32_t usartdiv;
    uint32_t mantissa;
    uint32_t fraction;
    usartdiv = (2*PeriphClk + BaudRate) / (2*BaudRate); // rounding

    if(Oversampling == 8){
        mantissa = usartdiv >> 3;        // integer part
        fraction = usartdiv & 0x7;        // lower 3 bits for fraction
        USARTx->CR1 |= (1U<<15);        // set OVER8 = 1
        USARTx->BRR = (mantissa << 3) | fraction;
    }
    else{
        mantissa = usartdiv >> 4;        // integer part
        fraction = usartdiv & 0xF;        // lower 4 bits for fraction
        USARTx->CR1 &= ~(1U<<15);       // clear OVER8 = 0
        USARTx->BRR = (mantissa << 4) | fraction;
    }
}
```

#### 
