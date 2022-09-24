#define DATA_LENGTH     3
#define NUM_BITS        8
#define SHIFT_RIGHT     false
#define AUTO_PULL       false
#define REG_WIDTH       32

#define DATA_PIN        25        
#define CLK_PIN         5  

//                   REF     FBDIV VCO       POSTDIV
// PLL SYS: 12 / 1 = 12MHz * 128 = 1536 / 6 / 2 = 128MHz
// PLL SYS: 12 / 1 = 12MHz * 64 = 768 / 4 / 4 = 64MHz
#define VCO_FREQ        768  
#define POST_DEV1       4
#define POST_DEV2       4

#define PIO_FREQ        1000   //Hz