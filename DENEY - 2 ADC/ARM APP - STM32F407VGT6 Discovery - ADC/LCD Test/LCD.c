/*
	Karakter LCD K�t�phanesi
	H�seyin Cemre Yilmaz
	hcemreyilmaz@gmail.com
    
    D�zenleyen
    Bahadir Aydinoglu  19.02.19
    bahadirayd@gmail.com
	
	Kullanim	: CubeMX ile proje olustururken 6 adet GPIO pini
							asagidaki gibi tanimlanmalidir.
							
							No.		| Pin Name| Type	| Alt. Func.	| Label
							----------------------------------------------
							xx		| Pxx			| I/O		| GPIO_Output | LCD_RS
							xx		| Pxx			| I/O		| GPIO_Output | LCD_EN
							xx		| Pxx			| I/O		| GPIO_Output | LCD_D4
							xx		| Pxx			| I/O		| GPIO_Output | LCD_D5
							xx		| Pxx			| I/O		| GPIO_Output | LCD_D6
							xx		| Pxx			| I/O		| GPIO_Output | LCD_D7
							
							LCD Shield i�in --->	LCD_RS -> PA9
																		LCD_EN -> PC7
																		LCD_D4 ->	PB5
																		LCD_D5 -> PB4
																		LCD_D6 -> PB10
																		LCD_D8 ->	PA8
																		
																		
							Pin ve port farketmeksizin, Output olarak tanimlanan pinler
							yukaridaki gibi isimlendirilmeli ve baglanti gerektigi gibi
							yapilmalidir.						
							
							Init �rnegi:							
							lcd_init(_LCD_4BIT, _LCD_FONT_5x8, _LCD_2LINE);
							
							4-Bit, 5x8 Font'lu, 2 satirli display kullanilacagi belirtiliyor.
							Farkli varyasyonlara LCD.h dosyasindan bakabilirsiniz.
                            
                            Kontrast ayari i�in pot kullanmak istemiyorsaniz baglantiniz asagidaki gibi olmali
                            
                            VDD-------------LCD VDD
                            |
                            10K
                            |                            
                            |---------------LCD VEE
                            |
                            1K
                            |
                            GND-------------LCD VSS
*/

#include "stm32f4xx_hal.h"
#include "main.h"
#include "LCD.h"
	
void Delay(uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
void lcd_delay(void)
{
	Delay(400);
}

/*	GECİKME GEREKSİNİMLERİ (HCLK 168 MHz için)
 *
 * 	Clock Döngü Süresi	: 1 / fHCLK = 5.95ns
 * 	For Döngü Süresi	: Ortalama 4 clock = 23.8ns
 * 	lcd_delay_us() Döngü Sayısı Hesabı: 1us gecikme için gereken döngü sayısı 1us/23.8ns = 42
 *
 *
 *	1. Enable Pulse		: Minimum 450ns
 *	2. Normal Komutlar	: 37us
 *	3. Uzun Komutlar	: 1.52ms
 *
 */

void lcd_delay_us(uint32_t delay_us)
{
    uint32_t i;
    // Her bir mikro saniye için 42 döngü
    for (i = 0; i < (delay_us * 42); i++)
    {
        __asm__("nop"); // Assembly seviyesinde 'no operation' komutu eklenir (optimizasyonu önler)
    }
}

void lcd_cmd(char cmd)
{
    // RS = 0 (Komut modu)
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);

    // EN = 0 (Hazırlık)
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

    // İlk nibble gönder (Yüksek nibble)
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (cmd & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (cmd & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (cmd & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (cmd & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // EN = 1 -> 0 (Komut Latch)
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // Minimum 450ns gecikme
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

    // İkinci nibble gönder (Düşük nibble)
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (cmd & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (cmd & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (cmd & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (cmd & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // EN = 1 -> 0 (Komut Latch)
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // Minimum 450ns gecikme
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

    // Komut sonrası gecikme
    HAL_Delay(2); // LCD'nin komutu işlemesi için süre
}


void lcd_char_cp(char out_char)
{
    // RS pini set et
    LCD_RS_GPIO_Port->ODR |= LCD_RS_Pin;

    // Tüm LCD pinlerini sıfırla
    LCD_EN_GPIO_Port->BSRR = (uint32_t)LCD_EN_Pin << 16;
    LCD_D4_GPIO_Port->BSRR = (uint32_t)LCD_D4_Pin << 16;
    LCD_D5_GPIO_Port->BSRR = (uint32_t)LCD_D5_Pin << 16;
    LCD_D6_GPIO_Port->BSRR = (uint32_t)LCD_D6_Pin << 16;
    LCD_D7_GPIO_Port->BSRR = (uint32_t)LCD_D7_Pin << 16;

    //lcd_delay();
    lcd_delay_us(50);

    // EN pini set et
    LCD_EN_GPIO_Port->ODR |= LCD_EN_Pin;

    // İlk nibble için bit kontrolü ve pin ayarı
    if ((out_char & 0x10) >> 4) LCD_D4_GPIO_Port->BSRR = LCD_D4_Pin; else LCD_D4_GPIO_Port->BSRR = (uint32_t)LCD_D4_Pin << 16;
    if ((out_char & 0x20) >> 5) LCD_D5_GPIO_Port->BSRR = LCD_D5_Pin; else LCD_D5_GPIO_Port->BSRR = (uint32_t)LCD_D5_Pin << 16;
    if ((out_char & 0x40) >> 6) LCD_D6_GPIO_Port->BSRR = LCD_D6_Pin; else LCD_D6_GPIO_Port->BSRR = (uint32_t)LCD_D6_Pin << 16;
    if ((out_char & 0x80) >> 7) LCD_D7_GPIO_Port->BSRR = LCD_D7_Pin; else LCD_D7_GPIO_Port->BSRR = (uint32_t)LCD_D7_Pin << 16;

    //lcd_delay();
    lcd_delay_us(50);

    // EN ve LCD pinlerini sıfırla
    LCD_EN_GPIO_Port->BSRR = (uint32_t)LCD_EN_Pin << 16;
    LCD_D4_GPIO_Port->BSRR = (uint32_t)LCD_D4_Pin << 16;
    LCD_D5_GPIO_Port->BSRR = (uint32_t)LCD_D5_Pin << 16;
    LCD_D6_GPIO_Port->BSRR = (uint32_t)LCD_D6_Pin << 16;
    LCD_D7_GPIO_Port->BSRR = (uint32_t)LCD_D7_Pin << 16;

    //lcd_delay();
    lcd_delay_us(50);

    // EN pini set et
    LCD_EN_GPIO_Port->ODR |= LCD_EN_Pin;

    // İkinci nibble için bit kontrolü ve pin ayarı
    if (out_char & 0x01) LCD_D4_GPIO_Port->BSRR = LCD_D4_Pin; else LCD_D4_GPIO_Port->BSRR = (uint32_t)LCD_D4_Pin << 16;
    if ((out_char & 0x02) >> 1) LCD_D5_GPIO_Port->BSRR = LCD_D5_Pin; else LCD_D5_GPIO_Port->BSRR = (uint32_t)LCD_D5_Pin << 16;
    if ((out_char & 0x04) >> 2) LCD_D6_GPIO_Port->BSRR = LCD_D6_Pin; else LCD_D6_GPIO_Port->BSRR = (uint32_t)LCD_D6_Pin << 16;
    if ((out_char & 0x08) >> 3) LCD_D7_GPIO_Port->BSRR = LCD_D7_Pin; else LCD_D7_GPIO_Port->BSRR = (uint32_t)LCD_D7_Pin << 16;

    //lcd_delay();
    lcd_delay_us(50);

    // EN ve LCD pinlerini sıfırla
    LCD_EN_GPIO_Port->BSRR = (uint32_t)LCD_EN_Pin << 16;
    LCD_D4_GPIO_Port->BSRR = (uint32_t)LCD_D4_Pin << 16;
    LCD_D5_GPIO_Port->BSRR = (uint32_t)LCD_D5_Pin << 16;
    LCD_D6_GPIO_Port->BSRR = (uint32_t)LCD_D6_Pin << 16;
    LCD_D7_GPIO_Port->BSRR = (uint32_t)LCD_D7_Pin << 16;
}


void lcd_out_cp(char *out_char)
{
	while(*out_char)
	{
		lcd_char_cp(*out_char++);
        //lcd_delay();
        lcd_delay_us(50);
	}
	
}


void lcd_init(char bits, char font, char lines)
{		
    /*HAL_Delay(250);
		lcd_cmd(_RETURN_HOME);
    HAL_Delay(50);
    lcd_cmd(0x20 | bits | font | lines);
    HAL_Delay(50);
    lcd_cmd(_LCD_INIT);
    HAL_Delay(50);
    lcd_cmd(0x0E);
    HAL_Delay(50);
    lcd_cmd(0x0C);
    HAL_Delay(50);
    lcd_cmd(0x01);
    HAL_Delay(100);*/
	HAL_Delay(250);         // İlk gecikme
	lcd_cmd(0x02);          // Return Home (Ekranı başlangıç konumuna getir)
	HAL_Delay(5);
	lcd_cmd(0x28);          // 4-bit mod, 2 satır, 5x8 font
	HAL_Delay(5);
	lcd_cmd(0x06);          // İmleç yönlendirme (Auto increment)
	HAL_Delay(5);
	lcd_cmd(0x0C);          // Display açık, imleç kapalı
	HAL_Delay(5);
	lcd_cmd(0x01);          // Clear Display
	HAL_Delay(5);

}

void lcd_gotoxy(unsigned char row, unsigned char column)
{	
	if(row == 1)
	{
		lcd_cmd(0x80 + (column - 1));
	}
	else if(row == 2)
	{
		lcd_cmd(0xC0 + (column - 1));
	} 
    HAL_Delay(5);    
    
}

void lcd_char(unsigned char row, unsigned char column, char out_char)
{
	lcd_gotoxy(row, column);
	lcd_char_cp(out_char);
}

void lcd_print(unsigned char row, unsigned char column, char *out_char)
{
	lcd_gotoxy(row, column);
	lcd_out_cp(out_char);  
    HAL_Delay(5);      
}
void lcd_clear(void) {
    
	lcd_cmd(_CLEAR);
    HAL_Delay(5);
    
}
void lcd_line1(void) {
	lcd_cmd(0x80);
}

void lcd_line2(void) {
	lcd_cmd(0xC0);
}
