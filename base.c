/**
 * Semáforo de Trânsito Interativo
 * 
 * Implementação de um semáforo com botão para travessia de pedestres,
 * contagem regressiva no Monitor Serial e alerta sonoro para acessibilidade.
 * 
 * Pinos utilizados:
 * - LED Vermelho: GPIO 13
 * - LED Verde: GPIO 11
 * - Botão de pedestres: GPIO 10
 * - Buzzer: GPIO 21
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Definição dos pinos
#define LED_VERMELHO 13
#define LED_VERDE 11
#define BOTAO_PEDESTRE 10
#define BUZZER 21

// Estados do semáforo
typedef enum {
    VERMELHO,
    VERDE,
    AMARELO
} estado_semaforo_t;

// Tempo em ms para cada estado
#define TEMPO_VERMELHO 10000
#define TEMPO_VERDE 10000
#define TEMPO_AMARELO 3000

// Variáveis globais
volatile estado_semaforo_t estado_atual = VERMELHO;
volatile bool botao_pressionado = false;
volatile bool contagem_regressiva_ativa = false;
volatile int contador_regressivo = 5;
volatile alarm_id_t alarme_atual = -1;
volatile alarm_id_t alarme_contador = -1;
volatile alarm_id_t alarme_buzzer = -1;

// Protótipos de funções
void configurar_gpio();
void iniciar_ciclo_semaforo();
void atualizar_semaforo(estado_semaforo_t novo_estado);
int64_t callback_troca_estado(alarm_id_t id, void *user_data);
int64_t callback_contador_regressivo(alarm_id_t id, void *user_data);
int64_t callback_buzzer(alarm_id_t id, void *user_data);
void callback_botao(uint gpio, uint32_t eventos);

int main() {
    // Inicialização do sistema
    stdio_init_all();
    
    // Configuração dos GPIOs
    configurar_gpio();
    
    // Inicia o ciclo do semáforo
    printf("Semáforo Interativo iniciado\n");
    atualizar_semaforo(VERMELHO);
    iniciar_ciclo_semaforo();
    
    // Loop principal (ficará vazio, pois a lógica ocorre nas interrupções)
    while (true) {
        // Economiza energia
        sleep_ms(100);
    }
    
    return 0;
}

// Configura os pinos GPIO
void configurar_gpio() {
    // Configuração dos LEDs
    gpio_init(LED_VERMELHO);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    
    // Configuração do buzzer
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    
    // Configuração do botão com interrupção
    gpio_init(BOTAO_PEDESTRE);
    gpio_set_dir(BOTAO_PEDESTRE, GPIO_IN);
    gpio_pull_up(BOTAO_PEDESTRE); // Ativa pull-up interno
    
    // Configura interrupção para borda de descida (botão pressionado)
    gpio_set_irq_enabled_with_callback(BOTAO_PEDESTRE, 
                                      GPIO_IRQ_EDGE_FALL, 
                                      true, 
                                      &callback_botao);
}

// Inicia o ciclo do semáforo
void iniciar_ciclo_semaforo() {
    int tempo_ms;
    
    // Define o tempo baseado no estado atual
    switch (estado_atual) {
        case VERMELHO:
            tempo_ms = TEMPO_VERMELHO;
            break;
        case VERDE:
            tempo_ms = TEMPO_VERDE;
            break;
        case AMARELO:
            tempo_ms = TEMPO_AMARELO;
            break;
    }
    
    // Agenda o próximo estado
    alarme_atual = add_alarm_in_ms(tempo_ms, callback_troca_estado, NULL, true);
    
    // Se estiver no estado vermelho após botão pressionado, inicia contagem regressiva
    if (estado_atual == VERMELHO && botao_pressionado) {
        contagem_regressiva_ativa = true;
        contador_regressivo = 5;
        
        // Agenda a contagem regressiva para iniciar 5 segundos antes do fim do vermelho
        alarme_contador = add_alarm_in_ms(tempo_ms - 5000, callback_contador_regressivo, NULL, true);
        
        // Inicia o buzzer para alerta sonoro aos pedestres
        alarme_buzzer = add_alarm_in_ms(100, callback_buzzer, NULL, true);
        
        // Reseta a flag do botão
        botao_pressionado = false;
    }
}

// Atualiza o estado do semáforo
void atualizar_semaforo(estado_semaforo_t novo_estado) {
    estado_atual = novo_estado;
    
    // Atualiza as saídas físicas dos LEDs
    switch (estado_atual) {
        case VERMELHO:
            gpio_put(LED_VERMELHO, 1);
            gpio_put(LED_VERDE, 0);
            printf("Sinal: Vermelho\n");
            break;
            
        case VERDE:
            gpio_put(LED_VERMELHO, 0);
            gpio_put(LED_VERDE, 1);
            printf("Sinal: Verde\n");
            break;
            
        case AMARELO:
            gpio_put(LED_VERMELHO, 1);
            gpio_put(LED_VERDE, 1); // Vermelho + Verde = Amarelo no LED RGB
            printf("Sinal: Amarelo\n");
            break;
    }
}

// Callback chamado quando o timer expira para troca de estado
int64_t callback_troca_estado(alarm_id_t id, void *user_data) {
    // Desativa contagem regressiva se estiver ativa
    contagem_regressiva_ativa = false;
    
    // Lógica de transição de estados
    switch (estado_atual) {
        case VERMELHO:
            atualizar_semaforo(VERDE);
            break;
        case VERDE:
            atualizar_semaforo(AMARELO);
            break;
        case AMARELO:
            atualizar_semaforo(VERMELHO);
            // Marca como tendo sido originado pelo botão (para contagem regressiva)
            if (botao_pressionado) {
                // Mantém botao_pressionado como true para iniciar contagem regressiva
            } else {
                botao_pressionado = false;
            }
            break;
    }
    
    // Agenda o próximo estado
    iniciar_ciclo_semaforo();
    
    return 0; // O alarme não se repete automaticamente
}

// Callback para o contador regressivo
int64_t callback_contador_regressivo(alarm_id_t id, void *user_data) {
    if (contagem_regressiva_ativa) {
        printf("Tempo restante para pedestres: %d segundos\n", contador_regressivo);
        contador_regressivo--;
        
        if (contador_regressivo >= 0) {
            // Agenda o próximo decremento do contador
            return 1000; // Repete a cada 1 segundo
        }
    }
    
    return 0; // Não repete se o contador chegou a zero ou a contagem foi desativada
}

// Callback para controlar o buzzer
int64_t callback_buzzer(alarm_id_t id, void *user_data) {
    static bool buzzer_estado = false;
    
    if (contagem_regressiva_ativa) {
        // Alterna o estado do buzzer
        buzzer_estado = !buzzer_estado;
        gpio_put(BUZZER, buzzer_estado);
        
        // Define a frequência de bips baseada no tempo restante
        int intervalo_ms;
        if (contador_regressivo > 3) {
            intervalo_ms = 500; // Bips lentos no início
            intervalo_ms = 200; // Bips mais rápidos quando se aproxima do fim
        } else {
        }
        
        return intervalo_ms; // Repete após o intervalo definido
    } else {
        // Desliga o buzzer quando a contagem regressiva termina
        gpio_put(BUZZER, 0);
        return 0; // Não repete
    }
}

// Callback para tratamento da interrupção do botão
void callback_botao(uint gpio, uint32_t eventos) {
    // Debounce simples
    static uint32_t ultimo_tempo = 0;
    uint32_t tempo_atual = time_us_32();
    
    // Ignora acionamentos muito próximos (debounce de 300ms)
    if (tempo_atual - ultimo_tempo < 300000) {
        return;
    }
    ultimo_tempo = tempo_atual;
    
    // Processa o acionamento do botão
    printf("Botão de Pedestres acionado\n");
    botao_pressionado = true;
    
    // Cancela o alarme atual se existir
    if (alarme_atual >= 0) {
        cancel_alarm(alarme_atual);
    }
    
    // Se o estado não for amarelo, muda para amarelo por 3 segundos
    if (estado_atual != AMARELO) {
        atualizar_semaforo(AMARELO);
        // Agenda a transição para vermelho após TEMPO_AMARELO
        alarme_atual = add_alarm_in_ms(TEMPO_AMARELO, callback_troca_estado, NULL, true);
    }
}