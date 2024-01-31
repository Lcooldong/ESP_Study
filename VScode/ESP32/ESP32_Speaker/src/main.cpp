#include <Arduino.h>

#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

int volume = 0;
void volumeChanged(int newVolume) {
    Serial.println(newVolume);
    volume = newVolume;
}

// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void* ptr) {
    Serial.println(a2dp_sink.to_str(state));
}

// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void* ptr) {
    Serial.println(a2dp_sink.to_str(state));
}

void setup() {
    Serial.begin(115200);

// Not Working - Test 
// const i2s_config_t i2s_config = {
//         .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
//         .sample_rate = 44100, // corrected by info from bluetooth
//         .bits_per_sample = (i2s_bits_per_sample_t)16, /* the DAC module will only take the 8bits from MSB */
//         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//         .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
//         .intr_alloc_flags = 0, // default interrupt priority
// //        .dma_buf_count = 8,
// //        .dma_buf_len = 64,
//         .dma_buf_count = 4,
//         .dma_buf_len = 512,
//         .use_apll = false
//     };

    const i2s_pin_config_t pin_config = {
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
    .mck_io_num = 0,
    #endif
    .bck_io_num = 27,
    .ws_io_num = 14,    // LRCK
    .data_out_num = 26,
    .data_in_num = I2S_PIN_NO_CHANGE
    };

    a2dp_sink.set_pin_config(pin_config);
    a2dp_sink.set_on_volumechange(volumeChanged);
    a2dp_sink.set_on_connection_state_changed(connection_state_changed);
    a2dp_sink.set_on_audio_state_changed(audio_state_changed);

    a2dp_sink.start("MyMusic");  

}


void loop() {

}