#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <string.h>

#include "commands.h"

// Count how many commands are in a given string
uint count_commands(char *command_str, char delimiter);

// Convert from string to hexadecimal
uint convert_to_hex(char *str);

// Return command string from a given user input char
char *get_command_string(char input);