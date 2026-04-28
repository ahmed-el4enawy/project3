#ifndef LCD_H
#define LCD_H

#include "STD_TYPES.h"

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(u8 row, u8 col);
void LCD_Print(const char *str);

#endif /* LCD_H */
