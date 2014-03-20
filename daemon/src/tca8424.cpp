/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * Routines for TCA8424
 *
 *
 *
 *
 */

#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>
#include "tca8424.h"

int file = 0;

int tca8424_reset(int file)
{
    char buf[4] = {0x00, 0x06, 0x00, 0x01};

    if (write(file, buf, 4) != 4)
       {
           close(file);
           return -3;
       }
    return 3;
}

int tca8424_leds(int file, char leds)
{
    char buf[9] = {0x00, 0x06, 0x20, 0x03, 0x00, 0x07, 0x01, 0x00, 0x00};

    buf[8] = leds;

    if (write(file, buf, 9) != 9)
       {
           close(file);
           return -4;
       }
    return file;
}


int tca8424_initComms(unsigned char addr)
{
    int file;

    /* open file and start ioctl */
    if ((file = open( "/dev/i2c-1", O_RDWR )) < 0)
    {
        return -1;
    }
    if (ioctl( file, I2C_SLAVE, addr) < 0)
    {
        close(file);
        return -2;
    }
    return file;
}

int tca8424_closeComms(int file)
{
    close(file);
    return file;
}

int tca8424_readInputReport(int file, char* report)
{
    char buf[13] = { 0x00, 0x06, 0x11, 0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int i;

    if (write(file, buf, 6) != 6)
    {
       close(file);
       return -5;
    }

    if (read( file, buf, 11 ) != 11)
    {
        close(file);
        return -6;
    }

    for (i=0 ; i< 11 ; i++)
        report[i] = buf[i];

    return file;

}

int tca8424_readMemory(int file, int start, int len, char* data)
{
    char buf[300] = {0};
    int i;

    buf[0] = start & 0xff;
    buf[1] = (start>>8) & 0xff;

    if (write(file, buf, 2) != 2)
    {
       close(file);
       return -11;
    }

    if (read( file, buf, len ) != len)
    {
        close(file);
        return -12;
    }

    for (i=0 ; i< len ; i++)
        data[i] = buf[i];

    return file;
}

/*
 *
 * dat Horrible key-press decoder
 *
 */

const char* tca8424_processKeyMap(char *map, int *code, int *isShift, int *isAlt)
{
    unsigned char k;

    int* c = code;
    int* shift = isShift;
    int* alt = isAlt;

    *shift = false;
    *alt = false;

    k = map[5];

    *c = 0; // Defaults to 0

    /* first check for shift and alt */
    if (k == 0x9E) { *shift = true; }
    if (k == 0xBC) { *shift = true; }
    if (k == 0xAE) { *alt = true; }
    if (k == 0xBE) { *alt = true; }

    /* if alt, use alternate key mapping */
    if (*alt)
    {
        k = map[6];
        if (k == 0xF1) { *c = KEY_1; return "Q 1"; }
        if (k == 0xA2) { *c = KEY_COMMA; *shift = true; return "Z <"; }
        if (k == 0xC2) { *c = KEY_1; *shift = true; return "S !"; }
        if (k == 0xF2) { *c = KEY_2; return "W 2"; }
        if (k == 0xA3) { *c = KEY_DOT; *shift = true; return "X >"; }
        if (k == 0xC3) { *c = KEY_3; *shift = true; return "D #"; }
        if (k == 0xF3) { *c = KEY_3; return "E 3"; }
        if (k == 0xA4) { *c = KEY_MINUS; *shift = true; return "C _"; }
        if (k == 0xC4) { *c = KEY_4; *shift = true; return "F $"; }
        if (k == 0xF4) { *c = KEY_4; return "R 4"; }
        if (k == 0xA5) { *c = KEY_MINUS; return "V -"; }
        if (k == 0xC5) { *c = KEY_5; *shift = true; return "G %"; }
        if (k == 0xF5) { *c = KEY_5; return "T 5"; }
        if (k == 0xA6) { *c = KEY_KPPLUS; return "B +"; }
        if (k == 0xC6) { *c = KEY_EQUAL; return "H ="; }
        if (k == 0xF6) { *c = KEY_6; return "Y 6"; }
        if (k == 0xA7) { *c = KEY_APOSTROPHE; *shift = true; return "N """; }
        if (k == 0xC7) { *c = KEY_7; *shift = true; return "J &"; }
        if (k == 0xF7) { *c = KEY_7; return "U 7"; }
        if (k == 0xA8) { *c = KEY_APOSTROPHE; return "M '"; }
        if (k == 0xC8) { *c = KEY_8; *shift = true; return "K *"; }
        if (k == 0xF8) { *c = KEY_8; return "I 8"; }
        if (k == 0xC9) { *c = KEY_9; *shift = true; return "L ("; }
        if (k == 0xF9) { *c = KEY_9; return "O 9"; }
        if (k == 0xAA) { *c = KEY_SEMICOLON; *shift = true; return ". :"; }
        if (k == 0xA9) { *c = KEY_SEMICOLON; return ", ;"; }
        if (k == 0xCA) { *c = KEY_0; *shift = true; return "? )"; }
        if (k == 0xFA) { *c = KEY_0; return "P 0"; }
        if (k == 0xB4) { *c = KEY_GRAVE; *shift = true; return "@ ~"; }
        if (k == 0xB8) { *c = KEY_EQUAL; *shift = true; return "/ ^"; }
        if (k == 0xFD) { *c = KEY_HOME; return "Left Arrow"; }
        if (k == 0x9D) { *c = KEY_END; return "Right Arrow"; }
        if (k == 0xFC) { *c = KEY_PAGEUP; return "Up arrow"; }
        if (k == 0xBD) { *c = KEY_PAGEDOWN; return "Down arrow"; }

        if (k == 0x00) return "! Released";

        return "* Unknown";
    }

    /* if shift, check the next key in map */
    if (*shift)
        k = map[6];

    if (k == 0xC1) { *c = KEY_A; return "A Fn-hold"; }
    if (k == 0xF1) { *c = KEY_Q; return "Q 1"; }
    if (k == 0xA2) { *c = KEY_Z; return "Z <"; }
    if (k == 0xC2) { *c = KEY_S; return "S !"; }
    if (k == 0xF2) { *c = KEY_W; return "W 2"; }
    if (k == 0xA3) { *c = KEY_X; return "X >"; }
    // if (k == 0xB3) { *c = KEY_KUKKUU; return "Search"; }
    if (k == 0xC3) { *c = KEY_D; return "D #"; }
    if (k == 0xF3) { *c = KEY_E; return "E 3"; }
    if (k == 0xA4) { *c = KEY_C; return "C _"; }
    if (k == 0xB4) { *c = KEY_2; *shift = true; return "@ ~"; }
    if (k == 0xC4) { *c = KEY_F; return "F $"; }
    if (k == 0xF4) { *c = KEY_R; return "R 4"; }
    if (k == 0xA5) { *c = KEY_V; return "V -"; }
    if (k == 0xB5) { *c = KEY_SPACE; return "Space1"; }
    if (k == 0xC5) { *c = KEY_G; return "G %"; }
    if (k == 0xF5) { *c = KEY_T; return "T 5"; }
    if (k == 0xA6) { *c = KEY_B; return "B +"; }
    if (k == 0xB6) { *c = KEY_SPACE; return "Space2"; }
    if (k == 0xC6) { *c = KEY_H; return "H ="; }
    if (k == 0xF6) { *c = KEY_Y; return "Y 6"; }
    if (k == 0xA7) { *c = KEY_N; return "N """; }
    if (k == 0xB7) { *c = KEY_SPACE; return "Space3 Sym"; }
    if (k == 0xC7) { *c = KEY_J; return "J &"; }
    if (k == 0xF7) { *c = KEY_U; return "U 7"; }
    if (k == 0xA8) { *c = KEY_M; return "M '"; }
    if (k == 0xB8) { *c = KEY_SLASH; return "/ ^"; }
    if (k == 0xC8) { *c = KEY_K; return "K *"; }
    if (k == 0xF8) { *c = KEY_I; return "I 8"; }
    if (k == 0xA9) { *c = KEY_COMMA; return ", ;"; }
    // if (k == 0xB9) { *c = KEY_KUKKUU; return "////"; }
    if (k == 0xC9) { *c = KEY_L; return "L ("; }
    if (k == 0xF9) { *c = KEY_O; return "O 9"; }
    if (k == 0xAA) { *c = KEY_DOT; return ". :"; }
    if (k == 0xCA) { *c = KEY_QUESTION; return "? )"; }
    if (k == 0xFA) { *c = KEY_P; return "P 0"; }
    if (k == 0xCB) { *c = KEY_ENTER; return "Return"; }
    if (k == 0xFB) { *c = KEY_BACKSPACE; return "Del"; }
    if (k == 0xFC) { *c = KEY_UP; return "Up arrow"; }
    if (k == 0x9C) { *c = KEY_ENTER; return "OK"; }
    if (k == 0xBD) { *c = KEY_DOWN; return "Down arrow"; }
    if (k == 0xFD) { *c = KEY_LEFT; return "Left Arrow"; }
    if (k == 0x9D) { *c = KEY_RIGHT; return "Right Arrow"; }

    if (k == 0x00) return "! Released";

    return "* Unknown";
}