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

        // Establish string and get the first token:
        token = strtok_r(user_input, ":", &next_token);

        // Command code
        uint comm_hex = convert_to_hex(token);

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

            uint data[5];      // Contains one byte in each cell; e.g. {E4, FF, FF, 0, 0}. Zero could be real data or dummy data depending on dir array
            uint dir[5];       // 1 for output data, 0 for input data; e.g. {1,1,0,0} means that the first word is output data, while the second is input data

            uint bit_16[4];      // 1 for 16 bit word, 0 for 8 bit word

            uint word_count = 0;
            uint byte_count = 0;

            // Decode command string
            while ((token != NULL))
            {
                uint nibble = strlen(token);
                uint bit = nibble * 4;

                // Enstablish whether is input data or output data
                if(token[0] == '?' && is_input_data_valid(token))
                {
                    data[byte_count] = 0;            // No data to transmit(will be ignored)
                    dir[byte_count] = 0;             // Input value

                    if(bit == 16)
                    {
                        byte_count++;

                        data[byte_count] = 0;            // No data to transmit(will be ignored)
                        dir[byte_count] = 0;             // Input va
                    }

                }
                else if(is_output_data_valid(token))
                {
                    uint b1 = nibble2byte(
                        (uint)hex2int(token[0]),
                        (uint)hex2int(token[1])
                    );

                    printf("Data(l8): %d\n", b1);

                    data[byte_count] = b1;      // Data to transmit(lower 8 bit)
                    dir[byte_count] = 1;        // Output value

                    if(bit == 16)
                    {
                        uint b2 = nibble2byte(
                            (uint)hex2int(token[2]),
                            (uint)hex2int(token[3])
                        );

                        printf("Data(u8): %d\n", b2);

                        data[byte_count] = b2;      // Data to transmit(upper 8 bit)
                        dir[byte_count] = 1;        // Output value
                    }
                }
                // If data is invalid, exit from cycle
                else
                {
                    // Debug
                    printf("Command is Invalid\n");
                    break;
                }

                // Flag if this word of data is 8 or 16 bit
                bit_16[word_count] = (bit == 16) ? 1 : 8;    

                // Debug
                if (word_count == 0)
                {
                    printf("Command: %s\n", token);
                }

                // Acquire next token
                token = strtok_r(NULL, ":", &next_token);
                
                // Increase token_count and byte_count
                word_count++;
                byte_count++;
            }

            // Transmit command/data and receive data
            do_bdm_command(pio, sm, data, array2dec(dir, byte_count, true), byte_count, pio_freq);

            // Wait the end of the operation
            sleep_ms(500);

            int i = 0;
            uint incoming_data[4] = {0, 0, 0, 0};
            // Read data from rx fifo
            while(pio_sm_is_rx_fifo_empty(pio, sm) == false)
            {
                incoming_data[i] = pio_sm_get(pio, sm);
                i++;

                printf("Byte %d: %d\n", i, (int)incoming_data[i]);
            }

            // Increase sync count
            sync_count++;
        }
        else
        {
            // Debug
            printf("Command not found\n");
        }
    }

    return 0;
}