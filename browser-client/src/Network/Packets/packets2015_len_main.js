/**
 * Network/Packets/packets2015_len_main.js
 *
 * Network Packet Length
 * Manage packets length
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 * This file is based in a part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2018-2023 Hercules Dev Team
 * Copyright (C) 2018-2022 Andrei Karas (4144)
 *
 * @author Alison Serafim
 */

const length_list = new Array();

function init(packetver) {
	// Packet: 0x0064
	length_list[0x0064] = 55;

	// Packet: 0x0065
	length_list[0x0065] = 17;

	// Packet: 0x0066
	length_list[0x0066] = 3;

	// Packet: 0x0067
	length_list[0x0067] = 37;

	// Packet: 0x0068
	length_list[0x0068] = 46;

	// Packet: 0x0069
	length_list[0x0069] = -1;

	// Packet: 0x006a
	length_list[0x006a] = 23;

	// Packet: 0x006b
	length_list[0x006b] = -1;

	// Packet: 0x006c
	length_list[0x006c] = 3;

	// Packet: 0x006d
	length_list[0x006d] = 149;

	// Packet: 0x006e
	length_list[0x006e] = 3;

	// Packet: 0x006f
	length_list[0x006f] = 2;

	// Packet: 0x0070
	length_list[0x0070] = 3;

	// Packet: 0x0071
	length_list[0x0071] = 28;

	// Packet: 0x0072
	length_list[0x0072] = 22;

	// Packet: 0x0073
	length_list[0x0073] = 11;

	// Packet: 0x0074
	length_list[0x0074] = 3;

	// Packet: 0x0075
	length_list[0x0075] = -1;

	// Packet: 0x0076
	length_list[0x0076] = 9;

	// Packet: 0x0077
	length_list[0x0077] = 5;

	// Packet: 0x0078
	length_list[0x0078] = 55;

	// Packet: 0x0079
	length_list[0x0079] = 53;

	// Packet: 0x007a
	length_list[0x007a] = 58;

	// Packet: 0x007b
	length_list[0x007b] = 60;

	// Packet: 0x007c
	length_list[0x007c] = 44;

	// Packet: 0x007d
	length_list[0x007d] = 2;

	// Packet: 0x007e
	length_list[0x007e] = 105;

	// Packet: 0x007f
	length_list[0x007f] = 6;

	// Packet: 0x0080
	length_list[0x0080] = 7;

	// Packet: 0x0081
	length_list[0x0081] = 3;

	// Packet: 0x0082
	length_list[0x0082] = 2;

	// Packet: 0x0083
	length_list[0x0083] = 2;

	// Packet: 0x0084
	length_list[0x0084] = 2;

	// Packet: 0x0085
	length_list[0x0085] = 10;

	// Packet: 0x0086
	length_list[0x0086] = 16;

	// Packet: 0x0087
	length_list[0x0087] = 12;

	// Packet: 0x0088
	length_list[0x0088] = 10;

	// Packet: 0x0089
	length_list[0x0089] = 11;

	// Packet: 0x008a
	length_list[0x008a] = 29;

	// Packet: 0x008b
	length_list[0x008b] = 23;

	// Packet: 0x008c
	length_list[0x008c] = 14;

	// Packet: 0x008d
	length_list[0x008d] = -1;

	// Packet: 0x008e
	length_list[0x008e] = -1;

	// Packet: 0x0090
	length_list[0x0090] = 7;

	// Packet: 0x0091
	length_list[0x0091] = 22;

	// Packet: 0x0092
	length_list[0x0092] = 28;

	// Packet: 0x0093
	length_list[0x0093] = 2;

	// Packet: 0x0094
	length_list[0x0094] = 19;

	// Packet: 0x0095
	length_list[0x0095] = 30;

	// Packet: 0x0096
	length_list[0x0096] = -1;

	// Packet: 0x0097
	length_list[0x0097] = -1;

	// Packet: 0x0098
	length_list[0x0098] = 3;

	// Packet: 0x0099
	length_list[0x0099] = -1;

	// Packet: 0x009a
	length_list[0x009a] = -1;

	// Packet: 0x009b
	length_list[0x009b] = 34;

	// Packet: 0x009c
	length_list[0x009c] = 9;

	// Packet: 0x009d
	length_list[0x009d] = 17;

	// Packet: 0x009e
	length_list[0x009e] = 17;

	// Packet: 0x009f
	length_list[0x009f] = 20;

	// Packet: 0x00a0
	length_list[0x00a0] = 23;

	// Packet: 0x00a1
	length_list[0x00a1] = 6;

	// Packet: 0x00a2
	length_list[0x00a2] = 14;

	// Packet: 0x00a3
	length_list[0x00a3] = -1;

	// Packet: 0x00a4
	length_list[0x00a4] = -1;

	// Packet: 0x00a5
	length_list[0x00a5] = -1;

	// Packet: 0x00a6
	length_list[0x00a6] = -1;

	// Packet: 0x00a7
	length_list[0x00a7] = 9;

	// Packet: 0x00a8
	length_list[0x00a8] = 7;

	// Packet: 0x00a9
	length_list[0x00a9] = 6;

	// Packet: 0x00aa
	length_list[0x00aa] = 9;

	// Packet: 0x00ab
	length_list[0x00ab] = 4;

	// Packet: 0x00ac
	length_list[0x00ac] = 7;

	// Packet: 0x00ae
	length_list[0x00ae] = -1;

	// Packet: 0x00af
	length_list[0x00af] = 6;

	// Packet: 0x00b0
	length_list[0x00b0] = 8;

	// Packet: 0x00b1
	length_list[0x00b1] = 8;

	// Packet: 0x00b2
	length_list[0x00b2] = 3;

	// Packet: 0x00b3
	length_list[0x00b3] = 3;

	// Packet: 0x00b4
	length_list[0x00b4] = -1;

	// Packet: 0x00b5
	length_list[0x00b5] = 6;

	// Packet: 0x00b6
	length_list[0x00b6] = 6;

	// Packet: 0x00b7
	length_list[0x00b7] = -1;

	// Packet: 0x00b8
	length_list[0x00b8] = 7;

	// Packet: 0x00b9
	length_list[0x00b9] = 6;

	// Packet: 0x00ba
	length_list[0x00ba] = 2;

	// Packet: 0x00bb
	length_list[0x00bb] = 5;

	// Packet: 0x00bc
	length_list[0x00bc] = 6;

	// Packet: 0x00bd
	length_list[0x00bd] = 44;

	// Packet: 0x00be
	length_list[0x00be] = 5;

	// Packet: 0x00bf
	length_list[0x00bf] = 3;

	// Packet: 0x00c0
	length_list[0x00c0] = 7;

	// Packet: 0x00c1
	length_list[0x00c1] = 2;

	// Packet: 0x00c2
	length_list[0x00c2] = 6;

	// Packet: 0x00c3
	length_list[0x00c3] = 8;

	// Packet: 0x00c4
	length_list[0x00c4] = 6;

	// Packet: 0x00c5
	length_list[0x00c5] = 7;

	// Packet: 0x00c6
	length_list[0x00c6] = -1;

	// Packet: 0x00c7
	length_list[0x00c7] = -1;

	// Packet: 0x00c8
	length_list[0x00c8] = -1;

	// Packet: 0x00c9
	length_list[0x00c9] = -1;

	// Packet: 0x00ca
	length_list[0x00ca] = 3;

	// Packet: 0x00cb
	length_list[0x00cb] = 3;

	// Packet: 0x00cc
	length_list[0x00cc] = 6;

	// Packet: 0x00cd
	length_list[0x00cd] = 3;

	// Packet: 0x00ce
	length_list[0x00ce] = 2;

	// Packet: 0x00cf
	length_list[0x00cf] = 27;

	// Packet: 0x00d0
	length_list[0x00d0] = 3;

	// Packet: 0x00d1
	length_list[0x00d1] = 4;

	// Packet: 0x00d2
	length_list[0x00d2] = 4;

	// Packet: 0x00d3
	length_list[0x00d3] = 2;

	// Packet: 0x00d4
	length_list[0x00d4] = -1;

	// Packet: 0x00d5
	length_list[0x00d5] = -1;

	// Packet: 0x00d6
	length_list[0x00d6] = 3;

	// Packet: 0x00d7
	length_list[0x00d7] = -1;

	// Packet: 0x00d8
	length_list[0x00d8] = 6;

	// Packet: 0x00d9
	length_list[0x00d9] = 14;

	// Packet: 0x00da
	length_list[0x00da] = 3;

	// Packet: 0x00db
	length_list[0x00db] = -1;

	// Packet: 0x00dc
	length_list[0x00dc] = 28;

	// Packet: 0x00dd
	length_list[0x00dd] = 29;

	// Packet: 0x00de
	length_list[0x00de] = -1;

	// Packet: 0x00df
	length_list[0x00df] = -1;

	// Packet: 0x00e0
	length_list[0x00e0] = 30;

	// Packet: 0x00e1
	length_list[0x00e1] = 30;

	// Packet: 0x00e2
	length_list[0x00e2] = 26;

	// Packet: 0x00e3
	length_list[0x00e3] = 2;

	// Packet: 0x00e4
	length_list[0x00e4] = 6;

	// Packet: 0x00e5
	length_list[0x00e5] = 26;

	// Packet: 0x00e6
	length_list[0x00e6] = 3;

	// Packet: 0x00e7
	length_list[0x00e7] = 3;

	// Packet: 0x00e8
	length_list[0x00e8] = 8;

	// Packet: 0x00e9
	length_list[0x00e9] = 19;

	// Packet: 0x00ea
	length_list[0x00ea] = 5;

	// Packet: 0x00eb
	length_list[0x00eb] = 2;

	// Packet: 0x00ec
	length_list[0x00ec] = 3;

	// Packet: 0x00ed
	length_list[0x00ed] = 2;

	// Packet: 0x00ee
	length_list[0x00ee] = 2;

	// Packet: 0x00ef
	length_list[0x00ef] = 2;

	// Packet: 0x00f0
	length_list[0x00f0] = 3;

	// Packet: 0x00f1
	length_list[0x00f1] = 2;

	// Packet: 0x00f2
	length_list[0x00f2] = 6;

	// Packet: 0x00f3
	length_list[0x00f3] = -1;

	// Packet: 0x00f4
	length_list[0x00f4] = 21;

	// Packet: 0x00f5
	length_list[0x00f5] = 11;

	// Packet: 0x00f6
	length_list[0x00f6] = 8;

	// Packet: 0x00f7
	length_list[0x00f7] = 17;

	// Packet: 0x00f8
	length_list[0x00f8] = 2;

	// Packet: 0x00f9
	length_list[0x00f9] = 26;

	// Packet: 0x00fa
	length_list[0x00fa] = 3;

	// Packet: 0x00fb
	length_list[0x00fb] = -1;

	// Packet: 0x00fc
	length_list[0x00fc] = 6;

	// Packet: 0x00fd
	length_list[0x00fd] = 27;

	// Packet: 0x00fe
	length_list[0x00fe] = 30;

	// Packet: 0x00ff
	length_list[0x00ff] = 10;

	// Packet: 0x0100
	length_list[0x0100] = 2;

	// Packet: 0x0101
	length_list[0x0101] = 6;

	// Packet: 0x0102
	length_list[0x0102] = 6;

	// Packet: 0x0103
	length_list[0x0103] = 30;

	// Packet: 0x0104
	length_list[0x0104] = 79;

	// Packet: 0x0105
	length_list[0x0105] = 31;

	// Packet: 0x0106
	length_list[0x0106] = 10;

	// Packet: 0x0107
	length_list[0x0107] = 10;

	// Packet: 0x0108
	length_list[0x0108] = -1;

	// Packet: 0x0109
	length_list[0x0109] = -1;

	// Packet: 0x010a
	length_list[0x010a] = 4;

	// Packet: 0x010b
	length_list[0x010b] = 6;

	// Packet: 0x010c
	length_list[0x010c] = 6;

	// Packet: 0x010d
	length_list[0x010d] = 2;

	// Packet: 0x010e
	length_list[0x010e] = 11;

	// Packet: 0x010f
	length_list[0x010f] = -1;

	// Packet: 0x0110
	length_list[0x0110] = 10;

	// Packet: 0x0111
	length_list[0x0111] = 39;

	// Packet: 0x0112
	length_list[0x0112] = 4;

	// Packet: 0x0113
	length_list[0x0113] = 25;

	// Packet: 0x0114
	length_list[0x0114] = 31;

	// Packet: 0x0115
	length_list[0x0115] = 35;

	// Packet: 0x0116
	length_list[0x0116] = 17;

	// Packet: 0x0117
	length_list[0x0117] = 18;

	// Packet: 0x0118
	length_list[0x0118] = 2;

	// Packet: 0x0119
	length_list[0x0119] = 13;

	// Packet: 0x011a
	length_list[0x011a] = 15;

	// Packet: 0x011b
	length_list[0x011b] = 20;

	// Packet: 0x011c
	length_list[0x011c] = 68;

	// Packet: 0x011d
	length_list[0x011d] = 2;

	// Packet: 0x011e
	length_list[0x011e] = 3;

	// Packet: 0x011f
	length_list[0x011f] = 16;

	// Packet: 0x0120
	length_list[0x0120] = 6;

	// Packet: 0x0121
	length_list[0x0121] = 14;

	// Packet: 0x0122
	length_list[0x0122] = -1;

	// Packet: 0x0123
	length_list[0x0123] = -1;

	// Packet: 0x0124
	length_list[0x0124] = 21;

	// Packet: 0x0125
	length_list[0x0125] = 8;

	// Packet: 0x0126
	length_list[0x0126] = 8;

	// Packet: 0x0127
	length_list[0x0127] = 8;

	// Packet: 0x0128
	length_list[0x0128] = 8;

	// Packet: 0x0129
	length_list[0x0129] = 8;

	// Packet: 0x012a
	length_list[0x012a] = 2;

	// Packet: 0x012b
	length_list[0x012b] = 2;

	// Packet: 0x012c
	length_list[0x012c] = 3;

	// Packet: 0x012d
	length_list[0x012d] = 4;

	// Packet: 0x012e
	length_list[0x012e] = 2;

	// Packet: 0x012f
	length_list[0x012f] = -1;

	// Packet: 0x0130
	length_list[0x0130] = 6;

	// Packet: 0x0131
	length_list[0x0131] = 86;

	// Packet: 0x0132
	length_list[0x0132] = 6;

	// Packet: 0x0133
	length_list[0x0133] = -1;

	// Packet: 0x0134
	length_list[0x0134] = -1;

	// Packet: 0x0135
	length_list[0x0135] = 7;

	// Packet: 0x0136
	length_list[0x0136] = -1;

	// Packet: 0x0137
	length_list[0x0137] = 6;

	// Packet: 0x0138
	length_list[0x0138] = 3;

	// Packet: 0x0139
	length_list[0x0139] = 16;

	// Packet: 0x013a
	length_list[0x013a] = 4;

	// Packet: 0x013b
	length_list[0x013b] = 4;

	// Packet: 0x013c
	length_list[0x013c] = 4;

	// Packet: 0x013d
	length_list[0x013d] = 6;

	// Packet: 0x013e
	length_list[0x013e] = 24;

	// Packet: 0x013f
	length_list[0x013f] = 26;

	// Packet: 0x0140
	length_list[0x0140] = 22;

	// Packet: 0x0141
	length_list[0x0141] = 14;

	// Packet: 0x0142
	length_list[0x0142] = 6;

	// Packet: 0x0143
	length_list[0x0143] = 10;

	// Packet: 0x0144
	length_list[0x0144] = 23;

	// Packet: 0x0145
	length_list[0x0145] = 19;

	// Packet: 0x0146
	length_list[0x0146] = 6;

	// Packet: 0x0147
	length_list[0x0147] = 39;

	// Packet: 0x0148
	length_list[0x0148] = 8;

	// Packet: 0x0149
	length_list[0x0149] = 9;

	// Packet: 0x014a
	length_list[0x014a] = 6;

	// Packet: 0x014b
	length_list[0x014b] = 27;

	// Packet: 0x014c
	length_list[0x014c] = -1;

	// Packet: 0x014d
	length_list[0x014d] = 2;

	// Packet: 0x014e
	length_list[0x014e] = 6;

	// Packet: 0x014f
	length_list[0x014f] = 6;

	// Packet: 0x0150
	length_list[0x0150] = 110;

	// Packet: 0x0151
	length_list[0x0151] = 6;

	// Packet: 0x0152
	length_list[0x0152] = -1;

	// Packet: 0x0153
	length_list[0x0153] = -1;

	// Packet: 0x0154
	length_list[0x0154] = -1;

	// Packet: 0x0155
	length_list[0x0155] = -1;

	// Packet: 0x0156
	length_list[0x0156] = -1;

	// Packet: 0x0157
	length_list[0x0157] = 6;

	// Packet: 0x0159
	length_list[0x0159] = 54;

	// Packet: 0x015a
	length_list[0x015a] = 66;

	// Packet: 0x015b
	length_list[0x015b] = 54;

	// Packet: 0x015c
	length_list[0x015c] = 90;

	// Packet: 0x015d
	length_list[0x015d] = 42;

	// Packet: 0x015e
	length_list[0x015e] = 6;

	// Packet: 0x015f
	length_list[0x015f] = 42;

	// Packet: 0x0160
	length_list[0x0160] = -1;

	// Packet: 0x0161
	length_list[0x0161] = -1;

	// Packet: 0x0162
	length_list[0x0162] = -1;

	// Packet: 0x0163
	length_list[0x0163] = -1;

	// Packet: 0x0164
	length_list[0x0164] = -1;

	// Packet: 0x0165
	length_list[0x0165] = 30;

	// Packet: 0x0166
	length_list[0x0166] = -1;

	// Packet: 0x0167
	length_list[0x0167] = 3;

	// Packet: 0x0168
	length_list[0x0168] = 14;

	// Packet: 0x0169
	length_list[0x0169] = 3;

	// Packet: 0x016a
	length_list[0x016a] = 30;

	// Packet: 0x016b
	length_list[0x016b] = 10;

	// Packet: 0x016c
	length_list[0x016c] = 43;

	// Packet: 0x016d
	length_list[0x016d] = 14;

	// Packet: 0x016e
	length_list[0x016e] = 186;

	// Packet: 0x016f
	length_list[0x016f] = 182;

	// Packet: 0x0170
	length_list[0x0170] = 14;

	// Packet: 0x0171
	length_list[0x0171] = 30;

	// Packet: 0x0172
	length_list[0x0172] = 10;

	// Packet: 0x0173
	length_list[0x0173] = 3;

	// Packet: 0x0174
	length_list[0x0174] = -1;

	// Packet: 0x0175
	length_list[0x0175] = 6;

	// Packet: 0x0176
	length_list[0x0176] = 106;

	// Packet: 0x0177
	length_list[0x0177] = -1;

	// Packet: 0x0178
	length_list[0x0178] = 4;

	// Packet: 0x0179
	length_list[0x0179] = 5;

	// Packet: 0x017a
	length_list[0x017a] = 4;

	// Packet: 0x017b
	length_list[0x017b] = -1;

	// Packet: 0x017c
	length_list[0x017c] = 6;

	// Packet: 0x017d
	length_list[0x017d] = 7;

	// Packet: 0x017e
	length_list[0x017e] = -1;

	// Packet: 0x017f
	length_list[0x017f] = -1;

	// Packet: 0x0180
	length_list[0x0180] = 6;

	// Packet: 0x0181
	length_list[0x0181] = 3;

	// Packet: 0x0182
	length_list[0x0182] = 106;

	// Packet: 0x0183
	length_list[0x0183] = 10;

	// Packet: 0x0184
	length_list[0x0184] = 10;

	// Packet: 0x0185
	length_list[0x0185] = 34;

	// Packet: 0x0187
	length_list[0x0187] = 6;

	// Packet: 0x0188
	length_list[0x0188] = 8;

	// Packet: 0x0189
	length_list[0x0189] = 4;

	// Packet: 0x018a
	length_list[0x018a] = 4;

	// Packet: 0x018b
	length_list[0x018b] = 4;

	// Packet: 0x018c
	length_list[0x018c] = 29;

	// Packet: 0x018d
	length_list[0x018d] = -1;

	// Packet: 0x018e
	length_list[0x018e] = 10;

	// Packet: 0x018f
	length_list[0x018f] = 6;

	// Packet: 0x0190
	length_list[0x0190] = 23;

	// Packet: 0x0191
	length_list[0x0191] = 86;

	// Packet: 0x0192
	length_list[0x0192] = 24;

	// Packet: 0x0193
	length_list[0x0193] = 2;

	// Packet: 0x0194
	length_list[0x0194] = 30;

	// Packet: 0x0195
	length_list[0x0195] = 102;

	// Packet: 0x0196
	length_list[0x0196] = 9;

	// Packet: 0x0197
	length_list[0x0197] = 4;

	// Packet: 0x0198
	length_list[0x0198] = 8;

	// Packet: 0x0199
	length_list[0x0199] = 4;

	// Packet: 0x019a
	length_list[0x019a] = 14;

	// Packet: 0x019b
	length_list[0x019b] = 10;

	// Packet: 0x019c
	length_list[0x019c] = -1;

	// Packet: 0x019d
	length_list[0x019d] = 6;

	// Packet: 0x019e
	length_list[0x019e] = 2;

	// Packet: 0x019f
	length_list[0x019f] = 6;

	// Packet: 0x01a0
	length_list[0x01a0] = 3;

	// Packet: 0x01a1
	length_list[0x01a1] = 3;

	// Packet: 0x01a2
	length_list[0x01a2] = 37;

	// Packet: 0x01a3
	length_list[0x01a3] = 5;

	// Packet: 0x01a4
	length_list[0x01a4] = 11;

	// Packet: 0x01a5
	length_list[0x01a5] = 26;

	// Packet: 0x01a6
	length_list[0x01a6] = -1;

	// Packet: 0x01a7
	length_list[0x01a7] = 4;

	// Packet: 0x01a8
	length_list[0x01a8] = 4;

	// Packet: 0x01a9
	length_list[0x01a9] = 6;

	// Packet: 0x01aa
	length_list[0x01aa] = 10;

	// Packet: 0x01ab
	length_list[0x01ab] = 12;

	// Packet: 0x01ac
	length_list[0x01ac] = 6;

	// Packet: 0x01ad
	length_list[0x01ad] = -1;

	// Packet: 0x01ae
	length_list[0x01ae] = 4;

	// Packet: 0x01af
	length_list[0x01af] = 4;

	// Packet: 0x01b0
	length_list[0x01b0] = 11;

	// Packet: 0x01b1
	length_list[0x01b1] = 7;

	// Packet: 0x01b2
	length_list[0x01b2] = -1;

	// Packet: 0x01b3
	length_list[0x01b3] = 67;

	// Packet: 0x01b4
	length_list[0x01b4] = 12;

	// Packet: 0x01b5
	length_list[0x01b5] = 18;

	// Packet: 0x01b6
	length_list[0x01b6] = 114;

	// Packet: 0x01b7
	length_list[0x01b7] = 6;

	// Packet: 0x01b8
	length_list[0x01b8] = 3;

	// Packet: 0x01b9
	length_list[0x01b9] = 6;

	// Packet: 0x01ba
	length_list[0x01ba] = 26;

	// Packet: 0x01bb
	length_list[0x01bb] = 26;

	// Packet: 0x01bc
	length_list[0x01bc] = 26;

	// Packet: 0x01bd
	length_list[0x01bd] = 26;

	// Packet: 0x01be
	length_list[0x01be] = 2;

	// Packet: 0x01bf
	length_list[0x01bf] = 3;

	// Packet: 0x01c0
	length_list[0x01c0] = 2;

	// Packet: 0x01c1
	length_list[0x01c1] = 14;

	// Packet: 0x01c2
	length_list[0x01c2] = 10;

	// Packet: 0x01c3
	length_list[0x01c3] = -1;

	// Packet: 0x01c4
	length_list[0x01c4] = 22;

	// Packet: 0x01c5
	length_list[0x01c5] = 22;

	// Packet: 0x01c6
	length_list[0x01c6] = 4;

	// Packet: 0x01c7
	length_list[0x01c7] = 2;

	// Packet: 0x01c8
	length_list[0x01c8] = 13;

	// Packet: 0x01c9
	length_list[0x01c9] = 97;

	// Packet: 0x01ca
	length_list[0x01ca] = 3;

	// Packet: 0x01cb
	length_list[0x01cb] = 9;

	// Packet: 0x01cc
	length_list[0x01cc] = 9;

	// Packet: 0x01cd
	length_list[0x01cd] = 30;

	// Packet: 0x01ce
	length_list[0x01ce] = 6;

	// Packet: 0x01cf
	length_list[0x01cf] = 28;

	// Packet: 0x01d0
	length_list[0x01d0] = 8;

	// Packet: 0x01d1
	length_list[0x01d1] = 14;

	// Packet: 0x01d2
	length_list[0x01d2] = 10;

	// Packet: 0x01d3
	length_list[0x01d3] = 35;

	// Packet: 0x01d4
	length_list[0x01d4] = 6;

	// Packet: 0x01d5
	length_list[0x01d5] = -1;

	// Packet: 0x01d6
	length_list[0x01d6] = 4;

	// Packet: 0x01d7
	length_list[0x01d7] = 11;

	// Packet: 0x01d8
	length_list[0x01d8] = 54;

	// Packet: 0x01d9
	length_list[0x01d9] = 53;

	// Packet: 0x01da
	length_list[0x01da] = 60;

	// Packet: 0x01db
	length_list[0x01db] = 2;

	// Packet: 0x01dc
	length_list[0x01dc] = -1;

	// Packet: 0x01dd
	length_list[0x01dd] = 47;

	// Packet: 0x01de
	length_list[0x01de] = 33;

	// Packet: 0x01df
	length_list[0x01df] = 6;

	// Packet: 0x01e0
	length_list[0x01e0] = 30;

	// Packet: 0x01e1
	length_list[0x01e1] = 8;

	// Packet: 0x01e2
	length_list[0x01e2] = 34;

	// Packet: 0x01e3
	length_list[0x01e3] = 14;

	// Packet: 0x01e4
	length_list[0x01e4] = 2;

	// Packet: 0x01e5
	length_list[0x01e5] = 6;

	// Packet: 0x01e6
	length_list[0x01e6] = 26;

	// Packet: 0x01e7
	length_list[0x01e7] = 2;

	// Packet: 0x01e8
	length_list[0x01e8] = 28;

	// Packet: 0x01e9
	length_list[0x01e9] = 81;

	// Packet: 0x01ea
	length_list[0x01ea] = 6;

	// Packet: 0x01eb
	length_list[0x01eb] = 10;

	// Packet: 0x01ec
	length_list[0x01ec] = 26;

	// Packet: 0x01ed
	length_list[0x01ed] = 2;

	// Packet: 0x01ee
	length_list[0x01ee] = -1;

	// Packet: 0x01ef
	length_list[0x01ef] = -1;

	// Packet: 0x01f0
	length_list[0x01f0] = -1;

	// Packet: 0x01f1
	length_list[0x01f1] = -1;

	// Packet: 0x01f2
	length_list[0x01f2] = 20;

	// Packet: 0x01f3
	length_list[0x01f3] = 10;

	// Packet: 0x01f4
	length_list[0x01f4] = 32;

	// Packet: 0x01f5
	length_list[0x01f5] = 9;

	// Packet: 0x01f6
	length_list[0x01f6] = 34;

	// Packet: 0x01f7
	length_list[0x01f7] = 14;

	// Packet: 0x01f8
	length_list[0x01f8] = 2;

	// Packet: 0x01f9
	length_list[0x01f9] = 6;

	// Packet: 0x01fa
	length_list[0x01fa] = 48;

	// Packet: 0x01fb
	length_list[0x01fb] = 56;

	// Packet: 0x01fc
	length_list[0x01fc] = -1;

	// Packet: 0x01fd
	length_list[0x01fd] = 15;

	// Packet: 0x01fe
	length_list[0x01fe] = 5;

	// Packet: 0x01ff
	length_list[0x01ff] = 10;

	// Packet: 0x0200
	length_list[0x0200] = 26;

	// Packet: 0x0201
	length_list[0x0201] = -1;

	// Packet: 0x0202
	if (packetver >= 20151209) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20151104) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20151021) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0202] = 18;
	} else if (packetver >= 20151007) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20151001) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150826) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20150805) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150603) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150520) {
		length_list[0x0202] = 6;
	} else if (packetver >= 20150429) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150401) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0202] = 26;
	} else if (packetver >= 20150318) {
		length_list[0x0202] = 4;
	} else if (packetver >= 20150225) {
		length_list[0x0202] = 26;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150211) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20150128) {
		length_list[0x0202] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x0202] = 2;
	}

	// Packet: 0x0203
	length_list[0x0203] = 10;

	// Packet: 0x0204
	length_list[0x0204] = 18;

	// Packet: 0x0205
	length_list[0x0205] = 26;

	// Packet: 0x0206
	length_list[0x0206] = 11;

	// Packet: 0x0207
	length_list[0x0207] = 34;

	// Packet: 0x0208
	length_list[0x0208] = 14;

	// Packet: 0x0209
	length_list[0x0209] = 36;

	// Packet: 0x020a
	length_list[0x020a] = 10;

	// Packet: 0x020d
	length_list[0x020d] = -1;

	// Packet: 0x020e
	length_list[0x020e] = 32;

	// Packet: 0x0212
	length_list[0x0212] = 26;

	// Packet: 0x0213
	length_list[0x0213] = 26;

	// Packet: 0x0214
	length_list[0x0214] = 42;

	// Packet: 0x0215
	length_list[0x0215] = 6;

	// Packet: 0x0216
	length_list[0x0216] = 6;

	// Packet: 0x0217
	length_list[0x0217] = 2;

	// Packet: 0x0218
	length_list[0x0218] = 2;

	// Packet: 0x0219
	length_list[0x0219] = 282;

	// Packet: 0x021a
	length_list[0x021a] = 282;

	// Packet: 0x021b
	length_list[0x021b] = 10;

	// Packet: 0x021c
	length_list[0x021c] = 10;

	// Packet: 0x021d
	length_list[0x021d] = 6;

	// Packet: 0x021e
	length_list[0x021e] = 6;

	// Packet: 0x021f
	length_list[0x021f] = 66;

	// Packet: 0x0220
	length_list[0x0220] = 10;

	// Packet: 0x0221
	length_list[0x0221] = -1;

	// Packet: 0x0222
	length_list[0x0222] = 6;

	// Packet: 0x0223
	length_list[0x0223] = 8;

	// Packet: 0x0224
	length_list[0x0224] = 10;

	// Packet: 0x0225
	length_list[0x0225] = 2;

	// Packet: 0x0226
	length_list[0x0226] = 282;

	// Packet: 0x0227
	length_list[0x0227] = 18;

	// Packet: 0x0228
	length_list[0x0228] = 18;

	// Packet: 0x0229
	length_list[0x0229] = 15;

	// Packet: 0x022a
	length_list[0x022a] = 58;

	// Packet: 0x022b
	length_list[0x022b] = 57;

	// Packet: 0x022c
	length_list[0x022c] = 65;

	// Packet: 0x022d
	if (packetver >= 20151223) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x022d] = 5;
	} else if (packetver >= 20151209) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20151125) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20151104) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20151007) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150923) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x022d] = 10;
	} else if (packetver >= 20150826) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x022d] = 10;
	} else if (packetver >= 20150805) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150715) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150603) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150429) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150225) {
		length_list[0x022d] = 5;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150211) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x022d] = 2;
	}

	// Packet: 0x022e
	length_list[0x022e] = 71;

	// Packet: 0x022f
	length_list[0x022f] = 5;

	// Packet: 0x0230
	length_list[0x0230] = 12;

	// Packet: 0x0231
	length_list[0x0231] = 26;

	// Packet: 0x0232
	length_list[0x0232] = 9;

	// Packet: 0x0233
	length_list[0x0233] = 11;

	// Packet: 0x0234
	length_list[0x0234] = 6;

	// Packet: 0x0235
	length_list[0x0235] = -1;

	// Packet: 0x0236
	length_list[0x0236] = 10;

	// Packet: 0x0237
	length_list[0x0237] = 2;

	// Packet: 0x0238
	length_list[0x0238] = 282;

	// Packet: 0x0239
	length_list[0x0239] = 11;

	// Packet: 0x023a
	length_list[0x023a] = 4;

	// Packet: 0x023b
	if (packetver >= 20151209) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20151111) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x023b] = -1;
	} else if (packetver >= 20151028) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20151021) {
		length_list[0x023b] = 19;
	} else if (packetver >= 20151007) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150916) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x023b] = 7;
	} else if (packetver >= 20150902) {
		length_list[0x023b] = 18;
	} else if (packetver >= 20150819) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150729) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x023b] = 90;
	} else if (packetver >= 20150715) {
		length_list[0x023b] = -1;
	} else if (packetver >= 20150708) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x023b] = 19;
	} else if (packetver >= 20150603) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150513) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x023b] = 5;
	} else if (packetver >= 20150429) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150325) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x023b] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x023b] = 5;
	} else if (packetver >= 20150225) {
		length_list[0x023b] = 36;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150211) {
		length_list[0x023b] = 4;
	} else if (packetver >= 20150204) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20150128) {
		length_list[0x023b] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x023b] = 2;
	}

	// Packet: 0x023c
	length_list[0x023c] = 6;

	// Packet: 0x023d
	length_list[0x023d] = 6;

	// Packet: 0x023e
	length_list[0x023e] = 8;

	// Packet: 0x023f
	length_list[0x023f] = 2;

	// Packet: 0x0240
	length_list[0x0240] = -1;

	// Packet: 0x0241
	length_list[0x0241] = 6;

	// Packet: 0x0242
	length_list[0x0242] = -1;

	// Packet: 0x0243
	length_list[0x0243] = 6;

	// Packet: 0x0244
	length_list[0x0244] = 6;

	// Packet: 0x0245
	length_list[0x0245] = 3;

	// Packet: 0x0246
	length_list[0x0246] = 4;

	// Packet: 0x0247
	length_list[0x0247] = 8;

	// Packet: 0x0248
	length_list[0x0248] = -1;

	// Packet: 0x0249
	length_list[0x0249] = 3;

	// Packet: 0x024a
	length_list[0x024a] = 70;

	// Packet: 0x024b
	length_list[0x024b] = 4;

	// Packet: 0x024c
	length_list[0x024c] = 8;

	// Packet: 0x024d
	length_list[0x024d] = 12;

	// Packet: 0x024e
	length_list[0x024e] = 6;

	// Packet: 0x024f
	length_list[0x024f] = 10;

	// Packet: 0x0250
	length_list[0x0250] = 3;

	// Packet: 0x0251
	length_list[0x0251] = 34;

	// Packet: 0x0252
	length_list[0x0252] = -1;

	// Packet: 0x0253
	length_list[0x0253] = 3;

	// Packet: 0x0254
	length_list[0x0254] = 3;

	// Packet: 0x0255
	length_list[0x0255] = 5;

	// Packet: 0x0256
	length_list[0x0256] = 5;

	// Packet: 0x0257
	length_list[0x0257] = 8;

	// Packet: 0x0258
	length_list[0x0258] = 2;

	// Packet: 0x0259
	length_list[0x0259] = 3;

	// Packet: 0x025a
	length_list[0x025a] = -1;

	// Packet: 0x025b
	length_list[0x025b] = 6;

	// Packet: 0x025c
	length_list[0x025c] = 4;

	// Packet: 0x025d
	length_list[0x025d] = 6;

	// Packet: 0x025e
	length_list[0x025e] = 4;

	// Packet: 0x025f
	length_list[0x025f] = 6;

	// Packet: 0x0260
	length_list[0x0260] = 6;

	// Packet: 0x0261
	length_list[0x0261] = 11;

	// Packet: 0x0262
	length_list[0x0262] = 11;

	// Packet: 0x0263
	length_list[0x0263] = 11;

	// Packet: 0x0264
	length_list[0x0264] = 20;

	// Packet: 0x0265
	length_list[0x0265] = 20;

	// Packet: 0x0266
	length_list[0x0266] = 30;

	// Packet: 0x0267
	length_list[0x0267] = 4;

	// Packet: 0x0268
	length_list[0x0268] = 4;

	// Packet: 0x0269
	length_list[0x0269] = 4;

	// Packet: 0x026a
	length_list[0x026a] = 4;

	// Packet: 0x026b
	length_list[0x026b] = 4;

	// Packet: 0x026c
	length_list[0x026c] = 4;

	// Packet: 0x026d
	length_list[0x026d] = 4;

	// Packet: 0x026f
	length_list[0x026f] = 2;

	// Packet: 0x0270
	length_list[0x0270] = 2;

	// Packet: 0x0271
	length_list[0x0271] = 40;

	// Packet: 0x0272
	length_list[0x0272] = 44;

	// Packet: 0x0273
	length_list[0x0273] = 30;

	// Packet: 0x0274
	length_list[0x0274] = 8;

	// Packet: 0x0275
	length_list[0x0275] = 37;

	// Packet: 0x0276
	length_list[0x0276] = -1;

	// Packet: 0x0277
	length_list[0x0277] = 84;

	// Packet: 0x0278
	length_list[0x0278] = 2;

	// Packet: 0x0279
	length_list[0x0279] = 2;

	// Packet: 0x027a
	length_list[0x027a] = -1;

	// Packet: 0x027b
	length_list[0x027b] = 14;

	// Packet: 0x027c
	length_list[0x027c] = 60;

	// Packet: 0x027d
	length_list[0x027d] = 62;

	// Packet: 0x027e
	length_list[0x027e] = -1;

	// Packet: 0x027f
	length_list[0x027f] = 8;

	// Packet: 0x0280
	length_list[0x0280] = 12;

	// Packet: 0x0281
	if (packetver >= 20151209) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20151104) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20151014) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0281] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150729) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0281] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150429) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150325) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0281] = 8;
	} else if (packetver >= 20150225) {
		length_list[0x0281] = 4;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150211) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0281] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x0281] = -1;
	}

	// Packet: 0x0282
	length_list[0x0282] = 284;

	// Packet: 0x0283
	length_list[0x0283] = 6;

	// Packet: 0x0284
	length_list[0x0284] = 14;

	// Packet: 0x0285
	length_list[0x0285] = 6;

	// Packet: 0x0286
	length_list[0x0286] = 4;

	// Packet: 0x0287
	length_list[0x0287] = -1;

	// Packet: 0x0288
	length_list[0x0288] = -1;

	// Packet: 0x0289
	length_list[0x0289] = 12;

	// Packet: 0x028a
	length_list[0x028a] = 18;

	// Packet: 0x028b
	length_list[0x028b] = -1;

	// Packet: 0x028c
	length_list[0x028c] = 46;

	// Packet: 0x028d
	length_list[0x028d] = 34;

	// Packet: 0x028e
	length_list[0x028e] = 4;

	// Packet: 0x028f
	length_list[0x028f] = 6;

	// Packet: 0x0290
	length_list[0x0290] = 4;

	// Packet: 0x0291
	length_list[0x0291] = 4;

	// Packet: 0x0292
	length_list[0x0292] = 2;

	// Packet: 0x0293
	length_list[0x0293] = 70;

	// Packet: 0x0294
	length_list[0x0294] = 10;

	// Packet: 0x0295
	length_list[0x0295] = -1;

	// Packet: 0x0296
	length_list[0x0296] = -1;

	// Packet: 0x0297
	length_list[0x0297] = -1;

	// Packet: 0x0298
	length_list[0x0298] = 8;

	// Packet: 0x0299
	length_list[0x0299] = 6;

	// Packet: 0x029a
	length_list[0x029a] = 27;

	// Packet: 0x029b
	length_list[0x029b] = 80;

	// Packet: 0x029c
	length_list[0x029c] = 66;

	// Packet: 0x029d
	length_list[0x029d] = -1;

	// Packet: 0x029e
	length_list[0x029e] = 11;

	// Packet: 0x029f
	length_list[0x029f] = 3;

	// Packet: 0x02a2
	length_list[0x02a2] = 8;

	// Packet: 0x02a5
	length_list[0x02a5] = 8;

	// Packet: 0x02a6
	length_list[0x02a6] = -1;

	// Packet: 0x02a7
	length_list[0x02a7] = -1;

	// Packet: 0x02aa
	length_list[0x02aa] = 4;

	// Packet: 0x02ab
	length_list[0x02ab] = 36;

	// Packet: 0x02ac
	length_list[0x02ac] = 6;

	// Packet: 0x02ad
	length_list[0x02ad] = 8;

	// Packet: 0x02b0
	length_list[0x02b0] = 85;

	// Packet: 0x02b1
	length_list[0x02b1] = -1;

	// Packet: 0x02b2
	length_list[0x02b2] = -1;

	// Packet: 0x02b3
	length_list[0x02b3] = 107;

	// Packet: 0x02b4
	length_list[0x02b4] = 6;

	// Packet: 0x02b5
	length_list[0x02b5] = -1;

	// Packet: 0x02b6
	length_list[0x02b6] = 7;

	// Packet: 0x02b7
	length_list[0x02b7] = 7;

	// Packet: 0x02b8
	length_list[0x02b8] = 22;

	// Packet: 0x02b9
	length_list[0x02b9] = 191;

	// Packet: 0x02ba
	length_list[0x02ba] = 11;

	// Packet: 0x02bb
	length_list[0x02bb] = 8;

	// Packet: 0x02bc
	length_list[0x02bc] = 6;

	// Packet: 0x02c1
	length_list[0x02c1] = -1;

	// Packet: 0x02c2
	length_list[0x02c2] = -1;

	// Packet: 0x02c4
	if (packetver >= 20151223) {
		length_list[0x02c4] = 8;
	} else if (packetver >= 20151118) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x02c4] = -1;
	} else if (packetver >= 20151028) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x02c4] = 36;
	} else if (packetver >= 20150715) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x02c4] = 90;
	} else if (packetver >= 20150520) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x02c4] = 8;
	} else if (packetver >= 20150304) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x02c4] = 26;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150107) {
		length_list[0x02c4] = 2;
	}

	// Packet: 0x02c5
	length_list[0x02c5] = 30;

	// Packet: 0x02c6
	length_list[0x02c6] = 30;

	// Packet: 0x02c7
	length_list[0x02c7] = 7;

	// Packet: 0x02c8
	length_list[0x02c8] = 3;

	// Packet: 0x02c9
	length_list[0x02c9] = 3;

	// Packet: 0x02ca
	length_list[0x02ca] = 3;

	// Packet: 0x02cb
	length_list[0x02cb] = 65;

	// Packet: 0x02cc
	length_list[0x02cc] = 4;

	// Packet: 0x02cd
	length_list[0x02cd] = 71;

	// Packet: 0x02ce
	length_list[0x02ce] = 10;

	// Packet: 0x02cf
	length_list[0x02cf] = 6;

	// Packet: 0x02d0
	length_list[0x02d0] = -1;

	// Packet: 0x02d1
	length_list[0x02d1] = -1;

	// Packet: 0x02d2
	length_list[0x02d2] = -1;

	// Packet: 0x02d3
	length_list[0x02d3] = 4;

	// Packet: 0x02d4
	length_list[0x02d4] = 29;

	// Packet: 0x02d5
	length_list[0x02d5] = 2;

	// Packet: 0x02d6
	length_list[0x02d6] = 6;

	// Packet: 0x02d7
	length_list[0x02d7] = -1;

	// Packet: 0x02d8
	length_list[0x02d8] = 10;

	// Packet: 0x02d9
	length_list[0x02d9] = 10;

	// Packet: 0x02da
	length_list[0x02da] = 3;

	// Packet: 0x02db
	length_list[0x02db] = -1;

	// Packet: 0x02dc
	length_list[0x02dc] = -1;

	// Packet: 0x02dd
	length_list[0x02dd] = 32;

	// Packet: 0x02de
	length_list[0x02de] = 6;

	// Packet: 0x02df
	length_list[0x02df] = 36;

	// Packet: 0x02e0
	length_list[0x02e0] = 34;

	// Packet: 0x02e1
	length_list[0x02e1] = 33;

	// Packet: 0x02e2
	length_list[0x02e2] = 20;

	// Packet: 0x02e3
	length_list[0x02e3] = 22;

	// Packet: 0x02e4
	length_list[0x02e4] = 11;

	// Packet: 0x02e5
	length_list[0x02e5] = 9;

	// Packet: 0x02e6
	length_list[0x02e6] = 6;

	// Packet: 0x02e7
	length_list[0x02e7] = -1;

	// Packet: 0x02e8
	length_list[0x02e8] = -1;

	// Packet: 0x02e9
	length_list[0x02e9] = -1;

	// Packet: 0x02ea
	length_list[0x02ea] = -1;

	// Packet: 0x02eb
	length_list[0x02eb] = 13;

	// Packet: 0x02ec
	length_list[0x02ec] = 67;

	// Packet: 0x02ed
	length_list[0x02ed] = 59;

	// Packet: 0x02ee
	length_list[0x02ee] = 60;

	// Packet: 0x02ef
	length_list[0x02ef] = 8;

	// Packet: 0x02f0
	length_list[0x02f0] = 10;

	// Packet: 0x02f1
	length_list[0x02f1] = 2;

	// Packet: 0x02f2
	length_list[0x02f2] = 2;

	// Packet: 0x02f3
	length_list[0x02f3] = -1;

	// Packet: 0x02f4
	length_list[0x02f4] = 3;

	// Packet: 0x02f5
	length_list[0x02f5] = 7;

	// Packet: 0x02f6
	length_list[0x02f6] = 7;

	// Packet: 0x035c
	length_list[0x035c] = 2;

	// Packet: 0x035d
	length_list[0x035d] = -1;

	// Packet: 0x035e
	length_list[0x035e] = 2;

	// Packet: 0x035f
	if (packetver >= 20151230) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20151125) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x035f] = 5;
	} else if (packetver >= 20151111) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20151104) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150916) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150826) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150715) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150603) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150520) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x035f] = 6;
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20150211) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x035f] = 6;
	}

	// Packet: 0x0360
	if (packetver >= 20151230) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20151125) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20151104) {
		length_list[0x0360] = 19;
	} else if (packetver >= 20151021) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150909) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0360] = 7;
	} else if (packetver >= 20150715) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150603) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150520) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150225) {
		length_list[0x0360] = 26;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150211) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0360] = 6;
	}

	// Packet: 0x0361
	if (packetver >= 20151223) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20151104) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20151021) {
		length_list[0x0361] = 8;
	} else if (packetver >= 20151007) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150923) {
		length_list[0x0361] = 18;
	} else if (packetver >= 20150916) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20150819) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150729) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0361] = 26;
	} else if (packetver >= 20150527) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150520) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20150429) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150415) {
		length_list[0x0361] = 8;
	} else if (packetver >= 20150225) {
		length_list[0x0361] = 5;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150211) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0361] = 2;
	}

	// Packet: 0x0362
	if (packetver >= 20151230) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0362] = 8;
	} else if (packetver >= 20151216) {
		length_list[0x0362] = 5;
	} else if (packetver >= 20151209) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20151118) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0362] = 5;
	} else if (packetver >= 20151104) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150902) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0362] = 36;
	} else if (packetver >= 20150819) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150722) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0362] = 10;
	} else if (packetver >= 20150701) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150603) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150429) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150408) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0362] = 7;
	} else if (packetver >= 20150311) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150225) {
		length_list[0x0362] = 6;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150211) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20150121) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0362] = 6;
	}

	// Packet: 0x0363
	if (packetver >= 20151209) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20151111) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0363] = 5;
	} else if (packetver >= 20151021) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20151014) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150819) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150701) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150617) {
		length_list[0x0363] = 36;
	} else if (packetver >= 20150603) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150520) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0363] = 19;
	} else if (packetver >= 20150507) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0363] = 5;
	} else if (packetver >= 20150422) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150401) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0363] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0363] = 6;
	} else if (packetver >= 20150225) {
		length_list[0x0363] = 6;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150211) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20150121) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0363] = 8;
	}

	// Packet: 0x0364
	if (packetver >= 20151230) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20151216) {
		length_list[0x0364] = 4;
	} else if (packetver >= 20151209) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20151111) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20151014) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150819) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150722) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0364] = -1;
	} else if (packetver >= 20150701) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0364] = 18;
	} else if (packetver >= 20150603) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150429) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150415) {
		length_list[0x0364] = 90;
	} else if (packetver >= 20150225) {
		length_list[0x0364] = 8;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150211) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20150121) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0364] = 8;
	}

	// Packet: 0x0365
	if (packetver >= 20151230) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20151223) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20151209) {
		length_list[0x0365] = 5;
	} else if (packetver >= 20151202) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20151125) {
		length_list[0x0365] = 5;
	} else if (packetver >= 20151118) {
		length_list[0x0365] = 26;
	} else if (packetver >= 20151104) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20151014) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150916) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0365] = 5;
	} else if (packetver >= 20150819) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150701) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0365] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150429) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150401) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0365] = 10;
	} else if (packetver >= 20150225) {
		length_list[0x0365] = 8;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150211) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20150128) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20150121) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0365] = 18;
	}

	// Packet: 0x0366
	if (packetver >= 20151230) {
		length_list[0x0366] = 8;
	} else if (packetver >= 20151209) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20151125) {
		length_list[0x0366] = 8;
	} else if (packetver >= 20151021) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20151014) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150923) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20150916) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150826) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150715) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0366] = 26;
	} else if (packetver >= 20150701) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150603) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150520) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150415) {
		length_list[0x0366] = 5;
	} else if (packetver >= 20150311) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0366] = 90;
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150211) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20150128) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0366] = 90;
	}

	// Packet: 0x0367
	if (packetver >= 20150909) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0367] = 6;
	} else if (packetver >= 20150408) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0367] = 6;
	} else if (packetver >= 20150325) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0367] = 90;
	} else if (packetver >= 20150225) {
		length_list[0x0367] = 90;
		// ignored packet from 2015-02-25iRagexe
	} else if (packetver >= 20150107) {
		length_list[0x0367] = 2;
	}

	// Packet: 0x0368
	if (packetver >= 20151230) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20151125) {
		length_list[0x0368] = -1;
	} else if (packetver >= 20151021) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150916) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150902) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0368] = 8;
	} else if (packetver >= 20150805) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0368] = 26;
	} else if (packetver >= 20150715) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0368] = 5;
	} else if (packetver >= 20150701) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150603) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150520) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20150217) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150211) {
		length_list[0x0368] = 5;
	} else if (packetver >= 20150204) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x0368] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0368] = 6;
	}

	// Packet: 0x0369
	if (packetver >= 20151230) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20151216) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0369] = 6;
	} else if (packetver >= 20151202) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20151125) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20151111) {
		length_list[0x0369] = 8;
	} else if (packetver >= 20151021) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20151014) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150916) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0369] = 36;
	} else if (packetver >= 20150826) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150715) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150701) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150603) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150520) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150311) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0369] = 7;
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150211) {
		length_list[0x0369] = 5;
	} else if (packetver >= 20150204) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20150128) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0369] = 7;
	}

	// Packet: 0x03dd
	length_list[0x03dd] = 18;

	// Packet: 0x03de
	length_list[0x03de] = 18;

	// Packet: 0x0436
	if (packetver >= 20151230) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20151223) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0436] = 12;
	} else if (packetver >= 20151209) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20151111) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0436] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20151007) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150902) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0436] = 18;
	} else if (packetver >= 20150805) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150729) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20150715) {
		length_list[0x0436] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20150701) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150617) {
		length_list[0x0436] = 19;
	} else if (packetver >= 20150603) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150429) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150318) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0436] = -1;
	} else if (packetver >= 20150304) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0436] = 19;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150211) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20150121) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20150114) {
		length_list[0x0436] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0436] = 4;
	}

	// Packet: 0x0437
	if (packetver >= 20151230) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20151209) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20151125) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20151104) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20151014) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150916) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0437] = 8;
	} else if (packetver >= 20150826) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150729) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20150722) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0437] = 10;
	} else if (packetver >= 20150701) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150610) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0437] = 8;
	} else if (packetver >= 20150527) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150520) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150408) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0437] = 7;
		// ignored packet from 2015-02-25iRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150211) {
		length_list[0x0437] = 12;
	} else if (packetver >= 20150204) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20150128) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0437] = 5;
	}

	// Packet: 0x0438
	if (packetver >= 20151209) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20151125) {
		length_list[0x0438] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20151014) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150916) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150826) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150729) {
		length_list[0x0438] = 5;
	} else if (packetver >= 20150722) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0438] = 5;
	} else if (packetver >= 20150708) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150701) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150610) {
		length_list[0x0438] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150520) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150318) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0438] = -1;
	} else if (packetver >= 20150217) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150211) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20150128) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0438] = 10;
	}

	// Packet: 0x0439
	length_list[0x0439] = 8;

	// Packet: 0x043d
	length_list[0x043d] = 8;

	// Packet: 0x043e
	length_list[0x043e] = -1;

	// Packet: 0x043f
	length_list[0x043f] = 25;

	// Packet: 0x0440
	length_list[0x0440] = 10;

	// Packet: 0x0441
	length_list[0x0441] = 4;

	// Packet: 0x0442
	length_list[0x0442] = -1;

	// Packet: 0x0443
	length_list[0x0443] = 8;

	// Packet: 0x0444
	length_list[0x0444] = -1;

	// Packet: 0x0445
	length_list[0x0445] = 10;

	// Packet: 0x0446
	length_list[0x0446] = 14;

	// Packet: 0x0447
	length_list[0x0447] = 2;

	// Packet: 0x0448
	length_list[0x0448] = -1;

	// Packet: 0x0449
	length_list[0x0449] = 4;

	// Packet: 0x044a
	length_list[0x044a] = 6;

	// Packet: 0x044b
	length_list[0x044b] = 2;

	// Packet: 0x07d7
	length_list[0x07d7] = 8;

	// Packet: 0x07d8
	length_list[0x07d8] = 8;

	// Packet: 0x07d9
	length_list[0x07d9] = 268;

	// Packet: 0x07da
	length_list[0x07da] = 6;

	// Packet: 0x07db
	length_list[0x07db] = 8;

	// Packet: 0x07dc
	length_list[0x07dc] = 6;

	// Packet: 0x07dd
	length_list[0x07dd] = 54;

	// Packet: 0x07de
	length_list[0x07de] = 30;

	// Packet: 0x07df
	length_list[0x07df] = 54;

	// Packet: 0x07e0
	length_list[0x07e0] = 58;

	// Packet: 0x07e1
	length_list[0x07e1] = 15;

	// Packet: 0x07e2
	length_list[0x07e2] = 8;

	// Packet: 0x07e3
	length_list[0x07e3] = 6;

	// Packet: 0x07e4
	if (packetver >= 20151216) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x07e4] = 26;
	} else if (packetver >= 20151202) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20151104) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x07e4] = 36;
	} else if (packetver >= 20150819) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x07e4] = 8;
	} else if (packetver >= 20150624) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x07e4] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150429) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150325) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x07e4] = 26;
	} else if (packetver >= 20150225) {
		length_list[0x07e4] = 2;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150204) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x07e4] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x07e4] = 6;
	}

	// Packet: 0x07e5
	length_list[0x07e5] = 4;

	// Packet: 0x07e6
	length_list[0x07e6] = 8;

	// Packet: 0x07e7
	length_list[0x07e7] = 32;

	// Packet: 0x07e8
	length_list[0x07e8] = -1;

	// Packet: 0x07e9
	length_list[0x07e9] = 5;

	// Packet: 0x07ea
	length_list[0x07ea] = 2;

	// Packet: 0x07eb
	length_list[0x07eb] = -1;

	// Packet: 0x07ec
	if (packetver >= 20151230) {
		length_list[0x07ec] = 90;
	} else if (packetver >= 20151216) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x07ec] = 6;
	} else if (packetver >= 20151202) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20151111) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x07ec] = 26;
	} else if (packetver >= 20151028) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20151021) {
		length_list[0x07ec] = 5;
	} else if (packetver >= 20151014) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150902) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x07ec] = 7;
	} else if (packetver >= 20150819) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150701) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150617) {
		length_list[0x07ec] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150429) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150225) {
		length_list[0x07ec] = 8;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150211) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20150121) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x07ec] = 8;
	}

	// Packet: 0x07ed
	length_list[0x07ed] = 10;

	// Packet: 0x07ee
	length_list[0x07ee] = 6;

	// Packet: 0x07ef
	length_list[0x07ef] = 8;

	// Packet: 0x07f0
	length_list[0x07f0] = 6;

	// Packet: 0x07f1
	length_list[0x07f1] = 18;

	// Packet: 0x07f2
	length_list[0x07f2] = 8;

	// Packet: 0x07f3
	length_list[0x07f3] = 6;

	// Packet: 0x07f4
	length_list[0x07f4] = 3;

	// Packet: 0x07f5
	length_list[0x07f5] = 6;

	// Packet: 0x07f6
	length_list[0x07f6] = 14;

	// Packet: 0x07f7
	length_list[0x07f7] = -1;

	// Packet: 0x07f8
	length_list[0x07f8] = -1;

	// Packet: 0x07f9
	length_list[0x07f9] = -1;

	// Packet: 0x07fa
	length_list[0x07fa] = 8;

	// Packet: 0x07fb
	length_list[0x07fb] = 25;

	// Packet: 0x07fc
	length_list[0x07fc] = 10;

	// Packet: 0x07fd
	length_list[0x07fd] = -1;

	// Packet: 0x07fe
	length_list[0x07fe] = 26;

	// Packet: 0x0800
	length_list[0x0800] = -1;

	// Packet: 0x0801
	length_list[0x0801] = -1;

	// Packet: 0x0802
	if (packetver >= 20151230) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0802] = 4;
	} else if (packetver >= 20151209) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20151125) {
		length_list[0x0802] = -1;
	} else if (packetver >= 20151118) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0802] = 4;
	} else if (packetver >= 20151104) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20151014) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150909) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0802] = 36;
	} else if (packetver >= 20150819) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150708) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0802] = 6;
	} else if (packetver >= 20150624) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150603) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150429) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150401) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0802] = 4;
	} else if (packetver >= 20150318) {
		length_list[0x0802] = -1;
	} else if (packetver >= 20150311) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0802] = 4;
	} else if (packetver >= 20150225) {
		length_list[0x0802] = 18;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150211) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20150121) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0802] = 26;
	}

	// Packet: 0x0803
	length_list[0x0803] = 4;

	// Packet: 0x0804
	length_list[0x0804] = 14;

	// Packet: 0x0805
	length_list[0x0805] = -1;

	// Packet: 0x0806
	length_list[0x0806] = 2;

	// Packet: 0x0807
	length_list[0x0807] = 4;

	// Packet: 0x0808
	length_list[0x0808] = 14;

	// Packet: 0x0809
	length_list[0x0809] = 50;

	// Packet: 0x080a
	length_list[0x080a] = 18;

	// Packet: 0x080b
	length_list[0x080b] = 6;

	// Packet: 0x080c
	length_list[0x080c] = 2;

	// Packet: 0x080d
	length_list[0x080d] = 3;

	// Packet: 0x080e
	length_list[0x080e] = 14;

	// Packet: 0x080f
	length_list[0x080f] = 20;

	// Packet: 0x0810
	length_list[0x0810] = 3;

	// Packet: 0x0811
	if (packetver >= 20151230) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20151216) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0811] = 6;
	} else if (packetver >= 20151202) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20151125) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20151104) {
		length_list[0x0811] = 18;
	} else if (packetver >= 20151021) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20151014) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20151007) {
		length_list[0x0811] = 26;
	} else if (packetver >= 20151001) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150916) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150715) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0811] = 19;
	} else if (packetver >= 20150701) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x0811] = 8;
	} else if (packetver >= 20150610) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0811] = 5;
	} else if (packetver >= 20150527) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150520) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150325) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0811] = 19;
	} else if (packetver >= 20150311) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150217) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150211) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0811] = -1;
	}

	// Packet: 0x0812
	length_list[0x0812] = 8;

	// Packet: 0x0813
	length_list[0x0813] = -1;

	// Packet: 0x0814
	length_list[0x0814] = 86;

	// Packet: 0x0815
	if (packetver >= 20151230) {
		length_list[0x0815] = 19;
	} else if (packetver >= 20151223) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20151209) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20151125) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20151111) {
		length_list[0x0815] = 26;
	} else if (packetver >= 20151021) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20151014) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150916) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150715) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150701) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150603) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150520) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150318) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0815] = 19;
	} else if (packetver >= 20150225) {
		length_list[0x0815] = 2;
		// ignored packet from 2015-02-25iRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150211) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0815] = -1;
	}

	// Packet: 0x0816
	length_list[0x0816] = 6;

	// Packet: 0x0817
	if (packetver >= 20151021) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0817] = 26;
	} else if (packetver >= 20151001) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0817] = -1;
	} else if (packetver >= 20150729) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0817] = 19;
	} else if (packetver >= 20150520) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0817] = 5;
	} else if (packetver >= 20150507) {
		length_list[0x0817] = 6;
	} else if (packetver >= 20150422) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0817] = 6;
	} else if (packetver >= 20150304) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0817] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0817] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0817] = 2;
	}

	// Packet: 0x0818
	length_list[0x0818] = -1;

	// Packet: 0x0819
	if (packetver >= 20151230) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20151209) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20151125) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20151014) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150916) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150902) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0819] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150715) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150701) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150610) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0819] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150520) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150415) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0819] = 8;
	} else if (packetver >= 20150401) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0819] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150225) {
		length_list[0x0819] = 2;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150211) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0819] = -1;
	}

	// Packet: 0x081a
	length_list[0x081a] = 4;

	// Packet: 0x081b
	length_list[0x081b] = 10;

	// Packet: 0x081c
	length_list[0x081c] = 10;

	// Packet: 0x081d
	length_list[0x081d] = 22;

	// Packet: 0x081e
	length_list[0x081e] = 8;

	// Packet: 0x081f
	length_list[0x081f] = -1;

	// Packet: 0x0820
	length_list[0x0820] = 11;

	// Packet: 0x0821
	length_list[0x0821] = 2;

	// Packet: 0x0822
	length_list[0x0822] = 9;

	// Packet: 0x0823
	length_list[0x0823] = -1;

	// Packet: 0x0824
	length_list[0x0824] = 6;

	// Packet: 0x0825
	length_list[0x0825] = -1;

	// Packet: 0x0827
	length_list[0x0827] = 6;

	// Packet: 0x0828
	length_list[0x0828] = 14;

	// Packet: 0x0829
	length_list[0x0829] = 12;

	// Packet: 0x082a
	length_list[0x082a] = 10;

	// Packet: 0x082b
	length_list[0x082b] = 6;

	// Packet: 0x082c
	length_list[0x082c] = 10;

	// Packet: 0x082d
	length_list[0x082d] = -1;

	// Packet: 0x0835
	if (packetver >= 20150923) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0835] = 6;
	} else if (packetver >= 20150722) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0835] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0835] = 8;
	} else if (packetver >= 20150422) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0835] = 6;
	} else if (packetver >= 20150225) {
		length_list[0x0835] = 2;
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0835] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0835] = 2;
	}

	// Packet: 0x0836
	length_list[0x0836] = -1;

	// Packet: 0x0837
	length_list[0x0837] = 3;

	// Packet: 0x0838
	if (packetver >= 20151230) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20151209) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20151125) {
		length_list[0x0838] = 18;
	} else if (packetver >= 20151021) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20151014) {
		length_list[0x0838] = -1;
	} else if (packetver >= 20151001) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150916) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150826) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150715) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150701) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150603) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0838] = 10;
	} else if (packetver >= 20150520) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150318) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0838] = 10;
	} else if (packetver >= 20150225) {
		length_list[0x0838] = 12;
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150211) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20150128) {
		length_list[0x0838] = 7;
	} else if (packetver >= 20150107) {
		length_list[0x0838] = 12;
	}

	// Packet: 0x0839
	length_list[0x0839] = 66;

	// Packet: 0x083a
	length_list[0x083a] = 5;

	// Packet: 0x083b
	length_list[0x083b] = 2;

	// Packet: 0x083c
	if (packetver >= 20151230) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20151209) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20151125) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20151014) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150916) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150902) {
		length_list[0x083c] = -1;
	} else if (packetver >= 20150819) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150722) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x083c] = 7;
	} else if (packetver >= 20150708) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150701) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150603) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x083c] = 36;
	} else if (packetver >= 20150520) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150408) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x083c] = -1;
	} else if (packetver >= 20150311) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x083c] = 12;
		// ignored packet from 2015-02-25iRagexe
	} else if (packetver >= 20150217) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150211) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150128) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20150114) {
		length_list[0x083c] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x083c] = 10;
	}

	// Packet: 0x083d
	length_list[0x083d] = 6;

	// Packet: 0x083e
	length_list[0x083e] = 26;

	// Packet: 0x0840
	length_list[0x0840] = -1;

	// Packet: 0x0841
	length_list[0x0841] = 4;

	// Packet: 0x0842
	length_list[0x0842] = 6;

	// Packet: 0x0843
	length_list[0x0843] = 6;

	// Packet: 0x0844
	length_list[0x0844] = 2;

	// Packet: 0x0845
	length_list[0x0845] = 10;

	// Packet: 0x0846
	length_list[0x0846] = 4;

	// Packet: 0x0847
	length_list[0x0847] = -1;

	// Packet: 0x0848
	length_list[0x0848] = -1;

	// Packet: 0x0849
	length_list[0x0849] = 16;

	// Packet: 0x084a
	length_list[0x084a] = 2;

	// Packet: 0x084b
	length_list[0x084b] = 19;

	// Packet: 0x084c
	length_list[0x084c] = 10;

	// Packet: 0x084d
	length_list[0x084d] = 10;

	// Packet: 0x084e
	length_list[0x084e] = 5;

	// Packet: 0x084f
	length_list[0x084f] = 6;

	// Packet: 0x0850
	length_list[0x0850] = 7;

	// Packet: 0x0851
	length_list[0x0851] = -1;

	// Packet: 0x0852
	length_list[0x0852] = 2;

	// Packet: 0x0853
	length_list[0x0853] = -1;

	// Packet: 0x0854
	length_list[0x0854] = -1;

	// Packet: 0x0855
	length_list[0x0855] = 6;

	// Packet: 0x0856
	length_list[0x0856] = -1;

	// Packet: 0x0857
	length_list[0x0857] = -1;

	// Packet: 0x0858
	length_list[0x0858] = -1;

	// Packet: 0x0859
	length_list[0x0859] = -1;

	// Packet: 0x085a
	if (packetver >= 20151021) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x085a] = 10;
	} else if (packetver >= 20150513) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x085a] = 8;
	} else if (packetver >= 20150415) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x085a] = 6;
	} else if (packetver >= 20150204) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x085a] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x085a] = 2;
	}

	// Packet: 0x085b
	if (packetver >= 20151230) {
		length_list[0x085b] = -1;
	} else if (packetver >= 20151223) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x085b] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x085b] = 36;
	} else if (packetver >= 20150909) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x085b] = 7;
	} else if (packetver >= 20150805) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x085b] = 8;
	} else if (packetver >= 20150225) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20150217) {
		length_list[0x085b] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x085b] = 2;
	}

	// Packet: 0x085c
	if (packetver >= 20151021) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x085c] = 7;
	} else if (packetver >= 20151001) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x085c] = 6;
	} else if (packetver >= 20150722) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x085c] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x085c] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x085c] = 2;
	}

	// Packet: 0x085d
	if (packetver >= 20151216) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x085d] = 6;
	} else if (packetver >= 20151118) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x085d] = -1;
	} else if (packetver >= 20151001) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x085d] = 26;
	} else if (packetver >= 20150909) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x085d] = 6;
	} else if (packetver >= 20150826) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x085d] = 8;
	} else if (packetver >= 20150401) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x085d] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x085d] = 2;
	}

	// Packet: 0x085e
	if (packetver >= 20151216) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x085e] = -1;
	} else if (packetver >= 20151202) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x085e] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x085e] = 8;
	} else if (packetver >= 20150715) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x085e] = 8;
	} else if (packetver >= 20150527) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x085e] = 6;
	} else if (packetver >= 20150422) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x085e] = 4;
	} else if (packetver >= 20150408) {
		length_list[0x085e] = 5;
	} else if (packetver >= 20150401) {
		length_list[0x085e] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x085e] = 2;
	}

	// Packet: 0x085f
	if (packetver >= 20151202) {
		length_list[0x085f] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x085f] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x085f] = 2;
	}

	// Packet: 0x0860
	if (packetver >= 20151104) {
		length_list[0x0860] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x0860] = 36;
	} else if (packetver >= 20151021) {
		length_list[0x0860] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0860] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x0860] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0860] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x0860] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0860] = 90;
	} else if (packetver >= 20150107) {
		length_list[0x0860] = 2;
	}

	// Packet: 0x0861
	if (packetver >= 20151230) {
		length_list[0x0861] = 26;
	} else if (packetver >= 20151216) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0861] = -1;
	} else if (packetver >= 20150916) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0861] = 4;
	} else if (packetver >= 20150902) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0861] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0861] = 2;
	}

	// Packet: 0x0862
	if (packetver >= 20151118) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0862] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20151007) {
		length_list[0x0862] = 4;
	} else if (packetver >= 20150826) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0862] = 8;
	} else if (packetver >= 20150325) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0862] = -1;
	} else if (packetver >= 20150311) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0862] = 8;
	} else if (packetver >= 20150217) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0862] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0862] = 2;
	}

	// Packet: 0x0863
	if (packetver >= 20151021) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0863] = 5;
	} else if (packetver >= 20150909) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0863] = 10;
	} else if (packetver >= 20150422) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0863] = -1;
	} else if (packetver >= 20150325) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0863] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0863] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0863] = 2;
	}

	// Packet: 0x0864
	if (packetver >= 20151230) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0864] = 6;
	} else if (packetver >= 20151216) {
		length_list[0x0864] = 8;
	} else if (packetver >= 20151001) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0864] = 5;
	} else if (packetver >= 20150610) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0864] = 26;
	} else if (packetver >= 20150520) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0864] = 8;
	} else if (packetver >= 20150507) {
		length_list[0x0864] = 26;
	} else if (packetver >= 20150204) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0864] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0864] = 2;
	}

	// Packet: 0x0865
	if (packetver >= 20151223) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0865] = 10;
	} else if (packetver >= 20150902) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0865] = 5;
	} else if (packetver >= 20150819) {
		length_list[0x0865] = 26;
	} else if (packetver >= 20150527) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0865] = 19;
	} else if (packetver >= 20150415) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0865] = 7;
	} else if (packetver >= 20150107) {
		length_list[0x0865] = 2;
	}

	// Packet: 0x0866
	if (packetver >= 20151230) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0866] = 19;
	} else if (packetver >= 20151216) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0866] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0866] = 2;
	}

	// Packet: 0x0867
	if (packetver >= 20151021) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0867] = -1;
	} else if (packetver >= 20150610) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0867] = 8;
	} else if (packetver >= 20150507) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0867] = 36;
	} else if (packetver >= 20150422) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0867] = 26;
	} else if (packetver >= 20150304) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0867] = 18;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150107) {
		length_list[0x0867] = 2;
	}

	// Packet: 0x0868
	if (packetver >= 20150527) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0868] = -1;
	} else if (packetver >= 20150422) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0868] = 10;
	} else if (packetver >= 20150408) {
		length_list[0x0868] = -1;
	} else if (packetver >= 20150121) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20150114) {
		length_list[0x0868] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0868] = 2;
	}

	// Packet: 0x0869
	if (packetver >= 20151230) {
		length_list[0x0869] = 5;
	} else if (packetver >= 20150923) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0869] = 7;
	} else if (packetver >= 20150729) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0869] = 6;
	} else if (packetver >= 20150624) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0869] = 8;
	} else if (packetver >= 20150422) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0869] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0869] = 2;
	}

	// Packet: 0x086a
	if (packetver >= 20151223) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x086a] = 8;
	} else if (packetver >= 20151028) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x086a] = 26;
	} else if (packetver >= 20150624) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x086a] = 18;
	} else if (packetver >= 20150610) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x086a] = 8;
	} else if (packetver >= 20150507) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x086a] = 19;
	} else if (packetver >= 20150318) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x086a] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x086a] = 2;
	}

	// Packet: 0x086b
	if (packetver >= 20151125) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x086b] = 8;
	} else if (packetver >= 20150902) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x086b] = 10;
	} else if (packetver >= 20150624) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x086b] = 5;
	} else if (packetver >= 20150415) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x086b] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x086b] = 2;
	}

	// Packet: 0x086c
	if (packetver >= 20150805) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x086c] = 8;
	} else if (packetver >= 20150422) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x086c] = 7;
	} else if (packetver >= 20150318) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x086c] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x086c] = 2;
	}

	// Packet: 0x086d
	if (packetver >= 20150805) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x086d] = 18;
	} else if (packetver >= 20150708) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x086d] = -1;
	} else if (packetver >= 20150311) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x086d] = -1;
	} else if (packetver >= 20150204) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x086d] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x086d] = 2;
	}

	// Packet: 0x086e
	if (packetver >= 20151230) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x086e] = 10;
	} else if (packetver >= 20151216) {
		length_list[0x086e] = 26;
	} else if (packetver >= 20150805) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x086e] = 5;
	} else if (packetver >= 20150415) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x086e] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x086e] = 2;
	}

	// Packet: 0x086f
	if (packetver >= 20151001) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x086f] = 10;
	} else if (packetver >= 20150909) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x086f] = 10;
	} else if (packetver >= 20150805) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x086f] = 8;
	} else if (packetver >= 20150722) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x086f] = 8;
	} else if (packetver >= 20150408) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x086f] = 5;
	} else if (packetver >= 20150325) {
		length_list[0x086f] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x086f] = 2;
	}

	// Packet: 0x0870
	if (packetver >= 20151223) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0870] = -1;
	} else if (packetver >= 20151209) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x0870] = 36;
	} else if (packetver >= 20151001) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0870] = 5;
	} else if (packetver >= 20150902) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0870] = -1;
	} else if (packetver >= 20150805) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0870] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0870] = 36;
	} else if (packetver >= 20150610) {
		length_list[0x0870] = 26;
	} else if (packetver >= 20150217) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0870] = 6;
	} else if (packetver >= 20150204) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0870] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0870] = 2;
	}

	// Packet: 0x0871
	if (packetver >= 20151118) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0871] = 26;
	} else if (packetver >= 20150916) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0871] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0871] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x0871] = 2;
	}

	// Packet: 0x0872
	if (packetver >= 20151230) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0872] = 18;
	} else if (packetver >= 20151021) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0872] = 5;
	} else if (packetver >= 20150715) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0872] = 2;
	}

	// Packet: 0x0873
	if (packetver >= 20150923) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0873] = 90;
	} else if (packetver >= 20150722) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0873] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0873] = 7;
	} else if (packetver >= 20150325) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0873] = 36;
	} else if (packetver >= 20150217) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0873] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0873] = 2;
	}

	// Packet: 0x0874
	if (packetver >= 20151223) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0874] = 90;
	} else if (packetver >= 20151021) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0874] = 10;
	} else if (packetver >= 20150204) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0874] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0874] = 2;
	}

	// Packet: 0x0875
	if (packetver >= 20151230) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0875] = 10;
	} else if (packetver >= 20150408) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0875] = -1;
	} else if (packetver >= 20150204) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0875] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0875] = 2;
	}

	// Packet: 0x0876
	if (packetver >= 20151230) {
		length_list[0x0876] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0876] = 6;
	} else if (packetver >= 20150204) {
		length_list[0x0876] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0876] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0876] = 2;
	}

	// Packet: 0x0877
	if (packetver >= 20150923) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0877] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0877] = 6;
	} else if (packetver >= 20150603) {
		length_list[0x0877] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0877] = 2;
	}

	// Packet: 0x0878
	if (packetver >= 20150729) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0878] = 10;
	} else if (packetver >= 20150415) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0878] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0878] = 2;
	}

	// Packet: 0x0879
	if (packetver >= 20151001) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0879] = 8;
	} else if (packetver >= 20150722) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0879] = 19;
	} else if (packetver >= 20150520) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0879] = 8;
	} else if (packetver >= 20150311) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0879] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0879] = 2;
	}

	// Packet: 0x087a
	if (packetver >= 20151216) {
		length_list[0x087a] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x087a] = 12;
	} else if (packetver >= 20150624) {
		length_list[0x087a] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x087a] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x087a] = 2;
	}

	// Packet: 0x087b
	if (packetver >= 20150916) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x087b] = 18;
	} else if (packetver >= 20150902) {
		length_list[0x087b] = 5;
	} else if (packetver >= 20150826) {
		length_list[0x087b] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x087b] = 18;
	} else if (packetver >= 20150318) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x087b] = 18;
	} else if (packetver >= 20150217) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x087b] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x087b] = 2;
	}

	// Packet: 0x087c
	if (packetver >= 20150722) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x087c] = 6;
	} else if (packetver >= 20150401) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x087c] = 8;
	} else if (packetver >= 20150128) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x087c] = 4;
	} else if (packetver >= 20150114) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x087c] = 26;
	}

	// Packet: 0x087d
	if (packetver >= 20150729) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x087d] = 5;
	} else if (packetver >= 20150708) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x087d] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x087d] = 12;
	} else if (packetver >= 20150204) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x087d] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x087d] = 2;
	}

	// Packet: 0x087e
	if (packetver >= 20150729) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x087e] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x087e] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x087e] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x087e] = 6;
	} else if (packetver >= 20150401) {
		length_list[0x087e] = 8;
	} else if (packetver >= 20150325) {
		length_list[0x087e] = 5;
	} else if (packetver >= 20150311) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x087e] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x087e] = 2;
	}

	// Packet: 0x087f
	if (packetver >= 20151216) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x087f] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x087f] = 12;
	} else if (packetver >= 20150909) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x087f] = 4;
	} else if (packetver >= 20150819) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150812) {
		length_list[0x087f] = 36;
	} else if (packetver >= 20150715) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x087f] = 4;
	} else if (packetver >= 20150415) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x087f] = 90;
	} else if (packetver >= 20150217) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x087f] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x087f] = 2;
	}

	// Packet: 0x0880
	if (packetver >= 20150805) {
		length_list[0x0880] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0880] = 12;
	} else if (packetver >= 20150722) {
		length_list[0x0880] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x0880] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0880] = 26;
	} else if (packetver >= 20150422) {
		length_list[0x0880] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0880] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0880] = 2;
	}

	// Packet: 0x0881
	if (packetver >= 20151230) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0881] = 7;
	} else if (packetver >= 20151021) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0881] = 8;
	} else if (packetver >= 20150923) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0881] = -1;
	} else if (packetver >= 20150805) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0881] = 7;
	} else if (packetver >= 20150610) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0881] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0881] = 2;
	}

	// Packet: 0x0882
	if (packetver >= 20150527) {
		length_list[0x0882] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0882] = -1;
	} else if (packetver >= 20150217) {
		length_list[0x0882] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0882] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0882] = 2;
	}

	// Packet: 0x0883
	if (packetver >= 20151202) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0883] = 5;
	} else if (packetver >= 20151021) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0883] = 6;
	} else if (packetver >= 20150916) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0883] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0883] = -1;
	} else if (packetver >= 20150520) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0883] = 18;
	} else if (packetver >= 20150401) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0883] = 12;
	} else if (packetver >= 20150318) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0883] = -1;
	} else if (packetver >= 20150217) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0883] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0883] = 2;
	}

	// Packet: 0x0884
	if (packetver >= 20151230) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0884] = 8;
	} else if (packetver >= 20151202) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0884] = 36;
	} else if (packetver >= 20151021) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0884] = -1;
	} else if (packetver >= 20150729) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0884] = 5;
	} else if (packetver >= 20150715) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0884] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0884] = 8;
	} else if (packetver >= 20150603) {
		length_list[0x0884] = 4;
	} else if (packetver >= 20150107) {
		length_list[0x0884] = 2;
	}

	// Packet: 0x0885
	if (packetver >= 20151223) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0885] = 36;
	} else if (packetver >= 20151202) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0885] = 4;
	} else if (packetver >= 20151118) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0885] = 7;
	} else if (packetver >= 20150617) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0885] = 5;
	} else if (packetver >= 20150520) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0885] = 6;
	} else if (packetver >= 20150401) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0885] = 10;
	} else if (packetver >= 20150304) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0885] = 4;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0885] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0885] = 2;
	}

	// Packet: 0x0886
	if (packetver >= 20151230) {
		length_list[0x0886] = 10;
	} else if (packetver >= 20151223) {
		length_list[0x0886] = -1;
	} else if (packetver >= 20151111) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20151001) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0886] = 5;
	} else if (packetver >= 20150916) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0886] = 19;
	} else if (packetver >= 20150902) {
		length_list[0x0886] = 12;
	} else if (packetver >= 20150805) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20150722) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0886] = 8;
	} else if (packetver >= 20150624) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0886] = 5;
	} else if (packetver >= 20150507) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0886] = 26;
	} else if (packetver >= 20150318) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0886] = 5;
	} else if (packetver >= 20150217) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0886] = 2;
	}

	// Packet: 0x0887
	if (packetver >= 20151111) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0887] = 6;
	} else if (packetver >= 20150909) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0887] = 8;
	} else if (packetver >= 20150513) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0887] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0887] = 2;
	}

	// Packet: 0x0888
	if (packetver >= 20150826) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0888] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0888] = 19;
	} else if (packetver >= 20150415) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0888] = 8;
	} else if (packetver >= 20150204) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0888] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0888] = 2;
	}

	// Packet: 0x0889
	if (packetver >= 20151021) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0889] = 6;
	} else if (packetver >= 20150513) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0889] = 18;
	} else if (packetver >= 20150415) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0889] = 36;
	} else if (packetver >= 20150325) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0889] = 7;
	} else if (packetver >= 20150107) {
		length_list[0x0889] = 2;
	}

	// Packet: 0x088a
	if (packetver >= 20150812) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x088a] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x088a] = 2;
	}

	// Packet: 0x088b
	if (packetver >= 20151223) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x088b] = 6;
	} else if (packetver >= 20151125) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x088b] = 36;
	} else if (packetver >= 20151111) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x088b] = 8;
	} else if (packetver >= 20150902) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x088b] = 26;
	} else if (packetver >= 20150610) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x088b] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x088b] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x088b] = 2;
	}

	// Packet: 0x088c
	if (packetver >= 20151202) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x088c] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x088c] = 8;
	} else if (packetver >= 20150527) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x088c] = -1;
	} else if (packetver >= 20150408) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x088c] = 6;
	} else if (packetver >= 20150325) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x088c] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x088c] = 2;
	}

	// Packet: 0x088d
	if (packetver >= 20151230) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x088d] = 5;
	} else if (packetver >= 20151202) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x088d] = 19;
	} else if (packetver >= 20151111) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x088d] = 5;
	} else if (packetver >= 20150909) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x088d] = 8;
	} else if (packetver >= 20150826) {
		length_list[0x088d] = 26;
	} else if (packetver >= 20150729) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x088d] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x088d] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x088d] = 2;
	}

	// Packet: 0x088e
	if (packetver >= 20151230) {
		length_list[0x088e] = 26;
	} else if (packetver >= 20151216) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x088e] = 10;
	} else if (packetver >= 20151021) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x088e] = 8;
	} else if (packetver >= 20151001) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x088e] = 6;
	} else if (packetver >= 20150422) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x088e] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x088e] = 2;
	}

	// Packet: 0x088f
	if (packetver >= 20151216) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x088f] = 8;
	} else if (packetver >= 20150916) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x088f] = 26;
	} else if (packetver >= 20150617) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x088f] = 5;
	} else if (packetver >= 20150507) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x088f] = 5;
	} else if (packetver >= 20150408) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x088f] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x088f] = 2;
	}

	// Packet: 0x0890
	if (packetver >= 20151230) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0890] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0890] = 2;
	}

	// Packet: 0x0891
	if (packetver >= 20151230) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0891] = -1;
	} else if (packetver >= 20150902) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0891] = 10;
	} else if (packetver >= 20150422) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0891] = 6;
	} else if (packetver >= 20150408) {
		length_list[0x0891] = -1;
	} else if (packetver >= 20150401) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0891] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0891] = 2;
	}

	// Packet: 0x0892
	if (packetver >= 20151001) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0892] = -1;
	} else if (packetver >= 20150909) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0892] = -1;
	} else if (packetver >= 20150311) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0892] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x0892] = 2;
	}

	// Packet: 0x0893
	if (packetver >= 20150708) {
		length_list[0x0893] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0893] = 4;
	} else if (packetver >= 20150401) {
		length_list[0x0893] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0893] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0893] = 2;
	}

	// Packet: 0x0894
	if (packetver >= 20151216) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0894] = 10;
	} else if (packetver >= 20150624) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x0894] = 4;
	} else if (packetver >= 20150507) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0894] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0894] = 2;
	}

	// Packet: 0x0895
	if (packetver >= 20151001) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0895] = 4;
	} else if (packetver >= 20150916) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0895] = 26;
	} else if (packetver >= 20150722) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0895] = -1;
	} else if (packetver >= 20150408) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0895] = 8;
	} else if (packetver >= 20150114) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0895] = 36;
	}

	// Packet: 0x0896
	if (packetver >= 20150722) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0896] = 90;
	} else if (packetver >= 20150318) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0896] = 7;
	} else if (packetver >= 20150304) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0896] = 2;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150107) {
		length_list[0x0896] = 2;
	}

	// Packet: 0x0897
	if (packetver >= 20151230) {
		length_list[0x0897] = 6;
	} else if (packetver >= 20150909) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0897] = 8;
	} else if (packetver >= 20150722) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0897] = 26;
	} else if (packetver >= 20150617) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0897] = 26;
	} else if (packetver >= 20150603) {
		length_list[0x0897] = -1;
	} else if (packetver >= 20150401) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0897] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0897] = 2;
	}

	// Packet: 0x0898
	if (packetver >= 20151230) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0898] = 90;
	} else if (packetver >= 20150422) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0898] = -1;
	} else if (packetver >= 20150408) {
		length_list[0x0898] = 6;
	} else if (packetver >= 20150401) {
		length_list[0x0898] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0898] = 2;
	}

	// Packet: 0x0899
	if (packetver >= 20151202) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0899] = 26;
	} else if (packetver >= 20150909) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0899] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0899] = 8;
	} else if (packetver >= 20150507) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0899] = 8;
	} else if (packetver >= 20150401) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0899] = 18;
	} else if (packetver >= 20150121) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20150114) {
		length_list[0x0899] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0899] = 2;
	}

	// Packet: 0x089a
	if (packetver >= 20151021) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x089a] = 4;
	} else if (packetver >= 20150805) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x089a] = 10;
	} else if (packetver >= 20150722) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x089a] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x089a] = 12;
	} else if (packetver >= 20150311) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x089a] = 26;
	} else if (packetver >= 20150204) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x089a] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x089a] = 2;
	}

	// Packet: 0x089b
	if (packetver >= 20151001) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x089b] = 10;
	} else if (packetver >= 20150916) {
		length_list[0x089b] = 18;
	} else if (packetver >= 20150805) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x089b] = 26;
	} else if (packetver >= 20150304) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x089b] = 8;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150107) {
		length_list[0x089b] = 2;
	}

	// Packet: 0x089c
	if (packetver >= 20151202) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x089c] = 7;
	} else if (packetver >= 20151118) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x089c] = 5;
	} else if (packetver >= 20150923) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x089c] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x089c] = 36;
	} else if (packetver >= 20150415) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x089c] = 5;
	} else if (packetver >= 20150401) {
		length_list[0x089c] = 36;
	} else if (packetver >= 20150325) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x089c] = 8;
	} else if (packetver >= 20150304) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x089c] = 8;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x089c] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x089c] = 2;
	}

	// Packet: 0x089d
	if (packetver >= 20151223) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x089d] = 7;
	} else if (packetver >= 20150715) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x089d] = 5;
	} else if (packetver >= 20150610) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x089d] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x089d] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x089d] = 2;
	}

	// Packet: 0x089e
	if (packetver >= 20151223) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x089e] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x089e] = 26;
	} else if (packetver >= 20150610) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x089e] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x089e] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x089e] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x089e] = 2;
	}

	// Packet: 0x089f
	if (packetver >= 20151202) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x089f] = -1;
	} else if (packetver >= 20151021) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x089f] = 5;
	} else if (packetver >= 20151001) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x089f] = 8;
	} else if (packetver >= 20150507) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x089f] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x089f] = 2;
	}

	// Packet: 0x08a0
	if (packetver >= 20151001) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x08a0] = 6;
	} else if (packetver >= 20150902) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x08a0] = 12;
	} else if (packetver >= 20150708) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x08a0] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x08a0] = 36;
	} else if (packetver >= 20150422) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x08a0] = -1;
	} else if (packetver >= 20150217) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x08a0] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x08a0] = 2;
	}

	// Packet: 0x08a1
	if (packetver >= 20151216) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x08a1] = 5;
	} else if (packetver >= 20150902) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x08a1] = 8;
	} else if (packetver >= 20150610) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x08a1] = 10;
	} else if (packetver >= 20150401) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x08a1] = 7;
	} else if (packetver >= 20150318) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x08a1] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x08a1] = 2;
	}

	// Packet: 0x08a2
	if (packetver >= 20151223) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x08a2] = 5;
	} else if (packetver >= 20151001) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x08a2] = 19;
	} else if (packetver >= 20150527) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x08a2] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x08a2] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x08a2] = 2;
	}

	// Packet: 0x08a3
	if (packetver >= 20151111) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x08a3] = 4;
	} else if (packetver >= 20150805) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x08a3] = 4;
	} else if (packetver >= 20150318) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x08a3] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x08a3] = 2;
	}

	// Packet: 0x08a4
	if (packetver >= 20150902) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x08a4] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x08a4] = 5;
	} else if (packetver >= 20150722) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x08a4] = 4;
	} else if (packetver >= 20150708) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x08a4] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x08a4] = 26;
	} else if (packetver >= 20150325) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x08a4] = -1;
	} else if (packetver >= 20150304) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x08a4] = 36;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x08a4] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x08a4] = 2;
	}

	// Packet: 0x08a5
	if (packetver >= 20151111) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x08a5] = 26;
	} else if (packetver >= 20151001) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x08a5] = 6;
	} else if (packetver >= 20150715) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x08a5] = -1;
	} else if (packetver >= 20150701) {
		length_list[0x08a5] = 26;
	} else if (packetver >= 20150408) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x08a5] = 8;
	} else if (packetver >= 20150318) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x08a5] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x08a5] = 2;
	}

	// Packet: 0x08a6
	if (packetver >= 20151001) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x08a6] = 12;
	} else if (packetver >= 20150708) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x08a6] = 10;
	} else if (packetver >= 20150507) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x08a6] = 8;
	} else if (packetver >= 20150318) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x08a6] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x08a6] = 2;
	}

	// Packet: 0x08a7
	if (packetver >= 20150401) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x08a7] = 90;
	} else if (packetver >= 20150107) {
		length_list[0x08a7] = 2;
	}

	// Packet: 0x08a8
	if (packetver >= 20150902) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x08a8] = -1;
	} else if (packetver >= 20150729) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x08a8] = 5;
	} else if (packetver >= 20150520) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x08a8] = 26;
	} else if (packetver >= 20150507) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x08a8] = 4;
	} else if (packetver >= 20150107) {
		length_list[0x08a8] = 2;
	}

	// Packet: 0x08a9
	if (packetver >= 20151223) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x08a9] = 6;
	} else if (packetver >= 20151202) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x08a9] = 6;
	} else if (packetver >= 20150909) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x08a9] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x08a9] = 2;
	}

	// Packet: 0x08aa
	if (packetver >= 20151230) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x08aa] = 26;
	} else if (packetver >= 20151021) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x08aa] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x08aa] = 7;
	} else if (packetver >= 20150107) {
		length_list[0x08aa] = 2;
	}

	// Packet: 0x08ab
	if (packetver >= 20151125) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x08ab] = 8;
	} else if (packetver >= 20150204) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x08ab] = 6;
	} else if (packetver >= 20150121) {
		length_list[0x08ab] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x08ab] = 2;
	}

	// Packet: 0x08ac
	if (packetver >= 20151223) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x08ac] = 8;
	} else if (packetver >= 20150923) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x08ac] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x08ac] = 26;
	} else if (packetver >= 20150722) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x08ac] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x08ac] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x08ac] = 2;
	}

	// Packet: 0x08ad
	if (packetver >= 20151202) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x08ad] = -1;
	} else if (packetver >= 20150805) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x08ad] = 6;
	} else if (packetver >= 20150715) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x08ad] = 36;
	} else if (packetver >= 20150701) {
		length_list[0x08ad] = 10;
	} else if (packetver >= 20150610) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x08ad] = 19;
	} else if (packetver >= 20150527) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x08ad] = 5;
	} else if (packetver >= 20150507) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x08ad] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x08ad] = 2;
	}

	// Packet: 0x08af
	length_list[0x08af] = 10;

	// Packet: 0x08b0
	length_list[0x08b0] = 17;

	// Packet: 0x08b1
	length_list[0x08b1] = -1;

	// Packet: 0x08b2
	length_list[0x08b2] = -1;

	// Packet: 0x08b3
	length_list[0x08b3] = -1;

	// Packet: 0x08b4
	length_list[0x08b4] = 2;

	// Packet: 0x08b5
	length_list[0x08b5] = 6;

	// Packet: 0x08b6
	length_list[0x08b6] = 3;

	// Packet: 0x08b8
	length_list[0x08b8] = 10;

	// Packet: 0x08b9
	length_list[0x08b9] = 12;

	// Packet: 0x08ba
	length_list[0x08ba] = 10;

	// Packet: 0x08bb
	length_list[0x08bb] = 8;

	// Packet: 0x08bc
	length_list[0x08bc] = 10;

	// Packet: 0x08bd
	length_list[0x08bd] = 8;

	// Packet: 0x08be
	length_list[0x08be] = 14;

	// Packet: 0x08bf
	length_list[0x08bf] = 8;

	// Packet: 0x08c0
	length_list[0x08c0] = -1;

	// Packet: 0x08c1
	length_list[0x08c1] = 2;

	// Packet: 0x08c2
	length_list[0x08c2] = 2;

	// Packet: 0x08c3
	length_list[0x08c3] = 10;

	// Packet: 0x08c4
	length_list[0x08c4] = 8;

	// Packet: 0x08c5
	length_list[0x08c5] = 6;

	// Packet: 0x08c6
	length_list[0x08c6] = 4;

	// Packet: 0x08c7
	length_list[0x08c7] = -1;

	// Packet: 0x08c8
	length_list[0x08c8] = 34;

	// Packet: 0x08c9
	length_list[0x08c9] = 2;

	// Packet: 0x08ca
	length_list[0x08ca] = -1;

	// Packet: 0x08cb
	length_list[0x08cb] = -1;

	// Packet: 0x08cc
	length_list[0x08cc] = 109;

	// Packet: 0x08cd
	length_list[0x08cd] = 10;

	// Packet: 0x08ce
	length_list[0x08ce] = 2;

	// Packet: 0x08cf
	length_list[0x08cf] = 10;

	// Packet: 0x08d0
	length_list[0x08d0] = 9;

	// Packet: 0x08d1
	length_list[0x08d1] = 7;

	// Packet: 0x08d2
	length_list[0x08d2] = 10;

	// Packet: 0x08d3
	length_list[0x08d3] = 10;

	// Packet: 0x08d4
	length_list[0x08d4] = 8;

	// Packet: 0x08d5
	length_list[0x08d5] = -1;

	// Packet: 0x08d6
	length_list[0x08d6] = 6;

	// Packet: 0x08d7
	length_list[0x08d7] = 28;

	// Packet: 0x08d8
	length_list[0x08d8] = 27;

	// Packet: 0x08d9
	length_list[0x08d9] = 30;

	// Packet: 0x08da
	length_list[0x08da] = 26;

	// Packet: 0x08db
	length_list[0x08db] = 27;

	// Packet: 0x08dc
	length_list[0x08dc] = 26;

	// Packet: 0x08dd
	length_list[0x08dd] = 27;

	// Packet: 0x08de
	length_list[0x08de] = 27;

	// Packet: 0x08df
	length_list[0x08df] = 50;

	// Packet: 0x08e0
	length_list[0x08e0] = 51;

	// Packet: 0x08e1
	length_list[0x08e1] = 51;

	// Packet: 0x08e2
	length_list[0x08e2] = 27;

	// Packet: 0x08e3
	length_list[0x08e3] = 149;

	// Packet: 0x08e4
	length_list[0x08e4] = 6;

	// Packet: 0x08fc
	length_list[0x08fc] = 30;

	// Packet: 0x08fd
	length_list[0x08fd] = 6;

	// Packet: 0x08fe
	length_list[0x08fe] = -1;

	// Packet: 0x08ff
	length_list[0x08ff] = 24;

	// Packet: 0x0900
	length_list[0x0900] = -1;

	// Packet: 0x0901
	length_list[0x0901] = -1;

	// Packet: 0x0902
	length_list[0x0902] = -1;

	// Packet: 0x0903
	length_list[0x0903] = -1;

	// Packet: 0x0904
	length_list[0x0904] = -1;

	// Packet: 0x0905
	length_list[0x0905] = -1;

	// Packet: 0x0906
	length_list[0x0906] = -1;

	// Packet: 0x0907
	length_list[0x0907] = 5;

	// Packet: 0x0908
	length_list[0x0908] = 5;

	// Packet: 0x090a
	length_list[0x090a] = 26;

	// Packet: 0x090d
	length_list[0x090d] = -1;

	// Packet: 0x090e
	length_list[0x090e] = 2;

	// Packet: 0x090f
	length_list[0x090f] = -1;

	// Packet: 0x0910
	length_list[0x0910] = 10;

	// Packet: 0x0911
	length_list[0x0911] = 30;

	// Packet: 0x0912
	length_list[0x0912] = 10;

	// Packet: 0x0913
	length_list[0x0913] = 30;

	// Packet: 0x0914
	length_list[0x0914] = -1;

	// Packet: 0x0915
	length_list[0x0915] = -1;

	// Packet: 0x0916
	length_list[0x0916] = 26;

	// Packet: 0x0917
	if (packetver >= 20150722) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0917] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x0917] = 2;
	}

	// Packet: 0x0918
	if (packetver >= 20150128) {
		length_list[0x0918] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0918] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0918] = 2;
	}

	// Packet: 0x0919
	if (packetver >= 20150826) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0919] = 8;
	} else if (packetver >= 20150729) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0919] = 4;
	} else if (packetver >= 20150708) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0919] = 36;
	} else if (packetver >= 20150401) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0919] = 36;
	} else if (packetver >= 20150217) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0919] = 90;
	} else if (packetver >= 20150128) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0919] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0919] = 2;
	}

	// Packet: 0x091a
	if (packetver >= 20151230) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x091a] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x091a] = 2;
	}

	// Packet: 0x091b
	if (packetver >= 20151230) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x091b] = 26;
	} else if (packetver >= 20150610) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x091b] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x091b] = 4;
	} else if (packetver >= 20150401) {
		length_list[0x091b] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x091b] = 2;
	}

	// Packet: 0x091c
	if (packetver >= 20151021) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x091c] = 26;
	} else if (packetver >= 20150527) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x091c] = 6;
	} else if (packetver >= 20150408) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x091c] = 90;
	} else if (packetver >= 20150318) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x091c] = 90;
	} else if (packetver >= 20150107) {
		length_list[0x091c] = 2;
	}

	// Packet: 0x091d
	if (packetver >= 20151230) {
		length_list[0x091d] = 8;
	} else if (packetver >= 20151223) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x091d] = 26;
	} else if (packetver >= 20151028) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x091d] = 5;
	} else if (packetver >= 20151014) {
		length_list[0x091d] = 36;
	} else if (packetver >= 20150527) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x091d] = 7;
	} else if (packetver >= 20150325) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x091d] = 8;
	} else if (packetver >= 20150128) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x091d] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x091d] = 2;
	}

	// Packet: 0x091e
	if (packetver >= 20151001) {
		length_list[0x091e] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x091e] = 8;
	} else if (packetver >= 20150826) {
		length_list[0x091e] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x091e] = 36;
	} else if (packetver >= 20150415) {
		length_list[0x091e] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x091e] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x091e] = 2;
	}

	// Packet: 0x091f
	if (packetver >= 20150715) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x091f] = 18;
	} else if (packetver >= 20150204) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x091f] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x091f] = 2;
	}

	// Packet: 0x0920
	if (packetver >= 20151230) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0920] = 5;
	} else if (packetver >= 20151216) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0920] = -1;
	} else if (packetver >= 20151202) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0920] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0920] = -1;
	} else if (packetver >= 20150325) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0920] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0920] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0920] = 2;
	}

	// Packet: 0x0921
	if (packetver >= 20151125) {
		length_list[0x0921] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0921] = 26;
	} else if (packetver >= 20150729) {
		length_list[0x0921] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0921] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0921] = 2;
	}

	// Packet: 0x0922
	if (packetver >= 20150610) {
		length_list[0x0922] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0922] = 36;
	} else if (packetver >= 20150422) {
		length_list[0x0922] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0922] = 8;
	} else if (packetver >= 20150408) {
		length_list[0x0922] = 5;
	} else if (packetver >= 20150401) {
		length_list[0x0922] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0922] = 2;
	}

	// Packet: 0x0923
	if (packetver >= 20151230) {
		length_list[0x0923] = 36;
	} else if (packetver >= 20151223) {
		length_list[0x0923] = 5;
	} else if (packetver >= 20150909) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0923] = 26;
	} else if (packetver >= 20150708) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0923] = 8;
	} else if (packetver >= 20150520) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0923] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x0923] = 2;
	}

	// Packet: 0x0924
	if (packetver >= 20151230) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0924] = 6;
	} else if (packetver >= 20150923) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0924] = 26;
	} else if (packetver >= 20150902) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0924] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0924] = 8;
	} else if (packetver >= 20150513) {
		length_list[0x0924] = 5;
	} else if (packetver >= 20150507) {
		length_list[0x0924] = 19;
	} else if (packetver >= 20150408) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0924] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x0924] = 2;
	}

	// Packet: 0x0925
	if (packetver >= 20151125) {
		length_list[0x0925] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0925] = 19;
	} else if (packetver >= 20150729) {
		length_list[0x0925] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0925] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x0925] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0925] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0925] = 2;
	}

	// Packet: 0x0926
	length_list[0x0926] = 2;

	// Packet: 0x0927
	if (packetver >= 20150826) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0927] = 19;
	} else if (packetver >= 20150520) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0927] = 4;
	} else if (packetver >= 20150325) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0927] = 5;
	} else if (packetver >= 20150204) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0927] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0927] = 2;
	}

	// Packet: 0x0928
	if (packetver >= 20151111) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0928] = 5;
	} else if (packetver >= 20150916) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0928] = 8;
	} else if (packetver >= 20150902) {
		length_list[0x0928] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0928] = 8;
	} else if (packetver >= 20150325) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0928] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x0928] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0928] = 2;
	}

	// Packet: 0x0929
	if (packetver >= 20150507) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0929] = 8;
	} else if (packetver >= 20150204) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0929] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x0929] = 2;
	}

	// Packet: 0x092a
	if (packetver >= 20151202) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x092a] = 10;
	} else if (packetver >= 20150909) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x092a] = 5;
	} else if (packetver >= 20150715) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x092a] = 8;
	} else if (packetver >= 20150415) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x092a] = 8;
	} else if (packetver >= 20150318) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x092a] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x092a] = 2;
	}

	// Packet: 0x092b
	if (packetver >= 20151001) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x092b] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x092b] = 90;
	} else if (packetver >= 20150617) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x092b] = -1;
	} else if (packetver >= 20150527) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x092b] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x092b] = 2;
	}

	// Packet: 0x092c
	if (packetver >= 20150708) {
		length_list[0x092c] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x092c] = 5;
	} else if (packetver >= 20150617) {
		length_list[0x092c] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x092c] = -1;
	} else if (packetver >= 20150401) {
		length_list[0x092c] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x092c] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x092c] = 2;
	}

	// Packet: 0x092d
	if (packetver >= 20151216) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x092d] = 8;
	} else if (packetver >= 20150909) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x092d] = 26;
	} else if (packetver >= 20150610) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x092d] = 6;
	} else if (packetver >= 20150114) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x092d] = 5;
	}

	// Packet: 0x092e
	if (packetver >= 20151202) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x092e] = 10;
	} else if (packetver >= 20151118) {
		length_list[0x092e] = 5;
	} else if (packetver >= 20150902) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x092e] = 90;
	} else if (packetver >= 20150729) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x092e] = 26;
	} else if (packetver >= 20150617) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x092e] = 4;
	} else if (packetver >= 20150513) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x092e] = -1;
	} else if (packetver >= 20150422) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x092e] = 6;
	} else if (packetver >= 20150318) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x092e] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x092e] = 2;
	}

	// Packet: 0x092f
	if (packetver >= 20151125) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x092f] = 8;
	} else if (packetver >= 20150923) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x092f] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x092f] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x092f] = 2;
	}

	// Packet: 0x0930
	if (packetver >= 20151216) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0930] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0930] = 90;
	} else if (packetver >= 20151001) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0930] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0930] = 2;
	}

	// Packet: 0x0931
	if (packetver >= 20150527) {
		length_list[0x0931] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0931] = 90;
	} else if (packetver >= 20150401) {
		length_list[0x0931] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0931] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0931] = 2;
	}

	// Packet: 0x0932
	if (packetver >= 20151216) {
		length_list[0x0932] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0932] = -1;
	} else if (packetver >= 20150401) {
		length_list[0x0932] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0932] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0932] = 2;
	}

	// Packet: 0x0933
	length_list[0x0933] = 2;

	// Packet: 0x0934
	if (packetver >= 20151021) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0934] = -1;
	} else if (packetver >= 20150923) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0934] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0934] = 2;
	}

	// Packet: 0x0935
	length_list[0x0935] = 2;

	// Packet: 0x0936
	if (packetver >= 20151001) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0936] = 90;
	} else if (packetver >= 20150916) {
		length_list[0x0936] = 5;
	} else if (packetver >= 20150527) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0936] = 10;
	} else if (packetver >= 20150325) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0936] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0936] = 2;
	}

	// Packet: 0x0937
	if (packetver >= 20150325) {
		length_list[0x0937] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0937] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x0937] = 2;
	}

	// Packet: 0x0938
	if (packetver >= 20150923) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0938] = 4;
	} else if (packetver >= 20150401) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0938] = 5;
	} else if (packetver >= 20150318) {
		length_list[0x0938] = 26;
	} else if (packetver >= 20150204) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0938] = 4;
	} else if (packetver >= 20150107) {
		length_list[0x0938] = 2;
	}

	// Packet: 0x0939
	if (packetver >= 20151202) {
		length_list[0x0939] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0939] = 6;
	} else if (packetver >= 20151111) {
		length_list[0x0939] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0939] = 8;
	} else if (packetver >= 20150408) {
		length_list[0x0939] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0939] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x0939] = 2;
	}

	// Packet: 0x093a
	if (packetver >= 20151230) {
		length_list[0x093a] = 4;
	} else if (packetver >= 20151111) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x093a] = -1;
	} else if (packetver >= 20150805) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x093a] = 6;
	} else if (packetver >= 20150408) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x093a] = 10;
	} else if (packetver >= 20150325) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x093a] = 5;
	} else if (packetver >= 20150311) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x093a] = 18;
	} else if (packetver >= 20150204) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x093a] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x093a] = 2;
	}

	// Packet: 0x093b
	if (packetver >= 20151216) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x093b] = 8;
	} else if (packetver >= 20151001) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x093b] = 26;
	} else if (packetver >= 20150902) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x093b] = 6;
	} else if (packetver >= 20150513) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x093b] = 5;
	} else if (packetver >= 20150408) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x093b] = -1;
	} else if (packetver >= 20150318) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x093b] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x093b] = 2;
	}

	// Packet: 0x093c
	if (packetver >= 20151125) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x093c] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x093c] = -1;
	} else if (packetver >= 20150715) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x093c] = 26;
	} else if (packetver >= 20150422) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x093c] = 6;
	} else if (packetver >= 20150325) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x093c] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x093c] = 2;
	}

	// Packet: 0x093d
	if (packetver >= 20150729) {
		length_list[0x093d] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x093d] = 12;
	} else if (packetver >= 20150527) {
		length_list[0x093d] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x093d] = 26;
	} else if (packetver >= 20150507) {
		length_list[0x093d] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x093d] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x093d] = 2;
	}

	// Packet: 0x093e
	if (packetver >= 20151202) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x093e] = 12;
	} else if (packetver >= 20150923) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x093e] = 10;
	} else if (packetver >= 20150722) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x093e] = 5;
	} else if (packetver >= 20150708) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x093e] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x093e] = 7;
	} else if (packetver >= 20150422) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x093e] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x093e] = 2;
	}

	// Packet: 0x093f
	if (packetver >= 20151014) {
		length_list[0x093f] = 2;
	} else if (packetver >= 20151007) {
		length_list[0x093f] = 5;
	} else if (packetver >= 20150805) {
		length_list[0x093f] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x093f] = -1;
	} else if (packetver >= 20150708) {
		length_list[0x093f] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x093f] = 12;
	} else if (packetver >= 20150610) {
		length_list[0x093f] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x093f] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x093f] = 2;
	}

	// Packet: 0x0940
	if (packetver >= 20151111) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0940] = 36;
	} else if (packetver >= 20151028) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x0940] = 26;
	} else if (packetver >= 20150916) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20150826) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0940] = 8;
	} else if (packetver >= 20150617) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x0940] = 10;
	} else if (packetver >= 20150603) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150527) {
		length_list[0x0940] = 12;
	} else if (packetver >= 20150520) {
		length_list[0x0940] = 8;
	} else if (packetver >= 20150401) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0940] = 26;
	} else if (packetver >= 20150304) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0940] = 2;
		// ignored packet from 2015-02-25iRagexe
		// ignored packet from 2015-02-25eRagexe
	} else if (packetver >= 20150107) {
		length_list[0x0940] = 2;
	}

	// Packet: 0x0941
	if (packetver >= 20150923) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0941] = 36;
	} else if (packetver >= 20150902) {
		length_list[0x0941] = 5;
	} else if (packetver >= 20150701) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0941] = 26;
	} else if (packetver >= 20150513) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0941] = 8;
	} else if (packetver >= 20150422) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0941] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x0941] = 2;
	}

	// Packet: 0x0942
	if (packetver >= 20151118) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0942] = 18;
	} else if (packetver >= 20150923) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0942] = 6;
	} else if (packetver >= 20150513) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0942] = 4;
	} else if (packetver >= 20150107) {
		length_list[0x0942] = 2;
	}

	// Packet: 0x0943
	if (packetver >= 20151125) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0943] = 6;
	} else if (packetver >= 20150507) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20150429) {
		length_list[0x0943] = 6;
	} else if (packetver >= 20150318) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0943] = 6;
	} else if (packetver >= 20150114) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0943] = 5;
	}

	// Packet: 0x0944
	if (packetver >= 20151223) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0944] = -1;
	} else if (packetver >= 20151021) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0944] = 12;
	} else if (packetver >= 20150722) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0944] = 26;
	} else if (packetver >= 20150217) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0944] = -1;
	} else if (packetver >= 20150204) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0944] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x0944] = 2;
	}

	// Packet: 0x0945
	if (packetver >= 20150902) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0945] = 5;
	} else if (packetver >= 20150527) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0945] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0945] = 2;
	}

	// Packet: 0x0946
	if (packetver >= 20151125) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x0946] = -1;
	} else if (packetver >= 20150708) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0946] = 6;
	} else if (packetver >= 20150617) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0946] = 90;
	} else if (packetver >= 20150408) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0946] = 4;
	} else if (packetver >= 20150318) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0946] = 4;
	} else if (packetver >= 20150304) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0946] = 26;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150121) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20150114) {
		length_list[0x0946] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x0946] = 2;
	}

	// Packet: 0x0947
	if (packetver >= 20151223) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0947] = 19;
	} else if (packetver >= 20150909) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0947] = 90;
	} else if (packetver >= 20150311) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0947] = 8;
	} else if (packetver >= 20150114) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x0947] = 19;
	}

	// Packet: 0x0948
	if (packetver >= 20151216) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0948] = 19;
	} else if (packetver >= 20150923) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0948] = -1;
	} else if (packetver >= 20150304) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0948] = 6;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150107) {
		length_list[0x0948] = 2;
	}

	// Packet: 0x0949
	if (packetver >= 20151230) {
		length_list[0x0949] = 5;
	} else if (packetver >= 20151223) {
		length_list[0x0949] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0949] = -1;
	} else if (packetver >= 20150617) {
		length_list[0x0949] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0949] = 10;
	} else if (packetver >= 20150408) {
		length_list[0x0949] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0949] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0949] = 2;
	}

	// Packet: 0x094a
	if (packetver >= 20151216) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x094a] = 18;
	} else if (packetver >= 20151118) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x094a] = 6;
	} else if (packetver >= 20150520) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x094a] = 26;
	} else if (packetver >= 20150401) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x094a] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x094a] = 2;
	}

	// Packet: 0x094b
	if (packetver >= 20150408) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x094b] = 10;
	} else if (packetver >= 20150107) {
		length_list[0x094b] = 2;
	}

	// Packet: 0x094c
	if (packetver >= 20150729) {
		length_list[0x094c] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x094c] = 10;
	} else if (packetver >= 20150325) {
		length_list[0x094c] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x094c] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x094c] = 2;
	}

	// Packet: 0x094d
	if (packetver >= 20150422) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x094d] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x094d] = 2;
	}

	// Packet: 0x094e
	if (packetver >= 20151230) {
		length_list[0x094e] = 6;
	} else if (packetver >= 20150708) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x094e] = 6;
	} else if (packetver >= 20150624) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x094e] = 26;
	} else if (packetver >= 20150527) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x094e] = 10;
	} else if (packetver >= 20150204) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x094e] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x094e] = 2;
	}

	// Packet: 0x094f
	if (packetver >= 20150923) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x094f] = 8;
	} else if (packetver >= 20150909) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x094f] = 6;
	} else if (packetver >= 20150826) {
		length_list[0x094f] = 5;
	} else if (packetver >= 20150805) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x094f] = 19;
	} else if (packetver >= 20150722) {
		length_list[0x094f] = 8;
	} else if (packetver >= 20150415) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x094f] = 18;
	} else if (packetver >= 20150304) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x094f] = 5;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150107) {
		length_list[0x094f] = 2;
	}

	// Packet: 0x0950
	if (packetver >= 20150722) {
		length_list[0x0950] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0950] = 6;
	} else if (packetver >= 20150401) {
		length_list[0x0950] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0950] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0950] = 2;
	}

	// Packet: 0x0951
	if (packetver >= 20151202) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0951] = 5;
	} else if (packetver >= 20151001) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20150923) {
		length_list[0x0951] = 7;
	} else if (packetver >= 20150902) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0951] = 6;
	} else if (packetver >= 20150729) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0951] = -1;
	} else if (packetver >= 20150325) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0951] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0951] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x0951] = 2;
	}

	// Packet: 0x0952
	if (packetver >= 20150304) {
		length_list[0x0952] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0952] = 8;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150204) {
		length_list[0x0952] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0952] = 90;
	} else if (packetver >= 20150107) {
		length_list[0x0952] = 2;
	}

	// Packet: 0x0953
	if (packetver >= 20150909) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x0953] = -1;
	} else if (packetver >= 20150513) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0953] = 36;
	} else if (packetver >= 20150422) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0953] = -1;
	} else if (packetver >= 20150408) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0953] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0953] = 2;
	}

	// Packet: 0x0954
	if (packetver >= 20151223) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0954] = -1;
	} else if (packetver >= 20150708) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0954] = 5;
	} else if (packetver >= 20150401) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0954] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0954] = 2;
	}

	// Packet: 0x0955
	if (packetver >= 20150805) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0955] = -1;
	} else if (packetver >= 20150610) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0955] = 5;
	} else if (packetver >= 20150513) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150507) {
		length_list[0x0955] = 6;
	} else if (packetver >= 20150429) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150422) {
		length_list[0x0955] = 36;
	} else if (packetver >= 20150415) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0955] = 6;
	} else if (packetver >= 20150304) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150225) {
		length_list[0x0955] = 5;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150128) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0955] = 8;
	} else if (packetver >= 20150114) {
		length_list[0x0955] = 4;
	} else if (packetver >= 20150107) {
		length_list[0x0955] = 2;
	}

	// Packet: 0x0956
	if (packetver >= 20151216) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0956] = 90;
	} else if (packetver >= 20151202) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0956] = 26;
	} else if (packetver >= 20151021) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0956] = 6;
	} else if (packetver >= 20150722) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0956] = 36;
	} else if (packetver >= 20150708) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0956] = 90;
	} else if (packetver >= 20150107) {
		length_list[0x0956] = 2;
	}

	// Packet: 0x0957
	if (packetver >= 20151202) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0957] = 8;
	} else if (packetver >= 20151118) {
		length_list[0x0957] = 4;
	} else if (packetver >= 20150617) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0957] = 6;
	} else if (packetver >= 20150415) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0957] = 19;
	} else if (packetver >= 20150318) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0957] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0957] = 26;
	} else if (packetver >= 20150121) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20150114) {
		length_list[0x0957] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0957] = 2;
	}

	// Packet: 0x0958
	if (packetver >= 20151118) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0958] = 8;
	} else if (packetver >= 20150729) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20150722) {
		length_list[0x0958] = 36;
	} else if (packetver >= 20150708) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0958] = 7;
	} else if (packetver >= 20150520) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20150513) {
		length_list[0x0958] = 6;
	} else if (packetver >= 20150507) {
		length_list[0x0958] = 26;
	} else if (packetver >= 20150318) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0958] = 6;
	} else if (packetver >= 20150217) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20150211) {
		length_list[0x0958] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0958] = 2;
	}

	// Packet: 0x0959
	if (packetver >= 20151202) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20151125) {
		length_list[0x0959] = 90;
	} else if (packetver >= 20150902) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0959] = -1;
	} else if (packetver >= 20150415) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x0959] = 6;
	} else if (packetver >= 20150325) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0959] = -1;
	} else if (packetver >= 20150128) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0959] = 19;
	} else if (packetver >= 20150107) {
		length_list[0x0959] = 2;
	}

	// Packet: 0x095a
	if (packetver >= 20150923) {
		length_list[0x095a] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x095a] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x095a] = 2;
	}

	// Packet: 0x095b
	if (packetver >= 20150909) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20150902) {
		length_list[0x095b] = 19;
	} else if (packetver >= 20150715) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x095b] = 6;
	} else if (packetver >= 20150527) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x095b] = 5;
	} else if (packetver >= 20150318) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x095b] = -1;
	} else if (packetver >= 20150107) {
		length_list[0x095b] = 2;
	}

	// Packet: 0x095c
	if (packetver >= 20151216) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x095c] = 4;
	} else if (packetver >= 20151125) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20151118) {
		length_list[0x095c] = 18;
	} else if (packetver >= 20150422) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x095c] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x095c] = 2;
	}

	// Packet: 0x095d
	if (packetver >= 20150617) {
		length_list[0x095d] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x095d] = 12;
	} else if (packetver >= 20150311) {
		length_list[0x095d] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x095d] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x095d] = 2;
	}

	// Packet: 0x095e
	if (packetver >= 20151230) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x095e] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x095e] = 8;
	} else if (packetver >= 20150916) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x095e] = 6;
	} else if (packetver >= 20150805) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x095e] = -1;
	} else if (packetver >= 20150415) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20150408) {
		length_list[0x095e] = 12;
	} else if (packetver >= 20150107) {
		length_list[0x095e] = 2;
	}

	// Packet: 0x095f
	if (packetver >= 20151014) {
		length_list[0x095f] = 2;
	} else if (packetver >= 20151007) {
		length_list[0x095f] = 5;
	} else if (packetver >= 20150708) {
		length_list[0x095f] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x095f] = 26;
	} else if (packetver >= 20150408) {
		length_list[0x095f] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x095f] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x095f] = 2;
	}

	// Packet: 0x0960
	if (packetver >= 20151007) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x0960] = 36;
	} else if (packetver >= 20150923) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0960] = 5;
	} else if (packetver >= 20150708) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0960] = 18;
	} else if (packetver >= 20150610) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0960] = 10;
	} else if (packetver >= 20150527) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0960] = 4;
	} else if (packetver >= 20150513) {
		length_list[0x0960] = -1;
	} else if (packetver >= 20150422) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0960] = 36;
	} else if (packetver >= 20150325) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150318) {
		length_list[0x0960] = 18;
	} else if (packetver >= 20150311) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0960] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0960] = 2;
	}

	// Packet: 0x0961
	if (packetver >= 20151216) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0961] = 7;
	} else if (packetver >= 20151021) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0961] = 6;
	} else if (packetver >= 20151007) {
		length_list[0x0961] = 19;
	} else if (packetver >= 20151001) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0961] = -1;
	} else if (packetver >= 20150826) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0961] = 26;
	} else if (packetver >= 20150805) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150729) {
		length_list[0x0961] = 36;
	} else if (packetver >= 20150722) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0961] = 8;
	} else if (packetver >= 20150527) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150520) {
		length_list[0x0961] = 8;
	} else if (packetver >= 20150422) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150415) {
		length_list[0x0961] = 18;
	} else if (packetver >= 20150311) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x0961] = 5;
	} else if (packetver >= 20150107) {
		length_list[0x0961] = 2;
	}

	// Packet: 0x0962
	if (packetver >= 20150916) {
		length_list[0x0962] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x0962] = 8;
	} else if (packetver >= 20150715) {
		length_list[0x0962] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x0962] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0962] = 2;
	}

	// Packet: 0x0963
	if (packetver >= 20150318) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0963] = 6;
	} else if (packetver >= 20150204) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0963] = 26;
	} else if (packetver >= 20150121) {
		length_list[0x0963] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0963] = 2;
	}

	// Packet: 0x0964
	if (packetver >= 20151216) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20151209) {
		length_list[0x0964] = 26;
	} else if (packetver >= 20151111) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20151104) {
		length_list[0x0964] = 6;
	} else if (packetver >= 20151021) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20151014) {
		length_list[0x0964] = 19;
	} else if (packetver >= 20150902) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0964] = 19;
	} else if (packetver >= 20150617) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20150610) {
		length_list[0x0964] = 5;
	} else if (packetver >= 20150408) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20150401) {
		length_list[0x0964] = 5;
	} else if (packetver >= 20150318) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20150311) {
		length_list[0x0964] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0964] = 2;
	}

	// Packet: 0x0965
	if (packetver >= 20151230) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0965] = 36;
	} else if (packetver >= 20150722) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20150715) {
		length_list[0x0965] = 18;
	} else if (packetver >= 20150107) {
		length_list[0x0965] = 2;
	}

	// Packet: 0x0966
	if (packetver >= 20151223) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0966] = 6;
	} else if (packetver >= 20151118) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0966] = 8;
	} else if (packetver >= 20150701) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20150624) {
		length_list[0x0966] = 5;
	} else if (packetver >= 20150211) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x0966] = 36;
	} else if (packetver >= 20150107) {
		length_list[0x0966] = 2;
	}

	// Packet: 0x0967
	if (packetver >= 20151230) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20151223) {
		length_list[0x0967] = 12;
	} else if (packetver >= 20151118) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0967] = 19;
	} else if (packetver >= 20151014) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20151007) {
		length_list[0x0967] = 36;
	} else if (packetver >= 20150826) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20150819) {
		length_list[0x0967] = 5;
	} else if (packetver >= 20150128) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20150121) {
		length_list[0x0967] = 26;
	} else if (packetver >= 20150107) {
		length_list[0x0967] = 2;
	}

	// Packet: 0x0968
	if (packetver >= 20151223) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20151216) {
		length_list[0x0968] = 10;
	} else if (packetver >= 20150902) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0968] = 4;
	} else if (packetver >= 20150708) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20150701) {
		length_list[0x0968] = 5;
	} else if (packetver >= 20150204) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20150128) {
		length_list[0x0968] = 6;
	} else if (packetver >= 20150107) {
		length_list[0x0968] = 2;
	}

	// Packet: 0x0969
	if (packetver >= 20151118) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x0969] = 36;
	} else if (packetver >= 20150923) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20150916) {
		length_list[0x0969] = 19;
	} else if (packetver >= 20150902) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20150826) {
		length_list[0x0969] = -1;
	} else if (packetver >= 20150610) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x0969] = -1;
	} else if (packetver >= 20150401) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20150325) {
		length_list[0x0969] = 8;
	} else if (packetver >= 20150107) {
		length_list[0x0969] = 2;
	}

	// Packet: 0x096a
	if (packetver >= 20151230) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20151209) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20151202) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20151125) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20151111) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20151104) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20151021) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20151014) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20151001) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150916) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150909) {
		length_list[0x096a] = 5;
	} else if (packetver >= 20150826) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150805) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150715) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150708) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150701) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150617) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150610) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150603) {
		length_list[0x096a] = 5;
	} else if (packetver >= 20150527) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150520) {
		length_list[0x096a] = 18;
	} else if (packetver >= 20150422) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150311) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150304) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150225) {
		length_list[0x096a] = 19;
		// ignored packet from 2015-02-25eRagexe
		// ignored packet from 2015-02-25hRagexe
	} else if (packetver >= 20150217) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150211) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150204) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20150128) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20150107) {
		length_list[0x096a] = 6;
	}

	// Packet: 0x096b
	length_list[0x096b] = 4;

	// Packet: 0x096c
	length_list[0x096c] = 6;

	// Packet: 0x096d
	length_list[0x096d] = -1;

	// Packet: 0x096e
	length_list[0x096e] = -1;

	// Packet: 0x096f
	length_list[0x096f] = 7;

	// Packet: 0x0970
	length_list[0x0970] = 31;

	// Packet: 0x0971
	length_list[0x0971] = 6;

	// Packet: 0x0972
	length_list[0x0972] = -1;

	// Packet: 0x0973
	length_list[0x0973] = 7;

	// Packet: 0x0974
	length_list[0x0974] = 2;

	// Packet: 0x0975
	length_list[0x0975] = -1;

	// Packet: 0x0976
	length_list[0x0976] = -1;

	// Packet: 0x0977
	length_list[0x0977] = 14;

	// Packet: 0x0978
	length_list[0x0978] = 6;

	// Packet: 0x0979
	length_list[0x0979] = 50;

	// Packet: 0x097a
	length_list[0x097a] = -1;

	// Packet: 0x097b
	length_list[0x097b] = -1;

	// Packet: 0x097c
	length_list[0x097c] = 4;

	// Packet: 0x097d
	length_list[0x097d] = 288;

	// Packet: 0x097e
	length_list[0x097e] = 12;

	// Packet: 0x097f
	length_list[0x097f] = -1;

	// Packet: 0x0980
	length_list[0x0980] = 7;

	// Packet: 0x0981
	length_list[0x0981] = -1;

	// Packet: 0x0982
	length_list[0x0982] = 7;

	// Packet: 0x0983
	length_list[0x0983] = 29;

	// Packet: 0x0984
	length_list[0x0984] = 28;

	// Packet: 0x0985
	length_list[0x0985] = -1;

	// Packet: 0x0986
	length_list[0x0986] = 10;

	// Packet: 0x0987
	length_list[0x0987] = -1;

	// Packet: 0x0988
	length_list[0x0988] = 6;

	// Packet: 0x0989
	length_list[0x0989] = 2;

	// Packet: 0x098a
	length_list[0x098a] = -1;

	// Packet: 0x098b
	length_list[0x098b] = 2;

	// Packet: 0x098c
	length_list[0x098c] = 4;

	// Packet: 0x098d
	length_list[0x098d] = -1;

	// Packet: 0x098e
	length_list[0x098e] = -1;

	// Packet: 0x098f
	length_list[0x098f] = -1;

	// Packet: 0x0990
	length_list[0x0990] = 31;

	// Packet: 0x0991
	length_list[0x0991] = -1;

	// Packet: 0x0992
	length_list[0x0992] = -1;

	// Packet: 0x0993
	length_list[0x0993] = -1;

	// Packet: 0x0994
	length_list[0x0994] = -1;

	// Packet: 0x0995
	length_list[0x0995] = -1;

	// Packet: 0x0996
	length_list[0x0996] = -1;

	// Packet: 0x0997
	length_list[0x0997] = -1;

	// Packet: 0x0998
	length_list[0x0998] = 8;

	// Packet: 0x0999
	length_list[0x0999] = 11;

	// Packet: 0x099a
	length_list[0x099a] = 9;

	// Packet: 0x099b
	length_list[0x099b] = 8;

	// Packet: 0x099c
	length_list[0x099c] = 6;

	// Packet: 0x099d
	length_list[0x099d] = -1;

	// Packet: 0x099e
	length_list[0x099e] = 12;

	// Packet: 0x099f
	length_list[0x099f] = -1;

	// Packet: 0x09a0
	length_list[0x09a0] = 6;

	// Packet: 0x09a1
	length_list[0x09a1] = 2;

	// Packet: 0x09a2
	length_list[0x09a2] = 6;

	// Packet: 0x09a3
	length_list[0x09a3] = -1;

	// Packet: 0x09a4
	length_list[0x09a4] = 18;

	// Packet: 0x09a5
	length_list[0x09a5] = 7;

	// Packet: 0x09a6
	length_list[0x09a6] = 12;

	// Packet: 0x09a7
	length_list[0x09a7] = 10;

	// Packet: 0x09a8
	length_list[0x09a8] = 16;

	// Packet: 0x09a9
	length_list[0x09a9] = 10;

	// Packet: 0x09aa
	length_list[0x09aa] = 16;

	// Packet: 0x09ab
	length_list[0x09ab] = 6;

	// Packet: 0x09ac
	length_list[0x09ac] = -1;

	// Packet: 0x09ad
	length_list[0x09ad] = 10;

	// Packet: 0x09ae
	length_list[0x09ae] = 17;

	// Packet: 0x09af
	length_list[0x09af] = 4;

	// Packet: 0x09b0
	length_list[0x09b0] = 8;

	// Packet: 0x09b1
	length_list[0x09b1] = 4;

	// Packet: 0x09b2
	length_list[0x09b2] = 8;

	// Packet: 0x09b3
	length_list[0x09b3] = 4;

	// Packet: 0x09b4
	length_list[0x09b4] = 6;

	// Packet: 0x09b5
	length_list[0x09b5] = 2;

	// Packet: 0x09b6
	length_list[0x09b6] = 6;

	// Packet: 0x09b7
	length_list[0x09b7] = 4;

	// Packet: 0x09b8
	length_list[0x09b8] = 6;

	// Packet: 0x09b9
	length_list[0x09b9] = 4;

	// Packet: 0x09ba
	length_list[0x09ba] = 2;

	// Packet: 0x09bb
	length_list[0x09bb] = 6;

	// Packet: 0x09bc
	length_list[0x09bc] = 6;

	// Packet: 0x09bd
	length_list[0x09bd] = 2;

	// Packet: 0x09be
	length_list[0x09be] = 2;

	// Packet: 0x09bf
	length_list[0x09bf] = 4;

	// Packet: 0x09c1
	length_list[0x09c1] = 10;

	// Packet: 0x09c2
	length_list[0x09c2] = -1;

	// Packet: 0x09c3
	length_list[0x09c3] = 8;

	// Packet: 0x09c4
	length_list[0x09c4] = 8;

	// Packet: 0x09c5
	length_list[0x09c5] = 1042;

	// Packet: 0x09c6
	length_list[0x09c6] = -1;

	// Packet: 0x09c7
	length_list[0x09c7] = 18;

	// Packet: 0x09c8
	length_list[0x09c8] = -1;

	// Packet: 0x09c9
	length_list[0x09c9] = -1;

	// Packet: 0x09ca
	length_list[0x09ca] = -1;

	// Packet: 0x09cb
	length_list[0x09cb] = 17;

	// Packet: 0x09cc
	length_list[0x09cc] = -1;

	// Packet: 0x09cd
	length_list[0x09cd] = 8;

	// Packet: 0x09ce
	length_list[0x09ce] = 102;

	// Packet: 0x09cf
	length_list[0x09cf] = -1;

	// Packet: 0x09d0
	length_list[0x09d0] = -1;

	// Packet: 0x09d1
	length_list[0x09d1] = 14;

	// Packet: 0x09d2
	length_list[0x09d2] = -1;

	// Packet: 0x09d3
	length_list[0x09d3] = -1;

	// Packet: 0x09d4
	length_list[0x09d4] = 2;

	// Packet: 0x09d5
	length_list[0x09d5] = -1;

	// Packet: 0x09d6
	length_list[0x09d6] = -1;

	// Packet: 0x09d7
	length_list[0x09d7] = -1;

	// Packet: 0x09d8
	length_list[0x09d8] = 2;

	// Packet: 0x09d9
	length_list[0x09d9] = 4;

	// Packet: 0x09da
	length_list[0x09da] = -1;

	// Packet: 0x09db
	length_list[0x09db] = -1;

	// Packet: 0x09dc
	length_list[0x09dc] = -1;

	// Packet: 0x09dd
	length_list[0x09dd] = -1;

	// Packet: 0x09de
	length_list[0x09de] = -1;

	// Packet: 0x09df
	length_list[0x09df] = 7;

	// Packet: 0x09e0
	length_list[0x09e0] = -1;

	// Packet: 0x09e1
	length_list[0x09e1] = 8;

	// Packet: 0x09e2
	length_list[0x09e2] = 8;

	// Packet: 0x09e3
	length_list[0x09e3] = 8;

	// Packet: 0x09e4
	length_list[0x09e4] = 8;

	// Packet: 0x09e5
	length_list[0x09e5] = 18;

	// Packet: 0x09e6
	length_list[0x09e6] = 22;

	// Packet: 0x09e7
	length_list[0x09e7] = 3;

	// Packet: 0x09e8
	length_list[0x09e8] = 11;

	// Packet: 0x09e9
	length_list[0x09e9] = 2;

	// Packet: 0x09ea
	length_list[0x09ea] = 11;

	// Packet: 0x09eb
	length_list[0x09eb] = -1;

	// Packet: 0x09ec
	length_list[0x09ec] = -1;

	// Packet: 0x09ed
	length_list[0x09ed] = 3;

	// Packet: 0x09ee
	length_list[0x09ee] = 11;

	// Packet: 0x09ef
	length_list[0x09ef] = 11;

	// Packet: 0x09f0
	length_list[0x09f0] = -1;

	// Packet: 0x09f1
	length_list[0x09f1] = 11;

	// Packet: 0x09f2
	length_list[0x09f2] = 12;

	// Packet: 0x09f3
	length_list[0x09f3] = 11;

	// Packet: 0x09f4
	length_list[0x09f4] = 12;

	// Packet: 0x09f5
	length_list[0x09f5] = 11;

	// Packet: 0x09f6
	length_list[0x09f6] = 11;

	// Packet: 0x09f7
	length_list[0x09f7] = 75;

	// Packet: 0x09f8
	length_list[0x09f8] = -1;

	// Packet: 0x09f9
	length_list[0x09f9] = 143;

	// Packet: 0x09fa
	length_list[0x09fa] = -1;

	// Packet: 0x09fb
	length_list[0x09fb] = -1;

	// Packet: 0x09fc
	length_list[0x09fc] = 6;

	// Packet: 0x09fd
	length_list[0x09fd] = -1;

	// Packet: 0x09fe
	length_list[0x09fe] = -1;

	// Packet: 0x09ff
	length_list[0x09ff] = -1;

	// Packet: 0x0a00
	length_list[0x0a00] = 269;

	// Packet: 0x0a01
	length_list[0x0a01] = 3;

	// Packet: 0x0a02
	length_list[0x0a02] = 4;

	// Packet: 0x0a03
	length_list[0x0a03] = 2;

	// Packet: 0x0a04
	length_list[0x0a04] = 6;

	// Packet: 0x0a05
	length_list[0x0a05] = 53;

	// Packet: 0x0a06
	length_list[0x0a06] = 6;

	// Packet: 0x0a07
	length_list[0x0a07] = 9;

	// Packet: 0x0a08
	length_list[0x0a08] = 26;

	// Packet: 0x0a09
	length_list[0x0a09] = 45;

	// Packet: 0x0a0a
	length_list[0x0a0a] = 47;

	// Packet: 0x0a0b
	length_list[0x0a0b] = 47;

	// Packet: 0x0a0c
	length_list[0x0a0c] = 56;

	// Packet: 0x0a0d
	length_list[0x0a0d] = -1;

	// Packet: 0x0a0e
	length_list[0x0a0e] = 14;

	// Packet: 0x0a0f
	length_list[0x0a0f] = -1;

	// Packet: 0x0a10
	length_list[0x0a10] = -1;

	// Packet: 0x0a11
	length_list[0x0a11] = -1;

	// Packet: 0x0a12
	length_list[0x0a12] = 27;

	// Packet: 0x0a13
	length_list[0x0a13] = 26;

	// Packet: 0x0a14
	length_list[0x0a14] = 10;

	// Packet: 0x0a15
	length_list[0x0a15] = 12;

	// Packet: 0x0a16
	length_list[0x0a16] = 26;

	// Packet: 0x0a17
	length_list[0x0a17] = 6;

	// Packet: 0x0a18
	length_list[0x0a18] = 14;

	// Packet: 0x0a19
	length_list[0x0a19] = 2;

	// Packet: 0x0a1a
	length_list[0x0a1a] = 23;

	// Packet: 0x0a1b
	length_list[0x0a1b] = 2;

	// Packet: 0x0a1c
	length_list[0x0a1c] = -1;

	// Packet: 0x0a1d
	length_list[0x0a1d] = 2;

	// Packet: 0x0a1e
	length_list[0x0a1e] = 3;

	// Packet: 0x0a1f
	length_list[0x0a1f] = 2;

	// Packet: 0x0a20
	length_list[0x0a20] = 21;

	// Packet: 0x0a21
	length_list[0x0a21] = 3;

	// Packet: 0x0a22
	length_list[0x0a22] = 5;

	// Packet: 0x0a23
	length_list[0x0a23] = -1;

	// Packet: 0x0a24
	length_list[0x0a24] = 66;

	// Packet: 0x0a25
	length_list[0x0a25] = 6;

	// Packet: 0x0a26
	length_list[0x0a26] = 7;

	// Packet: 0x0a27
	length_list[0x0a27] = 8;

	// Packet: 0x0a28
	length_list[0x0a28] = 3;

	// Packet: 0x0a29
	length_list[0x0a29] = 6;

	// Packet: 0x0a2a
	length_list[0x0a2a] = 6;

	// Packet: 0x0a2b
	length_list[0x0a2b] = 14;

	// Packet: 0x0a2c
	length_list[0x0a2c] = 12;

	// Packet: 0x0a2d
	length_list[0x0a2d] = -1;

	// Packet: 0x0a2e
	length_list[0x0a2e] = 6;

	// Packet: 0x0a2f
	length_list[0x0a2f] = 7;

	// Packet: 0x0a30
	length_list[0x0a30] = 106;

	// Packet: 0x0a31
	length_list[0x0a31] = -1;

	// Packet: 0x0a32
	length_list[0x0a32] = 2;

	// Packet: 0x0a33
	length_list[0x0a33] = 7;

	// Packet: 0x0a34
	length_list[0x0a34] = 6;

	// Packet: 0x0a35
	length_list[0x0a35] = 4;

	// Packet: 0x0a36
	length_list[0x0a36] = 7;

	// Packet: 0x0a37
	length_list[0x0a37] = 57;

	// Packet: 0x0a38
	if (packetver >= 20150128) {
		length_list[0x0a38] = 3;
	}

	// Packet: 0x0a39
	if (packetver >= 20150415) {
		length_list[0x0a39] = 36;
	} else if (packetver >= 20150211) {
		length_list[0x0a39] = 35;
	}

	// Packet: 0x0a3a
	if (packetver >= 20150311) {
		length_list[0x0a3a] = 12;
	}

	// Packet: 0x0a3b
	if (packetver >= 20150422) {
		length_list[0x0a3b] = -1;
	}

	// Packet: 0x0a3c
	if (packetver >= 20150520) {
		length_list[0x0a3c] = -1;
	}

	// Packet: 0x0a3d
	if (packetver >= 20150520) {
		length_list[0x0a3d] = 18;
	}

	// Packet: 0x0a3e
	if (packetver >= 20150603) {
		length_list[0x0a3e] = -1;
	}

	// Packet: 0x0a3f
	if (packetver >= 20150624) {
		length_list[0x0a3f] = 9;
	}

	// Packet: 0x0a40
	if (packetver >= 20150812) {
		length_list[0x0a40] = 11;
	}

	// Packet: 0x0a41
	if (packetver >= 20150909) {
		length_list[0x0a41] = 18;
	}

	// Packet: 0x0a42
	if (packetver >= 20150916) {
		length_list[0x0a42] = 43;
	}

	// Packet: 0x0a43
	if (packetver >= 20151007) {
		length_list[0x0a43] = 85;
	}

	// Packet: 0x0a44
	if (packetver >= 20151007) {
		length_list[0x0a44] = -1;
	}

	// Packet: 0x0a45
	if (packetver >= 20151125) {
		// removed
	} else if (packetver >= 20151104) {
		length_list[0x0a45] = 2;
	} else if (packetver >= 20151028) {
		length_list[0x0a45] = -1;
	}

	// Packet: 0x0a46
	if (packetver >= 20151104) {
		length_list[0x0a46] = 14;
	}

	// Packet: 0x0a47
	if (packetver >= 20151104) {
		length_list[0x0a47] = 3;
	}

	// Packet: 0x0a48
	if (packetver >= 20151104) {
		length_list[0x0a48] = 2;
	}

	// Packet: 0x0a49
	if (packetver >= 20151118) {
		length_list[0x0a49] = 22;
	}

	// Packet: 0x0a4a
	if (packetver >= 20151118) {
		length_list[0x0a4a] = 6;
	}

	// Packet: 0x0a4b
	if (packetver >= 20151118) {
		length_list[0x0a4b] = 22;
	}

	// Packet: 0x0a4c
	if (packetver >= 20151118) {
		length_list[0x0a4c] = 28;
	}

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
