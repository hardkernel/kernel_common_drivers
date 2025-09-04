/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef RDMA_REGS_HEADER_
#define RDMA_REGS_HEADER_

/* cbus reset ctrl */
#define RESETCTRL_RESET4  0x0004

#ifndef RDMA_AHB_START_ADDR_MAN
#define RDMA_AHB_START_ADDR_MAN     0x1100
#define RDMA_AHB_END_ADDR_MAN       0x1101
#define RDMA_AHB_START_ADDR_1       0x1102
#define RDMA_AHB_END_ADDR_1         0x1103
#define RDMA_AHB_START_ADDR_2       0x1104
#define RDMA_AHB_END_ADDR_2         0x1105
#define RDMA_AHB_START_ADDR_3       0x1106
#define RDMA_AHB_END_ADDR_3         0x1107

#define RDMA_AHB_START_ADDR_4       0x1108
#define RDMA_AHB_END_ADDR_4         0x1109
#define RDMA_AHB_START_ADDR_5       0x110a
#define RDMA_AHB_END_ADDR_5         0x110b
#define RDMA_AHB_START_ADDR_6       0x110c
#define RDMA_AHB_END_ADDR_6         0x110d
#define RDMA_AHB_START_ADDR_7       0x110e
#define RDMA_AHB_END_ADDR_7         0x110f

#define RDMA_ACCESS_AUTO            0x1110
#define RDMA_ACCESS_AUTO2           0x1111
#define RDMA_ACCESS_AUTO3           0x1112
#define RDMA_ACCESS_MAN             0x1113
#define RDMA_CTRL                   0x1114
#define RDMA_STATUS                 0x1115
#define RDMA_STATUS2                0x1116
#define RDMA_STATUS3                0x1117

#define RDMA_AUTO_SRC1_SEL          0x1123
#define RDMA_AUTO_SRC2_SEL          0x1124
#define RDMA_AUTO_SRC3_SEL          0x1125
#define RDMA_AUTO_SRC4_SEL          0x1126
#define RDMA_AUTO_SRC5_SEL          0x1127
#define RDMA_AUTO_SRC6_SEL          0x1128
#define RDMA_AUTO_SRC7_SEL          0x1129

#define RDMA_AHB_START_ADDR_MAN_MSB                0x1130
#define RDMA_AHB_END_ADDR_MAN_MSB                  0x1131
#define RDMA_AHB_START_ADDR_1_MSB                  0x1132
#define RDMA_AHB_END_ADDR_1_MSB                    0x1133
#define RDMA_AHB_START_ADDR_2_MSB                  0x1134
#define RDMA_AHB_END_ADDR_2_MSB                    0x1135
#define RDMA_AHB_START_ADDR_3_MSB                  0x1136
#define RDMA_AHB_END_ADDR_3_MSB                    0x1137
#define RDMA_AHB_START_ADDR_4_MSB                  0x1138
#define RDMA_AHB_END_ADDR_4_MSB                    0x1139
#define RDMA_AHB_START_ADDR_5_MSB                  0x113a
#define RDMA_AHB_END_ADDR_5_MSB                    0x113b
#define RDMA_AHB_START_ADDR_6_MSB                  0x113c
#define RDMA_AHB_END_ADDR_6_MSB                    0x113d
#define RDMA_AHB_START_ADDR_7_MSB                  0x113e
#define RDMA_AHB_END_ADDR_7_MSB                    0x113f

#define T3X_RDMA_CTRL                              0x1100
#define T3X_RDMA_STATUS                            0x1104
#define T3X_RDMA_STATUS1                           0x1105
#define T3X_RDMA_STATUS2                           0x1106
#define T3X_RDMA_STATUS3                           0x1107
#define T3X_RDMA_ACCESS_MAN                        0x1110
#define T3X_RDMA_AHB_START_ADDR_MAN_MSB            0x1111
#define T3X_RDMA_AHB_START_ADDR_MAN                0x1112
#define T3X_RDMA_AHB_END_ADDR_MAN_MSB              0x1113
#define T3X_RDMA_AHB_END_ADDR_MAN                  0x1114
#define T3X_RDMA_ACCESS_AUTO                       0x1120
#define T3X_RDMA_ACCESS_AUTO2                      0x1121
#define T3X_RDMA_ACCESS_AUTO3                      0x1122
#define T3X_RDMA_ACCESS_AUTO4                      0x1123
#define T3X_RDMA_AUTO_SRC1_SEL                     0x1124
#define T3X_RDMA_AHB_START_ADDR_1_MSB              0x1125
#define T3X_RDMA_AHB_START_ADDR_1                  0x1126
#define T3X_RDMA_AHB_END_ADDR_1_MSB                0x1127
#define T3X_RDMA_AHB_END_ADDR_1                    0x1128
#define T3X_RDMA_AUTO_SRC2_SEL                     0x1129
#define T3X_RDMA_AHB_START_ADDR_2_MSB              0x112a
#define T3X_RDMA_AHB_START_ADDR_2                  0x112b
#define T3X_RDMA_AHB_END_ADDR_2_MSB                0x112c
#define T3X_RDMA_AHB_END_ADDR_2                    0x112d
#define T3X_RDMA_AUTO_SRC3_SEL                     0x112e
#define T3X_RDMA_AHB_START_ADDR_3_MSB              0x112f
#define T3X_RDMA_AHB_START_ADDR_3                  0x1130
#define T3X_RDMA_AHB_END_ADDR_3_MSB                0x1131
#define T3X_RDMA_AHB_END_ADDR_3                    0x1132
#define T3X_RDMA_AUTO_SRC4_SEL                     0x1133
#define T3X_RDMA_AHB_START_ADDR_4_MSB              0x1134
#define T3X_RDMA_AHB_START_ADDR_4                  0x1135
#define T3X_RDMA_AHB_END_ADDR_4_MSB                0x1136
#define T3X_RDMA_AHB_END_ADDR_4                    0x1137
#define T3X_RDMA_AUTO_SRC5_SEL                     0x1138
#define T3X_RDMA_AHB_START_ADDR_5_MSB              0x1139
#define T3X_RDMA_AHB_START_ADDR_5                  0x113a
#define T3X_RDMA_AHB_END_ADDR_5_MSB                0x113b
#define T3X_RDMA_AHB_END_ADDR_5                    0x113c
#define T3X_RDMA_AUTO_SRC6_SEL                     0x113d
#define T3X_RDMA_AHB_START_ADDR_6_MSB              0x113e
#define T3X_RDMA_AHB_START_ADDR_6                  0x113f
#define T3X_RDMA_AHB_END_ADDR_6_MSB                0x1140
#define T3X_RDMA_AHB_END_ADDR_6                    0x1141
#define T3X_RDMA_AUTO_SRC7_SEL                     0x1142
#define T3X_RDMA_AHB_START_ADDR_7_MSB              0x1143
#define T3X_RDMA_AHB_START_ADDR_7                  0x1144
#define T3X_RDMA_AHB_END_ADDR_7_MSB                0x1145
#define T3X_RDMA_AHB_END_ADDR_7                    0x1146
#define T3X_RDMA_AUTO_SRC8_SEL                     0x1147
#define T3X_RDMA_AHB_START_ADDR_8_MSB              0x1148
#define T3X_RDMA_AHB_START_ADDR_8                  0x1149
#define T3X_RDMA_AHB_END_ADDR_8_MSB                0x114a
#define T3X_RDMA_AHB_END_ADDR_8                    0x114b
#define T3X_RDMA_AUTO_SRC9_SEL                     0x114c
#define T3X_RDMA_AHB_START_ADDR_9_MSB              0x114d
#define T3X_RDMA_AHB_START_ADDR_9                  0x114e
#define T3X_RDMA_AHB_END_ADDR_9_MSB                0x114f
#define T3X_RDMA_AHB_END_ADDR_9                    0x1150
#define T3X_RDMA_AUTO_SRC10_SEL                    0x1151
#define T3X_RDMA_AHB_START_ADDR_10_MSB             0x1152
#define T3X_RDMA_AHB_START_ADDR_10                 0x1153
#define T3X_RDMA_AHB_END_ADDR_10_MSB               0x1154
#define T3X_RDMA_AHB_END_ADDR_10                   0x1155
#define T3X_RDMA_AUTO_SRC11_SEL                    0x1156
#define T3X_RDMA_AHB_START_ADDR_11_MSB             0x1157
#define T3X_RDMA_AHB_START_ADDR_11                 0x1158
#define T3X_RDMA_AHB_END_ADDR_11_MSB               0x1159
#define T3X_RDMA_AHB_END_ADDR_11                   0x115a
#define T3X_RDMA_AUTO_SRC12_SEL                    0x115b
#define T3X_RDMA_AHB_START_ADDR_12_MSB             0x115c
#define T3X_RDMA_AHB_START_ADDR_12                 0x115d
#define T3X_RDMA_AHB_END_ADDR_12_MSB               0x115e
#define T3X_RDMA_AHB_END_ADDR_12                   0x115f
#define T3X_RDMA_AUTO_SRC13_SEL                    0x1160
#define T3X_RDMA_AHB_START_ADDR_13_MSB             0x1161
#define T3X_RDMA_AHB_START_ADDR_13                 0x1162
#define T3X_RDMA_AHB_END_ADDR_13_MSB               0x1163
#define T3X_RDMA_AHB_END_ADDR_13                   0x1164
#define T3X_RDMA_AUTO_SRC14_SEL                    0x1165
#define T3X_RDMA_AHB_START_ADDR_14_MSB             0x1166
#define T3X_RDMA_AHB_START_ADDR_14                 0x1167
#define T3X_RDMA_AHB_END_ADDR_14_MSB               0x1168
#define T3X_RDMA_AHB_END_ADDR_14                   0x1169
#define T3X_RDMA_AUTO_SRC15_SEL                    0x116a
#define T3X_RDMA_AHB_START_ADDR_15_MSB             0x116b
#define T3X_RDMA_AHB_START_ADDR_15                 0x116c
#define T3X_RDMA_AHB_END_ADDR_15_MSB               0x116d
#define T3X_RDMA_AHB_END_ADDR_15                   0x116e

#define RDMA_BUF_CTRL_MAN                          0x1170
//Bit 31:16     reg_buf_step_man    //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_man     //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_man     //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_1                            0x1171
//Bit 31:16     reg_buf_step_1      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_1       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_1       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_2                            0x1172
//Bit 31:16     reg_buf_step_2      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_2       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_2       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_3                            0x1173
//Bit 31:16     reg_buf_step_3      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_3       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_3       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_4                            0x1174
//Bit 31:16     reg_buf_step_4      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_4       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_4       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_5                            0x1175
//Bit 31:16     reg_buf_step_5      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_5       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_5       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_6                            0x1176
//Bit 31:16     reg_buf_step_6      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_6       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_6       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_7                            0x1177
//Bit 31:16     reg_buf_step_7      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_7       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_7       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_8                            0x1178
//Bit 31:16     reg_buf_step_8      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_8       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_8       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_9                            0x1179
//Bit 31:16     reg_buf_step_9      //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_9       //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_9       //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_10                           0x117a
//Bit 31:16     reg_buf_step_10     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_10      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_10      //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_11                           0x117b
//Bit 31:16     reg_buf_step_11     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_11      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_11      //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_12                           0x110c
//Bit 31:16     reg_buf_step_12     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_12      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_12      //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_13                           0x117d
//Bit 31:16     reg_buf_step_13     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_13      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_13      //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_14                           0x117e
//Bit 31:16     reg_buf_step_14     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_14      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_14      //unsigned, RW, default = 16
#define RDMA_BUF_CTRL_15                           0x117f
//Bit 31:16     reg_buf_step_15     //unsigned, RW, default = 0
//Bit 15:12     reserved
//Bit 11:8      reg_sft_bit_15      //unsigned, RW, default = 0
//Bit  7:5      reserved
//Bit  4:0      reg_buf_num_15      //unsigned, RW, default = 16
#define RDMA_BUF_LENGTH_MAN_0                      0x1180
//Bit 31:24     reg_buf_length_man_3   //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_man_2   //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_man_1   //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_man_0   //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_MAN_1                      0x1181
//Bit 31:24     reg_buf_length_man_7   //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_man_6   //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_man_5   //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_man_4   //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_MAN_2                      0x1182
//Bit 31:24     reg_buf_length_man_11  //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_man_10  //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_man_9   //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_man_8   //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_MAN_3                      0x1183
//Bit 31:24     reg_buf_length_man_15  //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_man_14  //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_man_13  //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_man_12  //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_1_0                        0x1184
//Bit 31:24     reg_buf_length_1_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_1_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_1_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_1_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_1_1                        0x1185
//Bit 31:24     reg_buf_length_1_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_1_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_1_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_1_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_1_2                        0x1186
//Bit 31:24     reg_buf_length_1_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_1_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_1_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_1_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_1_3                        0x1187
//Bit 31:24     reg_buf_length_1_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_1_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_1_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_1_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_2_0                        0x1188
//Bit 31:24     reg_buf_length_2_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_2_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_2_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_2_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_2_1                        0x1189
//Bit 31:24     reg_buf_length_2_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_2_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_2_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_2_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_2_2                        0x118a
//Bit 31:24     reg_buf_length_2_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_2_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_2_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_2_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_2_3                        0x118b
//Bit 31:24     reg_buf_length_2_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_2_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_2_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_2_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_3_0                        0x118c
//Bit 31:24     reg_buf_length_3_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_3_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_3_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_3_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_3_1                        0x118d
//Bit 31:24     reg_buf_length_3_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_3_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_3_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_3_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_3_2                        0x118e
//Bit 31:24     reg_buf_length_3_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_3_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_3_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_3_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_3_3                        0x118f
//Bit 31:24     reg_buf_length_3_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_3_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_3_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_3_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_4_0                        0x1190
//Bit 31:24     reg_buf_length_4_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_4_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_4_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_4_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_4_1                        0x1191
//Bit 31:24     reg_buf_length_4_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_4_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_4_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_4_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_4_2                        0x1192
//Bit 31:24     reg_buf_length_4_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_4_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_4_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_4_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_4_3                        0x1193
//Bit 31:24     reg_buf_length_4_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_4_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_4_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_4_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_5_0                        0x1194
//Bit 31:24     reg_buf_length_5_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_5_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_5_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_5_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_5_1                        0x1195
//Bit 31:24     reg_buf_length_5_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_5_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_5_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_5_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_5_2                        0x1196
//Bit 31:24     reg_buf_length_5_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_5_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_5_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_5_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_5_3                        0x1197
//Bit 31:24     reg_buf_length_5_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_5_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_5_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_5_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_6_0                        0x1198
//Bit 31:24     reg_buf_length_6_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_6_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_6_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_6_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_6_1                        0x1199
//Bit 31:24     reg_buf_length_6_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_6_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_6_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_6_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_6_2                        0x119a
//Bit 31:24     reg_buf_length_6_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_6_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_6_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_6_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_6_3                        0x119b
//Bit 31:24     reg_buf_length_6_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_6_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_6_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_6_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_7_0                        0x119c
//Bit 31:24     reg_buf_length_7_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_7_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_7_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_7_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_7_1                        0x119d
//Bit 31:24     reg_buf_length_7_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_7_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_7_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_7_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_7_2                        0x119e
//Bit 31:24     reg_buf_length_7_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_7_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_7_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_7_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_7_3                        0x119f
//Bit 31:24     reg_buf_length_7_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_7_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_7_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_7_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_8_0                        0x11a0
//Bit 31:24     reg_buf_length_8_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_8_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_8_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_8_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_8_1                        0x11a1
//Bit 31:24     reg_buf_length_8_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_8_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_8_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_8_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_8_2                        0x11a2
//Bit 31:24     reg_buf_length_8_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_8_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_8_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_8_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_8_3                        0x11a3
//Bit 31:24     reg_buf_length_8_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_8_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_8_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_8_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_9_0                        0x11a4
//Bit 31:24     reg_buf_length_9_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_9_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_9_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_9_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_9_1                        0x11a5
//Bit 31:24     reg_buf_length_9_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_9_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_9_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_9_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_9_2                        0x11a6
//Bit 31:24     reg_buf_length_9_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_9_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_9_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_9_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_9_3                        0x11a7
//Bit 31:24     reg_buf_length_9_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_9_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_9_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_9_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_10_0                       0x11a8
//Bit 31:24     reg_buf_length_10_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_10_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_10_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_10_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_10_1                       0x11a9
//Bit 31:24     reg_buf_length_10_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_10_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_10_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_10_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_10_2                       0x11aa
//Bit 31:24     reg_buf_length_10_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_10_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_10_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_10_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_10_3                       0x11ab
//Bit 31:24     reg_buf_length_10_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_10_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_10_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_10_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_11_0                       0x11ac
//Bit 31:24     reg_buf_length_11_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_11_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_11_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_11_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_11_1                       0x11ad
//Bit 31:24     reg_buf_length_11_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_11_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_11_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_11_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_11_2                       0x11ae
//Bit 31:24     reg_buf_length_11_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_11_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_11_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_11_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_11_3                       0x11af
//Bit 31:24     reg_buf_length_11_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_11_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_11_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_11_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_12_0                       0x11b0
//Bit 31:24     reg_buf_length_12_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_12_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_12_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_12_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_12_1                       0x11b1
//Bit 31:24     reg_buf_length_12_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_12_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_12_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_12_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_12_2                       0x11b2
//Bit 31:24     reg_buf_length_12_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_12_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_12_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_12_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_12_3                       0x11b3
//Bit 31:24     reg_buf_length_12_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_12_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_12_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_12_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_13_0                       0x11b4
//Bit 31:24     reg_buf_length_13_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_13_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_13_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_13_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_13_1                       0x11b5
//Bit 31:24     reg_buf_length_13_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_13_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_13_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_13_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_13_2                       0x11b6
//Bit 31:24     reg_buf_length_13_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_13_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_13_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_13_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_13_3                       0x11b7
//Bit 31:24     reg_buf_length_13_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_13_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_13_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_13_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_14_0                       0x11b8
//Bit 31:24     reg_buf_length_14_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_14_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_14_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_14_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_14_1                       0x11b9
//Bit 31:24     reg_buf_length_14_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_14_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_14_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_14_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_14_2                       0x11ba
//Bit 31:24     reg_buf_length_14_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_14_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_14_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_14_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_14_3                       0x11bb
//Bit 31:24     reg_buf_length_14_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_14_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_14_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_14_12    //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_15_0                       0x11bc
//Bit 31:24     reg_buf_length_15_3     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_15_2     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_15_1     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_15_0     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_15_1                       0x11bd
//Bit 31:24     reg_buf_length_15_7     //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_15_6     //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_15_5     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_15_4     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_15_2                       0x11be
//Bit 31:24     reg_buf_length_15_11    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_15_10    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_15_9     //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_15_8     //unsigned, RW, default = 0
#define RDMA_BUF_LENGTH_15_3                       0x11bf
//Bit 31:24     reg_buf_length_15_15    //unsigned, RW, default = 0
//Bit 23:16     reg_buf_length_15_14    //unsigned, RW, default = 0
//Bit 15:8      reg_buf_length_15_13    //unsigned, RW, default = 0
//Bit  7:0      reg_buf_length_15_12    //unsigned, RW, default = 0
#define RDMA_INT_MODEL_SEL                         0x11cf
//Bit 31:1      reserved
//Bit    0      reg_rdma_int_model     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_MAN                        0x11d0
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_man   //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_1                          0x11d1
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_1     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_2                          0x11d2
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_2     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_3                          0x11d3
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_3     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_4                          0x11d4
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_4     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_5                          0x11d5
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_5     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_6                          0x11d6
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_6     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_7                          0x11d7
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_7     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_8                          0x11d8
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_8     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_9                          0x11d9
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_9     //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_10                         0x11da
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_10    //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_11                         0x11db
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_11    //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_12                         0x11dc
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_12    //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_13                         0x11dd
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_13    //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_14                         0x11de
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_14    //unsigned, RW, default = 0
#define RDMA_BUF_CFG_EN_15                         0x11df
//Bit 31:1      reserved
//Bit 0         reg_addr_cfg_en_15    //unsigned, RW, default = 0
#define RDMA_BUF_RO_MAN                            0x11e0
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_man         //unsigned, RO, default = 0
#define RDMA_BUF_RO_1                              0x11e1
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_1           //unsigned, RO, default = 0
#define RDMA_BUF_RO_2                              0x11e2
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_2           //unsigned, RO, default = 0
#define RDMA_BUF_RO_3                              0x11e3
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_3           //unsigned, RO, default = 0
#define RDMA_BUF_RO_4                              0x11e4
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_4           //unsigned, RO, default = 0
#define RDMA_BUF_RO_5                              0x11e5
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_5           //unsigned, RO, default = 0
#define RDMA_BUF_RO_6                              0x11e6
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_6           //unsigned, RO, default = 0
#define RDMA_BUF_RO_7                              0x11e7
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_7           //unsigned, RO, default = 0
#define RDMA_BUF_RO_8                              0x11e8
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_8           //unsigned, RO, default = 0
#define RDMA_BUF_RO_9                              0x11e9
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_9           //unsigned, RO, default = 0
#define RDMA_BUF_RO_10                             0x11ea
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_10          //unsigned, RO, default = 0
#define RDMA_BUF_RO_11                             0x11eb
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_11          //unsigned, RO, default = 0
#define RDMA_BUF_RO_12                             0x11ec
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_12          //unsigned, RO, default = 0
#define RDMA_BUF_RO_13                             0x11ed
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_13          //unsigned, RO, default = 0
#define RDMA_BUF_RO_14                             0x11ee
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_14          //unsigned, RO, default = 0
#define RDMA_BUF_RO_15                             0x11ef
//Bit 31:4      reserved
//Bit  3:0      ro_buf_idx_15          //unsigned, RO, default = 0
#define RDMA_DONE_FLAG_SEL                         0x11f0
//Bit 31:16     reserved
//Bit 15:0      reg_rdma_done_flag_sel //unsigned, RW, default = 0
#define RDMA_BUF_IDX_CLR                           0x11f1
//Bit 31:16     reserved
//Bit 15:0      reg_idx_clr   //unsigned, RW, default = 0
#endif

#endif

