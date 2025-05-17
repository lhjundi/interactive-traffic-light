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
 * @file ssd1306.h
 * @brief Interface do driver para display OLED SSD1306
 * 
 * Este arquivo define as constantes e funções públicas para controle
 * de displays OLED baseados no controlador SSD1306 via I2C usando
 * o Raspberry Pi Pico.
 */

 #ifndef SSD1306_H
 #define SSD1306_H
 
 #include "pico/stdlib.h"         // Funções básicas do Raspberry Pi Pico
 #include "hardware/i2c.h"        // Funções de controle do I2C
 #include <string.h>             // Para memcpy e memset
 
 /**
  * @brief Definições básicas do display OLED SSD1306
  */
 #define SSD1306_I2C_ADDR 0x3C   // Endereço I2C padrão do display
 #define SSD1306_WIDTH 128       // Largura do display em pixels
 #define SSD1306_HEIGHT 64       // Altura do display em pixels
 
 /**
  * @brief Inicializa o display OLED
  * 
  * Configura o display com as definições padrão e o prepara para uso.
  * Deve ser chamada antes de qualquer outra função do display.
  * 
  * @param i2c Ponteiro para a instância I2C a ser utilizada
  */
 void ssd1306_init(i2c_inst_t *i2c);
 
 /**
  * @brief Limpa todo o conteúdo do display
  * 
  * Apaga todos os pixels do buffer interno.
  * Para aplicar a mudança, é necessário chamar ssd1306_update().
  */
 void ssd1306_clear();
 
 /**
  * @brief Atualiza o conteúdo do display
  * 
  * Envia o conteúdo do buffer interno para o display.
  * Deve ser chamada após modificações no buffer para que
  * as alterações sejam visíveis.
  * 
  * @param i2c Ponteiro para a instância I2C a ser utilizada
  */
 void ssd1306_update(i2c_inst_t *i2c);
 
 /**
  * @brief Define o estado de um pixel específico
  * 
  * @param x Coordenada X do pixel (0 a 127)
  * @param y Coordenada Y do pixel (0 a 63)
  * @param color true para acender o pixel, false para apagar
  * 
  * @note As coordenadas são verificadas para garantir que estejam
  * dentro dos limites do display. Coordenadas inválidas são ignoradas.
  */
 void ssd1306_draw_pixel(int x, int y, bool color);
 
 /**
  * @brief Desenha um caractere ASCII no display
  * 
  * Utiliza uma fonte 5x7 pixels para desenhar caracteres.
  * 
  * @param x Coordenada X inicial do caractere
  * @param y Coordenada Y inicial do caractere
  * @param c Caractere ASCII a ser desenhado (32-126)
  * @param color true para caractere aceso em fundo apagado,
  *              false para caractere apagado em fundo aceso
  * 
  * @note Caracteres fora do intervalo ASCII 32-126 são ignorados
  */
 void ssd1306_draw_char(int x, int y, char c, bool color);
 
 /**
  * @brief Desenha uma string de texto no display
  * 
  * Desenha uma sequência de caracteres, começando na posição especificada.
  * Cada caractere ocupa 6 pixels de largura (5 + 1 de espaço).
  * 
  * @param x Coordenada X inicial da string
  * @param y Coordenada Y inicial da string
  * @param str Ponteiro para a string a ser desenhada
  * @param color true para texto aceso em fundo apagado,
  *              false para texto apagado em fundo aceso
  * 
  * @note A string deve ser terminada em nulo.
  *       O texto continua até encontrar o final da string ou
  *       atingir o limite direito do display.
  */
 void ssd1306_draw_string(int x, int y, const char *str, bool color);
 
 #endif // SSD1306_H