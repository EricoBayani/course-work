/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

volatile int adc_result = 0;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    ADC_Start();
    UART_Start();
    LCD_Start();
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        CyDelay(1);
        adc_result = ADC_Read32();
        if (adc_result > 65535) adc_result = 65535;
        if (adc_result < 0) adc_result = 0;
        adc_result = (adc_result >> 8);
        
        if (adc_result > 245) adc_result = 255;
        if (adc_result < 10) adc_result = 0;
        
        LCD_ClearDisplay();
        LCD_PrintString("Value sent:");
        LCD_PrintNumber(adc_result);
        
        UART_PutChar((uint8_t) adc_result);
    }
}

/* [] END OF FILE */
