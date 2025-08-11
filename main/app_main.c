
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2c.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define TAG "app"

#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define LED_GPIO GPIO_NUM_2
#define LAMP_GPIO GPIO_NUM_4

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static esp_mqtt_client_handle_t mqtt_client = NULL;
static uint8_t bh1750_addr = 0x23;
static bool mqtt_connected = false;
static char modoAtual[16] = "automatico";

// --- I2C ---
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK)
        return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static void i2c_scan(void)
{
    ESP_LOGI(TAG, "Scan I2C iniciando...");
    for (int addr = 1; addr < 127; addr++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Dispositivo encontrado no endereço 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "Scan I2C finalizado.");
}

// --- BH1750 ---
static esp_err_t bh1750_init()
{
    uint8_t cmd = 0x10;
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, bh1750_addr, &cmd, 1, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Falha inicializando BH1750 no endereço 0x%02X: %s", bh1750_addr, esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_LOGI(TAG, "BH1750 inicializado no endereço 0x%02X", bh1750_addr);
    return ESP_OK;
}

static esp_err_t bh1750_read(float *lux)
{
    uint8_t data[2];
    esp_err_t ret = i2c_master_read_from_device(I2C_MASTER_NUM, bh1750_addr, data, 2, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Erro lendo BH1750: %s", esp_err_to_name(ret));
        return ret;
    }
    uint16_t raw = (data[0] << 8) | data[1];
    *lux = raw / 1.2f;
    return ESP_OK;
}

// --- Wi-Fi ---
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WiFi desconectado, reconectando...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        gpio_set_level(LED_GPIO, 0);
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "WiFi conectado, IP obtido");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        gpio_set_level(LED_GPIO, 1);
    }
}

// --- MQTT ---
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT conectado");
        esp_mqtt_client_subscribe(mqtt_client, "ads/embarcados/unidade2/modo", 1);
        esp_mqtt_client_subscribe(mqtt_client, "ads/embarcados/unidade2/comando", 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        ESP_LOGI(TAG, "MQTT desconectado");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Mensagem recebida no tópico: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "Conteúdo: %.*s", event->data_len, event->data);

        if (strncmp(event->topic, "ads/embarcados/unidade2/modo", event->topic_len) == 0)
        {
            memset(modoAtual, 0, sizeof(modoAtual));
            int len = event->data_len < sizeof(modoAtual) - 1 ? event->data_len : sizeof(modoAtual) - 1;
            memcpy(modoAtual, event->data, len);
            modoAtual[len] = '\0';

            for (int i = 0; i < len; i++)
            {
                if (modoAtual[i] == '\r' || modoAtual[i] == '\n' || modoAtual[i] == ' ')
                {
                    modoAtual[i] = '\0';
                    break;
                }
            }

            ESP_LOGI(TAG, "🔄 Modo alterado para: %s", modoAtual);

            if (mqtt_connected)
            {
                esp_mqtt_client_publish(mqtt_client, "ads/embarcados/unidade2/status_modo", modoAtual, 0, 1, 0);
                ESP_LOGI(TAG, "📤 Modo publicado em status_modo");
            }
        }

        if (strncmp(event->topic, "ads/embarcados/unidade2/comando", event->topic_len) == 0)
        {
            if (strcmp(modoAtual, "manual") == 0)
            {
                if (strncmp(event->data, "on", event->data_len) == 0)
                {
                    gpio_set_level(LAMP_GPIO, 1);
                    ESP_LOGI(TAG, "💡 Lâmpada ligada via comando manual");
                }
                else if (strncmp(event->data, "off", event->data_len) == 0)
                {
                    gpio_set_level(LAMP_GPIO, 0);
                    ESP_LOGI(TAG, "💡 Lâmpada desligada via comando manual");
                }
            }
            else
            {
                ESP_LOGI(TAG, "⚠️ Comando ignorado: modo atual é automático");
            }
        }
        break;

    default:
        break;
    }
}

// --- WiFi Init ---
static esp_err_t wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Conectando ao WiFi SSID: %s...", CONFIG_EXAMPLE_WIFI_SSID);

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(10000));
    return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_FAIL;
}

// --- MAIN ---
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    gpio_reset_pin(LAMP_GPIO);
    gpio_set_direction(LAMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LAMP_GPIO, 0);

    ESP_ERROR_CHECK(i2c_master_init());
    i2c_scan();

    ESP_ERROR_CHECK(wifi_init_sta());

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    esp_err_t ret = bh1750_init();
    if (ret != ESP_OK)
    {
        bh1750_addr = 0x5C;
        ret = bh1750_init();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Falha ao inicializar BH1750 nos dois endereços");
        }
    }

    while (1)
    {
        if (ret == ESP_OK)
        {
            float lux;
            ret = bh1750_read(&lux);
            if (ret == ESP_OK)
            {
                ESP_LOGI(TAG, "Luminosidade: %.2f lux", lux);
                char msg[32];
                snprintf(msg, sizeof(msg), "%.2f", lux);

                if (mqtt_connected)
                {
                    esp_mqtt_client_publish(mqtt_client, "ads/embarcados/unidade2/lux", msg, 0, 1, 0);
                }
                else
                {
                    ESP_LOGW(TAG, "MQTT não conectado, mensagem não enviada");
                }

                ESP_LOGI(TAG, "Modo atual no loop: %s", modoAtual);
                if (strcmp(modoAtual, "automatico") == 0)
                {
                    if (lux < 50.0)
                    {
                        gpio_set_level(LAMP_GPIO, 1);
                        ESP_LOGI(TAG, "💡 Lâmpada ligada automaticamente");
                    }
                    else
                    {
                        gpio_set_level(LAMP_GPIO, 0);
                        ESP_LOGI(TAG, "💡 Lâmpada desligada automaticamente");
                    }
                }
            }
            else
            {
                ESP_LOGW(TAG, "Erro ao ler luminosidade");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}