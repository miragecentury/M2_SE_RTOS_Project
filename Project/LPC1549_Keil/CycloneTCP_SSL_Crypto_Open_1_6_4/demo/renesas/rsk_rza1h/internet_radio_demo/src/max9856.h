/**
 * @file max9856.h
 * @brief MAX9856 audio CODEC
 *
 * @section License
 *
 * Copyright (C) 2010-2015 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.6.4
 **/

#ifndef _MAX9856_H
#define _MAX9856_H

//Dependencies
#include "os_port.h"
#include "error.h"

//MAX9856 slave address
#define MAX9856_ADDR 0x10

//MAX9856 register
#define MAX9856_REG_STATUS1         0x00
#define MAX9856_REG_STATUS2         0x01
#define MAX9856_REG_INT_EN          0x02
#define MAX9856_REG_CLK_CTRL        0x03
#define MAX9856_REG_DAC_SYS         0x04
#define MAX9856_REG_DAC_IF1         0x05
#define MAX9856_REG_DAC_IF2         0x06
#define MAX9856_REG_ADC_SYS         0x07
#define MAX9856_REG_ADC_IF1         0x08
#define MAX9856_REG_ADC_IF2         0x09
#define MAX9856_REG_ADC_LEVEL       0x0A
#define MAX9856_REG_HP_FILT         0x0B
#define MAX9856_REG_AGC_CTRL        0x0C
#define MAX9856_REG_AGC_THRES       0x0D
#define MAX9856_REG_ADC_MIXER1      0x0E
#define MAX9856_REG_ADC_MIXER2      0x0F
#define MAX9856_REG_OUT_MIXER       0x10
#define MAX9856_REG_DIN_GAIN        0x11
#define MAX9856_REG_AUXIN_GAIN      0x12
#define MAX9856_REG_LINEIN1_GAIN    0x13
#define MAX9856_REG_LINEIN2_GAIN    0x14
#define MAX9856_REG_MICL_GAIN       0x15
#define MAX9856_REG_MICR_GAIN       0x16
#define MAX9856_REG_MIC_MODE        0x17
#define MAX9856_REG_HPL_VOL         0x18
#define MAX9856_REG_HPR_VOL         0x19
#define MAX9856_REG_OUT_MODE        0x1A
#define MAX9856_REG_HEAD_DETECT_SYS 0x1B
#define MAX9856_REG_PM_SYS          0x1C

//STATUS1 register
#define STATUS1_CLD                 0x80
#define STATUS1_SLD                 0x40
#define STATUS1_ULK                 0x20
#define STATUS1_JKMIC               0x10
#define STATUS1_HPOCL               0x08
#define STATUS1_HPOCR               0x04
#define STATUS1_JDET                0x02
#define STATUS1_GPI                 0x01

//STATUS2 register
#define STATUS2_LSNS                0x80
#define STATUS2_KSNS                0x40
#define STATUS2_HSDETL              0x03
#define STATUS2_HSDETR              0xC0
#define STATUS2_JSDET               0x03

//INT_EN register
#define INT_EN_ICLD                 0x80
#define INT_EN_ISLD                 0x40
#define INT_EN_IULK                 0x20
#define INT_EN_IHPOCL               0x08
#define INT_EN_IHPOCR               0x04
#define INT_EN_IJDET                0x02
#define INT_EN_IGPI                 0x01

//CLK_CTRL register
#define LK_CTRL_PSCLK               0x70
#define LK_CTRL_MAS                 0x08
#define LK_CTRL_BSEL                0x07

#define LK_CTRL_PSCLK_DIS           0x00
#define LK_CTRL_PSCLK_10_16MHZ      0x10
#define LK_CTRL_PSCLK_16_20MHZ      0x20
#define LK_CTRL_PSCLK_20_32MHZ      0x30
#define LK_CTRL_PSCLK_32_40MHZ      0x40
#define LK_CTRL_PSCLK_40_60MHZ      0x50

#define LK_CTRL_BSEL_OFF            0x00
#define LK_CTRL_BSEL_48_LRCLK_D     0x02
#define LK_CTRL_BSEL_48_LRCLK_A     0x03
#define LK_CTRL_BSEL_PCLK_DIV_2     0x04
#define LK_CTRL_BSEL_PCLK_DIV_4     0x05
#define LK_CTRL_BSEL_PCLK_DIV_8     0x06
#define LK_CTRL_BSEL_PCLK_DIV_16    0x07

//DAC_SYS register
#define DAC_SYS_DWCI                0x80
#define DAC_SYS_DBCI                0x40
#define DAC_SYS_DRATE               0x30
#define DAC_SYS_DDLY                0x08
#define DAC_SYS_PCM                 0x04
#define DAC_SYS_DHF                 0x02
#define DAC_SYS_WS                  0x01

#define DAC_SYS_DRATE_LOW           0x00
#define DAC_SYS_DRATE_HIGH          0x20
#define DAC_SYS_DRATE_DIS           0x30

//DAC_IF1 register
#define DAC_IF1_DPLLEN              0x80
#define DAC_IF1_DACNI_H             0x7F

//DAC_IF2 register
#define DAC_IF2_DACNI_L             0xFF

//ADC_SYS register
#define ADC_SYS_AWCI                0x80
#define ADC_SYS_ABCI                0x40
#define ADC_SYS_APIN                0x30
#define ADC_SYS_ADLY                0x80

#define ADC_SYS_APIN_IN             0x00
#define ADC_SYS_APIN_WORD_CLK       0x10
#define ADC_SYS_APIN_OUT_LOW        0x20
#define ADC_SYS_APIN_OUT_HIGH       0x30

//ADC_IF1 register
#define ADC_IF1_APLLEN              0x80
#define ADC_IF1_ADCNI_H             0x7F

//ADC_IF2 register
#define ADC_IF2_ADCNI_L             0xFF

//ADC_LEVEL register
#define ADC_LEVEL_AGAIN             0xF0
#define ADC_LEVEL_ANTH              0x0F

#define ADC_LEVEL_AGAIN_3DB         0x00
#define ADC_LEVEL_AGAIN_2DB         0x10
#define ADC_LEVEL_AGAIN_1DB         0x20
#define ADC_LEVEL_AGAIN_0DB         0x30
#define ADC_LEVEL_AGAIN_M_1DB       0x40
#define ADC_LEVEL_AGAIN_M_2DB       0x50
#define ADC_LEVEL_AGAIN_M_3DB       0x60
#define ADC_LEVEL_AGAIN_M_4DB       0x70
#define ADC_LEVEL_AGAIN_M_5DB       0x80
#define ADC_LEVEL_AGAIN_M_6DB       0x90
#define ADC_LEVEL_AGAIN_M_7DB       0xA0
#define ADC_LEVEL_AGAIN_M_8DB       0xB0
#define ADC_LEVEL_AGAIN_M_9DB       0xC0
#define ADC_LEVEL_AGAIN_M_10DB      0xD0
#define ADC_LEVEL_AGAIN_M_11DB      0xE0
#define ADC_LEVEL_AGAIN_M_12DB      0xF0

#define ADC_LEVEL_ANTH_DIS          0x00
#define ADC_LEVEL_ANTH_M_64DB       0x06
#define ADC_LEVEL_ANTH_M_60DB       0x07
#define ADC_LEVEL_ANTH_M_56DB       0x08
#define ADC_LEVEL_ANTH_M_52DB       0x09
#define ADC_LEVEL_ANTH_M_48DB       0x0A
#define ADC_LEVEL_ANTH_M_44DB       0x0B
#define ADC_LEVEL_ANTH_M_40DB       0x0C
#define ADC_LEVEL_ANTH_M_36DB       0x0D
#define ADC_LEVEL_ANTH_M_32DB       0x0E
#define ADC_LEVEL_ANTH_M_28DB       0x0F

//HP_FILT register
#define HP_FILT_ADCHP               0x70
#define HP_FILT_DACHP               0x07

//AGC_CTRL register
#define AGC_CTRL_AGCRLS             0x70
#define AGC_CTRL_AGCATK             0x0C
#define AGC_CTRL_AGCHLD             0x03

#define AGC_CTRL_AGCRLS_78MS        0x00
#define AGC_CTRL_AGCRLS_156MS       0x10
#define AGC_CTRL_AGCRLS_312MS       0x20
#define AGC_CTRL_AGCRLS_625MS       0x30
#define AGC_CTRL_AGCRLS_1_25S       0x40
#define AGC_CTRL_AGCRLS_2_5S        0x50
#define AGC_CTRL_AGCRLS_5S          0x60
#define AGC_CTRL_AGCRLS_10S         0x70

#define AGC_CTRL_AGCATK_3MS         0x00
#define AGC_CTRL_AGCATK_12MS        0x04
#define AGC_CTRL_AGCATK_50MS        0x08
#define AGC_CTRL_AGCATK_200MS       0x0C

#define AGC_CTRL_AGCHLD_DIS         0x00
#define AGC_CTRL_AGCHLD_50MS        0x01
#define AGC_CTRL_AGCHLD_100MS       0x02
#define AGC_CTRL_AGCHLD_400MS       0x03

//AGC_THRES register
#define AGC_THRES_AGCSRC            0x10
#define AGC_THRES_AGCSTH            0x0F

#define AGC_THRES_AGCSTH_M_3DB      0x00
#define AGC_THRES_AGCSTH_M_4DB      0x01
#define AGC_THRES_AGCSTH_M_5DB      0x02
#define AGC_THRES_AGCSTH_M_6DB      0x03
#define AGC_THRES_AGCSTH_M_7DB      0x04
#define AGC_THRES_AGCSTH_M_8DB      0x05
#define AGC_THRES_AGCSTH_M_9DB      0x06
#define AGC_THRES_AGCSTH_M_10DB     0x07
#define AGC_THRES_AGCSTH_M_11DB     0x08
#define AGC_THRES_AGCSTH_M_12DB     0x09
#define AGC_THRES_AGCSTH_M_13DB     0x0A
#define AGC_THRES_AGCSTH_M_14DB     0x0B
#define AGC_THRES_AGCSTH_M_15DB     0x0C
#define AGC_THRES_AGCSTH_M_16DB     0x0D
#define AGC_THRES_AGCSTH_M_17DB     0x0E
#define AGC_THRES_AGCSTH_M_18DB     0x0F

//ADC_MIXER1 register
#define ADC_MIXER1_MXINL            0x1F

#define ADC_MIXER1_MXINL_NONE       0x00
#define ADC_MIXER1_MXINL_AUXOUT     0x10
#define ADC_MIXER1_MXINL_LINEIN1    0x08
#define ADC_MIXER1_MXINL_LINEIN2    0x04
#define ADC_MIXER1_MXINL_MICL       0x02
#define ADC_MIXER1_MXINL_MICR       0x01

//ADC_MIXER2 register
#define ADC_MIXER2_MXINR            0x1F

#define ADC_MIXER2_MXINR_NONE       0x00
#define ADC_MIXER2_MXINR_AUXOUT     0x10
#define ADC_MIXER2_MXINR_LINEIN1    0x08
#define ADC_MIXER2_MXINR_LINEIN2    0x04
#define ADC_MIXER2_MXINR_MICL       0x02
#define ADC_MIXER2_MXINR_MICR       0x01

//OUT_MIXER register
#define OUT_MIXER_MXOUTL            0xF0
#define OUT_MIXER_MXOUTR            0x0F

#define OUT_MIXER_MXOUTL_NONE       0x00
#define OUT_MIXER_MXOUTL_MIC_LR_PGA 0x80
#define OUT_MIXER_MXOUTL_LINEIN1    0x40
#define OUT_MIXER_MXOUTL_LINEIN2    0x20
#define OUT_MIXER_MXOUTL_DAC_OUT    0x10

#define OUT_MIXER_MXOUTR_NONE       0x00
#define OUT_MIXER_MXOUTR_MIC_LR_PGA 0x08
#define OUT_MIXER_MXOUTR_LINEIN1    0x04
#define OUT_MIXER_MXOUTR_LINEIN2    0x02
#define OUT_MIXER_MXOUTR_DAC_OUT    0x01

//DIN_GAIN register
#define DIN_GAIN_PGADS              0xFF

#define DIN_GAIN_PGADS_0DB          0x00
#define DIN_GAIN_PGADS_M_40DB       0xE5

//AUXIN_GAIN register
#define AUXIN_GAIN_PGAAUX           0x1F

#define AUXIN_GAIN_PGAAUX_30DB      0x00
#define AUXIN_GAIN_PGAAUX_0DB       0x0F
#define AUXIN_GAIN_PGAAUX_M_32DB    0x1F

//LINEIN1_GAIN register
#define LINEIN1_GAIN_PGAL1          0x1F

#define LINEIN1_GAIN_PGAL1_30DB     0x00
#define LINEIN1_GAIN_PGAL1_0DB      0x0F
#define LINEIN1_GAIN_PGAL1_M_32DB   0x1F

//LINEIN2_GAIN register
#define LINEIN2_GAIN_PGAL2          0x1F

#define LINEIN2_GAIN_PGAL2_30DB     0x00
#define LINEIN2_GAIN_PGAL2_0DB      0x0F
#define LINEIN2_GAIN_PGAL2_M_32DB   0x1F

//MICL_GAIN register
#define MICL_GAIN_PAENL             0x60
#define MICL_GAIN_PGAML             0x1F

#define MICL_GAIN_PAENL_DIS         0x00
#define MICL_GAIN_PAENL_0DB         0x20
#define MICL_GAIN_PAENL_20DB        0x40
#define MICL_GAIN_PAENL_30DB        0x60

#define MICL_GAIN_PGAML_20DB        0x00
#define MICL_GAIN_PGAML_0DB         0x14

//MICR_GAIN register
#define MICR_GAIN_PAENR             0x60
#define MICR_GAIN_PGAMR             0x1F

#define MICR_GAIN_PAENR_DIS         0x00
#define MICR_GAIN_PAENR_0DB         0x20
#define MICR_GAIN_PAENR_20DB        0x40
#define MICR_GAIN_PAENR_30DB        0x60

#define MICR_GAIN_PGAMR_20DB        0x00
#define MICR_GAIN_PGAMR_0DB         0x14

//MIC_MODE register
#define MIC_MODE_MMIC               0x08
#define MIC_MODE_MBSEL              0x04
#define MIC_MODE_LMICDIF            0x01

//HPL_VOL register
#define HPL_VOL_HPMUTE              0x40
#define HPL_VOL_HPVOLL              0x3F

#define HPL_VOL_HPVOLL_5_5DB        0x00
#define HPL_VOL_HPVOLL_0DB          0x09
#define HPL_VOL_HPVOLL_M_74DB       0x27
#define HPL_VOL_HPVOLL_MUTE         0x3F

//HPR_VOL register
#define HPR_VOL_HPVOLR              0x3F

#define HPR_VOL_HPVOLR_5_5DB        0x00
#define HPR_VOL_HPVOLR_0DB          0x09
#define HPR_VOL_HPVOLR_M_74DB       0x27
#define HPR_VOL_HPVOLR_MUTE         0x3F

//OUT_MODE register
#define OUT_MODE_VSEN               0x40
#define OUT_MODE_AUXDC              0x20
#define OUT_MODE_AUXMIX             0x10
#define OUT_MODE_HPMODE             0x03

#define OUT_MODE_HPMODE_SHUTDOWN    0x00
#define OUT_MODE_HPMODE_STD_MONO    0x01
#define OUT_MODE_HPMODE_DUAL_MONO   0x02
#define OUT_MODE_HPMODE_STEREO      0x03

//HEAD_DETECT_SYS register
#define HEAD_DETECT_SYS_JDETEN      0x08
#define HEAD_DETECT_SYS_EN          0x07

#define HEAD_DETECT_SYS_EN_DIS      0x00
#define HEAD_DETECT_SYS_EN_JACKSNS  0x04
#define HEAD_DETECT_SYS_EN_HPR      0x02
#define HEAD_DETECT_SYS_EN_HPL      0x01

//PM_SYS register
#define PM_SYS_SHDN                 0x80
#define PM_SYS_DIGEN                0x20
#define PM_SYS_LOUTEN               0x10
#define PM_SYS_DALEN                0x08
#define PM_SYS_DAREN                0x04
#define PM_SYS_ADLEN                0x02
#define PM_SYS_ADREN                0x01

//MAX9856 related functions
error_t max9856Init(void);

error_t max9856SetVolume(uint_t left, uint_t right);

error_t max9856WriteReg(uint8_t address, uint8_t data);
error_t max9856ReadReg(uint8_t address, uint8_t *data);

error_t max9856DumpReg(void);

#endif
