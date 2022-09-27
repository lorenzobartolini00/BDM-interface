#include "functions.h"
#include "config.h"


uint count_commands(char *command_str, char delimiter)
{
    return count_char(command_str, delimiter);
}


uint count_char(char *str, char c)
{
    int counter = 0;

    for(int i = 0; i < strlen(str); i++)
    {
        if(str[i] == c)
        {
            counter++;
        }
    }

    return counter;
}


bool is_input_data_valid(char *str)
{
    // Check if data is 8 or 16 bit
    uint nibble = strlen(str);

    if(nibble != 2 && nibble != 4)
    {
        return false;
    }

    // Check if input data does not contains invalid chars
    for(int j = 0; j < strlen(str); j++)
    {
        if(str[j] != '?')
        {
            printf("Invalid input\n");
            return false;
        }
    }

    return true;
}

bool is_output_data_valid(char *str)
{
    // Check if data is 8 or 16 bit
    uint nibble = strlen(str);

    if(nibble != 2 && nibble != 4)
    {
        return false;
    }

    // Check if output data does not contains invalid chars
    for(int j = 0; j < strlen(str); j++)
    {
        if(str[j] == '?')
        {
            printf("Invalid input\n");
            return false;
        }
    }

    return true;
}


uint convert_to_hex(char *str)
{
    return (uint)strtol(str, NULL, 16); 
}


bool is_command_valid(uint command)
{
    switch(command)
    {
        case 0xD5: return true; break;
        case 0xD6: return true; break;
        case 0x90: return true; break;
        case 0x08: return true; break;
        case 0x10: return true; break;
        case 0x18: return true; break;

        case 0x68: return true; break;
        case 0x69: return true; break;
        case 0x6B: return true; break;
        case 0x6C: return true; break;
        case 0x6F: return true; break;
        case 0x70: return true; break;
        case 0x71: return true; break;

        case 0x48: return true; break;
        case 0x49: return true; break;
        case 0x4B: return true; break;
        case 0x4C: return true; break;
        case 0x4F: return true; break;
        case 0x50: return true; break;
        case 0x51: return true; break;

        case 0xE4: return true; break;
        case 0xC4: return true; break;
        case 0xE0: return true; break;
        case 0xE1: return true; break;
        case 0xE8: return true; break;
        case 0xC0: return true; break;
        case 0xC1: return true; break;
        case 0xE2: return true; break;
        case 0xC2: return true; break;

        default: return false; break;
    }
}


bool is_delay_present(uint command)
{
    switch(command)
    {
        case 0xD5: return true; break;
        case 0xD6: return true; break;
        case 0x90: return true; break;
        case 0x08: return true; break;
        case 0x10: return true; break;
        case 0x18: return true; break;

        case 0x68: return true; break;
        case 0x69: return true; break;
        case 0x6B: return true; break;
        case 0x6C: return true; break;
        case 0x6F: return true; break;
        case 0x70: return true; break;
        case 0x71: return true; break;

        case 0x48: return true; break;
        case 0x49: return true; break;
        case 0x4B: return true; break;
        case 0x4C: return true; break;
        case 0x4F: return true; break;
        case 0x50: return true; break;
        case 0x51: return true; break;

        case 0xE4: return false; break;
        case 0xC4: return false; break;
        case 0xE0: return true; break;
        case 0xE1: return true; break;
        case 0xE8: return false; break;
        case 0xC0: return true; break;
        case 0xC1: return true; break;
        case 0xE2: return false; break;
        case 0xC2: return false; break;

        default: return false; break;
    }
}


void get_string(char *buffer)
{
    int i;

    char ch;

    scanf("%c", &ch);

    for (i = 0; ((i < BUFFER_LENGTH) && (ch != (char)EOF) && (ch != (char)NEW_LINE)); i++)
    {
        buffer[i] = (char) ch;

        printf("%c", buffer[i]);

        scanf("%c", &ch);
    }

    // Terminate string with a null character
    buffer[i] = '\0';

    printf("\n");
}