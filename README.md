# Aplicações com Comunicação Sem Fio para IoT

## Autor

* **Aluno:** Robson Mesquita Gomes
* **Matrícula:** 202510110980872
* **Mentora:** Gabriela

## **Objetivos**
* **Objetivo 1:** Desenvolver aplicações IoT utilizando microcontroladores e protocolos de comunicação.
* **Objetivo 2:** Realizar práticas de aplicações IoT.

---

### **Descrição do Projeto**

Este repositório contém os códigos desenvolvidos para a prática de comunicação sem fio em aplicações de Internet das Coisas (IoT). O projeto utiliza a linguagem **C/C++** para a placa **BitDogLab** e o protocolo **MQTT** para interagir com o broker público **HiveMQ**.

### **Configuração Essencial**

Para que os códigos possam se conectar à sua rede Wi-Fi, é necessário modificar o arquivo `CMakeLists.txt` .

Substitua `WIFI` pelo seu Wi-Fi SSID e `SENHA` pela sua senha:

```c
set(WIFI_SSID "WIFI" CACHE INTERNAL "WiFi SSID")
set(WIFI_PASSWORD "SENHA" CACHE INTERNAL "WiFi password")
```

### **Funcionalidades Implementadas**

#### **1. Envio de Dados para Servidor (`pico_w/recve`)**

É possível enviar comandos à placa por meio do tópico `pico_w/recve`.

##### Ligar Led
```json
{"msg": "acender"}
```

##### Desligar Led
```json
{"msg": "apagar"}
```

##### Tocar som
```json
{"msg": "som"}
```

#### **2. Status (`pico_w/status`)**

Esse tópico retorna o status do led, posição do joystick e temperatura da placa.

##### Exemplo de mensagem
```json
{
  "msg_count": 34,
  "blue_led_status": "on",
  "alarm_active": false,
  "red_led_oscillating": true,
  "joystick_x": 2158,
  "joystick_y": 1979,
  "temperature_c": 1104502016
}
```

### **Como Usar**


1.  **Clone o repositório** para o seu computador.
2.  **Configure o Wi-Fi**: Configure `CMakeList.txt`.
5.  **Interaja com um Cliente MQTT**: Utilize um cliente como MQTTX para:
    *   **Visualizar** as mensagens enviadas pelo primeiro script no tópico `pico_w/status`.
    *   **Publicar** as mensagens no tópico `pico_w/recve`.

## Demonstração

### Vídeos

#### Youtube
[https://youtube.com/shorts/yA00s8v3-gQ?feature=share](https://youtube.com/shorts/yA00s8v3-gQ?feature=share)

### Imagens

#### BitDogLab
![Foto da placa BitDogLab com os componentes conectados](/demo/bitdoglab.jpeg)

#### MQTTX
![Screenshot do cliente MQTTX mostrando a comunicação no tópico bitdoglab/test](/demo/mqttx.png)