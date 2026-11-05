#include "coco_classes.hpp"

// Standard COCO 80-class names (Ultralytics order). The model's detection
// head outputs 80 class channels, so every id in [0, 80) has a real name.
const char *coco_classes[] = {
    "person",        "bicycle",       "car",           "motorcycle",   "airplane",
    "bus",           "train",         "truck",         "boat",         "traffic light",
    "fire hydrant",  "stop sign",     "parking meter", "bench",        "bird",
    "cat",           "dog",           "horse",         "sheep",        "cow",
    "elephant",      "bear",          "zebra",         "giraffe",      "backpack",
    "umbrella",      "handbag",       "tie",           "suitcase",     "frisbee",
    "skis",          "snowboard",     "sports ball",   "kite",         "baseball bat",
    "baseball glove", "skateboard",    "surfboard",     "tennis racket","bottle",
    "wine glass",    "cup",           "fork",          "knife",        "spoon",
    "bowl",          "banana",        "apple",         "sandwich",     "orange",
    "broccoli",      "carrot",        "hot dog",       "pizza",        "donut",
    "cake",          "chair",         "couch",         "potted plant", "bed",
    "dining table",  "toilet",        "tv",            "laptop",       "mouse",
    "remote",        "keyboard",      "cell phone",    "microwave",    "oven",
    "toaster",       "sink",          "refrigerator",  "book",         "clock",
    "vase",          "scissors",      "teddy bear",    "hair drier",   "toothbrush",
};

const int coco_classes_count = sizeof(coco_classes) / sizeof(coco_classes[0]);
