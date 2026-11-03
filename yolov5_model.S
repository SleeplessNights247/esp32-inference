.section .rodata.embedded,"a"
.balign 16
.global yolov5_espdl
yolov5_espdl:
.global _binary_yolov5_espdl_start
_binary_yolov5_espdl_start:
.incbin "C:/Users/erick/Documents/PlatformIO/Projects/ESP32_Test/models/yolov5.espdl"
.global _binary_yolov5_espdl_end
_binary_yolov5_espdl_end:
.global _binary_yolov5_espdl_length
_binary_yolov5_espdl_length:
.long _binary_yolov5_espdl_end - _binary_yolov5_espdl_start
