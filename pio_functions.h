#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <string.h>

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "hardware/pio.h"

#include "config.h"
#include "commands.h"

// Define the time(in ms) to wait for a CDC connection to be established.
// This prevent initial program output being lost, at cost of requiring an active CDC connection
#ifndef PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS
#define PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 15000
#endif


// Print every clk sources frequency
void measure_freqs(void);

// Get the division value for pio_freq calculation
float get_pio_clk_div(float desired_freq);

// Init I/O
void start_usb_connection(void);

// Put one word in tx fifo
void put_tx_fifo(PIO pio, uint sm, uint data);

// Put data in TX FIFO
void fill_tx_fifo(PIO pio, uint sm, uint *data, uint length);

// Write bdm-out program to sm, add data to tx FIFO and start program. Clear pio instruction memory first.
void pio_data_out(PIO pio, uint sm, uint data, float pio_freq);