#include <stdbool.h>

void TM4C123_SSI_Init(void);
void Nokia_InitDisplay(void);
void Nokia_Write(char data, bool cmd);
void Nokia_WriteImg(char img[84][6]);
void Nokia_ClearScreen(void);