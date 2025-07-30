#include "main.h"


static uint32_t last_debounce_time[3] = {0, 0, 0};
uint adc_x_raw;
uint adc_y_raw;
uint brightness = 0;
bool increasing = true;




static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};


void pinos_start()
{
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_B);
    gpio_init(LED_PIN_G);
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_set_temp_sensor_enabled(true); // Habilita o sensor de temperatura interno
    gpio_set_function(LED_PIN_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_B, GPIO_FUNC_PWM);
    
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN_R);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    
    slice_num = pwm_gpio_to_slice_num(LED_PIN_B);
    pwm_init(slice_num, &config, true);
    
    slice_num = pwm_gpio_to_slice_num(LED_PIN_G);
    pwm_init(slice_num, &config, true);

    gpio_init(BUTTON6_PIN);
    gpio_set_dir(BUTTON6_PIN, GPIO_IN);
    gpio_pull_up(BUTTON6_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON6_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio5_callback);

    gpio_init(BUTTON5_PIN);
    gpio_set_dir(BUTTON5_PIN, GPIO_IN);
    gpio_pull_up(BUTTON5_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON5_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio5_callback);

    gpio_init(BUTTONJS_PIN);
    gpio_set_dir(BUTTONJS_PIN, GPIO_IN);
    gpio_pull_up(BUTTONJS_PIN);
    gpio_set_irq_enabled_with_callback(BUTTONJS_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio5_callback);

    // PWM do Buzzer
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_buzzer = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config_buzzer = pwm_get_default_config();
    pwm_config_set_clkdiv(&config_buzzer, 31.25f);  // Aproximadamente 4kHz
    pwm_init(slice_buzzer, &config_buzzer, true);
    pwm_set_gpio_level(BUZZER_PIN, 0); // Começa desligado
}


void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;


            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}


void gpio5_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == 5 && (now - last_debounce_time[0] > DEBOUNCE_DELAY_MS)) {
        last_debounce_time[0] = now;

        // Alterna o estado da oscilação do LED vermelho
        red_led_oscillating = !red_led_oscillating;

        if (!red_led_oscillating) {
            pwm_set_gpio_level(LED_PIN_R, 0);  // Apaga o LED vermelho
        }

        printf("Oscilação do LED vermelho: %s\n", red_led_oscillating ? "ATIVA" : "DESATIVADA");
    }

    if (gpio == 6 && (now - last_debounce_time[1] > DEBOUNCE_DELAY_MS)) {
        last_debounce_time[1] = now;
        alarme = !alarme;
    }

    if (gpio == 22 && (now - last_debounce_time[2] > DEBOUNCE_DELAY_MS)) {
        last_debounce_time[2] = now;
        posicao_js = true;
        printf("posicao_js: %u\n", posicao_js);
    }
}



void js()
{
    adc_select_input(0); // Canal ADC 0 para o eixo X
    adc_x_raw = adc_read();
    adc_select_input(1); // Canal ADC 1 para o eixo Y
    adc_y_raw = adc_read();
}

float ler_temperatura_placa() {
    /* Datasheet do RP2040, seção 4.9.5. Sensor de Temperatura
     * O sensor está conectado ao ADC_IN4.
     * Temperatura (em °C) = 27 - (Tensão_ADC - 0.706) / 0.001721
     */
    const float fator_conversao = 3.3f / (1 << 12);
    adc_select_input(4);
    uint16_t resultado_bruto = adc_read();
    float tensao = resultado_bruto * fator_conversao;
    float temp_c = 27.0f - (tensao - 0.706f) / 0.001721f;
    return temp_c;
}


void setup_pwm(uint gpio_pin) {
    // Configurar o GPIO como saída de PWM
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);


    // Obter o slice de PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);


    // Configurar o PWM com o padrão
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);  // Ajustar divisor de clock (frequência do PWM)
    pwm_init(slice_num, &config, true);
}


void update_pwm(uint gpio_pin) {
    // Atualizar o duty cycle baseado no brilho atual
    pwm_set_gpio_level(LED_PIN_R, brightness);
    printf("brilho_led_vermelho: %u\n",brightness);
    // Atualizar o valor do brilho
    if (increasing)
    {
        brightness = brightness+400;
        if (brightness >= PWM_STEPS)
        {
            increasing = false; // Começar a diminuir
        }
    }
    else
    {
        brightness = brightness-400;
        if (brightness == 0) {
            increasing = true; // Começar a aumentar
        }
    }
}


void pwm_led(uint gpio_pin, uint brilho)
{
    if(gpio_pin == LED_PIN_B)
    {
        pwm_set_gpio_level(LED_PIN_B, brilho);
    }
    else if (gpio_pin == LED_PIN_G)
    {
        pwm_set_gpio_level(LED_PIN_G, brilho);
    }
}


void buzzer_beep_pattern() {
    static bool buzzer_on = false;
    static uint64_t last_toggle_time = 0;
    uint64_t now = to_ms_since_boot(get_absolute_time());

    if (now - last_toggle_time >= 200) { // alterna a cada 200ms
        buzzer_on = !buzzer_on;
        gpio_put(BUZZER_PIN, buzzer_on);
        last_toggle_time = now;
    }
}
