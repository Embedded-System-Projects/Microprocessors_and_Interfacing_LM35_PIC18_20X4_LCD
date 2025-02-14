#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 8000000      // 8 MHz internal oscillator

// Define LCD control pins
#define RS LATDbits.LATD0
#define E  LATDbits.LATD1

// Define LCD data pins
#define D4 LATDbits.LATD4
#define D5 LATDbits.LATD5
#define D6 LATDbits.LATD6
#define D7 LATDbits.LATD7

// Function Prototypes
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_Set_Cursor(unsigned char row, unsigned char column);
void ADC_Init(void);
unsigned int ADC_Read(unsigned char channel);
float getTemperature(unsigned int adc_value);
void displayTemperature(float temperature);

void main(void) {
    OSCCON = 0x70;   // Set internal oscillator to 8 MHz

    // Initialize LCD and ADC
    LCD_Init();
    ADC_Init();

    while (1) {
        unsigned int adc_value = ADC_Read(0); // Read from channel AN0
        float temperature = getTemperature(adc_value);
        displayTemperature(temperature);
        __delay_ms(500); // Update every 500 ms
    }
}

// LCD Initialization Function
void LCD_Init(void) {
    TRISD = 0x00;   // Set PORTD as output
    LATD = 0x00;    // Clear LATD
    
    __delay_ms(20); // Wait for power to stabilize
    
    LCD_Command(0x02); // Initialize LCD in 4-bit mode
    LCD_Command(0x28); // 2 lines, 5x8 matrix
    LCD_Command(0x0C); // Display on, cursor off
    LCD_Command(0x06); // Increment cursor
    LCD_Command(0x01); // Clear display
    __delay_ms(2);     // Delay for command to process
}

// Send command to LCD
void LCD_Command(unsigned char cmd) {
    RS = 0;         // Command mode
    D4 = (cmd >> 4) & 0x01;
    D5 = (cmd >> 5) & 0x01;
    D6 = (cmd >> 6) & 0x01;
    D7 = (cmd >> 7) & 0x01;
    E = 1;
    __delay_us(40);
    E = 0;
    __delay_us(40);
    
    D4 = cmd & 0x01;
    D5 = (cmd >> 1) & 0x01;
    D6 = (cmd >> 2) & 0x01;
    D7 = (cmd >> 3) & 0x01;
    E = 1;
    __delay_us(40);
    E = 0;
    __delay_ms(2);
}

// Send character to LCD
void LCD_Char(unsigned char data) {
    RS = 1;         // Data mode
    D4 = (data >> 4) & 0x01;
    D5 = (data >> 5) & 0x01;
    D6 = (data >> 6) & 0x01;
    D7 = (data >> 7) & 0x01;
    E = 1;
    __delay_us(40);
    E = 0;
    __delay_us(40);
    
    D4 = data & 0x01;
    D5 = (data >> 1) & 0x01;
    D6 = (data >> 2) & 0x01;
    D7 = (data >> 3) & 0x01;
    E = 1;
    __delay_us(40);
    E = 0;
    __delay_ms(2);
}

// Send string to LCD
void LCD_String(const char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

// Set cursor position on LCD
void LCD_Set_Cursor(unsigned char row, unsigned char column) {
    unsigned char pos;
    switch (row) {
        case 1:
            pos = 0x80 + column - 1;
            break;
        case 2:
            pos = 0xC0 + column - 1;
            break;
        case 3:
            pos = 0x94 + column - 1;
            break;
        case 4:
            pos = 0xD4 + column - 1;
            break;
    }
    LCD_Command(pos);
}

// ADC Initialization Function
void ADC_Init(void) {
    ADCON0 = 0x01;  // Enable ADC, select AN0 channel
    ADCON1 = 0x0E;  // Set reference voltages, configure AN0 as analog
    ADCON2 = 0xA9;  // Right justified, 12 TAD, Fosc/8
}

// Read ADC value from specified channel
unsigned int ADC_Read(unsigned char channel) {
    ADCON0 &= 0xC5;  // Clear channel selection bits
    ADCON0 |= channel << 3;  // Select channel
    ADCON0bits.GO = 1;  // Start conversion
    while (ADCON0bits.GO);  // Wait for conversion to finish
    return (ADRESH << 8) | ADRESL;  // Return result
}

// Convert ADC value to temperature
float getTemperature(unsigned int adc_value) {
    return (adc_value * 5.0 / 1024.0) * 100.0;  // Convert ADC value to temperature
}

// Display temperature on LCD
void displayTemperature(float temperature) {
    char buffer[16];

    // Clear display
    LCD_Command(0x01);
    __delay_ms(5);

    // Display temperature value on row 1
    LCD_Set_Cursor(1, 1);
    snprintf(buffer, sizeof(buffer), "Temp: %.2f", temperature);
    LCD_String(buffer);
    
    // Display degree symbol and 'C' on row 2
    LCD_Set_Cursor(2, 1);
    LCD_String("Degree Celsius");

    // Display message on row 3
    LCD_Set_Cursor(3, 1);
    LCD_String("The temperature is:");

    // Display status on row 4
    LCD_Set_Cursor(4, 1);
    if (temperature <= 15.0) {
        LCD_String("Cold");
    } else if (temperature > 15.0 && temperature <= 27.0) {
        LCD_String("Normal");
    } else if (temperature > 27.0 && temperature <= 30.0){
        LCD_String("Hot");
    } else {
        LCD_String("Very Hot");
    }
}


