#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

/**
 * @brief Pin definitions for the BitDogLab project.
 *
 * This section defines the hardware pin assignments used throughout the BitDogLab project.
 */
#define GREEN_LED 11
#define RED_LED 13
#define BUTTON_A 5
#define BUTTON_B 6
#define BUZZER 21
#define BUZZER_FREQ 100

/**
 * @brief Definitions for the OLED display via I2C.
 *
 * This section contains the configuration parameters and pin definitions
 * related to the OLED display communication using the I2C protocol.
 */

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

/**
 * @enum traffic_light_state
 * @brief Possible states of the traffic light.
 *
 * Represents the colors of a traffic light in traffic control.
 */
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

/**
 * @brief Current traffic light state and its duration.
 *
 * This variable holds the current state of the traffic light along with
 * the time it should remain in this state. Declared volatile because it may
 * be modified by interrupt service routines or other concurrent contexts.
 */
volatile struct light_state current = {RED, 10000};

/**
 * @brief Flag indicating if button A has been pressed.
 *
 * Set to true by an interrupt or event handler when button A is pressed.
 * Declared volatile because it can be modified asynchronously.
 */
volatile bool button_A_pressed = false;

/**
 * @brief Flag indicating if button B has been pressed.
 *
 * Set to true by an interrupt or event handler when button B is pressed.
 * Declared volatile because it can be modified asynchronously.
 */
volatile bool button_B_pressed = false;

// Function prototypes

void turn_on_red_signal();
void turn_on_yellow_signal();
void turn_on_green_signal();
void setup();
void button_interrupt_handler(uint gpio, uint32_t events);
void change_state();
void pwm_init_buzzer(uint pin);
void beep(uint pin, uint32_t duration_ms);
int64_t beep_stop_callback(alarm_id_t id, void *user_data);
bool state_controller();
bool is_time_to_change();
void init_display();
void update_display();
char *get_state_string();

int some_button_pressed();

/**
 * @brief Returns a string representing the current traffic light state or pedestrian instruction.
 *
 * The returned string depends on button presses and the current traffic light state:
 * - If either button A or B is pressed and the light is RED, returns "Walk!".
 * - If either button A or B is pressed but the light is not RED, returns "Wait".
 * - Otherwise, returns the current traffic light color as a string: "RED", "YELLOW", or "GREEN".
 *
 * @return Pointer to a string literal representing the current state or pedestrian instruction.
 */
char* get_state_string(){
    if ((some_button_pressed()) && current.state == RED)
        return "Walk!";
    if (some_button_pressed()) return "Wait";
    if (current.state == RED) return "RED";
    if (current.state == YELLOW) return "YELLOW";
    if (current.state == GREEN) return "GREEN";
}

/**
 * @brief Checks if any pedestrian button is pressed.
 *
 * Returns a non-zero value if either button A or button B is currently pressed.
 *
 * @return int Non-zero if any button is pressed; zero otherwise.
 */
int some_button_pressed()
{
    return button_A_pressed || button_B_pressed;
}

/**
 * @brief Initializes the OLED display and I2C interface.
 *
 * Sets up the I2C peripheral with 400 kHz frequency,
 * configures SDA and SCL pins for I2C functionality with pull-ups,
 * and initializes the SSD1306 OLED display.
 *
 * Clears the display buffer and updates the display to show a blank screen.
 */
void init_display()
{
    // Inicializa o I2C com frequência de 400kHz
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED
    ssd1306_init(I2C_PORT);
    ssd1306_clear();
    ssd1306_update(I2C_PORT);
}

/**
 * @brief Updates the OLED display with current traffic light information.
 *
 * Displays the system title, current traffic light state, and additional messages
 * based on state and button interaction:
 * - If in RED state with ≤ 5 seconds remaining and a button is pressed, shows a countdown.
 * - If a button is pressed in other states, shows "Button Pressed!".
 * - Otherwise, shows "Waiting for button...".
 */
void update_display()
{
    char temp_str[32];
    char state_str[32];
    char countdown_str[32];

    ssd1306_clear();

    snprintf(temp_str, sizeof(temp_str), "Traffic Light System");
    ssd1306_draw_string(0, 0, temp_str, true);

    snprintf(state_str, sizeof(state_str), "Current State: %s", get_state_string());
    ssd1306_draw_string(0, 16, state_str, true);

    if (current.state == RED && current.duration <= 5000 && (some_button_pressed()))
    {
        snprintf(countdown_str, sizeof(countdown_str), "Countdown: %d s", current.duration / 1000);
        ssd1306_draw_string(0, 32, countdown_str, true);
    }
    else if ((some_button_pressed()))
    {
        snprintf(countdown_str, sizeof(countdown_str), "Button Pressed!");
        ssd1306_draw_string(0, 32, countdown_str, true);
    }
    else
    {
        snprintf(countdown_str, sizeof(countdown_str), "Waiting for button...");
        ssd1306_draw_string(0, 32, countdown_str, true);
    }
    ssd1306_update(I2C_PORT);
}

/**
 * @brief Callback function to stop the buzzer sound.
 *
 * This function is called when a timer/alarm expires.
 * It stops the PWM signal on the specified GPIO pin, effectively silencing the buzzer.
 *
 * @param id The alarm identifier (unused in this function).
 * @param user_data Pointer to the GPIO pin number (cast from void*).
 * @return Always returns 0 to indicate that the alarm should not be repeated.
 */
int64_t beep_stop_callback(alarm_id_t id, void *user_data)
{
    uint pin = (uintptr_t)user_data;
    pwm_set_gpio_level(pin, 0);
    return 0;
}

/**
 * @brief Activates the buzzer for a specified duration.
 *
 * Starts a PWM signal on the given GPIO pin with a 50% duty cycle to produce a beep.
 * Schedules a timer to stop the beep after the specified duration.
 *
 * @param pin GPIO pin connected to the buzzer.
 * @param duration_ms Duration of the beep in milliseconds.
 */
void beep(uint pin, uint32_t duration_ms)
{
    // Liga o buzzer com duty cycle de 50%
    pwm_set_gpio_level(pin, 2048);

    // Passa o pino como ponteiro para desligar depois
    add_alarm_in_ms(duration_ms, beep_stop_callback, (void *)(uintptr_t)pin, false);
}

/**
 * @brief Initializes the PWM configuration for the buzzer.
 *
 * Configures the specified GPIO pin for PWM output and sets the frequency
 * for buzzer operation. Initially sets the duty cycle to 0 (silent).
 *
 * The PWM frequency is calculated using the system clock, target buzzer frequency,
 * and a resolution of 12 bits (4096 steps).
 *
 * @param pin GPIO pin connected to the buzzer.
 */
void pwm_init_buzzer(uint pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQ * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

/**
 * @brief Manages traffic light state transitions and display updates.
 *
 * This function is called periodically (e.g., every 1 second) to:
 * - Decrease the remaining time for the current state.
 * - Update the OLED display.
 * - If in RED state and 5 seconds or less remain, and a button is pressed:
 *   - Print the remaining time.
 *   - Trigger a 5-second beep when the countdown reaches exactly 5 seconds.
 * - If the state duration reaches zero, transition to the next state.
 *
 * @return true Always returns true to indicate successful execution.
 */
bool state_controller()
{
    current.duration -= 1000;
    update_display();
    if (current.state == RED && current.duration <= 5000 && (some_button_pressed()))
        printf("Duration: %d seconds\n", current.duration / 1000);

    if ((some_button_pressed()) && current.state == RED && current.duration == 5000)
    {
        beep(BUZZER, 5000);
    }
    if (is_time_to_change())
        change_state();
    return true;
}

/**
 * @brief Checks if the traffic light should transition to the next state.
 *
 * Determines whether the current state's remaining duration has elapsed.
 *
 * @return true if the current duration is less than or equal to 0; false otherwise.
 */
bool is_time_to_change()
{
    return current.duration <= 0;
}

/**
 * @brief Transitions the traffic light to the next state.
 *
 * Updates the current state and its corresponding duration:
 * - RED → GREEN: resets button flags, activates green signal, sets duration to 10 seconds.
 * - GREEN → YELLOW: activates yellow signal, sets duration to 3 seconds.
 * - YELLOW → RED: activates red signal, sets duration to 10 seconds.
 *
 * This function also updates hardware outputs via signal control functions.
 */
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

/**
 * @brief GPIO interrupt handler for pedestrian button presses.
 *
 * Triggered on a falling edge (button press). When activated:
 * - Prints which button was pressed (A or B).
 * - Forces the traffic light to GREEN state.
 * - Sets the remaining duration to 1 second.
 * - Sets button_A_pressed to true (regardless of which button was pressed).
 *
 * @note The current implementation always sets the state to GREEN and
 *       only sets button_A_pressed = true, even for button B.
 *       Consider updating to distinguish between buttons A and B.
 *
 * @param gpio The GPIO pin that triggered the interrupt.
 * @param events Bitmask of GPIO interrupt events (e.g., falling edge).
 */
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

/**
 * @brief Turns on the green traffic light signal.
 *
 * Sets the green LED GPIO high and the red LED GPIO low.
 * Useful when transitioning to the GREEN state in the traffic light system.
 *
 * Also prints a status message to the console.
 */
void turn_on_green_signal()
{
    gpio_put(GREEN_LED, 1);
    gpio_put(RED_LED, 0);
    printf("Signal: Green!\n");
}

/**
 * @brief Turns on the red traffic light signal.
 *
 * Sets the red LED GPIO high and the green LED GPIO low.
 * Used when transitioning to the RED state in the traffic light system.
 *
 * Also prints a status message to the console.
 */
void turn_on_red_signal()
{
    gpio_put(GREEN_LED, 0);
    gpio_put(RED_LED, 1);
    printf("Signal: Red!\n");
}

/**
 * @brief Turns on the yellow traffic light signal.
 *
 * Sets both green and red LED GPIOs high to represent the yellow signal.
 * Used when transitioning to the YELLOW state in the traffic light system.
 *
 * Also prints a status message to the console.
 */
void turn_on_yellow_signal()
{
    gpio_put(GREEN_LED, 1);
    gpio_put(RED_LED, 1);
    printf("Signal: Yellow!\n");
}

/**
 * @brief Initializes hardware peripherals and prepares the system.
 *
 * - Initializes standard I/O.
 * - Configures GPIO pins for green and red LEDs as outputs, initially off.
 * - Configures pedestrian buttons A and B as inputs with pull-up resistors.
 * - Initializes PWM for the buzzer.
 * - Waits 2 seconds before starting.
 * - Prints a startup message.
 * - Turns on the red traffic light signal initially.
 */
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
    init_display();

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
