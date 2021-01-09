//#include "mbed.h"
#include "../lib/uopmsb/uop_msb_2_0_0.h"
#include "BMP280_SPI.h"

using namespace uop_msb_200;

//On board LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

//On board switch
DigitalIn BlueButton(USER_BUTTON);

//LCD Display
LCD_16X2_DISPLAY disp;

//Buzzer
Buzzer buzz;

//Traffic Lights
DigitalOut traf1RedLED(TRAF_RED1_PIN, 1);
DigitalOut traf1YelLED(TRAF_YEL1_PIN);
DigitalOut traf1GrnLED(TRAF_GRN1_PIN);
DigitalInOut traf2RedLED(TRAF_RED2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);
DigitalInOut traf2YelLED(TRAF_YEL2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);
DigitalInOut traf2GrnLED(TRAF_GRN2_PIN, PIN_OUTPUT, OpenDrainNoPull, 1);


//Light Levels
AnalogIn ldr(AN_LDR_PIN);

//LCD Backlight - consider PwmOut for this :)
PwmOut backLight(LCD_BKL_PIN);                       // set pwmout for the backlight



int main()
{
    //Environmental sensor;
    EnvironmentalSensor sensor;

    //LCD Backlight ON
    // backLight = 1;
    backLight.write(1.0);                             // set baclight intensity to 100%


    while (true) {
        led1 = 1;
        led2 = 1;
        led3 = 1;
        wait_us(500000);
        led1 = 0;
        led2 = 0;
        led3 = 0;  
        wait_us(500000);


        disp.cls();
        disp.printf("LDR: %0.3f", ldr.read());

        


        float temperature=0.0f, pressure=0.0f;
        temperature=sensor.getTemperature();
        pressure=sensor.getPressure();

        printf("%.1fC %.1fmBar\n",temperature,pressure);

        //-------------------------------------------------------------------------------------------------------------------------------------------------------
        // TEMPERATURE
        // ------------------------------------------------------------------------------------------------------------------------------------------------------
        
        int RR = 0, DD, QQ;                                                   // define integers for loops and statements
        float tempArray[5] = { 0, 0, 0, 0, 0 };                               // array for storing temp readings for average
        float tempAvg, sum = 0.0;                                             // define floats for calculating the average                                                        // define integer for category change        

        for( QQ = 0; QQ < 5; QQ++){                                           // for loop that takes the temp reading five times and stores them in an array
          float tempRead = sensor.getTemperature();                          
          tempArray[QQ] = tempRead;
          wait_us(10000);
        }

        for ( DD = 0; DD < 5; DD++ ) {                                        // for loop that adds the numbers in the array togther
        sum += tempArray[DD];
        }
        tempAvg = sum / 5;                                                    // divides result by 5 to find the average

        if( tempAvg <= 5 ){                                                   // FREEZING - if statement when the temp is <= 5 (freezing), alarm and flash red
            traf1YelLED = 0;                                                  // must switch off other LEDs when the category changes
            traf1GrnLED = 0;
            while( tempAvg <= 5){                                             // while loop that flashes red LED
               traf1RedLED = 1;
               wait_us(10000);
               traf1RedLED = 0;
               wait_us(10000);

               if( RR == 3 ){                                                 // if statement in the while loop that will periodically buzz every 3 flashes
                  buzz.playTone("A", uop_msb_200::Buzzer::HIGHER_OCTAVE);
                  wait_us(250000);
                  buzz.rest();
                  RR = 0;
               }
               RR++;
           }
        }
        else if( tempAvg > 5 && tempAvg <= 15 ){                             // COLD - else if when the temp is between 5 and 15 ( cold ), red LED is on
            traf1RedLED = 1;
            traf1YelLED = 0;
            traf1GrnLED = 0;
        }                                                                 
        else if( tempAvg > 15 && tempAvg <= 25 ){                            // WARM - else if when the temp is between 15 and 25 ( WARM ), Yellow LED is on
            traf1RedLED = 0;
            traf1YelLED = 1;
            traf1GrnLED = 0;
        }
        else if( tempAvg > 25 ){                                             // HOT - else if when the temp is over 25 ( Hot ), green LED is on
            traf1RedLED = 0;
            traf1YelLED = 0;
            traf1GrnLED = 1;
        }

        enum tempStates {FREEZING, COLD, WARM, HOT} state;                   // defining enum states to use for the hysteresis

        switch ( state ){                                                    // when the state is frezzing and the temp goes above 0c , it will print   
            case FREEZING:                                                   // the temperature is now cold and set the new state to cold
                if ( tempAvg > 5 ){
                   state =  COLD;
                   printf( "--- The temperature is now COLD ---\n");
                }
            break;
 
            case COLD:                                                      
                if( tempAvg <= 5 ){                                          // when the state is cold and the temp goes below 6c , it will print
                    state = FREEZING;                                        // the temperature is now freezing and set the new state to freezing
                    printf( "--- The temperature is now FREEZING ---\n");
                }
                if( tempAvg > 15 ){                                          // when the state is cold and the temp goes above 15c , it will print
                    state = WARM;                                            // the temperature is now WARM and set the new state to WARM
                    printf( "--- The temperature is now WARM ---\n");
                }            
            break;

            case WARM: 
                if( tempAvg <= 15 ){                                         // when the state is WARM and the temp goes below 16c , it will print
                    state = COLD;                                            // the temperature is now COLD and set the new state to COLD
                    printf( "--- The temperature is now COLD ---\n");
                }
                if( tempAvg > 25 ){                                          // when the state is WARM and the temp goes above 25c , it will print
                    state = HOT;                                             // the temperature is now HOT and set the new state to HOT
                    printf( "--- The temperature is now HOT ---\n");
                }
            break;
  
            case HOT: 
                if( tempAvg <= 25 ){                                         // when the state is HOT and the temp goes below 26c , it will print
                    state = WARM;
                    printf( "--- The temperature is now WARM ---\n");        // the temperature is now WARM and set the new state to WARM
                }
            break;
        }
        wait_us(10000);                                                       // wait 0.01 seconds

        //----------------------------------------------------------------------------------------------------------------------------------------------------
        //light
        //----------------------------------------------------------------------------------------------------------------------------------------------------
 
        int CC , EE ;                                                        // integers for loops and statements
        float Lightarray[5] = { 0, 0, 0, 0, 0 };                             // array to store light readings
        float Lightavg, add = 0.0;                                           // integers for sinding the average light

        for( EE = 0; EE < 5; EE++){                                          // for loop that reads the light level and stores it in an array
        float LDRread = ldr.read();                                          
        Lightarray[EE] = LDRread;
        wait_us(10000);
        }

        for ( CC = 0; CC < 5; CC++ ) {                                       // for loop adds all results in the array together
            add += Lightarray[CC];
        }
        Lightavg = add / 5;                                                  // divides reult by 5 to find average


    enum LightStates { DARK , LOW , DAY , INTENSE } States;                  // defining enum states to use for the hysteresis

    switch ( States ){
        case DARK:
        if ( Lightavg < 0.900 ){                                             // when the state is DARK and the lightavg goes below 0.900 , it will print  
                   States =  LOW;                                            // the the light level is now LOW and set the new state to LOW
                   printf( "--- The light level is now low ---\n");
                }
            break;

        case LOW:
        if ( Lightavg >= 0.900 ){
                   States =  DARK;                                           // when the state is LOW and the lightavg goes above 0.900 , it will print  
                   printf( "--- The light level is now dark ---\n");         // the the light level is now DARK and set the new state to DARK
                }
        if ( Lightavg <  0.600 ){
                   States =  DAY;                                            // when the state is LOW and the lightavg goes below 0.600 , it will print
                   printf( "--- The light level is now day ---\n");          // the the light level is now DAY and set the new state to DAY
                }
            break;

        case DAY:
        if ( Lightavg <= 0.200 ){
                   States =  INTENSE;                                        // when the state is DAY and the lightavg goes below 0.200 , it will print
                   printf( "--- The light level is now intense ---\n");      // the the light level is now INTENSE and set the new state to INTENSE
                }
                if ( Lightavg >= 0.600 ){
                   States =  LOW;                                            // when the state is DAY and the lightavg goes above 0.600 , it will print
                   printf( "--- The light level is now low ---\n");          // the the light level is now LOW and set the new state to LOW
                }
            break;

        case INTENSE:
        if ( Lightavg > 0.200 ){
                   States =  DAY;                                              // when the state is INTENSE and the lightavg goes above 0.200 , it will print
                   printf( "--- The light level is now day ---\n");            // the the light level is now DAY and set the new state to DAY
                }
            break;
    }

        wait_us(100000);                                                       // wait 0.1 seconds

        // pwmout backlight adjustment

       backLight.period(0.001f);                  // set period 1ms
       backLight.write(1.0);                      // set intesity to 100%

       if( States == DARK ){
           backLight.write(1.0);
       }
       else if( States == LOW ){
           backLight.write(0.75);
       }
       else if( States == LOW ){
           backLight.write(0.5);
       }
       else if( States == INTENSE ){
           backLight.write(0.25);
       }
     wait_us(100000);



        //------------------------------------------------------------------------------------------------------------------------------------------------------------

      
    }
}




