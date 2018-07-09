/*
   Copyright 2017 Andrew Cassidy

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#ifndef NOKIA5110_H
#define NOKIA5110_H

#include <mbed.h>
#include <stdbool.h>

// 4MHz clock frequency, maximum of the display
#define LCD_SPI_FREQ 400000

// 8 bits per command/data
#define LCD_SPI_BITS 0x08

// Polarity 0 Phase 0, may be different on your platform
/*
 mode | POL PHA
 -----+--------
   0  |  0   0
   1  |  0   1
   2  |  1   0
   3  |  1   1
*/
#define LCD_SPI_MODE 0x00

#define LCD_WIDTH 84
#define LCD_HEIGHT 48
#define LCD_BANKS 6
#define LCD_BYTES 504

#define LCD_POWERDOWN 0x04
#define LCD_ENTRYMODE 0x02
#define LCD_EXTENDEDINSTRUCTION 0x01

#define LCD_DISPLAYBLANK 0x0
#define LCD_DISPLAYNORMAL 0x4
#define LCD_DISPLAYALLON 0x1
#define LCD_DISPLAYINVERTED 0x5

// basic instruction set
#define LCD_FUNCTIONSET 0x20
#define LCD_DISPLAYCONTROL 0x08
#define LCD_SETYADDR 0x40
#define LCD_SETXADDR 0x80

// extended instruction set
#define LCD_SETTEMP 0x04
#define LCD_SETBIAS 0x10
#define LCD_SETVOP 0x80

typedef uint8_t pattern_t[8];

/**
 * @brief An API for using the Nokia 5110 display or other PCD8544-based
 * displays with mbed-os
 * @details The Nokia 5110 display is a 84x48 pixel single-bit LCD using the
 * PCD8544 controller.
 *  It is controlled by a modified version of the SPI protox.
 *
 *  If the API or test files dont work at first, try changing the contrast
 * setting. Different units
 *   will work best at different values. I've had this value range from 40 to 80
 *
 */
class Nokia5110 {
public:
    /**
     * @brief Mode for drawing pixels
     */
    enum Mode {
        pixel_copy = 0x0,
        pixel_or = 0x1,
        pixel_xor = 0x2,
        pixel_clr = 0x3,
        pixel_invt = 0x4,
        pixel_nor = 0x5,
        pixel_xnor = 0x6,
        pixel_nclr = 0x7
    };

    // patterns
    static const pattern_t pattern_black;
    static const pattern_t pattern_dkgrey;
    static const pattern_t pattern_grey;
    static const pattern_t pattern_ltgrey;
    static const pattern_t pattern_white;

    /**
     * @brief Mode for filling shapes
     */
    enum FillMode {
        solid,
        none,
        hatch,
        checkerboard,
        stripes_horiz,
        stripes_vert
    };

    /**
     * @brief constructor
     *
     * @param sce Chip Enable pin
     * @param rst Reset pin
     * @param dc D/C pin
     * @param dn data pin (MOSI)
     * @param sclk clock pin (SCLK)
     */
    Nokia5110(PinName sce, PinName rst, PinName dc, PinName dn, PinName sclk);

    /**
     * @brief initialize the display with given contrast and bias.
     *
     * @param con contrast for the display
     * @param bias bias for the display, should be 0x04 for the nokia 5110
     * display. only change for other PCD8544 displays
     */
    void init(uint8_t con = 40, uint8_t bias = 0x04);

    /**
     * @brief reset the display's memory
     */
    void reset();

    /**
     * @brief send a command to the display
     *
     * @param cmd command to send
     */
    void send_command(uint8_t cmd);

    /**
     * @brief send a byte of data to the display
     *
     * @param data data to send
     */
    void send_data(uint8_t data);

    /**
     * @brief sets the display's contrast
     *
     * @param con contrast, usually between 40 and 60 depending on your
     * display
     */
    void set_contrast(uint8_t con);

    /**
     * @brief sets the dispay's bias
     *
     * @param bias bias, should be 0x4 for the Nokia 5110 display
     */
    void set_bias(uint8_t bias);

    /**
     * @brief sets the display's display mode
     *
     * @param mode display mode (Blank, inverted, allon or normal)
     */
    void set_mode(uint8_t mode);

    /**
     * @brief turns the display on or off
     *
     * @param pow power, 0 = off, 1 = on
     */
    void set_power(uint8_t pow);

    /**
     * @brief sets the X value of the cursor
     *
     * @param col x coordinate (0-83)
     */
    void set_column(uint8_t col);

    /**
     * @brief sets the Y value of the cursor
     *
     * @param bank memory bank (0-5)
     */
    void set_bank(uint8_t bank);

    /**
     * @brief sets the X and Y values of the cursor
     *
     * @param col x coordinate (0-83)
     * @param bank memory bank (0-5)
     */
    void set_cursor(uint8_t col, uint8_t bank);

    /**
     * @brief clears the screen buffer
     */
    void clear_buffer();

    /**
     * @brief sends the screen buffer to the display
     */
    void fastdisplay();
    void display();

    /**
     * @brief draws a pixel to the screen buffer
     *
     * @param x x coordinate (0-83)
     * @param y y coordinate (0-47)
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void draw_pixel(uint8_t x, uint8_t y, const pattern_t pattern, Mode mode = pixel_copy);

    /**
     * @brief draws a pixel to the screen buffer
     *
     * @param x x coordinate (0-83)
     * @param y y coordinate (0-47)
     * @param value pixel value. 0 = white, 1 = black in normal mode
     * @param mode  draw mode (see above)
     */
    void draw_pixel(uint8_t x, uint8_t y, bool value, Mode mode = pixel_copy);

    /**
     * @brief gets the value of a pixel from the screen buffer
     *
     * @param x x coordinate (0-83)
     * @param y y coordinate (0-47)
     *
     * @return value of the pixel, 0 if white
     */
    uint8_t get_pixel(uint8_t x, uint8_t bank);

    /**
     * @brief draws a byte to the screen buffer
     *
     * @param x x coordinate, (0-83)
     * @param bank memory bank (0-5)
     * @param byte byte to draw
     */
    void draw_byte(uint8_t x, uint8_t bank, uint8_t byte);

    /**
     * @brief gets a byte from the screen buffer
     *
     * @param x x coordinate (0-83)
     * @param bank memory bank (0-5)
     *
     * @return byte from the screen buffer
     */
    uint8_t get_byte(uint8_t x, uint8_t y);

    /**
     * @brief prints a 7x5 character
     *
     * @param c character to draw
     * @param x x coordinate of upper left (0-83)
     * @param y y coordinate of upper left (0-47)
     * @param mode  draw mode (see above)
     *
     * @return next column to print to
     */
    uint8_t print_char(char c, uint8_t x, uint8_t y, Mode mode = pixel_copy);

    /**
     * @brief prints a string
     *
     * @param str string to print
     * @param x x coordinate of upper left (0-83)
     * @param y y coordinate of upper left (0-47)
     * @param chars maximum number of chars to print.
     *        -1 = no limit. stops at null byte
     * @param mode  draw mode (see above)
     *
     * @return next column to print to
     */
    uint8_t print_string(const char *str, uint8_t x, uint8_t y, int8_t chars = -1, Mode mode = pixel_copy);

    /**
     * @brief draws a bitmap in an unpadded format
     *
     * @param bmp pointer to the start of the bitmap
     * @param x x coordinate of upper left (0-83)
     * @param y y coordinate of upper left (0-47)
     * @param width bitmap width in pixels
     * @param height bitmap height in pixels
     */
    void draw_bitmap(const uint8_t *bmp, uint8_t x, uint8_t y, uint8_t width, uint8_t height, Mode mode = pixel_copy);

    /**
     * @brief draws a bitmap in the WBMP format
     *
     * @param wbmp pointer to the start of the bitmap
     * @param x x coordinate of upper left (0-83)
     * @param y y coordinate of upper left (0-47)
     */
    void draw_wbitmap(const uint8_t *wbmp, uint8_t x, uint8_t y, Mode mode = pixel_copy);

    /**
     * @brief draws a line
     *
     * @param x0 x coordinate of first point
     * @param y0 y coordinate of first point
     * @param x1 x coordinate of second point
     * @param y1 y coordinate of second point
     * @param mode  draw mode (see above)
     */
    void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   const pattern_t pattern = pattern_black,
                   Mode mode = pixel_copy);

    /**
     * @brief draws a horizontal line
     *
     * @param x0 x coordinate of first point
     * @param x1 x coordinate of second point
     * @param y  y coordinate of the line
     * @param mode  draw mode (see above)
     */
    void draw_hline(uint8_t x0, uint8_t x1, uint8_t y,
                    const pattern_t pattern = pattern_black,
                    Mode mode = pixel_copy);

    /**
     * @brief draws a vertical line
     *
     * @param y0 y coordinate of first point
     * @param y1 y coordinate of second point
     * @param x  x coordinate of the line
     * @param mode  draw mode (see above)
     */
    void draw_vline(uint8_t y0, uint8_t y1, uint8_t x,
                    const pattern_t pattern = pattern_black,
                    Mode mode = pixel_copy);

    /**
     * @brief draws an empty rectangle
     *
     * @param x0 column of the first point
     * @param y0 row of the first point
     * @param x1 column of the second point
     * @param y1 row of the second point
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void draw_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   const pattern_t pattern = pattern_black,
                   Mode mode = pixel_copy);

    /**
     * @brief fills a rectangle
     *
     * @param x0 column of the first point
     * @param y0 row of the first point
     * @param x1 column of the second point
     * @param y1 row of the second point
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void fill_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,
                   const pattern_t pattern = pattern_black,
                   Mode mode = pixel_copy);

    /**
     * @brief draws an empty rounded rectangle
     *
     * @param x0 column of first point
     * @param y0 row of first point
     * @param x1 column of second point
     * @param y1 row of second point
     * @param r radius
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void draw_rrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r,
                    const pattern_t pattern = pattern_black,
                    Mode mode = pixel_copy);

    /**
     * @brief fills a rounded rectangle
     *
     * @param x0 column of first point
     * @param y0 row of first point
     * @param x1 column of second point
     * @param y1 row of second point
     * @param r radius
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void fill_rrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r,
                    const pattern_t pattern = pattern_black,
                    Mode mode = pixel_copy);

    /**
     * @brief draws an empty circle
     * 
     * @param cx x coordinate of the center
     * @param cy y coordinate of the center
     * @param r radius of the circle
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void draw_circle(uint8_t cx, uint8_t cy, uint8_t r,
                     const pattern_t pattern = pattern_black,
                     Mode mode = pixel_copy);

    /**
     * @brief fills a circle
     *
     * @param cx x coordinate of the center
     * @param cy y coordinate of the center
     * @param r radius of the circle
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void fill_circle(uint8_t cx, uint8_t cy, uint8_t r,
                     const pattern_t pattern = pattern_black,
                     Mode mode = pixel_copy);

    /**
     * @brief draws an empty ellipse
     * 
     * @param cx x coordinate of the center
     * @param cy y coordinate of the center
     * @param a horizontal radius of the ellipse
     * @param b vertical radius of the ellipse
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void draw_ellipse(uint8_t cx, uint8_t cy, uint8_t a, uint8_t b,
                      const pattern_t pattern = pattern_black,
                      Mode mode = pixel_copy);

    /**
     * @brief fills an ellipse
     *
     * @param cx x coordinate of the center
     * @param cy y coordinate of the center
     * @param a horizontal radius of the ellipse
     * @param b vertical radius of the ellipse
     * @param pattern pattern to use
     * @param mode  draw mode (see above)
     */
    void fill_ellipse(uint8_t cx, uint8_t cy, uint8_t a, uint8_t b,
                      const pattern_t = pattern_black,
                      Mode mode = pixel_copy);

private:
    SPI *_lcd_SPI;

    DigitalOut *_sce;
    DigitalOut *_rst;
    DigitalOut *_dc;

    uint8_t _buffer[LCD_BYTES];
    static const uint8_t font[480];
};

#endif
