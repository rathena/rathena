/**
 * Network/Packets/packets2010_len_main.js
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
	if (packetver >= 20100803) {
		length_list[0x006d] = 134;
	} else if (packetver >= 20100728) {
		length_list[0x006d] = 114;
	} else if (packetver >= 20100727) {
		length_list[0x006d] = 130;
	} else if (packetver >= 20100105) {
		length_list[0x006d] = 114;
	}

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
	if (packetver >= 20101123) {
		length_list[0x00aa] = 9;
	} else if (packetver >= 20100105) {
		length_list[0x00aa] = 7;
	}

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

	// Packet: 0x0158
	if (packetver >= 20100817) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0158] = -1;
	}

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
	if (packetver >= 20101228) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0202] = 18;
	} else if (packetver >= 20101130) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0202] = 90;
	} else if (packetver >= 20100105) {
		length_list[0x0202] = 26;
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
	if (packetver >= 20101214) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x022d] = -1;
	} else if (packetver >= 20101130) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20100105) {
		length_list[0x022d] = 5;
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
	if (packetver >= 20101228) {
		length_list[0x023b] = 8;
	} else if (packetver >= 20101221) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x023b] = 6;
	} else if (packetver >= 20101207) {
		length_list[0x023b] = 36;
	} else if (packetver >= 20101123) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x023b] = 36;
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
	if (packetver >= 20101228) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0281] = 26;
	} else if (packetver >= 20101123) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0281] = 4;
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
	if (packetver >= 20100803) {
		length_list[0x0288] = -1;
	} else if (packetver >= 20100105) {
		length_list[0x0288] = 10;
	}

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
	if (packetver >= 20101123) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x02c4] = 26;
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
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f3] = -1;
	}

	// Packet: 0x02f4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f4] = -1;
	}

	// Packet: 0x02f5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f5] = -1;
	}

	// Packet: 0x02f6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f6] = -1;
	}

	// Packet: 0x02f7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f7] = -1;
	}

	// Packet: 0x02f8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f8] = -1;
	}

	// Packet: 0x02f9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02f9] = -1;
	}

	// Packet: 0x02fa
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02fa] = -1;
	}

	// Packet: 0x02fb
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02fb] = -1;
	}

	// Packet: 0x02fc
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02fc] = -1;
	}

	// Packet: 0x02fd
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02fd] = -1;
	}

	// Packet: 0x02fe
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02fe] = -1;
	}

	// Packet: 0x02ff
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x02ff] = -1;
	}

	// Packet: 0x0300
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0300] = -1;
	}

	// Packet: 0x0301
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0301] = -1;
	}

	// Packet: 0x0302
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0302] = -1;
	}

	// Packet: 0x0303
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0303] = -1;
	}

	// Packet: 0x0304
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0304] = -1;
	}

	// Packet: 0x0305
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0305] = -1;
	}

	// Packet: 0x0306
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0306] = -1;
	}

	// Packet: 0x0307
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0307] = -1;
	}

	// Packet: 0x0308
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0308] = -1;
	}

	// Packet: 0x0309
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0309] = -1;
	}

	// Packet: 0x030a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030a] = -1;
	}

	// Packet: 0x030b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030b] = -1;
	}

	// Packet: 0x030c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030c] = -1;
	}

	// Packet: 0x030d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030d] = -1;
	}

	// Packet: 0x030e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030e] = -1;
	}

	// Packet: 0x030f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x030f] = -1;
	}

	// Packet: 0x0310
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0310] = -1;
	}

	// Packet: 0x0311
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0311] = -1;
	}

	// Packet: 0x0312
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0312] = -1;
	}

	// Packet: 0x0313
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0313] = -1;
	}

	// Packet: 0x0314
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0314] = -1;
	}

	// Packet: 0x0315
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0315] = -1;
	}

	// Packet: 0x0316
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0316] = -1;
	}

	// Packet: 0x0317
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0317] = -1;
	}

	// Packet: 0x0318
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0318] = -1;
	}

	// Packet: 0x0319
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0319] = -1;
	}

	// Packet: 0x031a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031a] = -1;
	}

	// Packet: 0x031b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031b] = -1;
	}

	// Packet: 0x031c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031c] = -1;
	}

	// Packet: 0x031d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031d] = -1;
	}

	// Packet: 0x031e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031e] = -1;
	}

	// Packet: 0x031f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x031f] = -1;
	}

	// Packet: 0x0320
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0320] = -1;
	}

	// Packet: 0x0321
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0321] = -1;
	}

	// Packet: 0x0322
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0322] = -1;
	}

	// Packet: 0x0323
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0323] = -1;
	}

	// Packet: 0x0324
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0324] = -1;
	}

	// Packet: 0x0325
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0325] = -1;
	}

	// Packet: 0x0326
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0326] = -1;
	}

	// Packet: 0x0327
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0327] = -1;
	}

	// Packet: 0x0328
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0328] = -1;
	}

	// Packet: 0x0329
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0329] = -1;
	}

	// Packet: 0x032a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032a] = -1;
	}

	// Packet: 0x032b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032b] = -1;
	}

	// Packet: 0x032c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032c] = -1;
	}

	// Packet: 0x032d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032d] = -1;
	}

	// Packet: 0x032e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032e] = -1;
	}

	// Packet: 0x032f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x032f] = -1;
	}

	// Packet: 0x0330
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0330] = -1;
	}

	// Packet: 0x0331
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0331] = -1;
	}

	// Packet: 0x0332
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0332] = -1;
	}

	// Packet: 0x0333
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0333] = -1;
	}

	// Packet: 0x0334
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0334] = -1;
	}

	// Packet: 0x0335
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0335] = -1;
	}

	// Packet: 0x0336
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0336] = -1;
	}

	// Packet: 0x0337
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0337] = -1;
	}

	// Packet: 0x0338
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0338] = -1;
	}

	// Packet: 0x0339
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0339] = -1;
	}

	// Packet: 0x033a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033a] = -1;
	}

	// Packet: 0x033b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033b] = -1;
	}

	// Packet: 0x033c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033c] = -1;
	}

	// Packet: 0x033d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033d] = -1;
	}

	// Packet: 0x033e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033e] = -1;
	}

	// Packet: 0x033f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x033f] = -1;
	}

	// Packet: 0x0340
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0340] = -1;
	}

	// Packet: 0x0341
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0341] = -1;
	}

	// Packet: 0x0342
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0342] = -1;
	}

	// Packet: 0x0343
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0343] = -1;
	}

	// Packet: 0x0344
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0344] = -1;
	}

	// Packet: 0x0345
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0345] = -1;
	}

	// Packet: 0x0346
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0346] = -1;
	}

	// Packet: 0x0347
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0347] = -1;
	}

	// Packet: 0x0348
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0348] = -1;
	}

	// Packet: 0x0349
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0349] = -1;
	}

	// Packet: 0x034a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034a] = -1;
	}

	// Packet: 0x034b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034b] = -1;
	}

	// Packet: 0x034c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034c] = -1;
	}

	// Packet: 0x034d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034d] = -1;
	}

	// Packet: 0x034e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034e] = -1;
	}

	// Packet: 0x034f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x034f] = -1;
	}

	// Packet: 0x0350
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0350] = -1;
	}

	// Packet: 0x0351
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0351] = -1;
	}

	// Packet: 0x0352
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0352] = -1;
	}

	// Packet: 0x0353
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0353] = -1;
	}

	// Packet: 0x0354
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0354] = -1;
	}

	// Packet: 0x0355
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0355] = -1;
	}

	// Packet: 0x0356
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0356] = -1;
	}

	// Packet: 0x0357
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0357] = -1;
	}

	// Packet: 0x0358
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0358] = -1;
	}

	// Packet: 0x0359
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0359] = -1;
	}

	// Packet: 0x035a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x035a] = -1;
	}

	// Packet: 0x035b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x035b] = -1;
	}

	// Packet: 0x035c
	length_list[0x035c] = 2;

	// Packet: 0x035d
	length_list[0x035d] = -1;

	// Packet: 0x035e
	length_list[0x035e] = 2;

	// Packet: 0x035f
	if (packetver >= 20101221) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x035f] = 18;
	} else if (packetver >= 20101130) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20100105) {
		length_list[0x035f] = -1;
	}

	// Packet: 0x0360
	if (packetver >= 20101214) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20101123) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0360] = -1;
	}

	// Packet: 0x0361
	if (packetver >= 20101228) {
		length_list[0x0361] = 10;
	} else if (packetver >= 20101221) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20101130) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20100105) {
		length_list[0x0361] = -1;
	}

	// Packet: 0x0362
	if (packetver >= 20101221) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0362] = 90;
	} else if (packetver >= 20101207) {
		length_list[0x0362] = 5;
	} else if (packetver >= 20101130) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20100105) {
		length_list[0x0362] = -1;
	}

	// Packet: 0x0363
	if (packetver >= 20101228) {
		length_list[0x0363] = 12;
	} else if (packetver >= 20101221) {
		length_list[0x0363] = 19;
	} else if (packetver >= 20101207) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20101123) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0363] = -1;
	}

	// Packet: 0x0364
	if (packetver >= 20101214) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0364] = 12;
	} else if (packetver >= 20101123) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20100105) {
		length_list[0x0364] = -1;
	}

	// Packet: 0x0365
	if (packetver >= 20101228) {
		length_list[0x0365] = 6;
	} else if (packetver >= 20101130) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0365] = 36;
	} else if (packetver >= 20100105) {
		length_list[0x0365] = -1;
	}

	// Packet: 0x0366
	if (packetver >= 20101221) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20101207) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0366] = 26;
	} else if (packetver >= 20101123) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0366] = -1;
	}

	// Packet: 0x0367
	if (packetver >= 20101228) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0367] = -1;
	} else if (packetver >= 20101123) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0367] = -1;
	}

	// Packet: 0x0368
	if (packetver >= 20101228) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20101221) {
		length_list[0x0368] = 10;
	} else if (packetver >= 20101214) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0368] = 4;
	} else if (packetver >= 20101130) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0368] = -1;
	}

	// Packet: 0x0369
	if (packetver >= 20101228) {
		length_list[0x0369] = 26;
	} else if (packetver >= 20101207) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0369] = 90;
	} else if (packetver >= 20101123) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0369] = -1;
	}

	// Packet: 0x036a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036a] = -1;
	}

	// Packet: 0x036b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036b] = -1;
	}

	// Packet: 0x036c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036c] = -1;
	}

	// Packet: 0x036d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036d] = -1;
	}

	// Packet: 0x036e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036e] = -1;
	}

	// Packet: 0x036f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x036f] = -1;
	}

	// Packet: 0x0370
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0370] = -1;
	}

	// Packet: 0x0371
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0371] = -1;
	}

	// Packet: 0x0372
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0372] = -1;
	}

	// Packet: 0x0373
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0373] = -1;
	}

	// Packet: 0x0374
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0374] = -1;
	}

	// Packet: 0x0375
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0375] = -1;
	}

	// Packet: 0x0376
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0376] = -1;
	}

	// Packet: 0x0377
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0377] = -1;
	}

	// Packet: 0x0378
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0378] = -1;
	}

	// Packet: 0x0379
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0379] = -1;
	}

	// Packet: 0x037a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037a] = -1;
	}

	// Packet: 0x037b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037b] = -1;
	}

	// Packet: 0x037c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037c] = -1;
	}

	// Packet: 0x037d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037d] = -1;
	}

	// Packet: 0x037e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037e] = -1;
	}

	// Packet: 0x037f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x037f] = -1;
	}

	// Packet: 0x0380
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0380] = -1;
	}

	// Packet: 0x0381
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0381] = -1;
	}

	// Packet: 0x0382
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0382] = -1;
	}

	// Packet: 0x0383
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0383] = -1;
	}

	// Packet: 0x0384
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0384] = -1;
	}

	// Packet: 0x0385
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0385] = -1;
	}

	// Packet: 0x0386
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0386] = -1;
	}

	// Packet: 0x0387
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0387] = -1;
	}

	// Packet: 0x0388
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0388] = -1;
	}

	// Packet: 0x0389
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0389] = -1;
	}

	// Packet: 0x038a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038a] = -1;
	}

	// Packet: 0x038b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038b] = -1;
	}

	// Packet: 0x038c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038c] = -1;
	}

	// Packet: 0x038d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038d] = -1;
	}

	// Packet: 0x038e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038e] = -1;
	}

	// Packet: 0x038f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x038f] = -1;
	}

	// Packet: 0x0390
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0390] = -1;
	}

	// Packet: 0x0391
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0391] = -1;
	}

	// Packet: 0x0392
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0392] = -1;
	}

	// Packet: 0x0393
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0393] = -1;
	}

	// Packet: 0x0394
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0394] = -1;
	}

	// Packet: 0x0395
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0395] = -1;
	}

	// Packet: 0x0396
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0396] = -1;
	}

	// Packet: 0x0397
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0397] = -1;
	}

	// Packet: 0x0398
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0398] = -1;
	}

	// Packet: 0x0399
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0399] = -1;
	}

	// Packet: 0x039a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039a] = -1;
	}

	// Packet: 0x039b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039b] = -1;
	}

	// Packet: 0x039c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039c] = -1;
	}

	// Packet: 0x039d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039d] = -1;
	}

	// Packet: 0x039e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039e] = -1;
	}

	// Packet: 0x039f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x039f] = -1;
	}

	// Packet: 0x03a0
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a0] = -1;
	}

	// Packet: 0x03a1
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a1] = -1;
	}

	// Packet: 0x03a2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a2] = -1;
	}

	// Packet: 0x03a3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a3] = -1;
	}

	// Packet: 0x03a4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a4] = -1;
	}

	// Packet: 0x03a5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a5] = -1;
	}

	// Packet: 0x03a6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a6] = -1;
	}

	// Packet: 0x03a7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a7] = -1;
	}

	// Packet: 0x03a8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a8] = -1;
	}

	// Packet: 0x03a9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03a9] = -1;
	}

	// Packet: 0x03aa
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03aa] = -1;
	}

	// Packet: 0x03ab
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ab] = -1;
	}

	// Packet: 0x03ac
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ac] = -1;
	}

	// Packet: 0x03ad
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ad] = -1;
	}

	// Packet: 0x03ae
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ae] = -1;
	}

	// Packet: 0x03af
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03af] = -1;
	}

	// Packet: 0x03b0
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b0] = -1;
	}

	// Packet: 0x03b1
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b1] = -1;
	}

	// Packet: 0x03b2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b2] = -1;
	}

	// Packet: 0x03b3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b3] = -1;
	}

	// Packet: 0x03b4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b4] = -1;
	}

	// Packet: 0x03b5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b5] = -1;
	}

	// Packet: 0x03b6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b6] = -1;
	}

	// Packet: 0x03b7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b7] = -1;
	}

	// Packet: 0x03b8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b8] = -1;
	}

	// Packet: 0x03b9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03b9] = -1;
	}

	// Packet: 0x03ba
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ba] = -1;
	}

	// Packet: 0x03bb
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03bb] = -1;
	}

	// Packet: 0x03bc
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03bc] = -1;
	}

	// Packet: 0x03bd
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03bd] = -1;
	}

	// Packet: 0x03be
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03be] = -1;
	}

	// Packet: 0x03bf
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03bf] = -1;
	}

	// Packet: 0x03c0
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c0] = -1;
	}

	// Packet: 0x03c1
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c1] = -1;
	}

	// Packet: 0x03c2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c2] = -1;
	}

	// Packet: 0x03c3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c3] = -1;
	}

	// Packet: 0x03c4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c4] = -1;
	}

	// Packet: 0x03c5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c5] = -1;
	}

	// Packet: 0x03c6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c6] = -1;
	}

	// Packet: 0x03c7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c7] = -1;
	}

	// Packet: 0x03c8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c8] = -1;
	}

	// Packet: 0x03c9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03c9] = -1;
	}

	// Packet: 0x03ca
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ca] = -1;
	}

	// Packet: 0x03cb
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03cb] = -1;
	}

	// Packet: 0x03cc
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03cc] = -1;
	}

	// Packet: 0x03cd
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03cd] = -1;
	}

	// Packet: 0x03ce
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ce] = -1;
	}

	// Packet: 0x03cf
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03cf] = -1;
	}

	// Packet: 0x03d0
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d0] = -1;
	}

	// Packet: 0x03d1
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d1] = -1;
	}

	// Packet: 0x03d2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d2] = -1;
	}

	// Packet: 0x03d3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d3] = -1;
	}

	// Packet: 0x03d4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d4] = -1;
	}

	// Packet: 0x03d5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d5] = -1;
	}

	// Packet: 0x03d6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d6] = -1;
	}

	// Packet: 0x03d7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d7] = -1;
	}

	// Packet: 0x03d8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d8] = -1;
	}

	// Packet: 0x03d9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03d9] = -1;
	}

	// Packet: 0x03da
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03da] = -1;
	}

	// Packet: 0x03db
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03db] = -1;
	}

	// Packet: 0x03dc
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03dc] = -1;
	}

	// Packet: 0x03dd
	length_list[0x03dd] = 18;

	// Packet: 0x03de
	length_list[0x03de] = 18;

	// Packet: 0x03e2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e2] = -1;
	}

	// Packet: 0x03e3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e3] = -1;
	}

	// Packet: 0x03e4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e4] = -1;
	}

	// Packet: 0x03e5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e5] = -1;
	}

	// Packet: 0x03e6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e6] = -1;
	}

	// Packet: 0x03e7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e7] = -1;
	}

	// Packet: 0x03e8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e8] = -1;
	}

	// Packet: 0x03e9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03e9] = -1;
	}

	// Packet: 0x03ea
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ea] = -1;
	}

	// Packet: 0x03eb
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03eb] = -1;
	}

	// Packet: 0x03ec
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ec] = -1;
	}

	// Packet: 0x03ed
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ed] = -1;
	}

	// Packet: 0x03ee
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ee] = -1;
	}

	// Packet: 0x03ef
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ef] = -1;
	}

	// Packet: 0x03f0
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f0] = -1;
	}

	// Packet: 0x03f1
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f1] = -1;
	}

	// Packet: 0x03f2
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f2] = -1;
	}

	// Packet: 0x03f3
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f3] = -1;
	}

	// Packet: 0x03f4
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f4] = -1;
	}

	// Packet: 0x03f5
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f5] = -1;
	}

	// Packet: 0x03f6
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f6] = -1;
	}

	// Packet: 0x03f7
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f7] = -1;
	}

	// Packet: 0x03f8
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f8] = -1;
	}

	// Packet: 0x03f9
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03f9] = -1;
	}

	// Packet: 0x03fa
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03fa] = -1;
	}

	// Packet: 0x03fb
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03fb] = -1;
	}

	// Packet: 0x03fc
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03fc] = -1;
	}

	// Packet: 0x03fd
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03fd] = -1;
	}

	// Packet: 0x03fe
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03fe] = -1;
	}

	// Packet: 0x03ff
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x03ff] = -1;
	}

	// Packet: 0x0400
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0400] = -1;
	}

	// Packet: 0x0401
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0401] = -1;
	}

	// Packet: 0x0402
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0402] = -1;
	}

	// Packet: 0x0403
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0403] = -1;
	}

	// Packet: 0x0404
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0404] = -1;
	}

	// Packet: 0x0405
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0405] = -1;
	}

	// Packet: 0x0406
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0406] = -1;
	}

	// Packet: 0x0407
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0407] = -1;
	}

	// Packet: 0x0408
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0408] = -1;
	}

	// Packet: 0x0409
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0409] = -1;
	}

	// Packet: 0x040a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040a] = -1;
	}

	// Packet: 0x040b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040b] = -1;
	}

	// Packet: 0x040c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040c] = -1;
	}

	// Packet: 0x040d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040d] = -1;
	}

	// Packet: 0x040e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040e] = -1;
	}

	// Packet: 0x040f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x040f] = -1;
	}

	// Packet: 0x0410
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0410] = -1;
	}

	// Packet: 0x0411
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0411] = -1;
	}

	// Packet: 0x0412
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0412] = -1;
	}

	// Packet: 0x0413
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0413] = -1;
	}

	// Packet: 0x0414
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0414] = -1;
	}

	// Packet: 0x0415
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0415] = -1;
	}

	// Packet: 0x0416
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0416] = -1;
	}

	// Packet: 0x0417
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0417] = -1;
	}

	// Packet: 0x0418
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0418] = -1;
	}

	// Packet: 0x0419
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0419] = -1;
	}

	// Packet: 0x041a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041a] = -1;
	}

	// Packet: 0x041b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041b] = -1;
	}

	// Packet: 0x041c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041c] = -1;
	}

	// Packet: 0x041d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041d] = -1;
	}

	// Packet: 0x041e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041e] = -1;
	}

	// Packet: 0x041f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x041f] = -1;
	}

	// Packet: 0x0420
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0420] = -1;
	}

	// Packet: 0x0421
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0421] = -1;
	}

	// Packet: 0x0422
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0422] = -1;
	}

	// Packet: 0x0423
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0423] = -1;
	}

	// Packet: 0x0424
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0424] = -1;
	}

	// Packet: 0x0425
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0425] = -1;
	}

	// Packet: 0x0426
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0426] = -1;
	}

	// Packet: 0x0427
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0427] = -1;
	}

	// Packet: 0x0428
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0428] = -1;
	}

	// Packet: 0x0429
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0429] = -1;
	}

	// Packet: 0x042a
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042a] = -1;
	}

	// Packet: 0x042b
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042b] = -1;
	}

	// Packet: 0x042c
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042c] = -1;
	}

	// Packet: 0x042d
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042d] = -1;
	}

	// Packet: 0x042e
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042e] = -1;
	}

	// Packet: 0x042f
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x042f] = -1;
	}

	// Packet: 0x0430
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0430] = -1;
	}

	// Packet: 0x0431
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0431] = -1;
	}

	// Packet: 0x0432
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0432] = -1;
	}

	// Packet: 0x0433
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0433] = -1;
	}

	// Packet: 0x0434
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0434] = -1;
	}

	// Packet: 0x0435
	if (packetver >= 20101123) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x0435] = -1;
	}

	// Packet: 0x0436
	if (packetver >= 20101228) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0436] = 12;
	} else if (packetver >= 20101214) {
		length_list[0x0436] = 5;
	} else if (packetver >= 20101123) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0436] = 19;
	}

	// Packet: 0x0437
	if (packetver >= 20101228) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0437] = 8;
	} else if (packetver >= 20101207) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20101123) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x0437] = 7;
	}

	// Packet: 0x0438
	if (packetver >= 20101214) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0438] = 7;
	} else if (packetver >= 20101130) {
		length_list[0x0438] = 12;
	} else if (packetver >= 20100105) {
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
	if (packetver >= 20100817) {
		length_list[0x0442] = -1;
	} else if (packetver >= 20100105) {
		length_list[0x0442] = 8;
	}

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

	// Packet: 0x07d0
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d0] = 6;
	}

	// Packet: 0x07d1
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d1] = 2;
	}

	// Packet: 0x07d2
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d2] = -1;
	}

	// Packet: 0x07d3
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d3] = 4;
	}

	// Packet: 0x07d4
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d4] = 4;
	}

	// Packet: 0x07d5
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d5] = 4;
	}

	// Packet: 0x07d6
	if (packetver >= 20100701) {
		// removed
	} else if (packetver >= 20100105) {
		length_list[0x07d6] = 4;
	}

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
	if (packetver >= 20101228) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20101221) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x07e4] = -1;
	} else if (packetver >= 20101207) {
		length_list[0x07e4] = 26;
	} else if (packetver >= 20101123) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x07e4] = -1;
	}

	// Packet: 0x07e5
	if (packetver >= 20101012) {
		length_list[0x07e5] = 4;
	} else if (packetver >= 20100105) {
		length_list[0x07e5] = 8;
	}

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
	if (packetver >= 20101207) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x07ec] = -1;
	} else if (packetver >= 20101123) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20100105) {
		length_list[0x07ec] = 8;
	}

	// Packet: 0x07ed
	length_list[0x07ed] = 10;

	// Packet: 0x07ee
	length_list[0x07ee] = 6;

	// Packet: 0x07ef
	length_list[0x07ef] = 8;

	// Packet: 0x07f0
	if (packetver >= 20100209) {
		length_list[0x07f0] = 6;
	} else if (packetver >= 20100105) {
		length_list[0x07f0] = 8;
	}

	// Packet: 0x07f1
	if (packetver >= 20100629) {
		length_list[0x07f1] = 18;
	} else if (packetver >= 20100105) {
		length_list[0x07f1] = 15;
	}

	// Packet: 0x07f2
	if (packetver >= 20100629) {
		length_list[0x07f2] = 8;
	} else if (packetver >= 20100105) {
		length_list[0x07f2] = 6;
	}

	// Packet: 0x07f3
	if (packetver >= 20100629) {
		length_list[0x07f3] = 6;
	} else if (packetver >= 20100105) {
		length_list[0x07f3] = 4;
	}

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

	// Packet: 0x07ff
	length_list[0x07ff] = -1;

	// Packet: 0x0800
	length_list[0x0800] = -1;

	// Packet: 0x0801
	length_list[0x0801] = -1;

	// Packet: 0x0802
	if (packetver >= 20101228) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20101214) {
		length_list[0x0802] = 36;
	} else if (packetver >= 20101130) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0802] = 5;
	} else if (packetver >= 20100105) {
		length_list[0x0802] = 18;
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
	if (packetver >= 20100113) {
		length_list[0x080c] = 2;
	}

	// Packet: 0x080d
	if (packetver >= 20100113) {
		length_list[0x080d] = 3;
	}

	// Packet: 0x080e
	if (packetver >= 20100119) {
		length_list[0x080e] = 14;
	}

	// Packet: 0x080f
	if (packetver >= 20100223) {
		length_list[0x080f] = 20;
	}

	// Packet: 0x0810
	if (packetver >= 20100303) {
		length_list[0x0810] = 3;
	}

	// Packet: 0x0811
	if (packetver >= 20101228) {
		length_list[0x0811] = 90;
	} else if (packetver >= 20101221) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20101207) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0811] = 5;
	} else if (packetver >= 20101123) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20100303) {
		length_list[0x0811] = -1;
	}

	// Packet: 0x0812
	if (packetver >= 20100420) {
		length_list[0x0812] = 8;
	} else if (packetver >= 20100303) {
		length_list[0x0812] = 86;
	}

	// Packet: 0x0813
	if (packetver >= 20100309) {
		length_list[0x0813] = -1;
	} else if (packetver >= 20100303) {
		length_list[0x0813] = 6;
	}

	// Packet: 0x0814
	if (packetver >= 20100420) {
		length_list[0x0814] = 86;
	} else if (packetver >= 20100309) {
		length_list[0x0814] = 2;
	} else if (packetver >= 20100303) {
		length_list[0x0814] = 6;
	}

	// Packet: 0x0815
	if (packetver >= 20101228) {
		length_list[0x0815] = 7;
	} else if (packetver >= 20101221) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0815] = 5;
	} else if (packetver >= 20101207) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0815] = 6;
	} else if (packetver >= 20100420) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20100309) {
		length_list[0x0815] = 6;
	} else if (packetver >= 20100303) {
		length_list[0x0815] = -1;
	}

	// Packet: 0x0816
	if (packetver >= 20100309) {
		length_list[0x0816] = 6;
	}

	// Packet: 0x0817
	if (packetver >= 20101228) {
		length_list[0x0817] = -1;
	} else if (packetver >= 20101221) {
		length_list[0x0817] = 7;
	} else if (packetver >= 20101130) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0817] = 8;
	} else if (packetver >= 20100420) {
		length_list[0x0817] = 6;
	} else if (packetver >= 20100303) {
		length_list[0x0817] = -1;
	}

	// Packet: 0x0818
	if (packetver >= 20100309) {
		length_list[0x0818] = -1;
	} else if (packetver >= 20100303) {
		length_list[0x0818] = 6;
	}

	// Packet: 0x0819
	if (packetver >= 20101207) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0819] = 7;
	} else if (packetver >= 20101123) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20100420) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20100309) {
		length_list[0x0819] = 10;
	} else if (packetver >= 20100303) {
		length_list[0x0819] = 4;
	}

	// Packet: 0x081a
	if (packetver >= 20100420) {
		length_list[0x081a] = 4;
	} else if (packetver >= 20100413) {
		length_list[0x081a] = 10;
	} else if (packetver >= 20100331) {
		length_list[0x081a] = 6;
	} else if (packetver >= 20100309) {
		length_list[0x081a] = 4;
	}

	// Packet: 0x081b
	if (packetver >= 20100420) {
		length_list[0x081b] = 10;
	} else if (packetver >= 20100414) {
		length_list[0x081b] = 8;
	} else if (packetver >= 20100309) {
		length_list[0x081b] = 4;
	}

	// Packet: 0x081c
	if (packetver >= 20100420) {
		length_list[0x081c] = 10;
	} else if (packetver >= 20100309) {
		length_list[0x081c] = 6;
	}

	// Packet: 0x081d
	if (packetver >= 20100309) {
		length_list[0x081d] = 22;
	}

	// Packet: 0x081e
	if (packetver >= 20100309) {
		length_list[0x081e] = 8;
	}

	// Packet: 0x081f
	if (packetver >= 20100323) {
		length_list[0x081f] = -1;
	}

	// Packet: 0x0820
	if (packetver >= 20100413) {
		length_list[0x0820] = 11;
	}

	// Packet: 0x0821
	if (packetver >= 20100413) {
		length_list[0x0821] = 2;
	}

	// Packet: 0x0822
	if (packetver >= 20100413) {
		length_list[0x0822] = 9;
	}

	// Packet: 0x0823
	if (packetver >= 20100413) {
		length_list[0x0823] = -1;
	}

	// Packet: 0x0824
	if (packetver >= 20100420) {
		length_list[0x0824] = 6;
	}

	// Packet: 0x0825
	if (packetver >= 20100601) {
		length_list[0x0825] = -1;
	}

	// Packet: 0x0826
	if (packetver >= 20101019) {
		// removed
	} else if (packetver >= 20100601) {
		length_list[0x0826] = 4;
	}

	// Packet: 0x0827
	if (packetver >= 20100713) {
		length_list[0x0827] = 6;
	}

	// Packet: 0x0828
	if (packetver >= 20100713) {
		length_list[0x0828] = 14;
	}

	// Packet: 0x0829
	if (packetver >= 20100728) {
		length_list[0x0829] = 12;
	} else if (packetver >= 20100713) {
		length_list[0x0829] = 6;
	}

	// Packet: 0x082a
	if (packetver >= 20100713) {
		length_list[0x082a] = 10;
	}

	// Packet: 0x082b
	if (packetver >= 20100713) {
		length_list[0x082b] = 6;
	}

	// Packet: 0x082c
	if (packetver >= 20100720) {
		length_list[0x082c] = 10;
	} else if (packetver >= 20100713) {
		length_list[0x082c] = 14;
	}

	// Packet: 0x082d
	if (packetver >= 20101221) {
		length_list[0x082d] = -1;
	}

	// Packet: 0x0835
	if (packetver >= 20101228) {
		length_list[0x0835] = 5;
	} else if (packetver >= 20101214) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0835] = 19;
	} else if (packetver >= 20101130) {
		length_list[0x0835] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20100601) {
		length_list[0x0835] = -1;
	}

	// Packet: 0x0836
	if (packetver >= 20100601) {
		length_list[0x0836] = -1;
	}

	// Packet: 0x0837
	if (packetver >= 20100601) {
		length_list[0x0837] = 3;
	}

	// Packet: 0x0838
	if (packetver >= 20101228) {
		length_list[0x0838] = -1;
	} else if (packetver >= 20101214) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0838] = 8;
	} else if (packetver >= 20101130) {
		length_list[0x0838] = -1;
	} else if (packetver >= 20100608) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20100601) {
		length_list[0x0838] = 3;
	}

	// Packet: 0x0839
	if (packetver >= 20100608) {
		length_list[0x0839] = 66;
	}

	// Packet: 0x083a
	if (packetver >= 20100701) {
		length_list[0x083a] = 5;
	} else if (packetver >= 20100608) {
		length_list[0x083a] = 4;
	}

	// Packet: 0x083b
	if (packetver >= 20100608) {
		length_list[0x083b] = 2;
	}

	// Packet: 0x083c
	if (packetver >= 20101214) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20101130) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x083c] = 7;
	} else if (packetver >= 20100608) {
		length_list[0x083c] = 12;
	}

	// Packet: 0x083d
	if (packetver >= 20100608) {
		length_list[0x083d] = 6;
	}

	// Packet: 0x083e
	if (packetver >= 20100615) {
		length_list[0x083e] = 26;
	}

	// Packet: 0x083f
	if (packetver >= 20100629) {
		// removed
	} else if (packetver >= 20100622) {
		length_list[0x083f] = 22;
	}

	// Packet: 0x0840
	if (packetver >= 20100713) {
		length_list[0x0840] = -1;
	}

	// Packet: 0x0841
	if (packetver >= 20100714) {
		length_list[0x0841] = 4;
	} else if (packetver >= 20100713) {
		length_list[0x0841] = 19;
	}

	// Packet: 0x0842
	if (packetver >= 20100720) {
		length_list[0x0842] = 6;
	}

	// Packet: 0x0843
	if (packetver >= 20100720) {
		length_list[0x0843] = 6;
	}

	// Packet: 0x0844
	if (packetver >= 20100824) {
		length_list[0x0844] = 2;
	}

	// Packet: 0x0845
	if (packetver >= 20100824) {
		length_list[0x0845] = 10;
	}

	// Packet: 0x0846
	if (packetver >= 20100824) {
		length_list[0x0846] = 4;
	}

	// Packet: 0x0847
	if (packetver >= 20100824) {
		length_list[0x0847] = -1;
	}

	// Packet: 0x0848
	if (packetver >= 20100824) {
		length_list[0x0848] = -1;
	}

	// Packet: 0x0849
	if (packetver >= 20100914) {
		length_list[0x0849] = 16;
	} else if (packetver >= 20100824) {
		length_list[0x0849] = 12;
	}

	// Packet: 0x084a
	if (packetver >= 20101019) {
		length_list[0x084a] = 2;
	}

	// Packet: 0x084b
	if (packetver >= 20101019) {
		length_list[0x084b] = 19;
	}

	// Packet: 0x084c
	if (packetver >= 20101026) {
		length_list[0x084c] = 10;
	}

	// Packet: 0x084d
	if (packetver >= 20101026) {
		length_list[0x084d] = 10;
	}

	// Packet: 0x084e
	if (packetver >= 20101026) {
		length_list[0x084e] = 5;
	}

	// Packet: 0x084f
	if (packetver >= 20101026) {
		length_list[0x084f] = 6;
	}

	// Packet: 0x0850
	if (packetver >= 20101026) {
		length_list[0x0850] = 7;
	}

	// Packet: 0x0851
	if (packetver >= 20101102) {
		length_list[0x0851] = -1;
	}

	// Packet: 0x0852
	if (packetver >= 20101102) {
		length_list[0x0852] = 2;
	}

	// Packet: 0x0853
	if (packetver >= 20101102) {
		length_list[0x0853] = -1;
	}

	// Packet: 0x0854
	if (packetver >= 20101102) {
		length_list[0x0854] = -1;
	}

	// Packet: 0x0855
	if (packetver >= 20101102) {
		length_list[0x0855] = 6;
	}

	// Packet: 0x0856
	if (packetver >= 20101123) {
		length_list[0x0856] = -1;
	}

	// Packet: 0x0857
	if (packetver >= 20101123) {
		length_list[0x0857] = -1;
	}

	// Packet: 0x0858
	if (packetver >= 20101123) {
		length_list[0x0858] = -1;
	}

	// Packet: 0x0859
	if (packetver >= 20101123) {
		length_list[0x0859] = -1;
	}

	// Packet: 0x085a
	if (packetver >= 20101123) {
		length_list[0x085a] = 2;
	}

	// Packet: 0x085b
	if (packetver >= 20101123) {
		length_list[0x085b] = 2;
	}

	// Packet: 0x085c
	if (packetver >= 20101123) {
		length_list[0x085c] = 2;
	}

	// Packet: 0x085d
	if (packetver >= 20101123) {
		length_list[0x085d] = 2;
	}

	// Packet: 0x085e
	if (packetver >= 20101123) {
		length_list[0x085e] = 2;
	}

	// Packet: 0x085f
	if (packetver >= 20101123) {
		length_list[0x085f] = 2;
	}

	// Packet: 0x0860
	if (packetver >= 20101123) {
		length_list[0x0860] = 2;
	}

	// Packet: 0x0861
	if (packetver >= 20101123) {
		length_list[0x0861] = 2;
	}

	// Packet: 0x0862
	if (packetver >= 20101123) {
		length_list[0x0862] = 2;
	}

	// Packet: 0x0863
	if (packetver >= 20101123) {
		length_list[0x0863] = 2;
	}

	// Packet: 0x0864
	if (packetver >= 20101123) {
		length_list[0x0864] = 2;
	}

	// Packet: 0x0865
	if (packetver >= 20101123) {
		length_list[0x0865] = 2;
	}

	// Packet: 0x0866
	if (packetver >= 20101123) {
		length_list[0x0866] = 2;
	}

	// Packet: 0x0867
	if (packetver >= 20101123) {
		length_list[0x0867] = 2;
	}

	// Packet: 0x0868
	if (packetver >= 20101123) {
		length_list[0x0868] = 2;
	}

	// Packet: 0x0869
	if (packetver >= 20101123) {
		length_list[0x0869] = 2;
	}

	// Packet: 0x086a
	if (packetver >= 20101123) {
		length_list[0x086a] = 2;
	}

	// Packet: 0x086b
	if (packetver >= 20101123) {
		length_list[0x086b] = 2;
	}

	// Packet: 0x086c
	if (packetver >= 20101123) {
		length_list[0x086c] = 2;
	}

	// Packet: 0x086d
	if (packetver >= 20101123) {
		length_list[0x086d] = 2;
	}

	// Packet: 0x086e
	if (packetver >= 20101123) {
		length_list[0x086e] = 2;
	}

	// Packet: 0x086f
	if (packetver >= 20101123) {
		length_list[0x086f] = 2;
	}

	// Packet: 0x0870
	if (packetver >= 20101123) {
		length_list[0x0870] = 2;
	}

	// Packet: 0x0871
	if (packetver >= 20101123) {
		length_list[0x0871] = 2;
	}

	// Packet: 0x0872
	if (packetver >= 20101123) {
		length_list[0x0872] = 2;
	}

	// Packet: 0x0873
	if (packetver >= 20101123) {
		length_list[0x0873] = 2;
	}

	// Packet: 0x0874
	if (packetver >= 20101123) {
		length_list[0x0874] = 2;
	}

	// Packet: 0x0875
	if (packetver >= 20101123) {
		length_list[0x0875] = 2;
	}

	// Packet: 0x0876
	if (packetver >= 20101123) {
		length_list[0x0876] = 2;
	}

	// Packet: 0x0877
	if (packetver >= 20101123) {
		length_list[0x0877] = 2;
	}

	// Packet: 0x0878
	if (packetver >= 20101123) {
		length_list[0x0878] = 2;
	}

	// Packet: 0x0879
	if (packetver >= 20101123) {
		length_list[0x0879] = 2;
	}

	// Packet: 0x087a
	if (packetver >= 20101123) {
		length_list[0x087a] = 2;
	}

	// Packet: 0x087b
	if (packetver >= 20101123) {
		length_list[0x087b] = 2;
	}

	// Packet: 0x087c
	if (packetver >= 20101123) {
		length_list[0x087c] = 2;
	}

	// Packet: 0x087d
	if (packetver >= 20101123) {
		length_list[0x087d] = 2;
	}

	// Packet: 0x087e
	if (packetver >= 20101123) {
		length_list[0x087e] = 2;
	}

	// Packet: 0x087f
	if (packetver >= 20101123) {
		length_list[0x087f] = 2;
	}

	// Packet: 0x0880
	if (packetver >= 20101123) {
		length_list[0x0880] = 2;
	}

	// Packet: 0x0881
	if (packetver >= 20101123) {
		length_list[0x0881] = 2;
	}

	// Packet: 0x0882
	if (packetver >= 20101123) {
		length_list[0x0882] = 2;
	}

	// Packet: 0x0883
	if (packetver >= 20101123) {
		length_list[0x0883] = 2;
	}

	// Packet: 0x0884
	if (packetver >= 20101228) {
		length_list[0x0884] = 19;
	} else if (packetver >= 20101221) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0884] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0884] = 2;
	}

	// Packet: 0x0885
	if (packetver >= 20101228) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0885] = 8;
	} else if (packetver >= 20101214) {
		length_list[0x0885] = 19;
	} else if (packetver >= 20101207) {
		length_list[0x0885] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0885] = 2;
	}

	// Packet: 0x0886
	if (packetver >= 20101228) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20101221) {
		length_list[0x0886] = -1;
	} else if (packetver >= 20101207) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0886] = 10;
	} else if (packetver >= 20101123) {
		length_list[0x0886] = 2;
	}

	// Packet: 0x0887
	if (packetver >= 20101221) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x0887] = 4;
	} else if (packetver >= 20101207) {
		length_list[0x0887] = -1;
	} else if (packetver >= 20101130) {
		length_list[0x0887] = 18;
	} else if (packetver >= 20101123) {
		length_list[0x0887] = 2;
	}

	// Packet: 0x0888
	if (packetver >= 20101228) {
		length_list[0x0888] = 8;
	} else if (packetver >= 20101221) {
		length_list[0x0888] = 4;
	} else if (packetver >= 20101130) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0888] = 10;
	}

	// Packet: 0x0889
	if (packetver >= 20101228) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0889] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0889] = 26;
	} else if (packetver >= 20101130) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0889] = 19;
	}

	// Packet: 0x088a
	if (packetver >= 20101123) {
		length_list[0x088a] = 2;
	}

	// Packet: 0x088b
	if (packetver >= 20101123) {
		length_list[0x088b] = 2;
	}

	// Packet: 0x088c
	if (packetver >= 20101228) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x088c] = 5;
	} else if (packetver >= 20101207) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x088c] = 5;
	} else if (packetver >= 20101123) {
		length_list[0x088c] = 26;
	}

	// Packet: 0x088d
	if (packetver >= 20101228) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x088d] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x088d] = 12;
	} else if (packetver >= 20101123) {
		length_list[0x088d] = 2;
	}

	// Packet: 0x088e
	if (packetver >= 20101228) {
		length_list[0x088e] = 18;
	} else if (packetver >= 20101207) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x088e] = 8;
	} else if (packetver >= 20101123) {
		length_list[0x088e] = 2;
	}

	// Packet: 0x088f
	if (packetver >= 20101228) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x088f] = 10;
	} else if (packetver >= 20101214) {
		length_list[0x088f] = 6;
	} else if (packetver >= 20101207) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x088f] = 26;
	} else if (packetver >= 20101123) {
		length_list[0x088f] = 6;
	}

	// Packet: 0x0890
	if (packetver >= 20101228) {
		length_list[0x0890] = 4;
	} else if (packetver >= 20101214) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0890] = 5;
	} else if (packetver >= 20101123) {
		length_list[0x0890] = 2;
	}

	// Packet: 0x0891
	if (packetver >= 20101228) {
		length_list[0x0891] = 26;
	} else if (packetver >= 20101214) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0891] = 8;
	} else if (packetver >= 20101130) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0891] = 5;
	}

	// Packet: 0x0892
	if (packetver >= 20101228) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0892] = 8;
	} else if (packetver >= 20101214) {
		length_list[0x0892] = 10;
	} else if (packetver >= 20101123) {
		length_list[0x0892] = 2;
	}

	// Packet: 0x0893
	if (packetver >= 20101228) {
		length_list[0x0893] = -1;
	} else if (packetver >= 20101130) {
		length_list[0x0893] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0893] = 18;
	}

	// Packet: 0x0894
	if (packetver >= 20101207) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0894] = 5;
	} else if (packetver >= 20101123) {
		length_list[0x0894] = 2;
	}

	// Packet: 0x0895
	if (packetver >= 20101214) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0895] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0895] = 2;
	}

	// Packet: 0x0896
	if (packetver >= 20101228) {
		length_list[0x0896] = -1;
	} else if (packetver >= 20101214) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x0896] = 6;
	} else if (packetver >= 20101130) {
		length_list[0x0896] = 10;
	} else if (packetver >= 20101123) {
		length_list[0x0896] = 2;
	}

	// Packet: 0x0897
	if (packetver >= 20101228) {
		length_list[0x0897] = 8;
	} else if (packetver >= 20101221) {
		length_list[0x0897] = 36;
	} else if (packetver >= 20101214) {
		length_list[0x0897] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x0897] = 2;
	}

	// Packet: 0x0898
	if (packetver >= 20101228) {
		length_list[0x0898] = 10;
	} else if (packetver >= 20101221) {
		length_list[0x0898] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x0898] = 5;
	} else if (packetver >= 20101207) {
		length_list[0x0898] = 6;
	} else if (packetver >= 20101130) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x0898] = -1;
	}

	// Packet: 0x0899
	if (packetver >= 20101228) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x0899] = 5;
	} else if (packetver >= 20101214) {
		length_list[0x0899] = 26;
	} else if (packetver >= 20101207) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x0899] = -1;
	} else if (packetver >= 20101123) {
		length_list[0x0899] = 6;
	}

	// Packet: 0x089a
	if (packetver >= 20101123) {
		length_list[0x089a] = 2;
	}

	// Packet: 0x089b
	if (packetver >= 20101130) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x089b] = 26;
	}

	// Packet: 0x089c
	if (packetver >= 20101228) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x089c] = -1;
	} else if (packetver >= 20101214) {
		length_list[0x089c] = 8;
	} else if (packetver >= 20101123) {
		length_list[0x089c] = 2;
	}

	// Packet: 0x089d
	if (packetver >= 20101228) {
		length_list[0x089d] = 36;
	} else if (packetver >= 20101214) {
		length_list[0x089d] = 6;
	} else if (packetver >= 20101207) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x089d] = 36;
	} else if (packetver >= 20101123) {
		length_list[0x089d] = 2;
	}

	// Packet: 0x089e
	if (packetver >= 20101228) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x089e] = 90;
	} else if (packetver >= 20101214) {
		length_list[0x089e] = 8;
	} else if (packetver >= 20101207) {
		length_list[0x089e] = 18;
	} else if (packetver >= 20101130) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x089e] = 4;
	}

	// Packet: 0x089f
	if (packetver >= 20101221) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x089f] = -1;
	} else if (packetver >= 20101207) {
		length_list[0x089f] = 6;
	} else if (packetver >= 20101130) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x089f] = 8;
	}

	// Packet: 0x08a0
	if (packetver >= 20101228) {
		length_list[0x08a0] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x08a0] = 2;
	}

	// Packet: 0x08a1
	if (packetver >= 20101221) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x08a1] = -1;
	} else if (packetver >= 20101130) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x08a1] = 6;
	}

	// Packet: 0x08a2
	if (packetver >= 20101228) {
		length_list[0x08a2] = 5;
	} else if (packetver >= 20101214) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x08a2] = 8;
	} else if (packetver >= 20101130) {
		length_list[0x08a2] = 19;
	} else if (packetver >= 20101123) {
		length_list[0x08a2] = -1;
	}

	// Packet: 0x08a3
	if (packetver >= 20101221) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x08a3] = 26;
	} else if (packetver >= 20101207) {
		length_list[0x08a3] = -1;
	} else if (packetver >= 20101130) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x08a3] = 12;
	}

	// Packet: 0x08a4
	if (packetver >= 20101228) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x08a4] = 5;
	} else if (packetver >= 20101214) {
		length_list[0x08a4] = 10;
	} else if (packetver >= 20101123) {
		length_list[0x08a4] = 2;
	}

	// Packet: 0x08a5
	if (packetver >= 20101228) {
		length_list[0x08a5] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x08a5] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x08a5] = 2;
	}

	// Packet: 0x08a6
	if (packetver >= 20101123) {
		length_list[0x08a6] = 2;
	}

	// Packet: 0x08a7
	if (packetver >= 20101221) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x08a7] = 8;
	} else if (packetver >= 20101123) {
		length_list[0x08a7] = 2;
	}

	// Packet: 0x08a8
	if (packetver >= 20101123) {
		length_list[0x08a8] = 2;
	}

	// Packet: 0x08a9
	if (packetver >= 20101214) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x08a9] = 10;
	} else if (packetver >= 20101130) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x08a9] = -1;
	}

	// Packet: 0x08aa
	if (packetver >= 20101228) {
		length_list[0x08aa] = 5;
	} else if (packetver >= 20101221) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20101214) {
		length_list[0x08aa] = 7;
	} else if (packetver >= 20101207) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x08aa] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x08aa] = 2;
	}

	// Packet: 0x08ab
	if (packetver >= 20101207) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20101130) {
		length_list[0x08ab] = 4;
	} else if (packetver >= 20101123) {
		length_list[0x08ab] = 2;
	}

	// Packet: 0x08ac
	if (packetver >= 20101228) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x08ac] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x08ac] = 5;
	} else if (packetver >= 20101130) {
		length_list[0x08ac] = 6;
	} else if (packetver >= 20101123) {
		length_list[0x08ac] = 2;
	}

	// Packet: 0x08ad
	if (packetver >= 20101228) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20101221) {
		length_list[0x08ad] = 6;
	} else if (packetver >= 20101214) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20101207) {
		length_list[0x08ad] = 90;
	} else if (packetver >= 20101130) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20101123) {
		length_list[0x08ad] = 6;
	}

	// Packet: 0x08af
	if (packetver >= 20101228) {
		length_list[0x08af] = 10;
	}

	// Packet: 0x08b1
	if (packetver >= 20101221) {
		length_list[0x08b1] = -1;
	}

	// Packet: 0x08b2
	if (packetver >= 20101228) {
		length_list[0x08b2] = -1;
	}

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
