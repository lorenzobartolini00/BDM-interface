#define NUM_BITS        8
#define SHIFT_RIGHT     false
#define AUTO_PULL       false
#define REG_WIDTH       32

#define DATA_PIN        5  

//                   REF     FBDIV VCO       POSTDIV
// PLL SYS: 12 / 1 = 12MHz * 128 = 1536 / 6 / 2 = 128MHz    (Final)
// PLL SYS: 12 / 1 = 12MHz * 48 = 768 / 6 / 6 = 21.3MHz     (Test)
#define VCO_FREQ        768  
#define POST_DEV1       6
#define POST_DEV2       6

// Final freq: 16MHZ
// Test freq: 400Hz
#define PIO_FREQ        400   //Hz