#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

#define GREEN_LED 11
#define RED_LED 13
#define BUTTON 5

typedef enum{
    RED,
    YELLOW,
    GREEN
} traffic_light_state;

struct light_state
{
    traffic_light_state state;
    uint32_t duration;
};

volatile struct light_state current = {RED, 10000};

// Function prototypes
void turn_on_red_signal();
void turn_on_yellow_signal();
void turn_on_green_signal();
void turn_on_red_signal();
void turn_on_yellow_signal();
void setup();
void button_interrupt_handler(uint gpio, uint32_t events);
void change_state();
bool state_controller();
bool is_time_to_change();

bool state_controller(){
    printf("Duration: %d seconds\n", current.duration/1000);
    current.duration -= 1000;
    if(is_time_to_change())
        change_state();
    return true;
}

bool is_time_to_change()
{
    if (current.duration <= 0) return true;
    return false;
}

void change_state(){
    switch (current.state)
    {
        case RED:
            turn_on_green_signal();
            current.state = GREEN;
            current.duration = 10000;
            break;
        case GREEN:
            turn_on_yellow_signal();
            current.state = YELLOW;
            current.duration = 3000;
            break;
        case YELLOW:
            turn_on_red_signal();
            current.state = RED;
            current.duration = 10000;
            break;
    }
}

void button_interrupt_handler(uint gpio, uint32_t events)
{
    if (events & GPIO_IRQ_EDGE_FALL)
    {
        printf("Pedestrian button activated!\n");
        current.duration = 1000;
        current.state = GREEN;
    }
}

void turn_on_green_signal()
{
    gpio_put(GREEN_LED, 1);
    gpio_put(RED_LED, 0);
    printf("Signal: Green!\n");
}

void turn_on_red_signal()
{
    gpio_put(GREEN_LED, 0);
    gpio_put(RED_LED, 1);
    printf("Signal: Red!\n");
}

void turn_on_yellow_signal()
{
    gpio_put(GREEN_LED, 1);
    gpio_put(RED_LED, 1);
    printf("Signal: Yellow!\n");
}

void setup()
{
    stdio_init_all();

    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, 0);

    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);
    gpio_put(RED_LED, 0);

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);
    sleep_ms(2000);
    printf("Traffic Light System\n");
    turn_on_red_signal();
}

int main()
{
    setup();

    struct repeating_timer timer;

    add_repeating_timer_ms(-1000, state_controller, NULL, &timer);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_interrupt_handler);
    // add_alarm_in_ms(1000, state_controller, NULL, true);
    
    while (true)
    {
        tight_loop_contents();
    }

    return 0;
}
