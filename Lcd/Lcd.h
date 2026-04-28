#ifndef LCD_H
#define LCD_H

#include "Std_Types.h"

void Lcd_Init(void);
void Lcd_Clear(void);
void Lcd_SetCursor(uint8 row, uint8 col);
void Lcd_Print(const char *str);

#endif /* LCD_H */