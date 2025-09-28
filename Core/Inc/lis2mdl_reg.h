/*
 * lis2mdl_reg.h
 *
 *      Author: Tetramad
 */

#ifndef __LIS2MDL_REG_H
#define __LIS2MDL_REG_H

#define SLAVE_ADDRESS (0x1E << 1)

#define CFG_REG_A (0x60)

#define CFG_REG_A_COMP_TEMP_EN_Pos (7)
#define CFG_REG_A_COMP_TEMP_EN_Msk (0x1U << CFG_REG_A_COMP_TEMP_EN_Pos)
#define CFG_REG_A_COMP_TEMP_EN CFG_REG_A_COMP_TEMP_EN_Msk

#define CFG_REG_A_MD_Pos (0)
#define CFG_REG_A_MD_Msk (0x3U << CFG_REG_A_MD_Pos)
#define CFG_REG_A_MD_0 (0x1U << CFG_REG_A_MD_Pos)
#define CFG_REG_A_MD_1 (0x2U << CFG_REG_A_MD_Pos)

#define CFG_REG_B (0x61)

#define CFG_REG_B_LPF_Pos (0)
#define CFG_REG_B_LPF_Msk (0x1U << CFG_REG_B_LPF_Pos)
#define CFG_REG_B_LPF CFG_REG_B_LPF_Msk

#define CFG_REG_C (0x62)

#define CFG_REG_C_BDU_Pos (4)
#define CFG_REG_C_BDU_Msk (0x1U << CFG_REG_C_BDU_Pos)
#define CFG_REG_C_BDU CFG_REG_C_BDU_Msk

#define STATUS_REG (0x67)

#define STATUS_REG_ZYXDA_Pos (3)
#define STATUS_REG_ZYXDA_Msk (0x1U << STATUS_REG_ZYXDA_Pos)
#define STATUS_REG_ZYXDA STATUS_REG_ZYXDA_Msk

#define OUTX_L_REG (0x68)
#define OUTX_H_REG (0x69)
#define OUTY_L_REG (0x6A)
#define OUTY_H_REG (0x6B)
#define OUTZ_L_REG (0x6C)
#define OUTZ_H_REG (0x6D)
#define TEMP_OUT_L_REG (0x6E)
#define TEMP_OUT_H_REG (0x6F)

#endif /* __LIS2MDL_REG_H */
