#ifndef LEOPARD_EBD_DATA_H
#define LEOPARD_EBD_DATA_H
#include <stdint.h>


typedef enum {
    EBD_SIZE = 1,
    HEAD_CRC = 5,
    INDIVIDUAL_INF = 13,
    HOST_SELECT = 21,
    EXPOSURE_SP1 = 31,
    //EXPOSURE_SP2 = 34,
    FRAME_COUNT = 37,
    FRAME_ID = 40,
    FRAM_MASK = 43,
    COMPENSATION_LEVEL = 49,
    TEMPERATURE_INF = 53,
    ERR_CODE = 61,
    VOL = 89,
    ANALOG_GAIN_INF = 121,
    IMAGE_SIZE = 189,
    DRIVE_MODE = 197,
    SYNC_MODE = 201,
    WHITE_BALANCE_INF = 249,
}LEP_EBD_IDX_t;
#pragma pack (1)
typedef struct {
  uint16_t size;
}ebd_size_e01_t;

typedef struct {
  uint32_t crc;
}head_crc_e05_t;

typedef struct {
  uint16_t chip_id_e13;//e13-e14
  uint8_t undef_e15[2];//e15-e16
  uint8_t chip_id_e17;
  uint8_t chip_id_e18;
  uint16_t chip_id_e19;//e19-e20
  //uint16_t sensor_type_e225;//e255-e266
  //uint8_t chip_id_e226;//e266
}prv_inf_e13_t;


typedef struct {
    uint32_t framer_count;
}framer_count_e37_t;

typedef struct {
 uint8_t sp1[3];
 uint8_t sp2[3];
}exposure_e31_t;

typedef struct {
    uint16_t framer_id;
}framer_id_e41_t;

typedef struct {
  uint16_t temp1;
  uint16_t temp2;
}lep_temp_e53_t;

typedef struct {
  uint16_t level;
}comp_levl_e49_t;

typedef struct {
  uint32_t code_e61;
  uint8_t code_e66;
}lep_err_e61_t;

typedef struct {
    uint16_t vh;
    uint16_t vm;
    uint16_t vl;
}vol_e89_t;

typedef struct {
    uint16_t sp1h_e121;
    uint16_t sp1l_e123;
    uint16_t sp2h_e125;
    uint16_t sp2l_e127;
}anl_gain_e121_t;

typedef struct {
  uint16_t horizontal;
  uint16_t vertical;
}image_size_e189_t;

typedef struct {
    uint8_t mode;
}drive_mode_e197_t;

typedef struct {
    uint8_t method;//e201
    uint8_t op_mode;
}sync_mode_e201_t;

typedef struct {
 uint16_t cf0;//e249
 uint16_t cf1;//e251
 uint16_t cf2;//e253
 uint16_t cf3;//e255
}white_balance_e249_t;

typedef struct {
    ebd_size_e01_t size;
    head_crc_e05_t head_crc;
    prv_inf_e13_t indiv_inf;
    exposure_e31_t exposure;
    framer_count_e37_t frame_count;
    framer_id_e41_t frame_id;
    comp_levl_e49_t compensation;
    lep_temp_e53_t temper;
    lep_err_e61_t err_code;
    vol_e89_t vol;
    image_size_e189_t img_size;
    drive_mode_e197_t drive_mode;
    sync_mode_e201_t sync_mode;
    white_balance_e249_t white_banlance;
}lep_ebd_t;
#pragma pack()

#endif
