#define DATA_LENGTH     3
#define NUM_BITS        8
#define SHIFT_RIGHT     false
#define AUTO_PULL       false
#define REG_WIDTH       32

#define DATA_PIN        5  

//                   REF     FBDIV VCO       POSTDIV
// PLL SYS: 12 / 1 = 12MHz * 128 = 1536 / 6 / 2 = 128MHz
// PLL SYS: 12 / 1 = 12MHz * 64 = 768 / 4 / 4 = 64MHz
// PLL SYS: 12 / 1 = 12MHz * 48 = 768 / 6 / 6 = 21.3MHz
#define VCO_FREQ        768  
#define POST_DEV1       6
#define POST_DEV2       6

#define PIO_FREQ        400   //Hz