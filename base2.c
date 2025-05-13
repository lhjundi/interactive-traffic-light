// Usar a biblioteca "hardware/timer.h"
// Utilizar as funções de alarme, temporização, interrupção e Callback
// Todo o processo deve ocorrer fora do superloop
// Não usar pooling com monitoramento contínuo dos botões

/*
    * 1. Inicia na cor vermelha e permanece por 10s
    * 2. Após 10s muda para verde e permanece por 10s
    * 3. Em seguida aciona a cor amarela e permanece por 3s
    * 4. Retorna à rotina 1.
    * 
    * Se o Botão for pressionado acende o amarelo e permanece por 3s
    * Em seguida retorna à rotina 1.
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

#define RED_LED_PIN 13
#define GREEN_LED_PIN 11
#define BUTTON_PIN 10

typedef enum{
    RED,
    GREEN,
    YELLOW 
} traffic_light_state_t;

volatile traffic_light_state_t current_state = RED;

void setup(){
    stdio_init_all();
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

int main(){
}