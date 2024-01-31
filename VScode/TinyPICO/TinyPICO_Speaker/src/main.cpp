#include <Arduino.h>
#include <SPI.h>
//#include <BluetoothSerial.h>
#include <WiFi.h>
#include "TinyPICO.h"
#include "BluetoothA2DPSink.h"

#define c3_frequency  130.81
//BluetoothSerial serialBT;
char cmd;
TinyPICO tp = TinyPICO();

BluetoothA2DPSink a2dp_sink;

int32_t get_data_frames(Frame *frame, int32_t frame_count);

void setup() {
  i2s_pin_config_t my_pin_config = {
      .mck_io_num = I2S_PIN_NO_CHANGE,
      .bck_io_num = 26,
      .ws_io_num = 25,
      .data_out_num = 22,
      .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);

  static i2s_config_t i2s_config = {
      .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100, // updated automatically by A2DP
      .bits_per_sample = (i2s_bits_per_sample_t)32,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = true,
      .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
  };
  a2dp_sink.set_i2s_config(i2s_config);

  a2dp_sink.set_auto_reconnect(false);
  a2dp_sink.start("TINYPICO", get_data_frames);  
  a2dp_sink.set_volume(30);


  tp.DotStar_SetPixelColor(0xFF0000);
  tp.DotStar_Show();

  

}


void loop() {


  
}



int32_t get_data_frames(Frame *frame, int32_t frame_count) {
    static float m_time = 0.0;
    float m_amplitude = 10000.0;  // -32,768 to 32,767
    float m_deltaTime = 1.0 / 44100.0;
    float m_phase = 0.0;
    float pi_2 = PI * 2.0;
    // fill the channel data
    for (int sample = 0; sample < frame_count; ++sample) {
        float angle = pi_2 * c3_frequency * m_time + m_phase;
        frame[sample].channel1 = m_amplitude * sin(angle);
        frame[sample].channel2 = frame[sample].channel1;
        m_time += m_deltaTime;
    }
    // to prevent watchdog
    delay(1);

    return frame_count;
}