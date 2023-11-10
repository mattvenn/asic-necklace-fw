#include <defs.h>
//#include <math.h>
#include <stub.h>
#include <hw/common.h>
#include <uart.h>
#include <uart_api.h>

// --------------------------------------------------------
// Firmware routines
// --------------------------------------------------------

#define CLK 10
#define DAT 11

#define SW_L 5 // intermitant
#define SW_R 27 // working

#define NUM_LEDS 12
#define MIN 10
#define MAX 50

typedef struct {
    uint8_t red;      // 8 bits for red color
    uint8_t green;    // 8 bits for green color
    uint8_t blue;     // 8 bits for blue color
    uint8_t brightness; // 8 bits for brightness
} LED;

void configure_io()
{

    //  GPIO 0 is turned off to prevent toggling the debug pin; For debug, make this an output and
    //  drive it externally to ground.

    reg_mprj_io_0 = GPIO_MODE_MGMT_STD_ANALOG;

    // Changing configuration for IO[1-4] will interfere with programming flash. if you change them,
    // You may need to hold reset while powering up the board and initiating flash to keep the process
    // configuring these IO from their default values.

    // https://github.com/TinyTapeout/tinytapeout-02/blob/tt02/INFO.md#pinout

    reg_mprj_io_1 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_2 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_3 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_4 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_5 = GPIO_MODE_MGMT_STD_INPUT_NOPULL; // switch

    reg_mprj_io_8  = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_9  = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;  // led clock
    reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;  // data
    reg_mprj_io_12 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_13 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_14 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_15 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_INPUT_NOPULL; // r switch
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_32 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_33 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_34 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_35 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_36 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
    reg_mprj_io_37 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;

    // Initiate the serial transfer to configure IO
    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);
}

#define BIT_LENGTH 1
void wait(int w) { for (int j=0; j<w; ++j); }
void send_byte(uint8_t value)
{
    bool bit;
    for(int i = 0; i < 8; i ++)
    {
        bit = value & (1 << 7-i);
        reg_mprj_datal = (bit << DAT) + (1 << CLK);

        reg_mprj_datal = (bit << DAT); 
    }
}
void send_frame(LED led)
{
    send_byte(led.brightness);
    send_byte(led.red);
    send_byte(led.blue);
    send_byte(led.green);
}


void main()
{
    reg_gpio_mode1 = 1;
    reg_gpio_mode0 = 0;
    reg_gpio_ien = 1;
    reg_gpio_oe = 1;

    configure_io();

    int w = 25000;
    for (int i=0; i<4; ++i) {
    }


    LED myLED = {0, 120, 00, 255};
    LED startLED = {0, 0, 0, 0};
    LED endLED = {255, 255, 255, 255};
    
    while(true)
    {
        /* button test stuff
        myLED.red = ((reg_mprj_datal >> SW_L ) & 1) ? 0 : 50;
        myLED.blue = ((reg_mprj_datal >> SW_R ) & 1) ? 0 : 50;
            myLED.green = 0;
            send_frame(startLED);
            send_frame(myLED);
            send_frame(endLED);
        */

        int i, j;
        /*
        for(i = MIN; i < MAX; i ++)
        {
            reg_gpio_out = ! reg_gpio_out;
            myLED.green = i;
            send_frame(startLED);
            
            for(j = 0; j < NUM_LEDS; j ++)
                send_frame(myLED);

            send_frame(endLED);
        }
        for(i = MAX; i > MIN; i --)
        {
            reg_gpio_out = ! reg_gpio_out;
            myLED.green = i;
            send_frame(startLED);

            for(j = 0; j < NUM_LEDS; j ++)
                send_frame(myLED);

            send_frame(endLED);
        }
        */
            send_frame(startLED);
            
            for(j = 0; j < NUM_LEDS; j ++)
                send_frame(myLED);

            send_frame(endLED);
    
    }
}
