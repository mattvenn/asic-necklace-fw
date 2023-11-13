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

#define SW_L 5  // working
#define SW_R 27 // working

#define NUM_LEDS 12
#define MIN 30
#define MAX 220

typedef struct {
    uint8_t brightness; // 8 bits for brightness
    uint8_t red;      // 8 bits for red color
    uint8_t blue;     // 8 bits for blue color
    uint8_t green;    // 8 bits for green color
} LED;

LED LEDS[NUM_LEDS]; // extra 2 for the start and end frame

// switch globals
bool old_sw_l, old_sw_r, sw_l, sw_r = 0;

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
    reg_mprj_io_5 = GPIO_MODE_MGMT_STD_INPUT_NOPULL; // l switch

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


void check_switches()
{
    old_sw_l = sw_l;
    old_sw_r = sw_r;
    sw_l = reg_mprj_datal & (1 << SW_L);
    sw_r = reg_mprj_datal & (1 << SW_R);
    reg_gpio_out = sw_l || sw_r;
}

// APA102 datasheet: https://cdn-shop.adafruit.com/product-files/2477/APA102C-iPixelLED.pdf
// takes 12ms to do a chain of 12 LEDs executing out of RAM
// clock period = 23us
void update_chain()
{
    const unsigned char *bytePtr = &LEDS;
    bool bit;
    int i, j;
    // start frame: 32 bits of 0
    for(j = 0; j < 32; j ++)
    {
        reg_mprj_datal = (0 << DAT) + (1 << CLK);
        reg_mprj_datal = (0 << DAT);
    }

    // PWM data. datasheet is wrong regarding colour order.
    // Each LED has 4 bytes, [111,5 bits global brightness][8 bits red][8 bits blue][8 bits green]
    // We have pointer to memory of first LED, so just need to clock out each byte MSB first
    for(i = 0; i < (NUM_LEDS)*4; i ++)
    {
        for(j = 0; j < 8; j ++)
        {
            bit = bytePtr[i] & (1 << 7-j);
            reg_mprj_datal = (bit << DAT) + (1 << CLK);
            reg_mprj_datal = (bit << DAT);
        }
    }

    // end frame: 32 bits of 1
    for(j = 0; j < 32; j ++)
    {
        reg_mprj_datal = (1 << DAT) + (1 << CLK);
        reg_mprj_datal = (1 << DAT);
    }
}

/*
IMPORTANT!!
Do not add any code here. update_chain function gets copied to flash and the pointer math relies on there being no functions
defined between update_chain() and main()
*/

void main()
{
    reg_gpio_mode1 = 1;
    reg_gpio_mode0 = 0;
    reg_gpio_ien = 1;
    reg_gpio_oe = 1;

    configure_io();

    // program the chain
    for(int i = 0; i < NUM_LEDS; i ++)
    {
        LEDS[i].red = 0;
        LEDS[i].green = MIN;
        LEDS[i].blue = 0;
        LEDS[i].brightness = 255;
    }

    // setup running update_chain from ram
    uint16_t func[&main - &update_chain];
    uint16_t *src_ptr;
    uint16_t *dst_ptr;

    src_ptr = &update_chain;
    dst_ptr = func;
    while (src_ptr != &main)
        *(dst_ptr++) = *(src_ptr++);
    
    int i = 0;
    int step = 1;
    int mode = 0;
    uint8_t cycle = 0;
    while(true)
    {
        // run update_chain from RAM for speed
        ((void(*)())func)();

        // check switches and change modes
        check_switches();
        if(sw_l && !old_sw_l)
        {
            mode ++;
            if(mode == 4)
                mode = 0;
        }
        // little bit of feedback
        reg_gpio_out = (sw_r || sw_l);

        // handle the different modes
        switch(mode) {

            case 0:
                // green fade animation
                for(i = 0; i < NUM_LEDS; i ++)
                {
                    LEDS[i].green += step;
                    LEDS[i].red = 0;
                    LEDS[i].blue = 0;
                }

                if(LEDS[0].green > MAX)
                    step = -1;
                if(LEDS[0].green < MIN)
                    step = +1;
            break;

            case 1:
                for(i = 0; i < NUM_LEDS; i ++)
                {
                    LEDS[i].green = 0;
                    if(sw_r)
                        LEDS[i].red ++;
                }
            break;

            case 2:
                for(i = 0; i < NUM_LEDS; i ++)
                {
                    if(sw_r)
                        LEDS[i].green ++;
                }
            break;

            case 3:
                for(i = 0; i < NUM_LEDS; i ++)
                {
                    if(sw_r)
                        LEDS[i].blue ++;
                }
            break;

        }

    }
}
