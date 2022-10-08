#include "pio_functions.h"
#include "bdm-data.pio.h"
#include "bdm-sync.pio.h"
#include "hardware/claim.h"

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
    // Put data in TX FIFO. 
    for(int i = 0; i< length; i++)
    {
        put_tx_fifo(pio, sm, data[i], bit, shift_right);
    }
}


void put_tx_fifo(PIO pio, uint sm, uint data, uint bit, bool shift_right)
{
    // If shift_right is disabled, align data to the left.
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
    // When any operation ends, some data are transferred to rx fifo
    // (dummy bits in 'delay' and 'tx' operations, actual data in 'rx' operation)
    while(pio_sm_is_rx_fifo_empty(pio, sm));
}


uint pio_init(PIO pio, uint sm, const struct pio_program *pio_prog)
{
    // Stop running PIO program in the state machine
    pio_sm_set_enabled(pio, sm, false);

    // Clear instruction memory
    pio_clear_instruction_memory(pio);

    // Add PIO program to PIO instruction memory, SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, pio_prog);

    // Clear eventual data in rx and tx fifos
    pio_sm_clear_fifos(pio, sm);

    return offset;
}

void pio_add_instr(PIO pio, uint instr, uint offset)
{
    uint32_t save = hw_claim_lock();
    pio->instr_mem[offset] = instr;
    hw_claim_unlock(save);
}

// Actual commands---------------------------------------------------------------

// Data command
void do_bdm_command(PIO pio, uint sm, uint data, uint tx_bit, uint rx_bit, float pio_freq)
{
    // Clear memory and fifos and add program
    uint offset = pio_init(pio, sm, &bdm_data_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(pio_freq);

    // Initialize the program using the helper function in our .pio file
    bdm_data_program_init(pio, sm, offset, DATA_PIN, div, SHIFT_RIGHT, AUTO_PULL, AUTO_PUSH, tx_bit, rx_bit);

    // Put data in tx fifo:
    put_tx_fifo(pio, sm, data, tx_bit, SHIFT_RIGHT);

    // Set scratch x register by change pio instruction memory
    uint instr = pio_encode_set(pio_x, rx_bit);
    pio_add_instr(pio, instr, offset + 0);
    // pio_sm_exec(pio, sm, instr);

    // Start running bdm-data PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);
}

// Sync
float sync(PIO pio, uint sm, float pio_freq)
{
    // Clear memory and fifos and add program
    uint offset = pio_init(pio, sm, &bdm_sync_program);

    // Calculate the PIO clock divider 
    float div = get_pio_clk_div(pio_freq);

    // Initialize the program using the helper function in our .pio file
    bdm_sync_program_init(pio, sm, offset, DATA_PIN, div);

    // Put a dummy 32 bit value in tx fifo
    pio_sm_put_blocking(pio, sm, 0xFFFFFFFF);

    // Start running bdm-delay PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);

    // Wait for the target to set the pin low
    while(pio_sm_get_rx_fifo_level(pio, sm) < 1);
    absolute_time_t start_time = get_absolute_time();

    // Wait for the target to set the pin high
    while(pio_sm_get_rx_fifo_level(pio, sm) < 2);
    absolute_time_t stop_time = get_absolute_time();

    // Get 128*target_period in us
    uint64_t target_128_period_us = absolute_time_diff_us(start_time, stop_time);
    // Get target_period in us
    float target_period_us = (float)target_128_period_us/128;
    // Target_freq is in HZ
    float target_freq = (1/target_period_us)*1000000;

    // Debug
    printf("Measured frequency: %.2f Hz\n", target_freq);

    return target_freq;
}