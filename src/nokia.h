#include <stdbool.h>

#define NOKIA_SCREEN_V_SEGMENTS 6;

void TM4C123_SSI_Init(void);
void Nokia_InitDisplay(void);
void Nokia_Write(char data, bool cmd);
void Nokia_WriteImg(char img[84][6]);
void Nokia_ClearScreen(void);