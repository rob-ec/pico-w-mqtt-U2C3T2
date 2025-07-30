#include <time.h>
#include <stdio.h>
#include <string.h>
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt_priv.h"
#include "hardware/structs/rosc.h"


/* MACROS PI PICO */
#define LED_PIN_G 11
#define BUZZER_PIN 21
#define LED_PIN_B 12
#define LED_PIN_R 13
#define BUTTON5_PIN 5
#define BUTTON6_PIN 6
#define PWM_STEPS 2000
#define BUTTONJS_PIN 22
#define DEBOUNCE_DELAY_MS 700
#define FADE_STEP_DELAY (100)
/* END */


/* MACROS MQTT */
#define DEBUG_printf printf
#define MQTT_SERVER_HOST "broker.hivemq.com"
#define MQTT_SERVER_PORT 1883
#define MQTT_TLS 0
#define BUFFER_SIZE 256
/* END*/


/*VARIAVEIS*/
extern bool alarme;
extern bool posicao_js;
extern bool red_led_oscillating;
extern bool blue_led_status;
extern uint adc_x_raw;
extern uint adc_y_raw;
extern uint64_t green_led_on_time; // Timer para o LED de status de conexão
typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;
/* END */


/* FUNÇOES */
void pinos_start();
void gpio5_callback(uint gpio, uint32_t events);
void run_dns_lookup(MQTT_CLIENT_T *state);
void mqtt_run_test(MQTT_CLIENT_T *state);
void gpio_event_string(char *buf, uint32_t events);
void js();
void setup_pwm(uint gpio_pin);
void update_pwm(uint gpio_pin);
void pwm_led(uint gpio_pin, uint brilho);
/* END */
