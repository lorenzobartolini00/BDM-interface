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

//------------------------------------------------------------

    // Ask the user a character to insert, in order to determine what command to exceute
    char user_input;

    while(true)
    {
        printf("Enter a command\n");
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
            // Print command code
            printf("%d - %s) ", token_count+1, token);

            if(strcmp(token, "null") != 0)
            {
                // BDM command tx
                if (token_count == 0)
                {
                    uint comm_hex = convert_to_hex(token);

                    pio_data_out(pio, sm, comm_hex, PIO_FREQ);

                    // Debug
                    printf("Command: %s\n", token);
                }

                // Delay
                else if(strcmp(token, "d") == 0)
                {
                    // Debug
                    printf("delay 16 target BDC clock cycles\n");
                }
                // Read 8 bits
                else if(strcmp(token, "RD") == 0)
                {
                    // Debug
                    printf("8 bits of read data in the target-to-host direction\n");
                }
                // Write 8 bits
                else if(strcmp(token, "WD") == 0)
                {
                    // Debug
                    printf("8 bits of write data in the host-to-target direction\n");
                }
                // Read 16 bits
                else if(strcmp(token, "RD16") == 0)
                {
                    // Debug
                    printf("16 bits of read data in the target-to-host direction\n");
                }
                // Write 16 bits
                else if(strcmp(token, "WD16") == 0)
                {
                    // Debug
                    printf("16 bits of write data in the host-to-target direction\n");
                }
                // Address
                else if(strcmp(token, "AAAA") == 0)
                {
                    // Debug
                    printf("a 16-bit address in the host-to-target direction\n");
                }
                // Read STATUS
                else if(strcmp(token, "SS") == 0)
                {
                    // Debug
                    printf("the contents of BDCSCR in the target-to-host direction\n");
                }
                // Write control
                else if(strcmp(token, "CC") == 0)
                {
                    // Debug
                    printf(" 8 bits of write data for BDCSCR in the host-to-target direction\n");
                }
                // Read RBKP
                else if(strcmp(token, "RBKP") == 0)
                {
                    // Debug
                    printf("16 bits of read data in the target-to-host direction (from BDCBKPT breakpoint register)\n");
                }
                // Write WBKP
                else if(strcmp(token, "WBKP") == 0)
                {
                    // Debug
                    printf("16 bits of write data in the host-to-target direction (for BDCBKPT breakpoint register)\n");
                }

                // Acquire next token
                token = strtok(NULL, "/");
                
                // Increase token_count to keep track of the command queue
                token_count++;

                // Sleep 500 ms for extra delay between each command
                sleep_ms(500);
            }
            else
            {
                printf("Command not found\n");
                break;
            }
            
        }
    }

    return 0;
}