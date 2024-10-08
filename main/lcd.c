#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "lcd.h"

#define TAG "ST7735S"
#define	_DEBUG_		0

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;
static const int SPI_Frequency = SPI_MASTER_FREQ_20M;
//static const int SPI_Frequency = SPI_MASTER_FREQ_26M;
//static const int SPI_Frequency = SPI_MASTER_FREQ_40M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_80M;


void spi_master_init(ST7735_t * dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET)
{
	esp_err_t ret;

	gpio_reset_pin( GPIO_CS );
	gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_CS, 0 );

	gpio_reset_pin( GPIO_DC );
	gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_DC, 0 );

	gpio_reset_pin( GPIO_RESET );
	gpio_set_direction( GPIO_RESET, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_RESET, 0 );
	vTaskDelay( pdMS_TO_TICKS( 100 ) );
	gpio_set_level( GPIO_RESET, 1 );

	spi_bus_config_t buscfg = {
		.sclk_io_num = GPIO_SCLK,
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = -1,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1
	};

	ret = spi_bus_initialize( SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
	ESP_LOGD(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg={
		.clock_speed_hz = SPI_Frequency,
		.spics_io_num = GPIO_CS,
		.queue_size = 7,
		.flags = SPI_DEVICE_NO_DUMMY,
	};

	spi_device_handle_t handle;
	ret = spi_bus_add_device( SPI2_HOST, &devcfg, &handle);
	ESP_LOGD(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);
	dev->_dc = GPIO_DC;
	dev->_SPIHandle = handle;
}


bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength)
{
	spi_transaction_t SPITransaction;
	esp_err_t ret;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
#if 1
		ret = spi_device_transmit( SPIHandle, &SPITransaction );
#endif
#if 0
		ret = spi_device_polling_transmit( SPIHandle, &SPITransaction );
#endif
		assert(ret==ESP_OK); 
	}

	return true;
}

void LcdSetSendMode(ST7735_t * dev, int mode) {
    gpio_set_level( dev->_dc, mode );
}

bool spi_master_write_command(ST7735_t * dev, uint8_t cmd)
{
	uint8_t Byte = 0;
	Byte = cmd;
	gpio_set_level( dev->_dc, SPI_Command_Mode );
	return spi_master_write_byte( dev->_SPIHandle, &Byte, 1 );
}

bool spi_master_write_data_byte(ST7735_t * dev, uint8_t data)
{
	uint8_t Byte = 0;
	Byte = data;
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, &Byte, 1 );
}


bool spi_master_write_data_word(ST7735_t * dev, uint16_t data, int flag)
{
	uint8_t Byte[2];
	Byte[0] = (data >> 8) & 0xFF;
	Byte[1] = data & 0xFF;
	if (flag) printf("spi_master_write_data_word Byte=%02x %02x\n",Byte[0],Byte[1]);
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, Byte, 2);
}

bool spi_master_write_addr(ST7735_t * dev, uint16_t addr1, uint16_t addr2)
{
		static uint8_t Byte[4];
		Byte[0] = (addr1 >> 8) & 0xFF;
		Byte[1] = addr1 & 0xFF;
		Byte[2] = (addr2 >> 8) & 0xFF;
		Byte[3] = addr2 & 0xFF;
		gpio_set_level( dev->_dc, SPI_Data_Mode );
		return spi_master_write_byte( dev->_SPIHandle, Byte, 4);
}

bool spi_master_write_color(ST7735_t * dev, uint16_t color, uint16_t size)
{
		static uint8_t Byte[1024];
		int index = 0;
		for(int i=0;i<size;i++) {
				Byte[index++] = (color >> 8) & 0xFF;
				Byte[index++] = color & 0xFF;
		}
		gpio_set_level( dev->_dc, SPI_Data_Mode );
		return spi_master_write_byte( dev->_SPIHandle, Byte, size*2);
}

// Add 202001
bool spi_master_write_colors(ST7735_t * dev, uint16_t * colors, uint16_t size)
{
    static uint8_t Byte[1024];
    int index = 0;
    for(int i=0;i<size;i++) {
        Byte[index++] = (colors[i] >> 8) & 0xFF;
        Byte[index++] = colors[i] & 0xFF;
    }
    gpio_set_level( dev->_dc, SPI_Data_Mode );
    return spi_master_write_byte( dev->_SPIHandle, Byte, size*2);
}

void delayMS(int ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
}

void lcdInit(ST7735_t * dev, int width, int height, int offsetx, int offsety)
{
	dev->_width = width;
	dev->_height = height;
	dev->_offsetx = offsetx;
	dev->_offsety = offsety;
	dev->_font_direction = DIRECTION0;
	dev->_font_fill = false;
	dev->_font_underline = false;

	spi_master_write_command(dev, 0x01);	//Software Reset 
	delayMS(150);

	spi_master_write_command(dev, 0x11);	//Sleep Out
	delayMS(255);
	
	spi_master_write_command(dev, 0xB1);	//Frame Rate Control (In normal mode/ Full colors) 
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x2D);
	
	spi_master_write_command(dev, 0xB2);	//Frame Rate Control (In Idle mode/ 8-colors) 
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x2D);

	spi_master_write_command(dev, 0xB3);	//Frame Rate Control (In Partial mode/ full colors) 
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x2D);
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x2D);

	spi_master_write_command(dev, 0xB4);	//Display Inversion Control
	spi_master_write_data_byte(dev, 0x07);

	spi_master_write_command(dev, 0xC0);	//Power Control 1
	spi_master_write_data_byte(dev, 0xA2);
	spi_master_write_data_byte(dev, 0x02);
	spi_master_write_data_byte(dev, 0x84);

	spi_master_write_command(dev, 0xC1);	//Power Control 2
	spi_master_write_data_byte(dev, 0xC5);

	spi_master_write_command(dev, 0xC2);	//Power Control 3 (in Normal mode/ Full colors) 
	spi_master_write_data_byte(dev, 0x0A);
	spi_master_write_data_byte(dev, 0x00);

	spi_master_write_command(dev, 0xC3);	//Power Control 4 (in Idle mode/ 8-colors)
	spi_master_write_data_byte(dev, 0x8A);
	spi_master_write_data_byte(dev, 0x2A);

	spi_master_write_command(dev, 0xC4);	//Power Control 5 (in Partial mode/ full-colors)
	spi_master_write_data_byte(dev, 0x8A);
	spi_master_write_data_byte(dev, 0xEE);

	spi_master_write_command(dev, 0xC5);	//VCOM Control 1
	spi_master_write_data_byte(dev, 0x0E);

	spi_master_write_command(dev, 0x20);	//Display Inversion Off 

	spi_master_write_command(dev, 0x36);	//Memory Data Access Control 
	spi_master_write_data_byte(dev, 0xC8);	//BGR color filter panel
	//spi_master_write_data_byte(dev, 0xC0);	//RGB color filter panel

	spi_master_write_command(dev, 0x3A);	//Interface Pixel Format
	spi_master_write_data_byte(dev, 0x05);	//16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

	spi_master_write_command(dev, 0x2A);	//Column Address Set
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x02);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x81);

	spi_master_write_command(dev, 0x2B);	//Row Address Set
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0xA0);

	// spi_master_write_command(dev, 0x21);	//Display Inversion On

	spi_master_write_command(dev, 0xE0);	//Gamma (‘+’polarity) Correction Characteristics Setting
	spi_master_write_data_byte(dev, 0x02);
	spi_master_write_data_byte(dev, 0x1C);
	spi_master_write_data_byte(dev, 0x07);
	spi_master_write_data_byte(dev, 0x12);
	spi_master_write_data_byte(dev, 0x37);
	spi_master_write_data_byte(dev, 0x32);
	spi_master_write_data_byte(dev, 0x29);
	spi_master_write_data_byte(dev, 0x2D);
	spi_master_write_data_byte(dev, 0x29);
	spi_master_write_data_byte(dev, 0x25);
	spi_master_write_data_byte(dev, 0x2B);
	spi_master_write_data_byte(dev, 0x39);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x01);
	spi_master_write_data_byte(dev, 0x03);
	spi_master_write_data_byte(dev, 0x10);

	spi_master_write_command(dev, 0xE1);	//Gamma ‘-’polarity Correction Characteristics Setting
	spi_master_write_data_byte(dev, 0x03);
	spi_master_write_data_byte(dev, 0x1D);
	spi_master_write_data_byte(dev, 0x07);
	spi_master_write_data_byte(dev, 0x06);
	spi_master_write_data_byte(dev, 0x2E);
	spi_master_write_data_byte(dev, 0x2C);
	spi_master_write_data_byte(dev, 0x29);
	spi_master_write_data_byte(dev, 0x2D);
	spi_master_write_data_byte(dev, 0x2E);
	spi_master_write_data_byte(dev, 0x2E);
	spi_master_write_data_byte(dev, 0x37);
	spi_master_write_data_byte(dev, 0x3F);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x02);
	spi_master_write_data_byte(dev, 0x10);

	spi_master_write_command(dev, 0x13);	//Normal Display Mode On
	delayMS(10);

	spi_master_write_command(dev, 0x29);	//Display On
	delayMS(100);
}


// Draw pixel
// x:X coordinate
// y:Y coordinate
// color:color
void lcdDrawPixel(ST7735_t * dev, uint16_t x, uint16_t y, uint16_t color){
	if (x >= dev->_width) return;
	if (y >= dev->_height) return;

	uint16_t _x = x + dev->_offsetx;
	uint16_t _y = y + dev->_offsety;
	spi_master_write_command(dev, 0x2A);	// set column(x) address
	spi_master_write_addr(dev, _x, _x);
	spi_master_write_command(dev, 0x2B);	// set Page(y) address
	spi_master_write_addr(dev, _y, _y);
	spi_master_write_command(dev, 0x2C);	//	Memory Write
	spi_master_write_data_word(dev, color, 0);
}

// Draw multi pixel
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
void lcdDrawMultiPixels(ST7735_t * dev, uint16_t x, uint16_t y, uint16_t size, uint16_t * colors) {
    if (x+size > dev->_width) return;
    if (y >= dev->_height) return;

    uint16_t _x1 = x + dev->_offsetx;
    uint16_t _x2 = _x1 + (size-1);
    uint16_t _y1 = y + dev->_offsety;
    uint16_t _y2 = _y1;

    spi_master_write_command(dev, 0x2A);    // set column(x) address
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, 0x2B);    // set Page(y) address
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, 0x2C);    //  Memory Write
    spi_master_write_colors(dev, colors, size);
}

// Draw rectangule of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color
void lcdDrawFillRect(ST7735_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
	if (x1 >= dev->_width) return;
	if (x2 >= dev->_width) x2=dev->_width-1;
	if (y1 >= dev->_height) return;
	if (y2 >= dev->_height) y2=dev->_height-1;

	uint16_t _x1 = x1 + dev->_offsetx;
	uint16_t _x2 = x2 + dev->_offsetx;
	uint16_t _y1 = y1 + dev->_offsety;
	uint16_t _y2 = y2 + dev->_offsety;

	spi_master_write_command(dev, 0x2A);	// set column(x) address
	spi_master_write_addr(dev, _x1, _x2);
	spi_master_write_command(dev, 0x2B);	// set Page(y) address
	spi_master_write_addr(dev, _y1, _y2);
	spi_master_write_command(dev, 0x2C);	//	Memory Write

	for(int i=_x1;i<=_x2;i++){
		uint16_t size = _y2-_y1+1;
		spi_master_write_color(dev, color, size);
	}
}

// Display Off
void lcdDisplayOff(ST7735_t * dev) {
	spi_master_write_command(dev, 0x28);	//Display off
}
 
// Display On
void lcdDisplayOn(ST7735_t * dev) {
	spi_master_write_command(dev, 0x29);	//Display on
}

void LcdDrawScreen(ST7735_t *dev, uint16_t *colors, int x, int y, int w, int h) {
	spi_master_write_command(dev, 0x2A);	// set column(x) address
	spi_master_write_addr(dev, x, x + w);
	spi_master_write_command(dev, 0x2B);	// set Page(y) address
	spi_master_write_addr(dev, y, y + h);
	spi_master_write_command(dev, 0x2C);	//	Memory Write

    gpio_set_level( dev->_dc, SPI_Data_Mode );

    size_t max_len = 0;
    spi_bus_get_max_transaction_len(SPI2_HOST, &max_len);
    size_t len = w * h * 2;
    const uint8_t* ptr = (const uint8_t*)colors;
    while(len > 0) {
        size_t len_limit = len > max_len ? max_len : len;
        spi_master_write_byte( dev->_SPIHandle, ptr, len_limit);
        ptr += len_limit;
        len -= len_limit;
    }
}

// Fill screen
// color:color
void lcdFillScreen(ST7735_t * dev, uint16_t color) {
	lcdDrawFillRect(dev, 0, 0, dev->_width-1, dev->_height-1, color);
}

// Draw line
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End X coordinate
// y2:End Y coordinate
// color:color 
void lcdDrawLine(ST7735_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
	int i;
	int dx,dy;
	int sx,sy;
	int E;

	/* distance between two points */
	dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
	dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = ( x2 > x1 ) ? 1 : -1;
	sy = ( y2 > y1 ) ? 1 : -1;

	/* inclination < 1 */
	if ( dx > dy ) {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) {
			lcdDrawPixel(dev, x1, y1, color);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) {
			y1 += sy;
			E -= 2 * dx;
		}
	}

/* inclination >= 1 */
	} else {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) {
			lcdDrawPixel(dev, x1, y1, color);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}

// Draw rectangule
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// color:color
void lcdDrawRect(ST7735_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
	lcdDrawLine(dev, x1, y1, x2, y1, color);
	lcdDrawLine(dev, x2, y1, x2, y2, color);
	lcdDrawLine(dev, x2, y2, x1, y2, color);
	lcdDrawLine(dev, x1, y2, x1, y1, color);
}

// Draw round
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawCircle(ST7735_t * dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
	int x;
	int y;
	int err;
	int old_err;

	x=0;
	y=-r;
	err=2-2*r;
	do{
		lcdDrawPixel(dev, x0-x, y0+y, color); 
		lcdDrawPixel(dev, x0-y, y0-x, color); 
		lcdDrawPixel(dev, x0+x, y0-y, color); 
		lcdDrawPixel(dev, x0+y, y0+x, color); 
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);
}

// Draw round of filling
// x0:Central X coordinate
// y0:Central Y coordinate
// r:radius
// color:color
void lcdDrawFillCircle(ST7735_t * dev, uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
	int x;
	int y;
	int err;
	int old_err;
	int ChangeX;

	x=0;
	y=-r;
	err=2-2*r;
	ChangeX=1;
	do{
		if(ChangeX) {
			lcdDrawLine(dev, x0-x, y0-y, x0-x, y0+y, color);
			lcdDrawLine(dev, x0+x, y0-y, x0+x, y0+y, color);
		} // endif
		ChangeX=(old_err=err)<=x;
		if (ChangeX)			err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;
	} while(y<=0);
} 

// Draw rectangule with round corner
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// r:radius
// color:color
void lcdDrawRoundRect(ST7735_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t r, uint16_t color) {
	int x;
	int y;
	int err;
	int old_err;
	unsigned char temp;

	if(x1>x2) {
		temp=x1; x1=x2; x2=temp;
	} // endif
	  
	if(y1>y2) {
		temp=y1; y1=y2; y2=temp;
	} // endif

	ESP_LOGD(TAG, "x1=%d x2=%d delta=%d r=%d",x1, x2, x2-x1, r);
	ESP_LOGD(TAG, "y1=%d y2=%d delta=%d r=%d",y1, y2, y2-y1, r);
	if (x2-x1 < r) return; // Add 20190517
	if (y2-y1 < r) return; // Add 20190517

	x=0;
	y=-r;
	err=2-2*r;

	do{
		if(x) {
			lcdDrawPixel(dev, x1+r-x, y1+r+y, color); 
			lcdDrawPixel(dev, x2-r+x, y1+r+y, color); 
			lcdDrawPixel(dev, x1+r-x, y2-r-y, color); 
			lcdDrawPixel(dev, x2-r+x, y2-r-y, color);
		} // endif 
		if ((old_err=err)<=x)	err+=++x*2+1;
		if (old_err>y || err>x) err+=++y*2+1;	 
	} while(y<0);

	ESP_LOGD(TAG, "x1+r=%d x2-r=%d",x1+r, x2-r);
	lcdDrawLine(dev, x1+r,y1  ,x2-r,y1	,color);
	lcdDrawLine(dev, x1+r,y2  ,x2-r,y2	,color);
	lcdDrawLine(dev, x1  ,y1+r,x1  ,y2-r, color);
	lcdDrawLine(dev, x2  ,y1+r,x2  ,y2-r, color);  
} 

// Draw arrow
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// w:Width of the botom
// color:color
// Thanks http://k-hiura.cocolog-nifty.com/blog/2010/11/post-2a62.html
void lcdDrawArrow(ST7735_t * dev, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color) {
	double Vx= x1 - x0;
	double Vy= y1 - y0;
	double v = sqrt(Vx*Vx+Vy*Vy);
	//	 printf("v=%f\n",v);
	double Ux= Vx/v;
	double Uy= Vy/v;

	uint16_t L[2],R[2];
	L[0]= x1 - Uy*w - Ux*v;
	L[1]= y1 + Ux*w - Uy*v;
	R[0]= x1 + Uy*w - Ux*v;
	R[1]= y1 - Ux*w - Uy*v;
	//	 printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

	//	 lcdDrawLine(x0,y0,x1,y1,color);
	lcdDrawLine(dev, x1, y1, L[0], L[1], color);
	lcdDrawLine(dev, x1, y1, R[0], R[1], color);
	lcdDrawLine(dev, L[0], L[1], R[0], R[1], color);
}


// Draw arrow of filling
// x1:Start X coordinate
// y1:Start Y coordinate
// x2:End	X coordinate
// y2:End	Y coordinate
// w:Width of the botom
// color:color
void lcdDrawFillArrow(ST7735_t * dev, uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t w,uint16_t color) {
	double Vx= x1 - x0;
	double Vy= y1 - y0;
	double v = sqrt(Vx*Vx+Vy*Vy);
	//	 printf("v=%f\n",v);
	double Ux= Vx/v;
	double Uy= Vy/v;

	uint16_t L[2],R[2];
	L[0]= x1 - Uy*w - Ux*v;
	L[1]= y1 + Ux*w - Uy*v;
	R[0]= x1 + Uy*w - Ux*v;
	R[1]= y1 - Ux*w - Uy*v;
	//	 printf("L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);

	lcdDrawLine(dev, x0, y0, x1, y1, color);
	lcdDrawLine(dev, x1, y1, L[0], L[1], color);
	lcdDrawLine(dev, x1, y1, R[0], R[1], color);
	lcdDrawLine(dev, L[0], L[1], R[0], R[1], color);

	int ww;
	for(ww=w-1;ww>0;ww--) {
		L[0]= x1 - Uy*ww - Ux*v;
		L[1]= y1 + Ux*ww - Uy*v;
		R[0]= x1 + Uy*ww - Ux*v;
		R[1]= y1 - Ux*ww - Uy*v;
		//	   printf("Fill>L=%d-%d R=%d-%d\n",L[0],L[1],R[0],R[1]);
		lcdDrawLine(dev, x1, y1, L[0], L[1], color);
		lcdDrawLine(dev, x1, y1, R[0], R[1], color);
	}
}

// Set font direction
// dir:Direction
void lcdSetFontDirection(ST7735_t * dev, uint16_t dir) {
	dev->_font_direction = dir;
}

// Set font filling
// color:fill color
void lcdSetFontFill(ST7735_t * dev, uint16_t color) {
	dev->_font_fill = true;
	dev->_font_fill_color = color;
}

// UnSet font filling
void lcdUnsetFontFill(ST7735_t * dev) {
	dev->_font_fill = false;
}

// Set font underline
// color:frame color
void lcdSetFontUnderLine(ST7735_t * dev, uint16_t color) {
	dev->_font_underline = true;
	dev->_font_underline_color = color;
}

// UnSet font underline
void lcdUnsetFontUnderLine(ST7735_t * dev) {
	dev->_font_underline = false;
}