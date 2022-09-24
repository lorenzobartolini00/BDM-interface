#include "functions.h"
#include "bdm.pio.h"

#include "config.h"

int main(){
    // Start usb
    start_usb_connection();

    // Set sys clock
    set_sys_clock_pll(VCO_FREQ * MHZ, POST_DEV1, POST_DEV2);

    // Print all frequency
    measure_freqs();

    printf("Pio clock frequency: %.2f Hz\n", (float)PIO_FREQ);

    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

    // Add PIO program to PIO instruction memory, SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, &bdm_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(PIO_FREQ);
    printf("Pio clock divider: %.2f\n", div);

    // Initialize the program using the helper function in our .pio file
    bdm_program_init(pio, sm, offset, DATA_PIN, CLK_PIN, div, SHIFT_RIGHT, AUTO_PULL, NUM_BITS);

    uint data[DATA_LENGTH] = {
        0x55,       // 01010101
        0x08,       // 00001000
        0x7F        // 01111111
    };

    // Put data in TX FIFO. If shift_right is disabled, align data to the left.
    for(int i = 0; i< DATA_LENGTH; i++)
    {
        uint d = data[i];

        if(!SHIFT_RIGHT)
        {
            uint shift = REG_WIDTH - NUM_BITS;
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