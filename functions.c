#include "functions.h"


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

        case 'a': return READ_A; break;
        case 's': return READ_CCR; break;
        case 'd': return READ_PC; break;
        case 'f': return READ_HX; break;
        case 'g': return READ_SP; break;
        case 'h': return READ_NEXT; break;
        case 'j': return READ_NEXT_WS; break;

        case 'z': return WRITE_A; break;
        case 'x': return WRITE_CCR; break;
        case 'c': return WRITE_PC; break;
        case 'v': return WRITE_HX; break;
        case 'b': return WRITE_SP; break;
        case 'n': return WRITE_NEXT; break;
        case 'm': return WRITE_NEXT_WS; break;

        case '1': return READ_STATUS; break;
        case '2': return WRITE_CONTROL; break;
        case '3': return READ_BYTE; break;
        case '4': return READ_BYTE_WS; break;
        case '5': return READ_LAST; break;
        case '6': return WRITE_BYTE; break;
        case '7': return WRITE_BYTE_WS; break;
        case '8': return READ_BKPT; break;
        case '9': return WRITE_BKPT; break;

        default: return "null"; break;
    }
}