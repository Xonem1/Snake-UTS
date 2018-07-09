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

#include "Nokia5110.h"
#include "isqrt.h"


Nokia5110::Nokia5110(PinName sce, PinName rst, PinName dc, PinName dn, PinName sclk) {
    _lcd_SPI = new SPI(dn, NC, sclk);
    _lcd_SPI->format(LCD_SPI_BITS, LCD_SPI_MODE);
    _lcd_SPI->frequency(LCD_SPI_FREQ);

    _sce = new DigitalOut(sce, 1);
    _rst = new DigitalOut(rst, 1);
    _dc = new DigitalOut(dc, 0);
}

void Nokia5110::init(uint8_t con, uint8_t bias) {
    reset();
    wait_ms(10);
    set_contrast(con);
    set_bias(bias);
    set_mode(LCD_DISPLAYNORMAL);
}

void Nokia5110::reset() {
    _rst->write(0);
    wait_ms(500);
    _rst->write(1);
}

void Nokia5110::send_command(uint8_t cmd) {
    _sce->write(0);

    _lcd_SPI->write(cmd);

    _sce->write(1);
}

void Nokia5110::send_data(uint8_t data) {
    _dc->write(1);
    _sce->write(0);

    _lcd_SPI->write(data);

    _sce->write(1);
    _dc->write(0);
}

void Nokia5110::set_contrast(uint8_t con) {
    if (con > 0x7f) {
        con = 0x7f;
    }

    send_command(LCD_FUNCTIONSET | LCD_EXTENDEDINSTRUCTION);
    send_command(LCD_SETVOP | con);
    send_command(LCD_FUNCTIONSET);
}

void Nokia5110::set_bias(uint8_t bias) {
    if (bias > 0x08) {
        bias = 0x08;
    }

    send_command(LCD_FUNCTIONSET | LCD_EXTENDEDINSTRUCTION);
    send_command(LCD_SETBIAS | bias);
    send_command(LCD_FUNCTIONSET);
}

void Nokia5110::set_mode(uint8_t mode) {
    if (mode > 0x08) {
        mode = 0x08;
    }

    send_command(LCD_DISPLAYCONTROL | mode);
}

void Nokia5110::set_power(uint8_t pow) {
    pow = pow ? 0 : LCD_POWERDOWN;
    send_command(LCD_FUNCTIONSET | pow);
}

void Nokia5110::set_column(uint8_t col) {
    col %= LCD_WIDTH;
    send_command(LCD_SETXADDR | col);
}

void Nokia5110::set_bank(uint8_t bank) {
    bank %= LCD_BANKS;
    send_command(LCD_SETYADDR | bank);
}

void Nokia5110::set_cursor(uint8_t col, uint8_t bank) {
    set_column(col);
    set_bank(bank);
}

void Nokia5110::clear_buffer() {
    for (unsigned int i = 0; i < LCD_BYTES; i++) {
        _buffer[i] = 0x00;
    }
}
void Nokia5110::fastdisplay() {
    //set_bank(0);
    set_column(0);
    for (unsigned int i = 0; i < LCD_BYTES; i++) {
        send_data(_buffer[i]);
    }
}
void Nokia5110::display() {
    set_bank(0);
    set_column(0);
    for (unsigned int i = 0; i < LCD_BYTES; i++) {
        send_data(_buffer[i]);
    }
}

void Nokia5110::draw_pixel(uint8_t x, uint8_t y, const pattern_t pattern, Mode mode) {
    bool value = pattern[y % 8] & (1 << (x % 8)); // I am going to hell
    draw_pixel(x, y, value, mode);
}

void Nokia5110::draw_pixel(uint8_t x, uint8_t y, bool value, Mode mode) {
    if (mode & 0x4) {
        mode = (Mode) (mode & 0x3);
        value = !value;
    }

    if (mode == pixel_copy) {
        mode = value ? pixel_or : pixel_clr;
        value = true;
    }

    if (value) {
        x %= LCD_WIDTH;
        y %= LCD_HEIGHT;

        switch (mode) {
        default:
        case pixel_or:
            _buffer[x + (y / 8) * LCD_WIDTH] |= (1 << (y % 8));
            break;
        case pixel_xor:
            _buffer[x + (y / 8) * LCD_WIDTH] ^= (1 << (y % 8));
            break;
        case pixel_clr:
            _buffer[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
            break;
        }
    }
}

uint8_t Nokia5110::get_pixel(uint8_t x, uint8_t y) {
    x %= LCD_WIDTH;
    y %= LCD_HEIGHT;

    return _buffer[x + (y / 8) * LCD_WIDTH] & (1 << (y % 8));
}

void Nokia5110::draw_byte(uint8_t col, uint8_t bank, uint8_t byte) {
    col %= LCD_WIDTH;
    bank %= LCD_BANKS;

    _buffer[col + bank * LCD_WIDTH] = byte;
}

uint8_t Nokia5110::get_byte(uint8_t col, uint8_t bank) {
    col %= LCD_WIDTH;
    bank %= LCD_BANKS;

    return _buffer[col + bank * LCD_WIDTH];
}

uint8_t Nokia5110::print_char(char c, uint8_t x, uint8_t y, Mode mode) {
    x %= LCD_WIDTH;
    y %= LCD_HEIGHT;

    c -= 32;

    for (unsigned int i = 0; i < 5; i++) {
        for (unsigned int b = 0; b < 8; b++) {
            draw_pixel(x + i, y + b, font[(5 * c) + i] & (1 << b), mode);
        }
    }

    return x + 6;
}

uint8_t Nokia5110::print_string(const char *str, uint8_t x, uint8_t y, int8_t chars, Mode mode) {
    x %= LCD_WIDTH;
    y %= LCD_HEIGHT;

    while (*str && x + 6 <= LCD_WIDTH && chars-- != 0) {
        x = print_char(*str, x, y, mode);
        str++;
    }

    return x;
}

void Nokia5110::draw_bitmap(const uint8_t *bmp, uint8_t x, uint8_t y, uint8_t width, uint8_t height, Mode mode) {
    uint8_t mask = 0x80;

    for (uint8_t dy = 0; dy < height; dy++) {
        for (uint8_t dx = 0; dx < width; dx++) {
            draw_pixel(x + dx, y + dy, *bmp & mask, mode);
            mask >>= 1;

            if (mask == 0) { // if we reached the end of the byte
                mask = 0x80;
                bmp++;
            }
        }
    }
}

void Nokia5110::draw_wbitmap(const uint8_t *wbmp, uint8_t x, uint8_t y, Mode mode) {
    if (*wbmp++ != 0x00) { // image type, only supports 0
        return;
    }
    if (*wbmp++ != 0x00) { // always 0
        return;
    }
    uint8_t width = *wbmp++; // image width in pixels
    uint8_t height = *wbmp++; // image height in pixels

    uint8_t mask = 0x80;

    for (uint8_t dy = 0; dy < height; dy++) {
        for (uint8_t dx = 0; dx < width; dx++) {
            draw_pixel(x + dx, y + dy, *wbmp & mask, mode);
            mask >>= 1;

            if (mask == 0) { // if we reached the end of the byte
                mask = 0x80;
                wbmp++;
            }
        }
        if (mask != 0x80) {
            mask = 0x80; // wbmps pad out the end of each y, so reset the mask
            wbmp++;
        }
    }
}

void Nokia5110::draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const pattern_t pattern, Mode mode) {
    uint8_t dx = abs(x1 - x0);
    uint8_t dy = abs(y1 - y0);

    //use faster algorithms for horizontal and vertical lines
    if (dy == 0) {
        draw_hline(x0, x1, y0, pattern, mode);
        return;
    }
    if (dx == 0) {
        draw_vline(y0, y1, x0, pattern, mode);
        return;
    }

    //signs of x and y axes
    int8_t x_mult = (x0 > x1) ? -1 : 1;
    int8_t y_mult = (y0 > y1) ? -1 : 1;

    if (dy < dx) { //positive slope
        int8_t d = (2 * dy) - dx;
        uint8_t y = 0;
        for (uint8_t x = 0; x <= dx; x++) {
            draw_pixel(x0 + (x_mult * x), y0 + (y_mult * y), pattern, mode);
            if (d > 0) {
                y++;
                d -= dx;
            }
            d += dy;
        }
    } else { //negative slope
        int8_t d = (2 * dx) - dy;
        uint8_t x = 0;
        for (uint8_t y = 0; y <= dy; y++) {
            draw_pixel(x0 + (x_mult * x), y0 + (y_mult * y), pattern, mode);
            if (d > 0) {
                x++;
                d -= dy;
            }
            d += dx;
        }
    }
}

void Nokia5110::draw_hline(uint8_t x0, uint8_t x1, uint8_t y, const pattern_t pattern, Mode mode) {
    if (x0 > x1) {
        uint8_t tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    for (uint8_t x = x0; x <= x1; x++) {
        draw_pixel(x, y, pattern, mode);
    }
}

void Nokia5110::draw_vline(uint8_t y0, uint8_t y1, uint8_t x, const pattern_t pattern, Mode mode) {
    if (y0 > y1) {
        uint8_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (uint8_t y = y0; y <= y1; y++) {
        draw_pixel(x, y, pattern, mode);
    }
}

void Nokia5110::draw_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const pattern_t pattern, Mode mode) {
    draw_hline(x0, x1, y0, pattern, mode);
    draw_hline(x0, x1, y1, pattern, mode);
    draw_vline(y0, y1, x0, pattern, mode);
    draw_vline(y0, y1, x1, pattern, mode);
}

void Nokia5110::fill_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const pattern_t pattern, Mode mode) {
    if (x0 > x1) {
        uint8_t tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    if (y0 > y1) {
        uint8_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (uint8_t x = x0; x <= x1; x++) {
        for (uint8_t y = y0; y <= y1; y++) {
            draw_pixel(x, y, pattern, mode);
        }
    }
}

void Nokia5110::draw_rrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, const pattern_t pattern, Mode mode) {
    if (x0 > x1) {
        uint8_t tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    if (y0 > y1) {
        uint8_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    uint8_t cx0 = x0 + r;
    uint8_t cy0 = y0 + r;
    uint8_t cx1 = x1 - r;
    uint8_t cy1 = y1 - r;

    draw_hline(cx0, cx1, y0, pattern, mode);
    draw_hline(cx0, cx1, y1, pattern, mode);
    draw_vline(cy0, cy1, x0, pattern, mode);
    draw_vline(cy0, cy1, x1, pattern, mode);

    uint8_t x = r; // start at the cardinal points of the circle
    uint8_t y = 1;
    int8_t dx = 3 - (2 * r);
    int8_t dy = 1;
    int8_t err = 1; // difference of true radius squared and expected radius squared

    if (2 + dx > 0) {
        x--;
        err += dx;
        dx += 2;
    }

    // magic Bresenham voodoo
    while (x > y) {
        // draw each octant
        draw_pixel(cx1 + x, cy1 + y, pattern, mode);
        draw_pixel(cx1 + x, cy0 - y, pattern, mode);
        draw_pixel(cx0 - x, cy1 + y, pattern, mode);
        draw_pixel(cx0 - x, cy0 - y, pattern, mode);
        draw_pixel(cx1 + y, cy1 + x, pattern, mode);
        draw_pixel(cx1 + y, cy0 - x, pattern, mode);
        draw_pixel(cx0 - y, cy1 + x, pattern, mode);
        draw_pixel(cx0 - y, cy0 - x, pattern, mode);

        y++;
        err += dy;
        dy += 2;

        if (2 * err + dx > 0) {
            x--;
            err += dx;
            dx += 2;
        }
    }


    //draw 45° pixels
    draw_pixel(cx1 + x, cy1 + y, pattern, mode);
    draw_pixel(cx0 - x, cy1 + y, pattern, mode);
    draw_pixel(cx1 + x, cy0 - y, pattern, mode);
    draw_pixel(cx0 - x, cy0 - y, pattern, mode);
}

void Nokia5110::fill_rrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t r, const pattern_t pattern, Mode mode) {
    if (x0 > x1) {
        uint8_t tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    if (y0 > y1) {
        uint8_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    uint8_t cx0 = x0 + r;
    uint8_t cy0 = y0 + r;
    uint8_t cx1 = x1 - r;
    uint8_t cy1 = y1 - r;

    fill_rect(cx0, y0, cx1, y1, pattern, mode);

    uint8_t x = r; // start at the cardinal points of the circle
    uint8_t y = 1;
    int8_t dx = 3 - (2 * r);
    int8_t dy = 1;
    int8_t err = 1; // difference of true radius squared and expected radius squared

    // magic Bresenham voodoo
    while (x > y) {
        draw_vline(cy0 - x, cy1 + x, cx1 + y, pattern, mode);
        draw_vline(cy0 - x, cy1 + x, cx0 - y, pattern, mode);

        y++;
        err += dy;
        dy += 2;

        if (2 * err + dx > 0) {
            x--;
            err += dx;
            dx += 2;
            draw_vline(cy0 - (y - 1), cy1 + (y - 1), cx1 + (x + 1), pattern, mode);
            draw_vline(cy0 - (y - 1), cy1 + (y - 1), cx0 - (x + 1), pattern, mode);
        }
    }

    draw_vline(cy0 - y, cy1 + y, cx1 + x, pattern, mode);
    draw_vline(cy0 - y, cy1 + y, cx0 - x, pattern, mode);
}

void Nokia5110::draw_circle(uint8_t cx, uint8_t cy, uint8_t r, const pattern_t pattern, Mode mode) {
    if (!r) { // you cant have a radius of 0, silly
        draw_pixel(cx, cy, pattern, mode);
        return;
    }

    // draw the pixels in the cardinal directions
    draw_pixel(cx + r, cy, pattern, mode);
    draw_pixel(cx - r, cy, pattern, mode);
    draw_pixel(cx, cy + r, pattern, mode);
    draw_pixel(cx, cy - r, pattern, mode);

    uint8_t x = r; // start at the cardinal points of the circle
    uint8_t y = 1;
    int8_t dx = 3 - (2 * r);
    int8_t dy = 1;
    int8_t err = 1; // difference of true radius squared and expected radius squared

    if (2 + dx > 0) {
        x--;
        err += dx;
        dx += 2;
    }

    // magic Bresenham voodoo
    while (x > y) {
        // draw each octant
        draw_pixel(cx + x, cy + y, pattern, mode);
        draw_pixel(cx + x, cy - y, pattern, mode);
        draw_pixel(cx - x, cy + y, pattern, mode);
        draw_pixel(cx - x, cy - y, pattern, mode);
        draw_pixel(cx + y, cy + x, pattern, mode);
        draw_pixel(cx + y, cy - x, pattern, mode);
        draw_pixel(cx - y, cy + x, pattern, mode);
        draw_pixel(cx - y, cy - x, pattern, mode);

        y++;
        err += dy;
        dy += 2;

        if (2 * err + dx > 0) {
            x--;
            err += dx;
            dx += 2;
        }
    }

    //draw 45° pixels
    draw_pixel(cx + x, cy + y, pattern, mode);
    draw_pixel(cx - x, cy + y, pattern, mode);
    draw_pixel(cx + x, cy - y, pattern, mode);
    draw_pixel(cx - x, cy - y, pattern, mode);
}

void Nokia5110::fill_circle(uint8_t cx, uint8_t cy, uint8_t r, const uint8_t *pattern, Nokia5110::Mode mode) {
    if (!r) { // you cant have a radius of 0, silly
        draw_pixel(cx, cy, pattern, mode);
        return;
    }

    draw_vline(cy - r, cy + r, cx, pattern, mode);

    uint8_t x = r; // start at the cardinal points of the circle
    uint8_t y = 1;
    int8_t dx = 3 - (2 * r);
    int8_t dy = 1;
    int8_t err = 1; // difference of true radius squared and expected radius squared

    // magic Bresenham voodoo
    while (x > y) {
        draw_vline(cy - x, cy + x, cx + y, pattern, mode);
        draw_vline(cy - x, cy + x, cx - y, pattern, mode);

        y++;
        err += dy;
        dy += 2;

        if (2 * err + dx > 0) {
            x--;
            err += dx;
            dx += 2;
            draw_vline(cy - (y - 1), cy + (y - 1), cx + (x + 1), pattern, mode);
            draw_vline(cy - (y - 1), cy + (y - 1), cx - (x + 1), pattern, mode);
        }
    }

    draw_vline(cy - y, cy + y, cx + x, pattern, mode);
    draw_vline(cy - y, cy + y, cx - x, pattern, mode);
}

void Nokia5110::draw_ellipse(uint8_t cx, uint8_t cy, uint8_t a, uint8_t b, const pattern_t pattern, Mode mode) {
    if (!a) { // you cant have a radius of 0, silly
        draw_vline(cy - b, cy + b, cx, pattern, mode);
        return;
    }
    if (!b) { // you cant have a radius of 0, silly
        draw_hline(cx - a, cx + a, cy, pattern, mode);
        return;
    }

    draw_pixel(cx + a, cy, pattern, mode);
    draw_pixel(cx - a, cy, pattern, mode);
    draw_pixel(cx, cy + b, pattern, mode);
    draw_pixel(cx, cy - b, pattern, mode);

    uint16_t two_a_sqr = 2 * a * a;
    uint16_t two_b_sqr = 2 * b * b;

    uint8_t x = a; // start at the cardinal points
    uint8_t y = 1;
    int16_t dx = b * b * (1 - (2 * a));
    int16_t dy = 3 * a * a;
    int16_t err = a * a;
    uint8_t stop_x = a * a / (isqrt(a * a + b));

    if (dx + two_a_sqr > 0) {
        x--;
        err += dx;
        dx += two_b_sqr;
    }

    // section 1 (left and right)
    while (x >= stop_x) {
        draw_pixel(cx + x, cy + y, pattern, mode);
        draw_pixel(cx - x, cy + y, pattern, mode);
        draw_pixel(cx + x, cy - y, pattern, mode);
        draw_pixel(cx - x, cy - y, pattern, mode);

        y++;
        err += dy;
        dy += two_a_sqr;

        if ((err * 2) + dx > 0) {
            x--;
            err += dx;
            dx += two_b_sqr;
        }
    }

    uint8_t stop_y = y;
    x = 1;
    y = b;
    dx = 3 * b * b;
    dy = a * a * (1 - (2 * b));
    err = b * b;

    if (dy + two_b_sqr > 0) {
        y--;
        err += dy;
        dy += two_a_sqr;
    }

    // section 2 (top and bottom)
    while (x < stop_x) {
        draw_pixel(cx + x, cy + y, pattern, mode);
        draw_pixel(cx - x, cy + y, pattern, mode);
        draw_pixel(cx + x, cy - y, pattern, mode);
        draw_pixel(cx - x, cy - y, pattern, mode);

        x++;
        err += dx;
        dx += two_b_sqr;

        if ((err * 2) + dy > 0) {
            y--;
            err += dy;
            dy += two_a_sqr;
        }
    }

    if (y >= stop_y) {
        draw_vline(cy + y, cy + stop_y, cx + (x - 1), pattern, mode);
        draw_vline(cy - y, cy - stop_y, cx + (x - 1), pattern, mode);
        draw_vline(cy + y, cy + stop_y, cx - (x - 1), pattern, mode);
        draw_vline(cy - y, cy - stop_y, cx - (x - 1), pattern, mode);
    }
}

void Nokia5110::fill_ellipse(uint8_t cx, uint8_t cy, uint8_t a, uint8_t b, const pattern_t pattern, Mode mode) {
    if (!a) { // you cant have a radius of 0, silly
        draw_vline(cy - b, cy + b, cx, pattern, mode);
        return;
    }
    if (!b) { // you cant have a radius of 0, silly
        draw_hline(cx - a, cx + a, cy, pattern, mode);
        return;
    }

    draw_vline(cy + b, cy - b, cx, pattern, mode);

    uint16_t two_a_sqr = 2 * a * a;
    uint16_t two_b_sqr = 2 * b * b;

    int8_t x = a; // start at the cardinal points
    int8_t y = 1;
    int16_t dx = b * b * (1 - (2 * a));
    int16_t dy = 3 * a * a;
    int16_t err = a * a;
    uint8_t stop_x = a * a / (isqrt(a * a + b));

    if (dx + two_a_sqr > 0) {
        x--;
        err += dx;
        dx += two_b_sqr;
    }

    // section 1 (left and right)
    while (x >= stop_x) {
        y++;
        err += dy;
        dy += two_a_sqr;

        if ((err * 2) + dx > 0) {
            draw_vline(cy + (y - 1), cy - (y - 1), cx + x, pattern, mode);
            draw_vline(cy + (y - 1), cy - (y - 1), cx - x, pattern, mode);

            x--;
            err += dx;
            dx += two_b_sqr;
        }
    }

    x = 1;
    y = b;
    dx = 3 * b * b;
    dy = a * a * (1 - (2 * b));
    err = b * b;

    if (dy + two_b_sqr > 0) {
        y--;
        err += dy;
        dy += two_a_sqr;
    }

    // section 2 (top and bottom)
    while (x < stop_x) {
        draw_vline(cy + y, cy - y, cx + x, pattern, mode);
        draw_vline(cy + y, cy - y, cx - x, pattern, mode);

        x++;
        err += dx;
        dx += two_b_sqr;

        if ((err * 2) + dy > 0) {
            y--;
            err += dy;
            dy += two_a_sqr;
        }
    }
}

// patterns
const pattern_t Nokia5110::pattern_black = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const pattern_t Nokia5110::pattern_dkgrey = {0xEE, 0xBB, 0xEE, 0xBB, 0xEE, 0xBB, 0xEE, 0xBB};

const pattern_t Nokia5110::pattern_grey = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

const pattern_t Nokia5110::pattern_ltgrey = {0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44};

const pattern_t Nokia5110::pattern_white = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// font from
// https://developer.mbed.org/users/eencae/code/N5110/docs/tip/N5110_8h_source.html
const uint8_t Nokia5110::font[480] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // (space)
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // "\"
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08, // ->
    0x08, 0x1C, 0x2A, 0x08, 0x08  // <-
};

