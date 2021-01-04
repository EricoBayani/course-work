// almost all set up code shamelessly stolen from Varma
// necessary modifications to dim an LED writen by Erico Bayani

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <wiringPi.h> // add -lwiringPi to commandline

#include <errno.h> // errno

#define BAUDRATE B115200 // UART speed

#define LED_PIN 1


int main (int argc, char * argv[]){


  // set up the PWM
  wiringPiSetup();
  pinMode(LED_PIN, PWM_OUTPUT);

  struct termios serial; // Structure to contain UART parameters

  char* dev_id = "/dev/serial0"; // Hard coded serial location
  u_int8_t rxbuffer[1]; // Receive data buffer

  printf("Opening %s\n", dev_id);
  int uart_fd = open(dev_id, O_RDWR | O_NOCTTY | O_NDELAY);

  if (uart_fd == -1){ // Open failed
    perror(dev_id);
    return -1;
  }

  // Get UART configuration
  if (tcgetattr(uart_fd, &serial) < 0){
    perror("Getting configuration");
    return -1;
  }

  // Set UART parameters in the termios structure
  serial.c_iflag = 0;
  serial.c_oflag = 0;
  serial.c_lflag = 0;
  serial.c_cflag = BAUDRATE | CS8 | CREAD | PARENB | PARODD;
  // Speed setting + 8-bit data + Enable RX + Enable Parity + Odd Parity

  serial.c_cc[VMIN] = 0; // 0 for Nonblocking mode
  serial.c_cc[VTIME] = 0; // 0 for Nonblocking mode

  // Set the parameters by writing the configuration
  tcsetattr(uart_fd, TCSANOW, &serial);



  // Receive the data back.
  // This is a nonblocking read, so all data may not be received at once.
  printf("Entering loop\n");

  while(1){


      int read_bytes = read(uart_fd, rxbuffer, 1); // read 1 byte from UART into rxbuffer
      if (read_bytes < 0){
        if (errno != EAGAIN){
          perror("Read");
          return -1;
        }
      }

      pwmWrite(LED_PIN, rxbuffer[0]*4);

  }

  close(uart_fd);

  return 0;
}