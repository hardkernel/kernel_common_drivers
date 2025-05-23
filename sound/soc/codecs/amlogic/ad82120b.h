/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ad82120b.h  --  ad82120b ALSA SoC Audio driver
 *
 * Copyright 1998 Elite Semiconductor Memory Technology
 *
 * Author: ESMT Audio/Power Product BU Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __AD82120B_H__
#define __AD82120B_H__

#define AD82120B_REGISTER_COUNT				166
#define AD82120B_RAM1_TABLE_COUNT			162
#define AD82120B_RAM2_TABLE_COUNT			159

/* Register Address Map */
#define AD82120B_STATE_CTRL1_REG	0x00
#define AD82120B_STATE_CTRL2_REG	0x01
#define AD82120B_STATE_CTRL3_REG	0x02
#define AD82120B_VOLUME_CTRL_REG	0x03
#define AD82120B_STATE_CTRL4_REG	0x0C
#define AD82120B_STATE_CTRL5_REG	0x1A

#define CFADDR    0x1d
#define A1CF1     0x1e
#define A1CF2     0x1f
#define A1CF3     0x20
#define A1CF4     0x21
#define CFUD      0x32

#define AD82120B_DEVICE_ID_REG	0x37

#define AD82120B_CLK_DET_CTRL	0x56

#define AD82120B_FAULT_REG		0x4D
#define AD82120B_MAX_REG		0xA5

/* AD82120B_STATE_CTRL2_REG */
#define AD82120B_SSZ_DS			BIT(5)

/* AD82120B_STATE_CTRL1_REG */
#define AD82120B_SAIF_I2S		(0x0 << 5)
#define AD82120B_SAIF_LEFTJ		(0x1 << 5)
#define AD82120B_SAIF_FORMAT_MASK	GENMASK(7, 5)

/* AD82120B_STATE_CTRL3_REG */
#define AD82120B_MUTE			BIT(6)

/* AD82120B_STATE_CTRL5_REG */
#define AD82120B_SW_RESET			BIT(5)

/* AD82120B_CLK_DET_CTRL */
#define AD82120B_ASR_DET			BIT(7)

/* AD82120B_DEVICE_ID_REG */
#define AD82120B_DEVICE_ID 0xA0

/* AD82120B_STATE_CTRL4_REG */
#define AD82120B_CHIP_SYNC_EN			BIT(1)

/* AD82120B_FAULT_REG */
#define AD82120B_OCE			BIT(7)
#define AD82120B_OTE			BIT(6)
#define AD82120B_UVE			BIT(5)
#define AD82120B_DCDE			BIT(4)
#define AD82120B_BSUVE			BIT(3)
#define AD82120B_CLKE			BIT(2)
#define AD82120B_OVPE			BIT(1)
#define AD82120B_D_CLKE			BIT(0)

const u32 ad82120b_volume[] = {
	0xE6,		//0, -103dB
	0xE5,		//1, -102.5dB
	0xE4,		//2, -102dB
	0xE3,		//3, -101.5dB
	0xE2,		//4, -101dB
	0xE1,		//5, -100.5dB
	0xE0,		//6, -100dB
	0xDF,		//7, -99.5dB
	0xDE,		//8, -99dB
	0xDD,		//9, -98.5dB
	0xDC,		//10, -98dB
	0xDB,		//11, -97.5dB
	0xDA,		//12, -97dB
	0xD9,		//13, -96.5dB
	0xD8,		//14, -96dB
	0xD7,		//15, -95.5dB
	0xD6,		//16, -95dB
	0xD5,		//17, -94.5dB
	0xD4,		//18, -94dB
	0xD3,		//19, -93.5dB
	0xD2,		//20, -93dB
	0xD1,		//21, -92.5dB
	0xD0,		//22, -92dB
	0xCF,		//23, -91.5dB
	0xCE,		//24, -91dB
	0xCD,		//25, -90.5dB
	0xCC,		//26, -90dB
	0xCB,		//27, -89.5dB
	0xCA,		//28, -89dB
	0xC9,		//29, -88.5dB
	0xC8,		//30, -88dB
	0xC7,		//31, -87.5dB
	0xC6,		//32, -87dB
	0xC5,		//33, -86.5dB
	0xC4,		//34, -86dB
	0xC3,		//35, -85.5dB
	0xC2,		//36, -85dB
	0xC1,		//37, -84.5dB
	0xC0,		//38, -84dB
	0xBF,		//39, -83.5dB
	0xBE,		//40, -83dB
	0xBD,		//41, -82.5dB
	0xBC,		//42, -82dB
	0xBB,		//43, -81.5dB
	0xBA,		//44, -81dB
	0xB9,		//45, -80.5dB
	0xB8,		//46, -80dB
	0xB7,		//47, -79.5dB
	0xB6,		//48, -79dB
	0xB5,		//49, -78.5dB
	0xB4,		//50, -78dB
	0xB3,		//51, -77.5dB
	0xB2,		//52, -77dB
	0xB1,		//53, -76.5dB
	0xB0,		//54, -76dB
	0xAF,		//55, -75.5dB
	0xAE,		//56, -75dB
	0xAD,		//57, -74.5dB
	0xAC,		//58, -74dB
	0xAB,		//59, -73.5dB
	0xAA,		//60, -73dB
	0xA9,		//61, -72.5dB
	0xA8,		//62, -72dB
	0xA7,		//63, -71.5dB
	0xA6,		//64, -71dB
	0xA5,		//65, -70.5dB
	0xA4,		//66, -70dB
	0xA3,		//67, -69.5dB
	0xA2,		//68, -69dB
	0xA1,		//69, -68.5dB
	0xA0,		//70, -68dB
	0x9F,		//71, -67.5dB
	0x9E,		//72, -67dB
	0x9D,		//73, -66.5dB
	0x9C,		//74, -66dB
	0x9B,		//75, -65.5dB
	0x9A,		//76, -65dB
	0x99,		//77, -64.5dB
	0x98,		//78, -64dB
	0x97,		//79, -63.5dB
	0x96,		//80, -63dB
	0x95,		//81, -62.5dB
	0x94,		//82, -62dB
	0x93,		//83, -61.5dB
	0x92,		//84, -61dB
	0x91,		//85, -60.5dB
	0x90,		//86, -60dB
	0x8F,		//87, -59.5dB
	0x8E,		//88, -59dB
	0x8D,		//89, -58.5dB
	0x8C,		//90, -58dB
	0x8B,		//91, -57dB
	0x8A,		//92, -57dB
	0x89,		//93, -56.5dB
	0x88,		//94, -56dB
	0x87,		//95, -55.5dB
	0x86,		//96, -55dB
	0x85,		//97, -54.5dB
	0x84,		//98, -54dB
	0x83,		//99, -53.5dB
	0x82,		//100, -53dB
	0x81,		//101, -52.5dB
	0x80,		//102, -52dB
	0x7F,		//103, -51.5dB
	0x7E,		//104, -51dB
	0x7D,		//105, -50.5dB
	0x7C,		//106, -50dB
	0x7B,		//107, -49.5dB
	0x7A,		//108, -49dB
	0x79,		//109, -48.5dB
	0x78,		//110, -48dB
	0x77,		//111, -47.5dB
	0x76,		//112, -47dB
	0x75,		//113, -46.5dB
	0x74,		//114, -46dB
	0x73,		//115, -45.5dB
	0x72,		//116, -45dB
	0x71,		//117, -44.5dB
	0x70,		//118, -44dB
	0x6F,		//119, -43.5dB
	0x6E,		//120, -43dB
	0x6D,		//121, -42.5dB
	0x6C,		//122, -42dB
	0x6B,		//123, -41.5dB
	0x6A,		//124, -41dB
	0x69,		//125, -40.5dB
	0x68,		//126, -40dB
	0x67,		//127, -39.5dB
	0x66,		//128, -39dB
	0x65,		//129, -38.5dB
	0x64,		//130, -38dB
	0x63,		//131, -37.5dB
	0x62,		//132, -37dB
	0x61,		//133, -36.5dB
	0x60,		//134, -36dB
	0x5F,		//135, -35.5dB
	0x5E,		//136, -35dB
	0x5D,		//137, -34.5dB
	0x5C,		//138, -34dB
	0x5B,		//139, -33.5dB
	0x5A,		//140, -33dB
	0x59,		//141, -32.5dB
	0x58,		//142, -32dB
	0x57,		//143, -31.5dB
	0x56,		//144, -31dB
	0x55,		//145, -30.5dB
	0x54,		//146, -30dB
	0x53,		//147, -29.5dB
	0x52,		//148, -29dB
	0x51,		//149, -28.5dB
	0x50,		//150, -28dB
	0x4F,		//151, -27.5dB
	0x4E,		//152, -27dB
	0x4D,		//153, -26.5dB
	0x4C,		//154, -26dB
	0x4B,		//155, -25.5dB
	0x4A,		//156, -25dB
	0x49,		//157, -24.5dB
	0x48,		//158, -24dB
	0x47,		//159, -23.5dB
	0x46,		//160, -23dB
	0x45,		//161, -22.5dB
	0x44,		//162, -22dB
	0x43,		//163, -21.5dB
	0x42,		//164, -21dB
	0x41,		//165, -20.5dB
	0x40,		//166, -20dB
	0x3F,		//167, -19.5dB
	0x3E,		//168, -19dB
	0x3D,		//169, -18.5dB
	0x3C,		//170, -18dB
	0x3B,		//171, -17.5dB
	0x3A,		//172, -17dB
	0x39,		//173, -16.5dB
	0x38,		//174, -16dB
	0x37,		//175, -15.5dB
	0x36,		//176, -15dB
	0x35,		//177, -14.5dB
	0x34,		//178, -14dB
	0x33,		//179, -13.5dB
	0x32,		//180, -13dB
	0x31,		//181, -12.5dB
	0x30,		//182, -12dB
	0x2F,		//183, -11.5dB
	0x2E,		//184, -11dB
	0x2D,		//185, -10.5dB
	0x2C,		//186, -10dB
	0x2B,		//187, -9.5dB
	0x2A,		//188, -9dB
	0x29,		//189, -8.5dB
	0x28,		//190, -8dB
	0x27,		//191, -7.5dB
	0x26,		//192, -7dB
	0x25,		//193, -6.5dB
	0x24,		//194, -6dB
	0x23,		//195, -5.5dB
	0x22,		//196, -5dB
	0x21,		//197, -4.5dB
	0x20,		//198, -4dB
	0x1F,		//199, -3.5dB
	0x1E,		//200, -3dB
	0x1D,		//201, -2.5dB
	0x1C,		//202, -2dB
	0x1B,		//203, -1.5dB
	0x1A,		//204, -1dB
	0x19,		//205, -0.5dB
	0x18,		//206, 0dB
	0x17,		//207, 0.5dB
	0x16,		//208, 1dB
	0x15,		//209, 1.5dB
	0x14,		//210, 2dB
	0x13,		//211, 2.5dB
	0x12,		//212, 3dB
	0x11,		//213, 3.5dB
	0x10,		//214, 4dB
	0x0F,		//215, 4.5dB
	0x0E,		//216, 5dB
	0x0D,		//217, 5.5dB
	0x0C,		//218, 6dB
	0x0B,		//219, 6.5dB
	0x0A,		//220, 7dB
	0x09,		//221, 7.5dB
	0x08,		//222, 8dB
	0x07,		//223, 8.5dB
	0x06,		//224, 9dB
	0x05,		//225, 9.5dB
	0x04,		//226, 10dB
	0x03,		//227, 10.5dB
	0x02,		//228, 11dB
	0x01,		//229, 11.5dB
	0x00,		//230, 12dB
};

#endif /* __AD82120B_H__ */
