set(srcs    main.cpp)

set(requires    esp-dl)

idf_build_get_property(component_targets __COMPONENT_TARGETS)
if ("___idf_espressif__esp-dl" IN_LIST component_targets)
   idf_component_get_property(espdl_dir espressif__esp-dl COMPONENT_DIR)
elseif("___idf_esp-dl" IN_LIST component_targets)
   idf_component_get_property(espdl_dir esp-dl COMPONENT_DIR)
endif()
set(cmake_dir ${espdl_dir}/fbs_loader/cmake)

if (IDF_TARGET STREQUAL "esp32s3")
    set(model_file "${CMAKE_CURRENT_SOURCE_DIR}/models/yolov5.espdl")
    get_filename_component(model_file "${model_file}" ABSOLUTE)
    file(TO_CMAKE_PATH "${model_file}" model_file_asm)
    set(model_asm "${CMAKE_CURRENT_BINARY_DIR}/yolov5_model.S")
    set(model_asm_content ".section .rodata.embedded,\"a\"\n.balign 16\n.global yolov5_espdl\nyolov5_espdl:\n.global _binary_yolov5_espdl_start\n_binary_yolov5_espdl_start:\n.incbin \"${model_file_asm}\"\n.global _binary_yolov5_espdl_end\n_binary_yolov5_espdl_end:\n.global _binary_yolov5_espdl_length\n_binary_yolov5_espdl_length:\n.long _binary_yolov5_espdl_end - _binary_yolov5_espdl_start\n")

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