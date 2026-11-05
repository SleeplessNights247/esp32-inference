#include "dl_model_base.hpp" // Required for dl::Model
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "yolo26.hpp" // Use the Official Component
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "yolo26_detect";

// =========================================================================
// 🛠️ CLASS LABELS (Auto-selected by CMake based on MODEL_FILENAME)
// =========================================================================

#include "coco_classes.hpp"
const char **current_classes = coco_classes;
// =========================================================================

// --- Model Embedding ---
extern const uint8_t model_binary_start[] asm("_binary_yolo26n_512_s8_s3_espdl_start");

// --- Image Embedding ---
extern const uint8_t person_jpg_start[] asm("_binary_person_jpg_start");
extern const uint8_t person_jpg_end[] asm("_binary_person_jpg_end");
extern const uint8_t person2_jpg_start[] asm("_binary_person2_jpg_start");
extern const uint8_t person2_jpg_end[] asm("_binary_person2_jpg_end");

void test_inference(dl::Model *model, YOLO26 &processor, const uint8_t *jpg_data, size_t jpg_len, const char *name)
{
    ESP_LOGI("image:", "%s", name);

    // 1. Decode JPEG
    auto img = processor.decode_jpeg(jpg_data, jpg_len);

    // 2. Preprocess (Measure Time)
    // Automatically does letterboxing and fills model inputs natively
    int64_t start_pre = esp_timer_get_time();
    processor.preprocess(img);
    int64_t end_pre = esp_timer_get_time();

    // 3. Inference (Measure Time)
    int64_t start_inf = esp_timer_get_time();
    // Measured on this board: MULTI_CORE 5229 ms vs SINGLE_CORE 4490 ms.
    // The per-layer core hand-off costs more than the parallelism saves here,
    // so stay on the default single-core path.
    model->run();
    int64_t end_inf = esp_timer_get_time();

    // 4. Post-Process (Measure Time)
    int64_t start_post = esp_timer_get_time();
    auto results = processor.postprocess(model->get_outputs());
    int64_t end_post = esp_timer_get_time();

    // Calculate Latencies (µs → ms)
    uint32_t lat_pre = (uint32_t)((end_pre - start_pre) / 1000);
    uint32_t lat_inf = (uint32_t)((end_inf - start_inf) / 1000);
    uint32_t lat_post = (uint32_t)((end_post - start_post) / 1000);

    ESP_LOGI(TAG, "Pre: %lu ms | Inf: %lu ms | Post: %lu ms", lat_pre, lat_inf, lat_post);

    // Results arrive sorted by score, so instance ids are stable per image.
    int instance = 0;
    for (const auto &res : results) {
        // The postprocessor only scores class 0 ("person"); keep the bounds
        // check so a bad class id can never walk off the name table.
        const char *label = (res.category >= 0 && res.category < coco_classes_count)
                        ? current_classes[res.category] : "INVALID";
        ESP_LOGI("YOLO26",
                 "[category: %s, instance: %d, score: %.2f, x1: %d, y1: %d, x2: %d, y2: %d]",
                 label,
                 ++instance,
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }

    heap_caps_free(img.data);
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "YOLO26 Firmware Example");

    // 1. Load Model (Using the Magic Symbol from CMake)
    ESP_LOGI(TAG, "Loading Model from Flash...");
    dl::Model *model = new dl::Model((const char *)model_binary_start,
                                     fbs::MODEL_LOCATION_IN_FLASH_RODATA,
                                     0,
                                     dl::MEMORY_MANAGER_GREEDY,
                                     nullptr,
                                     true);

    // --- Diagnostics: dtype of the graph we actually embedded, and where memory landed ---
    ESP_LOGI(TAG, "Internal free: %u B | PSRAM free: %u B",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    for (auto &kv : model->get_inputs()) {
        dl::TensorBase *t = kv.second;
        ESP_LOGI(TAG, "input '%s': dtype=%d exponent=%d shape=[%d,%d,%d,%d]",
                 kv.first.c_str(), (int)t->dtype, (int)t->exponent,
                 t->shape[0], t->shape[1], t->shape[2], t->shape[3]);
    }

    // 2. Initialize Processor
    YOLO26 processor(model, YOLO_TARGET_K, YOLO_CONF_THRESH, current_classes);

    // 3. Run Tests
    test_inference(model, processor, person_jpg_start, (size_t)(person_jpg_end - person_jpg_start), "person.jpg");
    test_inference(model, processor, person2_jpg_start, (size_t)(person2_jpg_end - person2_jpg_start), "person2.jpg");


    delete model;
}