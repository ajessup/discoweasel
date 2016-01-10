#include <stdbool.h>

#define NOKIA_SCREEN_V_SEGMENTS 6
#define NOKIA_SCREEN_V_SEGMENTS_HEIGHT 8

#define NOKIA_SCREEN_COLS 84
#define NOKIA_SCREEN_ROWS NOKIA_SCREEN_V_SEGMENTS*NOKIA_SCREEN_V_SEGMENTS_HEIGHT

void Nokia_InitDisplay(void);
void Nokia_Write(char data, bool cmd);
void Nokia_WriteImg(bool img[NOKIA_SCREEN_COLS][NOKIA_SCREEN_ROWS]);
void Nokia_ClearScreen(void);