// All code stolen from example 3 of Getting started with DMA
#include "project.h"

#define TABLE_LENGTH 128
#define DMA_BYTES_PER_BURST 1
#define DMA_REQUEST_PER_BURST 1

#define THRESHOLD 100
#define FREQ_DIVIDER(x) (uint16_t) 24000000/(x*TABLE_LENGTH) // x is frequency you want in Hz

/* This table stores the 128 points in Flash for smoother sine wave generation */
CYCODE const uint8 sineTable_base[TABLE_LENGTH] = 
{
	128, 134, 140, 147, 153, 159, 165, 171,
	177, 183, 188, 194, 199, 204, 209, 214,
	218, 223, 227, 231, 234, 238, 241, 244,
	246, 248, 250, 252, 253, 254, 255, 255,
	255, 255, 255, 254, 253, 252, 250, 248,
	246, 244, 241, 238, 234, 231, 227, 223,
	218, 214, 209, 204, 199, 194, 188, 183, 
	177, 171, 165, 159, 153, 147, 140, 134, 
	128, 122, 115, 109, 103,  97,  91,  85,
	 79,  73,  68,  62,  57,  52,  47,  42,
	 37,  33,  29,  25,  22,  18,  15,  12,
	 10,   7,   6,   4,   2,   1,   1,   0,
	  0,   0,   1,   1,   2,   4,   6,   7,
	 10,  12,  15,  18,  22,  25,  29,  33,
	 37,  42,  47,  52,  57,  62,  68,  73,
	 79,  85,  91,  97, 103, 109, 115, 122
};	 



CYCODE uint8 sineTable_shifted[TABLE_LENGTH*2] = 
{
	128, 134, 140, 147, 153, 159, 165, 171,
	177, 183, 188, 194, 199, 204, 209, 214,
	218, 223, 227, 231, 234, 238, 241, 244,
	246, 248, 250, 252, 253, 254, 255, 255,
	255, 255, 255, 254, 253, 252, 250, 248,
	246, 244, 241, 238, 234, 231, 227, 223,
	218, 214, 209, 204, 199, 194, 188, 183, 
	177, 171, 165, 159, 153, 147, 140, 134, 
	128, 122, 115, 109, 103,  97,  91,  85,
	 79,  73,  68,  62,  57,  52,  47,  42,
	 37,  33,  29,  25,  22,  18,  15,  12,
	 10,   7,   6,   4,   2,   1,   1,   0,
	  0,   0,   1,   1,   2,   4,   6,   7,
	 10,  12,  15,  18,  22,  25,  29,  33,
	 37,  42,  47,  52,  57,  62,  68,  73,
	 79,  85,  91,  97, 103, 109, 115, 122,
    
	128, 134, 140, 147, 153, 159, 165, 171,
	177, 183, 188, 194, 199, 204, 209, 214,
	218, 223, 227, 231, 234, 238, 241, 244,
	246, 248, 250, 252, 253, 254, 255, 255,
	255, 255, 255, 254, 253, 252, 250, 248,
	246, 244, 241, 238, 234, 231, 227, 223,
	218, 214, 209, 204, 199, 194, 188, 183, 
	177, 171, 165, 159, 153, 147, 140, 134, 
	128, 122, 115, 109, 103,  97,  91,  85,
	 79,  73,  68,  62,  57,  52,  47,  42,
	 37,  33,  29,  25,  22,  18,  15,  12,
	 10,   7,   6,   4,   2,   1,   1,   0,
	  0,   0,   1,   1,   2,   4,   6,   7,
	 10,  12,  15,  18,  22,  25,  29,  33,
	 37,  42,  47,  52,  57,  62,  68,  73,
	 79,  85,  91,  97, 103, 109, 115, 122
};

/* Variable declarations for DMA . 
* These variables are defined as global variables to avoid "may be used before being set" warning 
* issued by the PSoC 5 compilers  MDK/RVDS.In this case these variables are automatically initialized to zero */
uint8 DMA_Chan_base;               /* The DMA Channel */
uint8 DMA_TD_base[2];	          /* The DMA Transaction Descriptor (TD) */  	
uint8 DMA_Chan_shifted;               /* The DMA Channel */
uint8 DMA_TD_shifted[2];	          /* The DMA Transaction Descriptor (TD) */

static volatile int adc_result = 0;
static uint32_t shift_ptr = (uint32_t) sineTable_shifted;
int main()
{
    Clock_base_SetDividerValue(50);
   /* Start VDAC  */
    VDAC8_base_Start();
    VDAC8_shifted_Start();
	
	/* Defines for DMA configuration */
    #if (defined(__C51__))  /* Source base address when PSoC3 is used */    
	    #define DMA_SRC_BASE (CYDEV_FLS_BASE)   
    #else  					/* Source base address when PSoC5 is used */
	    #define DMA_SRC_BASE (&sineTable_base[0])
        #define DMA_SHIFTED_SRC_BASE (&sineTable_shifted[0])
	#endif    
    
	#define DMA_DST_BASE (CYDEV_PERIPH_BASE)  /* Destination base address */

    // Initalize 2 DMA channels, 1 for the base wave LUT, another for the Shifted table LUT, that's twice the size
    DMA_Chan_base = DMA_base_DmaInitialize(DMA_BYTES_PER_BURST, DMA_REQUEST_PER_BURST, HI16(DMA_SRC_BASE), HI16(DMA_DST_BASE) );
    DMA_Chan_shifted = DMA_shifted_DmaInitialize(DMA_BYTES_PER_BURST, DMA_REQUEST_PER_BURST, HI16(DMA_SHIFTED_SRC_BASE), HI16(DMA_DST_BASE) );
	
    // Allocate the TDs for the channels
    DMA_TD_base[0] = CyDmaTdAllocate();  	
    DMA_TD_shifted[0] = CyDmaTdAllocate();  
    // configure the two TDs
	CyDmaTdSetConfiguration(DMA_TD_base[0], TABLE_LENGTH, DMA_TD_base[0], TD_INC_SRC_ADR); 	 
    CyDmaTdSetConfiguration(DMA_TD_shifted[0], TABLE_LENGTH, DMA_TD_shifted[0], TD_INC_SRC_ADR);
    // Set the lower sources address to their LUTS and the lower dest addresses to their respective VDAX
    CyDmaTdSetAddress(DMA_TD_base[0], LO16((uint32)sineTable_base), LO16((uint32)VDAC8_base_Data_PTR) );
    CyDmaTdSetAddress(DMA_TD_shifted[0], LO16((uint32)sineTable_shifted), LO16((uint32)VDAC8_shifted_Data_PTR) );
    // Set the first TDs
    CyDmaChSetInitialTd(DMA_Chan_base, DMA_TD_base[0]); 	
    CyDmaChSetInitialTd(DMA_Chan_shifted, DMA_TD_shifted[0]);
	// Turn the channels on
    CyDmaChEnable(DMA_Chan_base, 1); 	
    CyDmaChEnable(DMA_Chan_shifted, 1);
    
    // start the other components of the project
    ADC_Start();
    LCD_Start();

    for(;;)
    {
       CyDelay(10);

        adc_result = ADC_Read32();
        if (adc_result > 65535) adc_result = 65545;
        if (adc_result < 0) adc_result = 0;
        // mess with the result of the adc until it fits into the length of the base table
        adc_result = (adc_result >> 8) - 30;
        if (adc_result > TABLE_LENGTH) adc_result = TABLE_LENGTH;
        if (adc_result < 0) adc_result = 0;
    
        // shifting the address to start reading the lookup table from creates the appearance pf phase shift
        shift_ptr = (uint32_t) sineTable_shifted + adc_result;
        
        CyDmaTdSetAddress(DMA_TD_shifted[0], LO16(shift_ptr), LO16((uint32)VDAC8_shifted_Data_PTR) );
        
        // mess with the result from the adc to scale into 0-360
        float scaled_result = (float)adc_result * 2.8125f;
        
        // display the phase shift
        LCD_ClearDisplay();
        LCD_PrintNumber((int)scaled_result);
        LCD_PrintString(" Degrees");
    
    }
    
    return 0;
}

/* [] END OF FILE */
