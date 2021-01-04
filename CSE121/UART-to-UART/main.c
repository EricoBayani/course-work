// Interrupt version
#include "project.h"
#include <stdlib.h>


#define ARRAY_SIZE 4096
#define SHOW_TIME 1 // uncomment to show time
#define RX1_STATUS_ERRORS (UART1_RX_STS_PAR_ERROR | UART1_RX_STS_STOP_ERROR | UART1_RX_STS_OVERRUN)
#define RX2_STATUS_ERRORS (UART2_RX_STS_PAR_ERROR | UART2_RX_STS_STOP_ERROR | UART2_RX_STS_OVERRUN)

#define SAT_ERRORS 1000

 uint32_t errors = 0;
//static uint32_t start_time = 0;
//static uint32_t end_time = 0;
//
//uint8_t source[ARRAY_SIZE];
//uint8_t dest[ARRAY_SIZE];
//
//    static int source_index = 0;
//    static int dest_index = 0;

//static int transfer_done = 0; // 0 means false, transfer is not done

uint8_t send_byte = 0;
uint8_t recv_byte1 = 0;
uint8_t recv_byte2 = 0;
uint8_t old_byte;

uint8_t expect;
uint8_t got;
int byte1_init = 0; // first error_free byte received, 0  is false


//static uint32_t bytes_sent = 0;
uint32_t bytes_received = 0;


CY_ISR(TX1_ISR){
    //while((UART1_ReadTxStatus() & UART1_TX_STS_FIFO_NOT_FULL)){
        UART1_PutChar(send_byte++);
    //}
    
    // disable the ISR after the whole array is done
//    if (source_index == ARRAY_SIZE) TX1_ISR_Disable();
}


CY_ISR(RX1_ISR){
    
    if ((UART1_ReadRxStatus() & RX1_STATUS_ERRORS)&& errors < SAT_ERRORS ) errors++; //&& errors < SAT_ERRORS
    //while(UART1_ReadRxStatus() & UART1_RX_STS_FIFO_NOTEMPTY){
    recv_byte1 = UART1_ReadRxData();
    bytes_received++;
    
    expect = old_byte+1;
    got = recv_byte1;
    
    if ((byte1_init) && ((expect != got)) && errors < SAT_ERRORS ){ 
        errors++;
    }
    byte1_init = 1; // 1 means initial byte initialized
    old_byte = recv_byte1;
    //}
//    if (dest_index == ARRAY_SIZE){
//        transfer_done = 1; 
        //end_time = Timer_ReadCounter();
        //Timer_Stop();
//    }
}
//CY_ISR(TX2_ISR){
//    while((UART2_ReadTxStatus() & UART2_TX_STS_FIFO_NOT_FULL)){
//        UART2_PutChar(source[source_index++]);
//    }
//    tx_count++;
//    // disable the ISR after the whole array is done
//    if (source_index == ARRAY_SIZE) TX2_ISR_Disable();
//}
//
//
CY_ISR(RX2_ISR){

    //if ((UART2_ReadRxStatus() & RX2_STATUS_ERRORS) && errors < SAT_ERRORS) errors++; //
    //while(UART1_ReadRxStatus() & UART1_RX_STS_FIFO_NOTEMPTY){
    recv_byte2 = UART2_ReadRxData();
    UART2_WriteTxData(recv_byte2);
    //bytes_received++;
    

}
int main(void)
{
    CyGlobalIntEnable;
    LCD_Start();
    RX1_ISR_StartEx(RX1_ISR);
    TX1_ISR_StartEx(TX1_ISR);
    RX2_ISR_StartEx(RX2_ISR);
//    TX2_ISR_StartEx(TX2_ISR);
    UART1_Start();
    UART2_Start();

    for(;;)
    {
        #ifdef SHOW_TIME
        CyDelay(1000);
        LCD_ClearDisplay();
        LCD_PrintString("Errors:");
        LCD_PrintU32Number(errors);
        LCD_Position(1,0);
        LCD_PrintString("KB/s:");
        LCD_PrintU32Number(bytes_received/1000);
        bytes_received = 0;
        #else
        CyDelay(1000);
        LCD_ClearDisplay();
        LCD_PrintString("expect:");
        LCD_PrintU32Number(expect);
        LCD_Position(1,0);
        LCD_PrintString("actual:");
        LCD_PrintU32Number(got);
        bytes_received = 0;
        #endif
        
    
    }
    return 0;
}

/* [] END OF FILE */
