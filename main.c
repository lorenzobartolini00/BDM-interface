#include "functions.h"
#include "bdm-out.pio.h"

#include "config.h"
#include "commands.h"

int main(){
    // Start usb
    start_usb_connection();
    /*

    char str[] = ACK_ENABLE;

    uint num_comm = count_commands(str, '/');

    char *token = NULL;

    // Establish string and get the first token:
    token = strtok(str, "/");

    // While there are tokens in "str"
    while ((token != NULL))
    {
        printf(" %s\n", token);
        token = strtok(NULL, "/");
    }

    */

    // Set sys clock to 64MHz
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
    uint offset = pio_add_program(pio, &bdm_out_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(PIO_FREQ);
    printf("Pio clock divider: %.2f\n", div);

    // Initialize the program using the helper function in our .pio file
    bdm_out_program_init(pio, sm, offset, DATA_PIN, div, SHIFT_RIGHT, AUTO_PULL, NUM_BITS);

    
    uint data[DATA_LENGTH] = {
        0x55,       // 01010101
        0x08,       // 00001000
        0x7F        
    };

    fill_tx_fifo(pio, sm, data, DATA_LENGTH);

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