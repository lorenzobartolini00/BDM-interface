#include "functions.h"
#include "pio_functions.h"


int main(){
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

    // Set GPIO to be pulled up
    gpio_pull_up(DATA_PIN);

//------------------------------------------------------------
    while(true)
    {
        printf("Enter a command: \n");
        char user_input[BUFFER_LENGTH + 1];

        // Ask the user an input string, in order to determine what command to exceute
        get_string(user_input);

        char *token;
        char *next_token;
        uint token_count = 0;

        // Establish string and get the first token:
        token = strtok_r(user_input, ":", &next_token);

        // Command code
        uint comm_hex = convert_to_hex(token);

        // Whether to perform a delay operation
        bool do_delay = is_delay_present(comm_hex);

        // While there are tokens in "commands_string"
        while ((token != NULL))
        {
            if (token_count == 0)
            {
                // Check if the command is valid
                if (is_command_valid(comm_hex))
                {
                    // Transmit command
                    pio_data_out(pio, sm, comm_hex, PIO_FREQ, COMMAND_BITS);

                    // Debug
                    printf("Command: %s\n", token);

                    wait_end_operation(pio, sm);

                    // Debug
                    printf("Empty\n");
                }
                else
                {
                    // Debug
                    printf("Command not found\n");
                    break;
                }
            }
            else
            {
                // Count how many bit in incoming data
                uint nibble = strlen(token);
                uint bit = nibble * 4;

                // Check whether is input data or output data
                // If first char is '?', then is input data, otherwise is output
                if (token[0] == '?' && is_input_data_valid(token))
                {
                    if(do_delay)
                    {
                        // Sleep 500 ms for extra delay between each command
                        sleep_ms(500);

                        // Only one delay per command. After delay takes place, set do_delay flag to false
                        do_delay = false;

                        delay(pio, sm, PIO_FREQ, DELAY_CYCLES);

                        wait_end_operation(pio, sm);
                    }
                    // Debug
                    printf("Read %d bit\n", bit);
                }
                else if(is_output_data_valid(token))
                {
                    // Transmit data
                    uint data = convert_to_hex(token);
                    pio_data_out(pio, sm, data, PIO_FREQ, bit);

                    // Debug
                    printf("Write %d bit: %d\n", bit, data);

                    wait_end_operation(pio, sm);

                    // Debug
                    printf("Empty\n");

                    if(do_delay)
                    {
                        // Sleep 500 ms for extra delay between each command
                        sleep_ms(500);

                        // Only one delay per command. After delay takes place, set do_delay flag to false
                        do_delay = false;

                        delay(pio, sm, PIO_FREQ, DELAY_CYCLES);

                        wait_end_operation(pio, sm);
                    }
                }
                // If data is invalid, exit from cycle
                else
                {
                    break;
                }
            }

            // Acquire next token
            token = strtok_r(NULL, ":", &next_token);
            
            // Increase token_count to keep track of the command queue
            token_count++;

            // Sleep 500 ms for extra delay between each command
            sleep_ms(500);
        }
    }

    return 0;
}