#include <GunShot_Chainsaw_Detection_inferencing.h>
#include <SPI.h>
#include <LoRa.h>

// ------------------- LoRa Settings -------------------
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_CS    18
#define LORA_RST   23
#define LORA_IRQ   26
#define LORA_FREQ  865E6   // India region

String NODE_ID = "TS001";

// ------------------- EI Audio Buffers -------------------
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static signed short sampleBuffer[sample_buffer_size];

static bool debug_nn = false;

// ------------------- Forward Declarations -------------------
static bool microphone_inference_start(uint32_t n_samples);
static bool microphone_inference_record(void);
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);
static void audio_inference_callback(uint32_t n_bytes);

// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("TerraSono SLAVE - EdgeAI + LoRa TX");

    // ---------------- LoRa Init ----------------
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

    if (!LoRa.begin(LORA_FREQ)) {
        Serial.println("LoRa init failed!");
        while (1);
    }
    Serial.println("LoRa OK");

    // ---------------- Edge Impulse Init ----------------
    run_classifier_init();

    if (!microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE)) {
        Serial.println("ERR: Audio buffer fail");
        return;
    }

    Serial.println("Listening...");
}

// ------------------------------------------------------------
void loop() {

    if (!microphone_inference_record()) {
        Serial.println("ERR: Failed to record audio");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR ei_err = run_classifier_continuous(&signal, &result, debug_nn);

    if (ei_err != EI_IMPULSE_OK) {
        Serial.printf("Classifier Error %d\n", ei_err);
        return;
    }

    // ------------------- Event Detection -------------------
    float confidence_threshold = 0.70;
    const char* top_label = "none";
    float top_score = 0.0;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float score = result.classification[ix].value;
        if (score > top_score) {
            top_score = score;
            top_label = result.classification[ix].label;
        }
    }

    // ------------------- Only Send if Event -------------------
    if (top_score >= confidence_threshold) {
        
        unsigned long ts = millis();

        // Create JSON packet
        String json = "{";
        json += "\"node_id\":\"" + NODE_ID + "\",";
        json += "\"event\":\"" + String(top_label) + "\",";
        json += "\"confidence\":" + String(top_score, 2) + ",";
        json += "\"timestamp\":" + String(ts);
        json += "}";

        Serial.println("EVENT DETECTED:");
        Serial.println(json);

        // ------------------- Send over LoRa -------------------
        LoRa.beginPacket();
        LoRa.print(json);
        LoRa.endPacket();

        Serial.println("LoRa TX OK\n");
    }
}

//
// ------- AUDIO + EI SUPPORT FUNCTIONS (Unmodified from EI) -------
//

static void audio_inference_callback(uint32_t n_bytes)
{
    for(int i = 0; i < n_bytes>>1; i++) {
        inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

        if(inference.buf_count >= inference.n_samples) {
            inference.buf_select ^= 1;
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

static bool microphone_inference_record(void)
{
    if (inference.buf_ready == 1) {
        Serial.println("Buffer overrun!");
        return false;
    }

    while (inference.buf_ready == 0) {
        delay(1);
    }

    inference.buf_ready = 0;
    return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

// ------------------------------------------------------------
// I2S Initialization (same as your original code)
// ------------------------------------------------------------
static int i2s_init(uint32_t sampling_rate) {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = sampling_rate,
      .bits_per_sample = (i2s_bits_per_sample_t)16,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_I2S,
      .dma_buf_count = 8,
      .dma_buf_len = 512,
      .use_apll = false
  };
  i2s_pin_config_t pin_config = {
      .bck_io_num = 26,
      .ws_io_num  = 32,
      .data_out_num = -1,
      .data_in_num  = 33
  };

  esp_err_t ret = 0;
  ret = i2s_driver_install((i2s_port_t)1, &i2s_config, 0, NULL);
  ret |= i2s_set_pin((i2s_port_t)1, &pin_config);
  ret |= i2s_zero_dma_buffer((i2s_port_t)1);

  return ret;
}

static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    inference.buf_select = 0;
    inference.buf_count  = 0;
    inference.n_samples  = n_samples;
    inference.buf_ready  = 0;

    if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
        Serial.println("I2S Start Failed");
    }

    xTaskCreate(capture_samples, "CaptureSamples", 32 * 1024, (void*)sample_buffer_size, 10, NULL);
    return true;
}

static void capture_samples(void* arg)
{
  size_t bytes_read;
  uint32_t bytes_to_read = (uint32_t)arg;

  while (1) {
    i2s_read((i2s_port_t)1, (void*)sampleBuffer, bytes_to_read, &bytes_read, 100);

    for (int x = 0; x < bytes_to_read/2; x++) {
        sampleBuffer[x] = sampleBuffer[x] * 8;
    }

    audio_inference_callback(bytes_to_read);
  }
}

