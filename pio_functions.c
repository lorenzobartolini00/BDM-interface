#include "pio_functions.h"
#include "bdm-out.pio.h"
#include "bdm-in.pio.h"
#include "bdm-delay.pio.h"

// Utils------------------------------------------------------
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


void fill_tx_fifo(PIO pio, uint sm, uint *data, uint length, uint bit, bool shift_right)
{
    // Put data in TX FIFO. If shift_right is disabled, align data to the left.
    for(int i = 0; i< length; i++)
    {
        put_tx_fifo(pio, sm, data[i], bit, shift_right);
    }

    // Debug
    printf("TX FIFO filled\n");
}


void put_tx_fifo(PIO pio, uint sm, uint data, uint bit, bool shift_right)
{
    if(!shift_right)
    {
        uint shift = REG_WIDTH - bit;
        data = data << shift;
    }

    pio_sm_put_blocking(pio, sm, data);
}


void wait_end_operation(PIO pio, uint sm)
{
    // Wait for an operation to complete. 
    // When any of the operations end, some data are transferred to rx fifo
    // (dummy bits in 'delay' and 'tx' operations, actual data in 'rx' operation)
    while(pio_sm_is_rx_fifo_empty(pio, sm));
}


uint pio_init(PIO pio, uint sm, const struct pio_program *pio_prog)
{
    // Stop running PIO program in the state machine
    pio_sm_set_enabled(pio, sm, false);

    // Clear instruction memory
    pio_clear_instruction_memory(pio);

    // Add bdm-delay PIO program to PIO instruction memory, SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, pio_prog);

    // Clear eventual data in rx and tx fifos
    pio_sm_clear_fifos(pio, sm);

    return offset;
}

// Actual commands---------------------------------------------------------------

// Tx command
void pio_data_out(PIO pio, uint sm, uint data, float pio_freq, uint num_bit)
{
    // Put in tx fifo num_bit(8 or 16) to read
    uint offset = pio_init(pio, sm, &bdm_out_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(pio_freq);

    // Initialize the program using the helper function in our .pio file
    bdm_out_program_init(pio, sm, offset, DATA_PIN, div, SHIFT_RIGHT, AUTO_PULL, num_bit);

    // Put data in tx fifo
    put_tx_fifo(pio, sm, data, num_bit, SHIFT_RIGHT);

    // Start running bdm-out PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);
}

// Rx command
void pio_data_in(PIO pio, uint sm, float pio_freq, uint num_bit)
{
    // Clear memory and fifos and add program
    uint offset = pio_init(pio, sm, &bdm_in_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(pio_freq);

    // Initialize the program using the helper function in our .pio file
    bdm_in_program_init(pio, sm, offset, DATA_PIN, div, SHIFT_RIGHT, AUTO_PUSH, num_bit);

    // Put num_bit in tx fifo
    pio_sm_put_blocking(pio, sm, num_bit - 1);

    // Start running bdm-out PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);
}

// Delay
void delay(PIO pio, uint sm, float pio_freq, uint cycles)
{
    // See bdm-delay.pio for reference
    cycles = cycles > 2 ? (cycles -3) : cycles;

    // Clear memory and fifos and add program
    uint offset = pio_init(pio, sm, &bdm_delay_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(pio_freq);

    // Initialize the program using the helper function in our .pio file
    bdm_delay_program_init(pio, sm, offset, div);

    // Put number of cycles in tx fifo
    pio_sm_put_blocking(pio, sm, cycles);

    // Start running bdm-delay PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);
}