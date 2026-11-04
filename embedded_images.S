.section .rodata.embedded,"a"
.balign 16
.global _binary_person_jpg_start
_binary_person_jpg_start:
.incbin "C:/Users/erick/Documents/PlatformIO/Projects/ESP32_Test/images/person.jpg"
.global _binary_person_jpg_end
_binary_person_jpg_end:
.balign 16
.global _binary_person2_jpg_start
_binary_person2_jpg_start:
.incbin "C:/Users/erick/Documents/PlatformIO/Projects/ESP32_Test/images/person2.jpg"
.global _binary_person2_jpg_end
_binary_person2_jpg_end:
