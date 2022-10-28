/*
Embedded Challenge - Need for Speed
Author: Cavin Roger Nunes
       
*/

#include "mbed.h"
#include <stdlib.h>
#include <string.h>

SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1); //CS

// Documents
// Manual for dev board: https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf
// Gyroscope datasheet: https://www.mouser.com/datasheet/2/389/dm00168691-1798633.pdf

int main() {
    int i=0;
    // Chip must be deselected
    cs = 1;
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8,3);
    spi.frequency(1000000);
    cs = 0;                         //chip select = 0
      spi.write(0x20);              //SPI Access the Control_Register1 at address 20H and writing to it
      spi.write(0x0F);              //SPI Setting last 4 bits (0000 1111) to enable the x, y, z axis on the gyro
    cs = 1;                         //Deselecting the chip after initialtzation chip select = 1
    cs = 0;                         //chip select = 0
      spi.write(0xA0);              //SPI Reading from the Control_Register1. 
      int Controlreg_1 = spi.write(0x00);     //Passing 0000 values As no data is passed in SPI read operation
      printf("Control Register 1  = 0x%X\n", Controlreg_1);  //Printing Control_Register1 values to terminal
    cs = 1;                         //Deselecting the chip. chip select = 1

    //Initialize variables
    int step=0;                     // No of steps travelled by the User
    int cumsum =0;                  // Cumulative angulare velocity for the duration of a step
    int count=0;                    // No of readings collected per step
    int firststep=0;                // Identify  when the user goes from stationary to a motion and vice versa
    int distance=0;                 // Total distance travelled by the user

    while (i<40)                    // Assume i from 0 to 40. Each loop is 0.5s. Hence total runtime would be 20s
    {   
      cs = 0;                       // Select the device by setting chip select low
      spi.write(0xE8);              // Send 0xE8, the command to read X_L register
      short X_L = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short X_H = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short X_axis = X_H << 8 | X_L;// Shifting values 8 bit left to combine both high and low reg values
      float scaler = 8.75 * 0.001;  // Scalar multiplier with offset to normalize the angular velocity
      short final_X=(short)((float)X_axis*scaler); // Normalizing gyroscope values along x axis
      cs = 1;                       // Deselect the device
      wait_us(166666);              // Delay between reading the axis
      cs = 0;                       // Select the device by setting chip select low
      spi.write(0xEA);              // Send 0xEA, the command to read Y_L register
      short Y_L = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short Y_H = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short Y_axis = Y_H << 8 | Y_L;// Shifting values 8 bit left to combine both high and low reg values
      scaler = 8.75 * 0.001;        // Scalar multiplier with offset to normalize the angular velocity
      short final_Y=(short)((float)Y_axis*scaler); // Normalizing gyroscope values along x axis  
      cs = 1;                       // Deselect the device
      wait_us(166666);              // Delay between reading the axis
      cs = 0;                       // Select the device by setting chip select low
      spi.write(0xEC);              // Send 0xEC, the command to read Z_L register
      short Z_L = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short Z_H = spi.write(0x00);  // Send a dummy byte to receive the contents register pair
      short Z_axis = Z_H << 8 | Z_L;// Shifting values 8 bit left to combine both high and low reg values
      scaler = 8.75 * 0.001;        // Scalar multiplier with offset to normalize the angular velocity
      short final_Z=(short)((float)Z_axis*scaler); // Normalizing gyroscope values along x axis
      cs = 1;                       // Deselect the device
      wait_us(166666);              // Delay between reading the axis
      //Printing the output of the Gyroscope along all x,y,z axis at every 0.5s
      printf(" Final X_axis = %d\n", final_X);
      printf(" Final Y_axis = %d\n", final_Y);
      printf(" Final Z_axis = %d\n", final_Z);
      i++;                          //Counter for while loop condition (i<40) 
      // If the value of Z is within these threshold then the user is stationary or between
      // the motion of 2 steps
      if(final_Z<=100 && final_Z>=-100){
        if(count!=0){
          int avgangvel=cumsum/count;
          printf("\nAverage angular velocity of the step: %d radians/sec\n",avgangvel);
          // Distance from the board to the center or the top of the leg = 19.68 inches or 0.5 meters
          // Therefore the average Linear forward moment is angular velocity by 0.5
          int avglinvel= (abs(avgangvel)* 0.5)/10;
          // distance = linear velocity X time which we consider as 0.2 seconds the intervals we are taking
          // we add all distances of each step to get the total distance
          distance=distance + (avglinvel *0.5);
          printf("Average Linear forward moment: %d metres/sec\n",avglinvel);
          printf("Distance travelled: %d metres\n\n",distance);
        }
        // Reset values for the next step
        cumsum=0;
        count=0;
      }
      
      // If the value of Z is outside the threshold of 50 then the user is in the motion of a step
      // and cumulative velocity over a step is taken
      if(!(final_Z<=50 && final_Z>=-50)){
        cumsum=cumsum + abs(final_Z); 
        //If the value of firststep changes from 0 to 1 then we count it as a step other velocity values 
        // in a step thus do not affect the no of steps taken
        if(firststep==0){
           firststep=1;
           step=step+1;
           printf("\nNo of steps taken: %d\n\n", step);
        }
        step=step+1;
           printf("\nNo of steps taken: %d\n\n", step);
        // Increment count to reflect the number of reading
        count=count+1;
      }
    }
}
