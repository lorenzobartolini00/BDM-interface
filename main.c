#include "functions.h"
#include "pio_functions.h"


int main(){
//------------------------Initialization----------------------
    // Start usb
    start_usb_connection();

    // Set GPIO function to SIO
    gpio_init(LED_PIN );
    gpio_set_dir(LED_PIN, true);

    // Turn on LED when device turn on
    gpio_put(LED_PIN, true);

    // Set sys clock to 64MHz
    set_sys_clock_pll(VCO_FREQ * MHZ, POST_DEV1, POST_DEV2);

    // Print all frequency
    measure_freqs();

    printf("Pio clock frequency: %.2f Hz\n", (float)PIO_FREQ);

    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

    // Turn off LED when it's time to reset MCU
    gpio_put(LED_PIN, false);

    // Keep the line low at the rising edge of MCU reset
    gpio_init(DATA_PIN );
    gpio_set_dir(DATA_PIN, true);
    gpio_put(DATA_PIN, false);

    // Sleep for 5 seconds, while MCU enters Active Background Mode
    sleep_ms(5000);

    gpio_put(DATA_PIN, true);
    gpio_set_dir(DATA_PIN, false);

    // Set GPIO to be pulled up
    gpio_pull_up(DATA_PIN);

    // Set GPIO function to PIO_0
    gpio_set_function(DATA_PIN, GPIO_FUNC_PIO0 );

    // Transmit dummy command
    pio_data_out(pio, sm, 0x00, PIO_FREQ, COMMAND_BITS);
    wait_end_operation(pio, sm);

    // Sleep for 1000 ms to abort dummy command
    sleep_ms(5000);

//------------------------------------------------------------
    // Initial frequency
    float pio_freq = PIO_FREQ;
    uint sync_count = 0;

    while(true)
    {
        // Turn on LED when device is ready
        gpio_put(LED_PIN, true);

        printf("Frequency: %.2f Hz; Enter a command: ", pio_freq);
        char user_input[BUFFER_LENGTH + 1];

        // Ask the user an input string, in order to determine what command to exceute
        get_string(user_input);
        // Turn off LED when device is running a command
        gpio_put(LED_PIN, false);

        char *token;
        char *next_token;
        uint token_count = 0;

        // Establish string and get the first token:
        token = strtok_r(user_input, ":", &next_token);

        // Command code
        uint comm_hex = convert_to_hex(token);

        // When performing a delay operation
        uint delay_position = get_delay_position(comm_hex);

        // Do a SYNC command when SYNC command is entered
        if(is_command_sync(token))
        {
            pio_freq = sync(pio, sm, SYNC_FREQ);
            sync_count = 0;
        }
        // Check if the command is valid
        else if(is_command_valid(comm_hex)) 
        {
            // Do a SYNC command when sync_count reaches SYNC_COUNT_THRESHOLD
            if(sync_count >= SYNC_COUNT_THRESHOLD && AUTO_SYNC)
            {
                pio_freq = sync(pio, sm, SYNC_FREQ);
                sync_count = 0;
            }

            // While there are tokens in "commands_string"
            while ((token != NULL))
            {
                if (token_count == 0)
                {
                    // Transmit command
                    pio_data_out(pio, sm, comm_hex, pio_freq, COMMAND_BITS);

                    // Debug
                    printf("Command: %s\n", token);

                    wait_end_operation(pio, sm);
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
                        // Read data from pin and save to rx fifo
                        uint data;
                        pio_data_in(pio, sm, pio_freq, bit);

                        wait_end_operation(pio, sm);

                        // Read data from rx fifo
                        data = pio_sm_get_blocking(pio, sm);

                        // Debug
                        printf("Read %d bit: %d\n", bit, data);
                    }
                    else if(is_output_data_valid(token))
                    {
                        // Transmit data
                        uint data = convert_to_hex(token);
                        pio_data_out(pio, sm, data, pio_freq, bit);

                        // Debug
                        printf("Write %d bit: %d\n", bit, data);

                        wait_end_operation(pio, sm);
                    }
                    // If data is invalid, exit from cycle
                    else
                    {
                        // Debug
                        printf("Command is Invalid\n");
                        break;
                    }
                }

                // Do a delay if expected
                if(delay_position != -1 && token_count == delay_position)
                {
                    // Debug
                    printf("Delay\n");

                    delay(pio, sm, pio_freq);

                    wait_end_operation(pio, sm);
                }
                // Acquire next token
                token = strtok_r(NULL, ":", &next_token);
                
                // Increase token_count to keep track of the command queue
                token_count++;

                // Increase sync count.
                sync_count++;
            }
        }
        else
        {
            // Debug
            printf("Command not found\n");
        }
    }

    return 0;
}