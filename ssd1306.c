/*
 *  Copyright (C) 2025 Lucas Jundi Hikazudani
 *  Contact: lhjundi <at> outlook <dot> com
 *
 *  This file is part of Smart Bag (Transport of thermosensitive medicines).
 *
 *  Smart Bag is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Smart Bag is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Smart Bag.  If not, see <https://www.gnu.org/licenses/>
 */


/**
 * @file ssd1306.c
 * @brief Implementação do driver para display OLED SSD1306
 * 
 * Este arquivo implementa as funções necessárias para controlar um display
 * OLED baseado no controlador SSD1306 via I2C no Raspberry Pi Pico.
 */

#include "ssd1306.h"

/**
 * @brief Fonte de caracteres 5x7 pixels
 * 
 * Array que define os padrões de bits para caracteres ASCII de 32 a 126.
 * Cada caractere é definido em uma matriz 5x7 pixels.
 */
static const uint8_t font5x7[][5] = {
    // Definição dos caracteres ASCII de 32 (espaço) a 126 (~)
    // Cada linha representa um caractere com 5 bytes, onde cada byte
    // define uma coluna vertical de 8 pixels
    {0x00,0x00,0x00,0x00,0x00}, // Espaço
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x14,0x08,0x3E,0x08,0x14}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x72,0x49,0x49,0x49,0x46}, // 2
    {0x21,0x41,0x49,0x4D,0x33}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x31}, // 6
    {0x41,0x21,0x11,0x09,0x07}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x46,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x08,0x14,0x22,0x41,0x00}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x00,0x41,0x22,0x14,0x08}, // >
    {0x02,0x01,0x59,0x09,0x06}, // ?
    {0x3E,0x41,0x5D,0x59,0x4E}, // @
    {0x7C,0x12,0x11,0x12,0x7C}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x00,0x7F,0x41,0x41,0x00}, // [
    {0x02,0x04,0x08,0x10,0x20}, // Backslash
    {0x00,0x41,0x41,0x7F,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04}, // ^
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x00,0x01,0x02,0x04,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x0C,0x52,0x52,0x52,0x3E}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x20,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z
    {0x00,0x08,0x36,0x41,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00}, // }
    {0x08,0x08,0x2A,0x1C,0x08}, // ->
    {0x08,0x1C,0x2A,0x08,0x08}  // <-
};

/**
 * @brief Buffer de framebuffer do display
 * 
 * Armazena o estado de todos os pixels do display.
 * O display é organizado em 8 páginas de 128 pixels de largura.
 */
static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/**
 * @brief Envia um comando para o display
 * 
 * @param i2c Instância I2C a ser utilizada
 * @param cmd Comando a ser enviado
 */
static void ssd1306_write_command(i2c_inst_t *i2c, uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};  // 0x00 indica byte de comando
    i2c_write_blocking(i2c, SSD1306_I2C_ADDR, data, 2, false);
}

/**
 * @brief Envia dados para o display
 * 
 * @param i2c Instância I2C a ser utilizada
 * @param data Ponteiro para os dados
 * @param len Quantidade de bytes a serem enviados
 */
static void ssd1306_write_data(i2c_inst_t *i2c, uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = 0x40; // 0x40 indica byte de dados
    memcpy(buf + 1, data, len);
    i2c_write_blocking(i2c, SSD1306_I2C_ADDR, buf, len + 1, false);
}

/**
 * @brief Inicializa o display OLED
 * 
 * Configura os registradores do controlador SSD1306 com os valores
 * necessários para operação normal do display.
 * 
 * @param i2c Instância I2C a ser utilizada
 */
void ssd1306_init(i2c_inst_t *i2c) {
    sleep_ms(100); // Aguarda estabilização do display

    // Sequência de inicialização conforme datasheet
    ssd1306_write_command(i2c, 0xAE); // Display OFF
    ssd1306_write_command(i2c, 0x20); // Set Memory Addressing Mode
    ssd1306_write_command(i2c, 0x00); // Horizontal Addressing Mode
    ssd1306_write_command(i2c, 0xB0); // Set Page Start Address for Page Addressing Mode
    ssd1306_write_command(i2c, 0xC8); // COM Output Scan Direction remapped mode
    ssd1306_write_command(i2c, 0x00); // Set low column address
    ssd1306_write_command(i2c, 0x10); // Set high column address
    ssd1306_write_command(i2c, 0x40); // Set start line address
    ssd1306_write_command(i2c, 0x81); // Set contrast control
    ssd1306_write_command(i2c, 0xFF);
    ssd1306_write_command(i2c, 0xA1); // Set segment re-map 0 to 127
    ssd1306_write_command(i2c, 0xA6); // Set normal display
    ssd1306_write_command(i2c, 0xA8); // Set multiplex ratio(1 to 64)
    ssd1306_write_command(i2c, 0x3F); // 1/64 duty
    ssd1306_write_command(i2c, 0xA4); // Output follows RAM content
    ssd1306_write_command(i2c, 0xD3); // Set display offset
    ssd1306_write_command(i2c, 0x00); // No offset
    ssd1306_write_command(i2c, 0xD5); // Set display clock divide ratio/oscillator frequency
    ssd1306_write_command(i2c, 0xF0); // Set divide ratio
    ssd1306_write_command(i2c, 0xD9); // Set pre-charge period
    ssd1306_write_command(i2c, 0x22);
    ssd1306_write_command(i2c, 0xDA); // Set com pins hardware configuration
    ssd1306_write_command(i2c, 0x12);
    ssd1306_write_command(i2c, 0xDB); // Set vcomh
    ssd1306_write_command(i2c, 0x20);
    ssd1306_write_command(i2c, 0x8D); // Set DC-DC enable
    ssd1306_write_command(i2c, 0x14);
    ssd1306_write_command(i2c, 0xAF); // Turn on SSD1306 panel
}

/**
 * @brief Limpa o buffer do display
 * 
 * Preenche o buffer com zeros, apagando todos os pixels
 */
void ssd1306_clear() {
    memset(buffer, 0, sizeof(buffer));
}

/**
 * @brief Atualiza o conteúdo do display
 * 
 * Envia o conteúdo do buffer para o display, página por página
 * 
 * @param i2c Instância I2C a ser utilizada
 */
void ssd1306_update(i2c_inst_t *i2c) {
    for (uint8_t page = 0; page < 8; page++) {
        // Configura endereço da página
        ssd1306_write_command(i2c, 0xB0 + page);
        ssd1306_write_command(i2c, 0x00);  // Coluna inicial baixa
        ssd1306_write_command(i2c, 0x10);  // Coluna inicial alta

        // Envia dados da página
        ssd1306_write_data(i2c, &buffer[SSD1306_WIDTH * page], SSD1306_WIDTH);
    }
}

/**
 * @brief Define o estado de um pixel no buffer
 * 
 * @param x Coordenada X (0-127)
 * @param y Coordenada Y (0-63)
 * @param color true para pixel aceso, false para apagado
 */
void ssd1306_draw_pixel(int x, int y, bool color) {
    // Verifica limites
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
        return;

    // Calcula posição no buffer e bit correspondente
    if (color)
        buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

/**
 * @brief Desenha um caractere no display
 * 
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param c Caractere a ser desenhado (ASCII 32-126)
 * @param color true para pixels acesos, false para apagados
 */
void ssd1306_draw_char(int x, int y, char c, bool color) {
    if (c < 32 || c > 126)
        return;

    // Desenha cada coluna do caractere
    for (int i = 0; i < 5; i++) {
        uint8_t line = font5x7[c - 32][i];
        for (int j = 0; j < 8; j++) {
            ssd1306_draw_pixel(x + i, y + j, (line & 0x01) ? color : !color);
            line >>= 1;
        }
    }
}

/**
 * @brief Desenha uma string no display
 * 
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param str String a ser desenhada
 * @param color true para pixels acesos, false para apagados
 */
void ssd1306_draw_string(int x, int y, const char *str, bool color) {
    while (*str) {
        ssd1306_draw_char(x, y, *str++, color);
        x += 6; // Avançar 6 pixels (5 de largura + 1 de espaço)
    }
}