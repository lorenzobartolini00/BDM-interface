#include "functions.h"
#include "bdm-out.pio.h"

#include "config.h"
#include "commands.h"

int main(){
    // 

//------------------------Initialization----------------------
    // Start usb
    start_usb_connection();

    // Set sys clock to 64MHz
    set_sys_clock_pll(VCO_FREQ * MHZ, POST_DEV1, POST_DEV2);

    // Print all frequency
    measure_freqs();

    printf("Pio clock frequency: %.2f Hz\n", (float)PIO_FREQ);

    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

//------------------------------------------------------------

    // Ask the user a character to insert, in order to determine what command to exceute
    char user_input;

    while(true)
    {
        printf("Enter a character\n");
        scanf("%c", &user_input);

        // Get command string depending on what character the user has typed
        char *command_string = get_command_string(user_input);
        char str[15];
        strcpy(str, command_string);

        char *token;

        // Establish string and get the first token:
        token = strtok(str, "/");
        uint token_count = 0;

        // While there are tokens in "commands_string"
        while ((token != NULL))
        {
            if (token_count == 0)
            {
                uint comm_hex = convert_to_hex(token);

                // Add bdm-out PIO program to PIO instruction memory, SDK will find location and
                // return with the memory offset of the program.
                uint offset = pio_add_program(pio, &bdm_out_program);

                // Calculate the PIO clock divider 
                float div = get_pio_clk_div(PIO_FREQ);
                printf("Pio clock divider: %.2f\n", div);

                // Initialize the program using the helper function in our .pio file
                bdm_out_program_init(pio, sm, offset, DATA_PIN, div, SHIFT_RIGHT, AUTO_PULL, NUM_BITS);

                // Put data in tx fifo
                put_tx_fifo(pio, sm, comm_hex);

                // Start running bdm-out PIO program in the state machine
                pio_sm_set_enabled(pio, sm, true);
            }
            else if(strcmp(token, "d"))
            {
                uint delay = 0xFF;

                // Put data in tx fifo
                put_tx_fifo(pio, sm, delay);
            }

            token_count++;
            
            printf(" %s\n", token);
            token = strtok(NULL, "/");

            sleep_ms(1000);
        }
    }

    return 0;
}