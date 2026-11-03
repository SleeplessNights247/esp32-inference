#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "dl_model_base.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

extern const uint8_t model_espdl[] asm("_binary_yolov5_espdl_start");

static const char *TAG = "yolov5";

// Prints a per-op-type table (calls, total time, %, average) plus the slowest layers.
static void print_op_summary(dl::Model *model, const char *label, size_t top_layers = 0)
{
    auto info = model->get_module_info();

    struct OpStat {
        std::string type;
        int calls = 0;
        uint64_t us = 0;
    };
    std::map<std::string, OpStat> by_type;
    struct Layer {
        std::string name, type;
        uint32_t us;
    };
    std::vector<Layer> layers;
    uint64_t total_us = 0;

    for (const auto &kv : info) {
        if (kv.first == "total") continue;
        OpStat &s = by_type[kv.second.type];
        s.type = kv.second.type;
        s.calls++;
        s.us += kv.second.latency;
        total_us += kv.second.latency;
        Layer layer;
        layer.name = kv.first;
        layer.type = kv.second.type;
        layer.us = kv.second.latency;
        layers.push_back(layer);
    }

    std::vector<OpStat> rows;
    for (auto &kv : by_type) rows.push_back(kv.second);
    std::sort(rows.begin(), rows.end(), [](const OpStat &a, const OpStat &b) { return a.us > b.us; });

    printf("\n%s per-op timing | Summed ops: %llu us (%.2f ms)\n",
           label, (unsigned long long)total_us, total_us / 1000.0);
    printf("--- Per-op profile (us, one invoke) ---\n");
    for (const auto &r : rows) {
        printf("%-24s %3d calls %10llu us\n", r.type.c_str(), r.calls, (unsigned long long)r.us);
    }
    if (top_layers == 0) {
        printf("\n");
        return;
    }

    std::sort(layers.begin(), layers.end(), [](const Layer &a, const Layer &b) { return a.us > b.us; });
    printf("\n--- Slowest %u layers ---\n", (unsigned)std::min(top_layers, layers.size()));
    for (size_t i = 0; i < std::min(top_layers, layers.size()); i++) {
        printf("%10u us %5.1f%%  %-16s %s\n", (unsigned)layers[i].us,
               total_us ? 100.0 * layers[i].us / total_us : 0.0,
               layers[i].type.c_str(), layers[i].name.c_str());
    }
    printf("\n");
}

// Whole-invoke latency for a given runtime mode (average of n runs after one warm-up).
static void time_invoke(dl::Model *model, dl::runtime_mode_t mode, const char *label, int n = 3)
{
    model->run(mode);  // warm-up
    int64_t t0 = esp_timer_get_time();
    for (int i = 0; i < n; i++) model->run(mode);
    int64_t us = (esp_timer_get_time() - t0) / n;
    printf("%-14s Invoke: %lld us (%.2f ms)\n", label, (long long)us, us / 1000.0);
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Application started; loading YOLOv5 model from flash");

    // Keep parameter in FLASH, saves PSRAM/internal RAM, lower performance.
    dl::Model *model = new dl::Model((const char *)model_espdl,
                                     fbs::MODEL_LOCATION_IN_FLASH_RODATA,
                                     0,
                                     dl::MEMORY_MANAGER_GREEDY,
                                     nullptr,
                                     true);

    ESP_LOGI(TAG, "Model loaded; running embedded inference test");
    ESP_ERROR_CHECK(model->test());
    ESP_LOGI(TAG, "Embedded inference test passed");

    model->profile_memory(); 
    printf("\n");
    time_invoke(model, dl::RUNTIME_MODE_SINGLE_CORE, "Core: 1");
    time_invoke(model, dl::RUNTIME_MODE_MULTI_CORE, "Core: 2");
    print_op_summary(model, "Cores: 1");

    delete model;
}