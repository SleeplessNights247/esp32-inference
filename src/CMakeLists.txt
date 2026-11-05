set(srcs    main.cpp
            yolo26.cpp
            coco_classes.cpp)

set(requires    esp-dl)

set(image_asm "${CMAKE_CURRENT_BINARY_DIR}/embedded_images.S")
set(image_asm_content ".section .rodata.embedded,\"a\"\n")
foreach(image_file images/person.jpg images/person2.jpg)
    set(image_path "${CMAKE_CURRENT_SOURCE_DIR}/${image_file}")
    get_filename_component(image_name "${image_file}" NAME)
    string(REPLACE "." "_" image_symbol "${image_name}")
    file(TO_CMAKE_PATH "${image_path}" image_path_asm)
    string(APPEND image_asm_content
        ".balign 16\n.global _binary_${image_symbol}_start\n_binary_${image_symbol}_start:\n.incbin \"${image_path_asm}\"\n.global _binary_${image_symbol}_end\n_binary_${image_symbol}_end:\n")
endforeach()

if(EXISTS "${image_asm}")
    file(READ "${image_asm}" existing_image_asm_content)
endif()
if(NOT "${existing_image_asm_content}" STREQUAL "${image_asm_content}")
    file(WRITE "${image_asm}" "${image_asm_content}")
endif()
list(APPEND srcs "${image_asm}")
set_property(SOURCE "${image_asm}" APPEND PROPERTY OBJECT_DEPENDS
             "${CMAKE_CURRENT_SOURCE_DIR}/images/person.jpg"
             "${CMAKE_CURRENT_SOURCE_DIR}/images/person2.jpg")

idf_build_get_property(component_targets __COMPONENT_TARGETS)
if ("___idf_espressif__esp-dl" IN_LIST component_targets)
   idf_component_get_property(espdl_dir espressif__esp-dl COMPONENT_DIR)
elseif("___idf_esp-dl" IN_LIST component_targets)
   idf_component_get_property(espdl_dir esp-dl COMPONENT_DIR)
endif()

if (IDF_TARGET STREQUAL "esp32s3")
    set(model_file "${CMAKE_CURRENT_SOURCE_DIR}/models/yolo26n_512_s8_s3.espdl")
    get_filename_component(model_file "${model_file}" ABSOLUTE)
    file(TO_CMAKE_PATH "${model_file}" model_file_asm)
    set(model_asm "${CMAKE_CURRENT_BINARY_DIR}/yolo26n_512_s8_s3_model.S")
    set(model_asm_content ".section .rodata.embedded,\"a\"\n.balign 16\n.global yolo26n_512_s8_s3_espdl\nyolo26n_512_s8_s3_espdl:\n.global _binary_yolo26n_512_s8_s3_espdl_start\n_binary_yolo26n_512_s8_s3_espdl_start:\n.incbin \"${model_file_asm}\"\n.global _binary_yolo26n_512_s8_s3_espdl_end\n_binary_yolo26n_512_s8_s3_espdl_end:\n.global _binary_yolo26n_512_s8_s3_espdl_length\n_binary_yolo26n_512_s8_s3_espdl_length:\n.long _binary_yolo26n_512_s8_s3_espdl_end - _binary_yolo26n_512_s8_s3_espdl_start\n")

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${model_file}")
    if(EXISTS "${model_asm}")
        file(READ "${model_asm}" existing_model_asm_content)
    endif()
    if(NOT "${existing_model_asm_content}" STREQUAL "${model_asm_content}")
        file(WRITE "${model_asm}" "${model_asm_content}")
    endif()

    set_property(SOURCE "${model_asm}" APPEND PROPERTY OBJECT_DEPENDS "${model_file}")
    list(APPEND srcs "${model_asm}")
endif()

idf_component_register(SRCS ${srcs}
                       REQUIRES ${requires})