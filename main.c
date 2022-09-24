#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "hardware/pio.h"
#include "bdm.pio.h"

// Define the time(in ms) to wait for a CDC connection to be established.
// This prevent initial program output being lost, at cost of requiring an active CDC connection
#ifndef PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS
#define PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 15000
#endif

#define data_length     3
#define data_bits       8
#define shift_right     false
#define auto_pull       false
#define register_width  32

#define data_pin 5        // LED pin
#define clock_pin 25        // LED pin

void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);

    // Can't measure clk_ref / xosc as it is the ref
}

int main(){
    // Initialise I/O
    stdio_init_all();

    // Explicitly initialize USB stdio and add it to the current set of stdin drivers.
    // Wait for a CDC connection from the host before returning
    if(stdio_usb_init())
    {
        sleep_ms(5000);
        printf("CDC connection enstablished!\n");
    }

    // Configure sys_pll
    //                   REF     FBDIV VCO       POSTDIV
    // PLL SYS: 12 / 1 = 12MHz * 128 = 1536 / 6 / 2 = 128MHz
    // PLL SYS: 12 / 1 = 12MHz * 64 = 768 / 4 / 4 = 64MHz
    set_sys_clock_pll(768 * MHZ, 4, 4);

    measure_freqs();

    static const float pio_freq = 1 * KHZ;  
    printf("Pio clock frequency: %.2f Hz\n", pio_freq);

    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);
    // Add PIO program to PIO instruction memory, SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, &bdm_program);

    // Calculate the PIO clock divider 
    float div = (float)clock_get_hz(clk_sys) / pio_freq;
    printf("Pio clock divider: %.2f\n", div);


    // Initialize the program using the helper function in our .pio file
    bdm_program_init(pio, sm, offset, data_pin, clock_pin, div, shift_right, auto_pull, data_bits);

    uint data[data_length] = {
        0x55,       // 01010101
        0x08,       // 00001000
        0x7F        // 01111111
    };

    // Put data in TX FIFO. If shift_right is disabled, align data to the left.
    for(int i = 0; i< data_length; i++)
    {
        uint d = data[i];

        if(!shift_right)
        {
            uint shift = register_width - data_bits;
            d = d << shift;
        }

        pio_sm_put_blocking(pio, sm, d);
    }

    printf("TX FIFO full\n");

    // Start running our PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);

    // Do nothing
    while(!pio_sm_is_tx_fifo_empty(pio, sm))
    {
        sleep_ms(1000);
    }

    printf("TX FIFO empty");

    return 0;
}