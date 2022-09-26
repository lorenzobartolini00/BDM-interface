#include "functions.h"


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


float get_pio_clk_div(float desired_pio_freq)
{
    return (float)clock_get_hz(clk_sys) / desired_pio_freq;
}

void start_usb_connection(void)
{
    // Initialise I/O
    stdio_init_all();

    // Explicitly initialize USB stdio and add it to the current set of stdin drivers.
    // Wait for a CDC connection from the host before returning
    if(stdio_usb_init())
    {
        sleep_ms(5000);
        printf("CDC connection enstablished!\n");
    }
}

void fill_tx_fifo(PIO pio, uint sm, uint *data, uint length)
{
    // Put data in TX FIFO. If shift_right is disabled, align data to the left.
    for(int i = 0; i< length; i++)
    {
        put_tx_fifo(pio, sm, data[i]);
    }

    // Debug
    printf("TX FIFO filled\n");
}


void put_tx_fifo(PIO pio, uint sm, uint data)
{
    if(!SHIFT_RIGHT)
    {
        uint shift = REG_WIDTH - NUM_BITS;
        data = data << shift;
    }

    pio_sm_put_blocking(pio, sm, data);
}


uint count_commands(char *command_str, char delimiter)
{
    int counter = 0;

    for(int i = 0; i < strlen(command_str); i++)
    {
        if(command_str[i] == delimiter)
        {
            counter++;
        }
    }

    return counter + 1;
}


uint convert_to_hex(char *str)
{
    return (uint)strtol(str, NULL, 16); 
}

char* get_command_string(char input)
{
    switch(input)
    {
        case 'q': return ACK_ENABLE; break;
        case 'w': return ACK_DISABLED; break;
        case 'e': return BACKGROUND; break;
        case 'r': return GO; break;
        case 't': return TRACE1; break;
        case 'y': return TAGGO; break;

        case 'a': return READ_CCR; break;
        case 's': return READ_PC; break;
        case 'd': return READ_HX; break;
        case 'f': return READ_SP; break;
        case 'g': return READ_NEXT; break;
        case 'h': return READ_NEXT_WS; break;

        default: return "null"; break;
    }
}