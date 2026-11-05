#pragma once
#include "coco_classes.hpp"
#include "dl_detect_define.hpp"
#include "dl_image_preprocessor.hpp"
#include "dl_image_process.hpp"
#include "dl_tensor_base.hpp"
#include <map>
#include <string>

// Default Configuration
#define YOLO_TARGET_K 32
#define YOLO_CONF_THRESH 0.50f

// Class restriction. The model is a single-class ("person") detector, but its
// exported head still carries 80 class channels, so every grid cell is also
// scored against classes the model was never trained on (that is where the
// stray "bicycle"/"car"/"dog" hits came from). Only this class id is evaluated;
// set to -1 to scan every channel again.
#define YOLO_CLASS_FILTER 0

// Default Configuration

class YOLO26 {
private:
    // --- State (Calculated/Configured) ---
    std::vector<int> grid_sizes; // Calculated in preprocess
    int num_classes = 0;         // Calculated in postprocess
    int target_k;
    float conf_thresh;
    int class_filter;            // -1 = all classes, otherwise a single class id

    // --- Optimization ---
    // Official ESP-DL Image Preprocessor (Handles Resizing, Letterboxing, and SIMD Quantization)
    dl::image::ImagePreprocessor *m_image_preprocessor;

    // Constants
    const int strides[3] = {8, 16, 32};

    // --- Helpers ---

    // Dequantization Helper removed (using native esp-dl)

    /**
     * @brief Templated decoding loop to seamlessly handle both INT8 and INT16 tensors
     */
    template <typename T>
    void decode_grid(dl::TensorBase *p_box,
                     dl::TensorBase *p_cls,
                     int stride,
                     int grid_h,
                     int grid_w,
                     std::vector<dl::detect::result_t> &candidates);

public:
    const char **class_names;

    /**
     * @brief Constructor.
     * Initializes configuration state and configures the ESP-DL ImagePreprocessor.
     *
     * @param model Pointer to the loaded dl::Model
     * @param k Max detections (Default: 32)
     * @param thresh Confidence threshold (Default: 0.10f)
     * @param classes Class name array (Default: coco_classes)
     * @param class_id_filter Only this class id is evaluated (Default: YOLO_CLASS_FILTER, -1 = all)
     */
    YOLO26(dl::Model *model,
           int k = YOLO_TARGET_K,
           float thresh = YOLO_CONF_THRESH,
           const char **classes = coco_classes,
           int class_id_filter = YOLO_CLASS_FILTER);

    ~YOLO26();

    /**
     * @brief Decodes JPEG to RGB888.
     */
    dl::image::img_t decode_jpeg(const uint8_t *jpg_data, size_t jpg_len);

    /**
     * @brief Preprocesses image natively using ESP-DL (Letterbox + SIMD Quantization directly into Model RAM).
     *
     * @param img Input image (RGB888)
     */
    void preprocess(const dl::image::img_t &img);

    /**
     * @brief Post-processes outputs using stored state.
     *
     * OPTIMIZATION: Uses Integer Thresholding to skip floating point math.
     *
     * @param outputs Map of model outputs
     * @return std::vector<dl::detect::result_t>
     */
    std::vector<dl::detect::result_t> postprocess(const std::map<std::string, dl::TensorBase *> &outputs);
};