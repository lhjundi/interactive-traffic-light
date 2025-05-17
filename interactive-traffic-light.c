#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

#define GREEN_LED 11
#define RED_LED 13
#define BUTTON_A 5
#define BUTTON_B 6
#define BUZZER 21
#define BUZZER_FREQ 100

typedef enum
{
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
volatile bool button_A_pressed = false;
volatile bool button_B_pressed = false;

// Function prototypes
void turn_on_red_signal();
void turn_on_yellow_signal();
void turn_on_green_signal();
void turn_on_red_signal();
void turn_on_yellow_signal();
void setup();
void button_interrupt_handler(uint gpio, uint32_t events);
void change_state();
void pwm_init_buzzer(uint pin);
void beep(uint pin, uint32_t duration_ms);
int64_t beep_stop_callback(alarm_id_t id, void *user_data);
bool state_controller();
bool is_time_to_change();

int64_t beep_stop_callback(alarm_id_t id, void *user_data)
{
    uint pin = (uintptr_t)user_data;
    pwm_set_gpio_level(pin, 0);
    return 0;
}

void beep(uint pin, uint32_t duration_ms)
{
    // Liga o buzzer com duty cycle de 50%
    pwm_set_gpio_level(pin, 2048);

    // Passa o pino como ponteiro para desligar depois
    add_alarm_in_ms(duration_ms, beep_stop_callback, (void *)(uintptr_t)pin, false);
}

void pwm_init_buzzer(uint pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQ * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

bool state_controller()
{
    current.duration -= 1000;
    if (current.state == RED && current.duration <= 5000 && (button_A_pressed || button_B_pressed))
        printf("Duration: %d seconds\n", current.duration / 1000);

    if ((button_A_pressed || button_B_pressed) && current.state == RED && current.duration == 5000)
    {
        beep(BUZZER, 5000);
    }
    if (is_time_to_change())
        change_state();
    return true;
}

bool is_time_to_change()
{
    if (current.duration <= 0)
        return true;
    return false;
}

void change_state()
{
    switch (current.state)
    {
    case RED:
        button_A_pressed = false;
        button_B_pressed = false;
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
        printf("Pedestrian button %c activated!\n", gpio == BUTTON_A ? 'A' : 'B');
        current.duration = 1000;
        current.state = GREEN;
        button_A_pressed = true;
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

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    pwm_init_buzzer(BUZZER);
    sleep_ms(2000);
    printf("Traffic Light System\n");
    turn_on_red_signal();
}

int main()
{
    setup();

    struct repeating_timer timer;
    struct repeating_timer buzzer_timer;

    add_repeating_timer_ms(-1000, state_controller, NULL, &timer);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_interrupt_handler);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);

    while (true)
    {
        tight_loop_contents();
    }

    return 0;
}
