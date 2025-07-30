#include "main.h"
#include <math.h>
#define PUB_DELAY_MS 1000
static uint32_t counter = 0;
static uint32_t last_time = 0;
err_t mqtt_test_connect(MQTT_CLIENT_T *state);
bool red_led_oscillating = true;
static void mqtt_sub_request_cb(void *arg, err_t err);



void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("DNS resolved: %s\n", ip4addr_ntoa(ipaddr));
    } else {
        DEBUG_printf("DNS resolution failed.\n");
    }
}


void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Running DNS lookup for %s...\n", MQTT_SERVER_HOST);
    if (dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state) == ERR_INPROGRESS) {
        while (state->remote_addr.addr == 0) {
            cyw43_arch_poll();
            sleep_ms(10);
        }
    }
}


static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("Incoming message on topic: %s\n", topic);
}


static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[BUFFER_SIZE];
    if (len < BUFFER_SIZE) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        DEBUG_printf("Message received: %s\n", buffer);

        if (strstr(buffer, "\"msg\": \"acender\"") != NULL) {
            pwm_led(LED_PIN_B, 2000);
            blue_led_status = true;
        } else if (strstr(buffer, "\"msg\": \"apagar\"") != NULL) {
            pwm_led(LED_PIN_B, 0);
            blue_led_status = false;
        } else if (strstr(buffer, "\"msg\": \"som\"") != NULL) {
    pwm_set_gpio_level(BUZZER_PIN, 750);  // Liga o buzzer
    sleep_ms(500);                        // Toca por 0,5s
    pwm_set_gpio_level(BUZZER_PIN, 0);    // Desliga
}
    } else {
        DEBUG_printf("Message too large, discarding.\n");
    }
}


static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        pwm_led(LED_PIN_G, 2000);  
        DEBUG_printf("MQTT connected.\n");

        // Este é o local correto para se inscrever nos tópicos.
        // Isso garante que a inscrição ocorra toda vez que a conexão for bem-sucedida.
        mqtt_set_inpub_callback(client, mqtt_pub_start_cb, mqtt_pub_data_cb, NULL);
        mqtt_sub_unsub(client, "pico_w/recve", 0, mqtt_sub_request_cb, NULL, 1);
        // Inicia um timer não-bloqueante para apagar o LED verde após um tempo
        green_led_on_time = to_ms_since_boot(get_absolute_time());
    } else {
        // Não é necessário controlar o LED vermelho aqui, pois a lógica de reconexão já o fará piscar.
        DEBUG_printf("MQTT connection failed: %d\n", status);
    }
}


static void mqtt_pub_request_cb(void *arg, err_t err) {
    // Apenas exibe a mensagem se houver um erro.
    // err = 0 (ERR_OK) significa sucesso, então não poluímos o log.
    if (err != ERR_OK) {
        DEBUG_printf("MQTT publish error: %d\n", err);
    }
}


static void mqtt_sub_request_cb(void *arg, err_t err) {
    DEBUG_printf("Subscription request status: %d\n", err);
}


err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_time >= PUB_DELAY_MS) {
        last_time = now;
        counter++;

        char buffer[BUFFER_SIZE];

        // Atualiza o estado do alarme e do buzzer
        if (alarme) {
            pwm_set_gpio_level(BUZZER_PIN, 750); // Som contínuo
        } else {
            pwm_set_gpio_level(BUZZER_PIN, 0); // Desliga buzzer
        }

        // Sempre lê os valores do joystick para incluí-los na mensagem
        js();
        // Lê a temperatura da placa
        float temperatura = ler_temperatura_placa();
        int temperatura_int = (int)roundf(temperatura);

        // Monta uma única mensagem JSON com todos os dados de status
        snprintf(buffer, BUFFER_SIZE,
                 "{\"msg_count\": %u, \"blue_led_status\": \"%s\", \"alarm_active\": %s, \"red_led_oscillating\": %s, \"joystick_x\": %u, \"joystick_y\": %u, \"temperature_c\": %d}",
                 counter, blue_led_status ? "on" : "off", alarme ? "true" : "false", red_led_oscillating ? "true" : "false", adc_x_raw, adc_y_raw, temperatura_int);

        // Adicionamos este printf para depuração.
        // Você poderá ver no monitor serial exatamente o que está sendo enviado.
        DEBUG_printf("Publishing to pico_w/status: %s\n", buffer);

        // Publica a mensagem consolidada no tópico de status
        return mqtt_publish(state->mqtt_client, "pico_w/status", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
    }
    return ERR_OK;
}


err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW";
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}


void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();
    if (!state->mqtt_client) {
        DEBUG_printf("Failed to create MQTT client\n");
        return;
    }


    if (mqtt_test_connect(state) == ERR_OK) {
        while (1) {
            cyw43_arch_poll();
            // A lógica principal agora verifica a conexão e publica/atualiza o estado.
            if (mqtt_client_is_connected(state->mqtt_client)) {
                mqtt_test_publish(state);

                // Lógica não-bloqueante para apagar o LED verde de conexão após 2 segundos
                if (green_led_on_time != 0 && (to_ms_since_boot(get_absolute_time()) - green_led_on_time > 2000)) {
                    pwm_led(LED_PIN_G, 0);
                    green_led_on_time = 0; // Desativa o timer
                }
                static uint64_t last_time_pwm = 0;
                uint64_t current_time = to_ms_since_boot(get_absolute_time());
                if (red_led_oscillating && (current_time - last_time_pwm >= FADE_STEP_DELAY)) {
                    update_pwm(LED_PIN_R);
                    last_time_pwm = current_time;
                }
                sleep_ms(50);
            } else {
                // Se desconectado, tenta reconectar. A inscrição ocorrerá no callback.
                DEBUG_printf("Reconnecting...\n");
                sleep_ms(250);
                mqtt_test_connect(state);
            }
        }
    }
}
