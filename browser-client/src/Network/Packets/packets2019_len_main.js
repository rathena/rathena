/**
 * Network/Packets/packets2019_len_main.js
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
	length_list[0x006d] = 157;

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
	if (packetver >= 20190904) {
		length_list[0x007e] = 46;
	} else if (packetver >= 20190109) {
		length_list[0x007e] = 105;
	}

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
	length_list[0x009d] = 19;

	// Packet: 0x009e
	length_list[0x009e] = 19;

	// Packet: 0x009f
	length_list[0x009f] = 20;

	// Packet: 0x00a0
	length_list[0x00a0] = 33;

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
	length_list[0x00e9] = 29;

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
	length_list[0x00f4] = 31;

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
	length_list[0x010a] = 6;

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
	length_list[0x0110] = 14;

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
	length_list[0x0124] = 31;

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
	length_list[0x018e] = 18;

	// Packet: 0x018f
	length_list[0x018f] = 8;

	// Packet: 0x0190
	length_list[0x0190] = 23;

	// Packet: 0x0191
	if (packetver >= 20190904) {
		length_list[0x0191] = 27;
	} else if (packetver >= 20190109) {
		length_list[0x0191] = 86;
	}

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
	length_list[0x01a3] = 7;

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
	length_list[0x01ae] = 6;

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
	length_list[0x01c4] = 32;

	// Packet: 0x01c5
	length_list[0x01c5] = 32;

	// Packet: 0x01c6
	length_list[0x01c6] = 4;

	// Packet: 0x01c7
	length_list[0x01c7] = 2;

	// Packet: 0x01c8
	length_list[0x01c8] = 15;

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
	length_list[0x01d7] = 15;

	// Packet: 0x01d8
	length_list[0x01d8] = 58;

	// Packet: 0x01d9
	length_list[0x01d9] = 57;

	// Packet: 0x01da
	length_list[0x01da] = 64;

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
	length_list[0x01fd] = 25;

	// Packet: 0x01fe
	length_list[0x01fe] = 5;

	// Packet: 0x01ff
	length_list[0x01ff] = 10;

	// Packet: 0x0200
	length_list[0x0200] = 26;

	// Packet: 0x0201
	length_list[0x0201] = -1;

	// Packet: 0x0202
	length_list[0x0202] = 26;

	// Packet: 0x0203
	length_list[0x0203] = 10;

	// Packet: 0x0204
	length_list[0x0204] = 18;

	// Packet: 0x0205
	length_list[0x0205] = 26;

	// Packet: 0x0206
	length_list[0x0206] = 35;

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
	length_list[0x0223] = 10;

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
	length_list[0x022a] = 62;

	// Packet: 0x022b
	length_list[0x022b] = 61;

	// Packet: 0x022c
	length_list[0x022c] = 69;

	// Packet: 0x022d
	length_list[0x022d] = 5;

	// Packet: 0x022e
	length_list[0x022e] = 73;

	// Packet: 0x022f
	length_list[0x022f] = 7;

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
	length_list[0x023b] = 36;

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
	length_list[0x025b] = 8;

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
	length_list[0x0281] = 4;

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
	length_list[0x0298] = 10;

	// Packet: 0x0299
	length_list[0x0299] = 8;

	// Packet: 0x029a
	length_list[0x029a] = 37;

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
	length_list[0x02b8] = 32;

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
	length_list[0x02c4] = 26;

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
	length_list[0x02d4] = 39;

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
	length_list[0x02ec] = 71;

	// Packet: 0x02ed
	length_list[0x02ed] = 63;

	// Packet: 0x02ee
	length_list[0x02ee] = 64;

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
	length_list[0x035f] = 5;

	// Packet: 0x0360
	length_list[0x0360] = 6;

	// Packet: 0x0361
	length_list[0x0361] = 5;

	// Packet: 0x0362
	length_list[0x0362] = 6;

	// Packet: 0x0363
	length_list[0x0363] = 6;

	// Packet: 0x0364
	length_list[0x0364] = 8;

	// Packet: 0x0365
	length_list[0x0365] = 8;

	// Packet: 0x0366
	length_list[0x0366] = 10;

	// Packet: 0x0367
	if (packetver >= 20190904) {
		length_list[0x0367] = 31;
	} else if (packetver >= 20190109) {
		length_list[0x0367] = 90;
	}

	// Packet: 0x0368
	length_list[0x0368] = 6;

	// Packet: 0x0369
	length_list[0x0369] = 6;

	// Packet: 0x03dd
	length_list[0x03dd] = 18;

	// Packet: 0x03de
	length_list[0x03de] = 18;

	// Packet: 0x0436
	length_list[0x0436] = 19;

	// Packet: 0x0437
	length_list[0x0437] = 7;

	// Packet: 0x0438
	length_list[0x0438] = 10;

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
	length_list[0x0445] = 12;

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
	length_list[0x07e4] = -1;

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
	length_list[0x07ec] = 8;

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
	length_list[0x0802] = 18;

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
	length_list[0x080f] = 30;

	// Packet: 0x0810
	length_list[0x0810] = 3;

	// Packet: 0x0811
	length_list[0x0811] = -1;

	// Packet: 0x0812
	length_list[0x0812] = 8;

	// Packet: 0x0813
	length_list[0x0813] = -1;

	// Packet: 0x0814
	length_list[0x0814] = 86;

	// Packet: 0x0815
	length_list[0x0815] = 2;

	// Packet: 0x0816
	length_list[0x0816] = 6;

	// Packet: 0x0817
	length_list[0x0817] = 6;

	// Packet: 0x0818
	length_list[0x0818] = -1;

	// Packet: 0x0819
	length_list[0x0819] = -1;

	// Packet: 0x081a
	length_list[0x081a] = 4;

	// Packet: 0x081b
	length_list[0x081b] = 12;

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
	length_list[0x0824] = 8;

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
	length_list[0x0835] = -1;

	// Packet: 0x0836
	length_list[0x0836] = -1;

	// Packet: 0x0837
	length_list[0x0837] = 3;

	// Packet: 0x0838
	length_list[0x0838] = 2;

	// Packet: 0x0839
	length_list[0x0839] = 66;

	// Packet: 0x083a
	length_list[0x083a] = 5;

	// Packet: 0x083b
	length_list[0x083b] = 2;

	// Packet: 0x083c
	length_list[0x083c] = 14;

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
	length_list[0x084b] = 21;

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
	length_list[0x085a] = 2;

	// Packet: 0x085b
	length_list[0x085b] = 2;

	// Packet: 0x085c
	length_list[0x085c] = 2;

	// Packet: 0x085d
	length_list[0x085d] = 2;

	// Packet: 0x085e
	length_list[0x085e] = 2;

	// Packet: 0x085f
	length_list[0x085f] = 2;

	// Packet: 0x0860
	length_list[0x0860] = 2;

	// Packet: 0x0861
	length_list[0x0861] = 2;

	// Packet: 0x0862
	length_list[0x0862] = 2;

	// Packet: 0x0863
	length_list[0x0863] = 2;

	// Packet: 0x0864
	length_list[0x0864] = 2;

	// Packet: 0x0865
	length_list[0x0865] = 2;

	// Packet: 0x0866
	length_list[0x0866] = 2;

	// Packet: 0x0867
	length_list[0x0867] = 2;

	// Packet: 0x0868
	length_list[0x0868] = 2;

	// Packet: 0x0869
	length_list[0x0869] = 2;

	// Packet: 0x086a
	length_list[0x086a] = 2;

	// Packet: 0x086b
	length_list[0x086b] = 2;

	// Packet: 0x086c
	length_list[0x086c] = 2;

	// Packet: 0x086d
	length_list[0x086d] = 2;

	// Packet: 0x086e
	length_list[0x086e] = 2;

	// Packet: 0x086f
	length_list[0x086f] = 2;

	// Packet: 0x0870
	length_list[0x0870] = 2;

	// Packet: 0x0871
	length_list[0x0871] = 2;

	// Packet: 0x0872
	length_list[0x0872] = 2;

	// Packet: 0x0873
	length_list[0x0873] = 2;

	// Packet: 0x0874
	length_list[0x0874] = 2;

	// Packet: 0x0875
	length_list[0x0875] = 2;

	// Packet: 0x0876
	length_list[0x0876] = 2;

	// Packet: 0x0877
	length_list[0x0877] = 2;

	// Packet: 0x0878
	length_list[0x0878] = 2;

	// Packet: 0x0879
	length_list[0x0879] = 2;

	// Packet: 0x087a
	length_list[0x087a] = 2;

	// Packet: 0x087b
	length_list[0x087b] = 2;

	// Packet: 0x087c
	length_list[0x087c] = 2;

	// Packet: 0x087d
	length_list[0x087d] = 2;

	// Packet: 0x087e
	length_list[0x087e] = 2;

	// Packet: 0x087f
	length_list[0x087f] = 2;

	// Packet: 0x0880
	length_list[0x0880] = 2;

	// Packet: 0x0881
	length_list[0x0881] = 2;

	// Packet: 0x0882
	length_list[0x0882] = 2;

	// Packet: 0x0883
	length_list[0x0883] = 2;

	// Packet: 0x0884
	length_list[0x0884] = 2;

	// Packet: 0x0885
	length_list[0x0885] = 2;

	// Packet: 0x0886
	length_list[0x0886] = 2;

	// Packet: 0x0887
	length_list[0x0887] = 2;

	// Packet: 0x0888
	length_list[0x0888] = 2;

	// Packet: 0x0889
	length_list[0x0889] = 2;

	// Packet: 0x088a
	length_list[0x088a] = 2;

	// Packet: 0x088b
	length_list[0x088b] = 2;

	// Packet: 0x088c
	length_list[0x088c] = 2;

	// Packet: 0x088d
	length_list[0x088d] = 2;

	// Packet: 0x088e
	length_list[0x088e] = 2;

	// Packet: 0x088f
	length_list[0x088f] = 2;

	// Packet: 0x0890
	length_list[0x0890] = 2;

	// Packet: 0x0891
	length_list[0x0891] = 2;

	// Packet: 0x0892
	length_list[0x0892] = 2;

	// Packet: 0x0893
	length_list[0x0893] = 2;

	// Packet: 0x0894
	length_list[0x0894] = 2;

	// Packet: 0x0895
	length_list[0x0895] = 2;

	// Packet: 0x0896
	length_list[0x0896] = 2;

	// Packet: 0x0897
	length_list[0x0897] = 2;

	// Packet: 0x0898
	length_list[0x0898] = 2;

	// Packet: 0x0899
	length_list[0x0899] = 2;

	// Packet: 0x089a
	length_list[0x089a] = 2;

	// Packet: 0x089b
	length_list[0x089b] = 2;

	// Packet: 0x089c
	length_list[0x089c] = 2;

	// Packet: 0x089d
	length_list[0x089d] = 2;

	// Packet: 0x089e
	length_list[0x089e] = 2;

	// Packet: 0x089f
	length_list[0x089f] = 2;

	// Packet: 0x08a0
	length_list[0x08a0] = 2;

	// Packet: 0x08a1
	length_list[0x08a1] = 2;

	// Packet: 0x08a2
	length_list[0x08a2] = 2;

	// Packet: 0x08a3
	length_list[0x08a3] = 2;

	// Packet: 0x08a4
	length_list[0x08a4] = 2;

	// Packet: 0x08a5
	length_list[0x08a5] = 2;

	// Packet: 0x08a6
	length_list[0x08a6] = 2;

	// Packet: 0x08a7
	length_list[0x08a7] = 2;

	// Packet: 0x08a8
	length_list[0x08a8] = 2;

	// Packet: 0x08a9
	length_list[0x08a9] = 2;

	// Packet: 0x08aa
	length_list[0x08aa] = 2;

	// Packet: 0x08ab
	length_list[0x08ab] = 2;

	// Packet: 0x08ac
	length_list[0x08ac] = 2;

	// Packet: 0x08ad
	length_list[0x08ad] = 2;

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
	length_list[0x08e3] = 157;

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
	length_list[0x0917] = 2;

	// Packet: 0x0918
	length_list[0x0918] = 2;

	// Packet: 0x0919
	length_list[0x0919] = 2;

	// Packet: 0x091a
	length_list[0x091a] = 2;

	// Packet: 0x091b
	length_list[0x091b] = 2;

	// Packet: 0x091c
	length_list[0x091c] = 2;

	// Packet: 0x091d
	length_list[0x091d] = 2;

	// Packet: 0x091e
	length_list[0x091e] = 2;

	// Packet: 0x091f
	length_list[0x091f] = 2;

	// Packet: 0x0920
	length_list[0x0920] = 2;

	// Packet: 0x0921
	length_list[0x0921] = 2;

	// Packet: 0x0922
	length_list[0x0922] = 2;

	// Packet: 0x0923
	length_list[0x0923] = 2;

	// Packet: 0x0924
	length_list[0x0924] = 2;

	// Packet: 0x0925
	length_list[0x0925] = 2;

	// Packet: 0x0926
	length_list[0x0926] = 2;

	// Packet: 0x0927
	length_list[0x0927] = 2;

	// Packet: 0x0928
	length_list[0x0928] = 2;

	// Packet: 0x0929
	length_list[0x0929] = 2;

	// Packet: 0x092a
	length_list[0x092a] = 2;

	// Packet: 0x092b
	length_list[0x092b] = 2;

	// Packet: 0x092c
	length_list[0x092c] = 2;

	// Packet: 0x092d
	length_list[0x092d] = 2;

	// Packet: 0x092e
	length_list[0x092e] = 2;

	// Packet: 0x092f
	length_list[0x092f] = 2;

	// Packet: 0x0930
	length_list[0x0930] = 2;

	// Packet: 0x0931
	length_list[0x0931] = 2;

	// Packet: 0x0932
	length_list[0x0932] = 2;

	// Packet: 0x0933
	length_list[0x0933] = 2;

	// Packet: 0x0934
	length_list[0x0934] = 2;

	// Packet: 0x0935
	length_list[0x0935] = 2;

	// Packet: 0x0936
	length_list[0x0936] = 2;

	// Packet: 0x0937
	length_list[0x0937] = 2;

	// Packet: 0x0938
	length_list[0x0938] = 2;

	// Packet: 0x0939
	length_list[0x0939] = 2;

	// Packet: 0x093a
	length_list[0x093a] = 2;

	// Packet: 0x093b
	length_list[0x093b] = 2;

	// Packet: 0x093c
	length_list[0x093c] = 2;

	// Packet: 0x093d
	length_list[0x093d] = 2;

	// Packet: 0x093e
	length_list[0x093e] = 2;

	// Packet: 0x093f
	length_list[0x093f] = 2;

	// Packet: 0x0940
	length_list[0x0940] = 2;

	// Packet: 0x0941
	length_list[0x0941] = 2;

	// Packet: 0x0942
	length_list[0x0942] = 2;

	// Packet: 0x0943
	length_list[0x0943] = 2;

	// Packet: 0x0944
	length_list[0x0944] = 2;

	// Packet: 0x0945
	length_list[0x0945] = 2;

	// Packet: 0x0946
	length_list[0x0946] = 2;

	// Packet: 0x0947
	length_list[0x0947] = 2;

	// Packet: 0x0948
	length_list[0x0948] = 2;

	// Packet: 0x0949
	length_list[0x0949] = 2;

	// Packet: 0x094a
	length_list[0x094a] = 2;

	// Packet: 0x094b
	length_list[0x094b] = 2;

	// Packet: 0x094c
	length_list[0x094c] = 2;

	// Packet: 0x094d
	length_list[0x094d] = 2;

	// Packet: 0x094e
	length_list[0x094e] = 2;

	// Packet: 0x094f
	length_list[0x094f] = 2;

	// Packet: 0x0950
	length_list[0x0950] = 2;

	// Packet: 0x0951
	length_list[0x0951] = 2;

	// Packet: 0x0952
	length_list[0x0952] = 2;

	// Packet: 0x0953
	length_list[0x0953] = 2;

	// Packet: 0x0954
	length_list[0x0954] = 2;

	// Packet: 0x0955
	length_list[0x0955] = 2;

	// Packet: 0x0956
	length_list[0x0956] = 2;

	// Packet: 0x0957
	length_list[0x0957] = 2;

	// Packet: 0x0958
	length_list[0x0958] = 2;

	// Packet: 0x0959
	length_list[0x0959] = 2;

	// Packet: 0x095a
	length_list[0x095a] = 2;

	// Packet: 0x095b
	length_list[0x095b] = 2;

	// Packet: 0x095c
	length_list[0x095c] = 2;

	// Packet: 0x095d
	length_list[0x095d] = 2;

	// Packet: 0x095e
	length_list[0x095e] = 2;

	// Packet: 0x095f
	length_list[0x095f] = 2;

	// Packet: 0x0960
	length_list[0x0960] = 2;

	// Packet: 0x0961
	length_list[0x0961] = 2;

	// Packet: 0x0962
	length_list[0x0962] = 2;

	// Packet: 0x0963
	length_list[0x0963] = 2;

	// Packet: 0x0964
	length_list[0x0964] = 2;

	// Packet: 0x0965
	length_list[0x0965] = 2;

	// Packet: 0x0966
	length_list[0x0966] = 2;

	// Packet: 0x0967
	length_list[0x0967] = 2;

	// Packet: 0x0968
	length_list[0x0968] = 2;

	// Packet: 0x0969
	length_list[0x0969] = 2;

	// Packet: 0x096a
	length_list[0x096a] = 2;

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
	length_list[0x0990] = 41;

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
	length_list[0x09ad] = 12;

	// Packet: 0x09ae
	length_list[0x09ae] = 19;

	// Packet: 0x09af
	length_list[0x09af] = 4;

	// Packet: 0x09b0
	length_list[0x09b0] = 10;

	// Packet: 0x09b1
	length_list[0x09b1] = 4;

	// Packet: 0x09b2
	length_list[0x09b2] = 10;

	// Packet: 0x09b3
	length_list[0x09b3] = 6;

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
	length_list[0x09c3] = 10;

	// Packet: 0x09c4
	length_list[0x09c4] = 10;

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
	length_list[0x09e6] = 24;

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
	length_list[0x09f7] = 77;

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
	length_list[0x0a05] = 63;

	// Packet: 0x0a06
	length_list[0x0a06] = 6;

	// Packet: 0x0a07
	length_list[0x0a07] = 9;

	// Packet: 0x0a08
	length_list[0x0a08] = 26;

	// Packet: 0x0a09
	length_list[0x0a09] = 55;

	// Packet: 0x0a0a
	length_list[0x0a0a] = 57;

	// Packet: 0x0a0b
	length_list[0x0a0b] = 57;

	// Packet: 0x0a0c
	length_list[0x0a0c] = 66;

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
	length_list[0x0a1a] = 25;

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
	length_list[0x0a20] = 23;

	// Packet: 0x0a21
	length_list[0x0a21] = 3;

	// Packet: 0x0a22
	length_list[0x0a22] = 7;

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
	length_list[0x0a37] = 69;

	// Packet: 0x0a38
	length_list[0x0a38] = 3;

	// Packet: 0x0a39
	length_list[0x0a39] = 36;

	// Packet: 0x0a3a
	length_list[0x0a3a] = 12;

	// Packet: 0x0a3b
	length_list[0x0a3b] = -1;

	// Packet: 0x0a3c
	length_list[0x0a3c] = -1;

	// Packet: 0x0a3d
	length_list[0x0a3d] = 20;

	// Packet: 0x0a3e
	length_list[0x0a3e] = -1;

	// Packet: 0x0a3f
	length_list[0x0a3f] = 11;

	// Packet: 0x0a40
	length_list[0x0a40] = 11;

	// Packet: 0x0a41
	length_list[0x0a41] = 18;

	// Packet: 0x0a42
	length_list[0x0a42] = 43;

	// Packet: 0x0a43
	length_list[0x0a43] = 85;

	// Packet: 0x0a44
	length_list[0x0a44] = -1;

	// Packet: 0x0a46
	length_list[0x0a46] = 14;

	// Packet: 0x0a47
	length_list[0x0a47] = 3;

	// Packet: 0x0a48
	length_list[0x0a48] = 2;

	// Packet: 0x0a49
	length_list[0x0a49] = 22;

	// Packet: 0x0a4a
	length_list[0x0a4a] = 6;

	// Packet: 0x0a4b
	length_list[0x0a4b] = 22;

	// Packet: 0x0a4c
	length_list[0x0a4c] = 28;

	// Packet: 0x0a4d
	length_list[0x0a4d] = -1;

	// Packet: 0x0a4e
	length_list[0x0a4e] = 6;

	// Packet: 0x0a4f
	length_list[0x0a4f] = -1;

	// Packet: 0x0a50
	length_list[0x0a50] = 4;

	// Packet: 0x0a51
	length_list[0x0a51] = 34;

	// Packet: 0x0a52
	length_list[0x0a52] = 20;

	// Packet: 0x0a53
	length_list[0x0a53] = 10;

	// Packet: 0x0a54
	length_list[0x0a54] = -1;

	// Packet: 0x0a55
	length_list[0x0a55] = 2;

	// Packet: 0x0a56
	length_list[0x0a56] = 6;

	// Packet: 0x0a57
	length_list[0x0a57] = 6;

	// Packet: 0x0a58
	length_list[0x0a58] = 8;

	// Packet: 0x0a59
	length_list[0x0a59] = -1;

	// Packet: 0x0a5a
	length_list[0x0a5a] = 2;

	// Packet: 0x0a5b
	length_list[0x0a5b] = 7;

	// Packet: 0x0a5c
	length_list[0x0a5c] = 18;

	// Packet: 0x0a5d
	length_list[0x0a5d] = 6;

	// Packet: 0x0a68
	length_list[0x0a68] = 3;

	// Packet: 0x0a69
	length_list[0x0a69] = 6;

	// Packet: 0x0a6a
	length_list[0x0a6a] = 12;

	// Packet: 0x0a6b
	length_list[0x0a6b] = -1;

	// Packet: 0x0a6c
	length_list[0x0a6c] = 7;

	// Packet: 0x0a6d
	length_list[0x0a6d] = -1;

	// Packet: 0x0a6e
	length_list[0x0a6e] = -1;

	// Packet: 0x0a6f
	length_list[0x0a6f] = -1;

	// Packet: 0x0a70
	length_list[0x0a70] = 2;

	// Packet: 0x0a71
	length_list[0x0a71] = -1;

	// Packet: 0x0a72
	length_list[0x0a72] = 61;

	// Packet: 0x0a73
	length_list[0x0a73] = 2;

	// Packet: 0x0a74
	length_list[0x0a74] = 8;

	// Packet: 0x0a76
	length_list[0x0a76] = 80;

	// Packet: 0x0a77
	length_list[0x0a77] = 15;

	// Packet: 0x0a78
	length_list[0x0a78] = 15;

	// Packet: 0x0a79
	length_list[0x0a79] = -1;

	// Packet: 0x0a7b
	length_list[0x0a7b] = -1;

	// Packet: 0x0a7c
	length_list[0x0a7c] = -1;

	// Packet: 0x0a7d
	length_list[0x0a7d] = -1;

	// Packet: 0x0a7e
	length_list[0x0a7e] = -1;

	// Packet: 0x0a7f
	length_list[0x0a7f] = -1;

	// Packet: 0x0a80
	length_list[0x0a80] = 6;

	// Packet: 0x0a81
	length_list[0x0a81] = 4;

	// Packet: 0x0a82
	length_list[0x0a82] = 46;

	// Packet: 0x0a83
	length_list[0x0a83] = 46;

	// Packet: 0x0a84
	length_list[0x0a84] = 94;

	// Packet: 0x0a85
	length_list[0x0a85] = 82;

	// Packet: 0x0a86
	length_list[0x0a86] = -1;

	// Packet: 0x0a87
	length_list[0x0a87] = -1;

	// Packet: 0x0a88
	length_list[0x0a88] = 2;

	// Packet: 0x0a89
	length_list[0x0a89] = 61;

	// Packet: 0x0a8a
	length_list[0x0a8a] = 6;

	// Packet: 0x0a8b
	length_list[0x0a8b] = 2;

	// Packet: 0x0a8c
	length_list[0x0a8c] = 2;

	// Packet: 0x0a8d
	length_list[0x0a8d] = -1;

	// Packet: 0x0a8e
	length_list[0x0a8e] = 2;

	// Packet: 0x0a8f
	length_list[0x0a8f] = 2;

	// Packet: 0x0a90
	length_list[0x0a90] = 3;

	// Packet: 0x0a91
	length_list[0x0a91] = -1;

	// Packet: 0x0a92
	length_list[0x0a92] = -1;

	// Packet: 0x0a93
	length_list[0x0a93] = 3;

	// Packet: 0x0a94
	length_list[0x0a94] = 2;

	// Packet: 0x0a95
	length_list[0x0a95] = 4;

	// Packet: 0x0a96
	length_list[0x0a96] = 61;

	// Packet: 0x0a97
	length_list[0x0a97] = 8;

	// Packet: 0x0a98
	length_list[0x0a98] = 10;

	// Packet: 0x0a99
	length_list[0x0a99] = 4;

	// Packet: 0x0a9a
	length_list[0x0a9a] = 10;

	// Packet: 0x0a9b
	length_list[0x0a9b] = -1;

	// Packet: 0x0a9c
	length_list[0x0a9c] = 2;

	// Packet: 0x0a9d
	length_list[0x0a9d] = 4;

	// Packet: 0x0a9e
	length_list[0x0a9e] = 2;

	// Packet: 0x0a9f
	length_list[0x0a9f] = 2;

	// Packet: 0x0aa0
	length_list[0x0aa0] = 2;

	// Packet: 0x0aa1
	length_list[0x0aa1] = 4;

	// Packet: 0x0aa2
	length_list[0x0aa2] = -1;

	// Packet: 0x0aa3
	length_list[0x0aa3] = 9;

	// Packet: 0x0aa4
	length_list[0x0aa4] = 2;

	// Packet: 0x0aa5
	length_list[0x0aa5] = -1;

	// Packet: 0x0aa6
	length_list[0x0aa6] = 36;

	// Packet: 0x0aa7
	length_list[0x0aa7] = 6;

	// Packet: 0x0aa8
	length_list[0x0aa8] = 5;

	// Packet: 0x0aa9
	length_list[0x0aa9] = -1;

	// Packet: 0x0aaa
	length_list[0x0aaa] = -1;

	// Packet: 0x0aab
	length_list[0x0aab] = -1;

	// Packet: 0x0aac
	length_list[0x0aac] = 69;

	// Packet: 0x0aad
	length_list[0x0aad] = 51;

	// Packet: 0x0aae
	length_list[0x0aae] = 2;

	// Packet: 0x0aaf
	length_list[0x0aaf] = 6;

	// Packet: 0x0ab0
	length_list[0x0ab0] = 6;

	// Packet: 0x0ab1
	length_list[0x0ab1] = 14;

	// Packet: 0x0ab2
	length_list[0x0ab2] = 7;

	// Packet: 0x0ab3
	length_list[0x0ab3] = 19;

	// Packet: 0x0ab4
	length_list[0x0ab4] = 6;

	// Packet: 0x0ab5
	length_list[0x0ab5] = 2;

	// Packet: 0x0ab6
	length_list[0x0ab6] = 8;

	// Packet: 0x0ab7
	length_list[0x0ab7] = 4;

	// Packet: 0x0ab8
	length_list[0x0ab8] = 2;

	// Packet: 0x0ab9
	length_list[0x0ab9] = 47;

	// Packet: 0x0aba
	length_list[0x0aba] = 2;

	// Packet: 0x0abb
	length_list[0x0abb] = 2;

	// Packet: 0x0abc
	length_list[0x0abc] = -1;

	// Packet: 0x0abd
	length_list[0x0abd] = 10;

	// Packet: 0x0abe
	length_list[0x0abe] = -1;

	// Packet: 0x0abf
	length_list[0x0abf] = -1;

	// Packet: 0x0ac0
	length_list[0x0ac0] = 26;

	// Packet: 0x0ac1
	length_list[0x0ac1] = 26;

	// Packet: 0x0ac2
	length_list[0x0ac2] = -1;

	// Packet: 0x0ac3
	length_list[0x0ac3] = 2;

	// Packet: 0x0ac4
	length_list[0x0ac4] = -1;

	// Packet: 0x0ac5
	length_list[0x0ac5] = 156;

	// Packet: 0x0ac6
	length_list[0x0ac6] = 156;

	// Packet: 0x0ac7
	length_list[0x0ac7] = 156;

	// Packet: 0x0ac8
	length_list[0x0ac8] = 2;

	// Packet: 0x0ac9
	length_list[0x0ac9] = -1;

	// Packet: 0x0aca
	length_list[0x0aca] = 3;

	// Packet: 0x0acb
	length_list[0x0acb] = 12;

	// Packet: 0x0acc
	length_list[0x0acc] = 18;

	// Packet: 0x0acd
	length_list[0x0acd] = 23;

	// Packet: 0x0ace
	length_list[0x0ace] = 4;

	// Packet: 0x0acf
	length_list[0x0acf] = 68;

	// Packet: 0x0ad0
	length_list[0x0ad0] = 11;

	// Packet: 0x0ad1
	length_list[0x0ad1] = -1;

	// Packet: 0x0ad2
	length_list[0x0ad2] = 30;

	// Packet: 0x0ad3
	length_list[0x0ad3] = -1;

	// Packet: 0x0ad4
	length_list[0x0ad4] = -1;

	// Packet: 0x0ad5
	length_list[0x0ad5] = 2;

	// Packet: 0x0ad6
	length_list[0x0ad6] = 2;

	// Packet: 0x0ad7
	length_list[0x0ad7] = 8;

	// Packet: 0x0ad8
	length_list[0x0ad8] = 8;

	// Packet: 0x0ad9
	length_list[0x0ad9] = -1;

	// Packet: 0x0ada
	length_list[0x0ada] = 32;

	// Packet: 0x0adb
	length_list[0x0adb] = -1;

	// Packet: 0x0adc
	length_list[0x0adc] = 6;

	// Packet: 0x0add
	length_list[0x0add] = 24;

	// Packet: 0x0ade
	length_list[0x0ade] = 6;

	// Packet: 0x0adf
	length_list[0x0adf] = 58;

	// Packet: 0x0ae0
	length_list[0x0ae0] = 30;

	// Packet: 0x0ae1
	length_list[0x0ae1] = 28;

	// Packet: 0x0ae2
	length_list[0x0ae2] = 7;

	// Packet: 0x0ae3
	length_list[0x0ae3] = -1;

	// Packet: 0x0ae4
	length_list[0x0ae4] = 89;

	// Packet: 0x0ae5
	length_list[0x0ae5] = -1;

	// Packet: 0x0ae6
	length_list[0x0ae6] = 10;

	// Packet: 0x0ae7
	length_list[0x0ae7] = 38;

	// Packet: 0x0ae8
	length_list[0x0ae8] = 2;

	// Packet: 0x0ae9
	length_list[0x0ae9] = 13;

	// Packet: 0x0aec
	length_list[0x0aec] = 2;

	// Packet: 0x0aed
	length_list[0x0aed] = 2;

	// Packet: 0x0aee
	length_list[0x0aee] = 2;

	// Packet: 0x0aef
	length_list[0x0aef] = 2;

	// Packet: 0x0af0
	length_list[0x0af0] = 10;

	// Packet: 0x0af2
	length_list[0x0af2] = 40;

	// Packet: 0x0af3
	length_list[0x0af3] = -1;

	// Packet: 0x0af4
	length_list[0x0af4] = 11;

	// Packet: 0x0af5
	length_list[0x0af5] = 3;

	// Packet: 0x0af6
	length_list[0x0af6] = 88;

	// Packet: 0x0af7
	length_list[0x0af7] = 32;

	// Packet: 0x0af8
	length_list[0x0af8] = 11;

	// Packet: 0x0af9
	length_list[0x0af9] = 6;

	// Packet: 0x0afa
	if (packetver >= 20191204) {
		length_list[0x0afa] = 58;
	} else if (packetver >= 20190109) {
		length_list[0x0afa] = 54;
	}

	// Packet: 0x0afb
	length_list[0x0afb] = -1;

	// Packet: 0x0afc
	length_list[0x0afc] = 16;

	// Packet: 0x0afd
	length_list[0x0afd] = -1;

	// Packet: 0x0afe
	length_list[0x0afe] = -1;

	// Packet: 0x0aff
	length_list[0x0aff] = -1;

	// Packet: 0x0b00
	length_list[0x0b00] = 8;

	// Packet: 0x0b01
	length_list[0x0b01] = 56;

	// Packet: 0x0b02
	length_list[0x0b02] = 26;

	// Packet: 0x0b03
	length_list[0x0b03] = -1;

	// Packet: 0x0b04
	if (packetver >= 20190807) {
		length_list[0x0b04] = 90;
	} else if (packetver >= 20190605) {
		length_list[0x0b04] = 72;
	} else if (packetver >= 20190109) {
		length_list[0x0b04] = 80;
	}

	// Packet: 0x0b05
	length_list[0x0b05] = 63;

	// Packet: 0x0b07
	length_list[0x0b07] = -1;

	// Packet: 0x0b08
	length_list[0x0b08] = -1;

	// Packet: 0x0b09
	length_list[0x0b09] = -1;

	// Packet: 0x0b0a
	length_list[0x0b0a] = -1;

	// Packet: 0x0b0b
	length_list[0x0b0b] = 4;

	// Packet: 0x0b0c
	length_list[0x0b0c] = 155;

	// Packet: 0x0b0d
	length_list[0x0b0d] = 10;

	// Packet: 0x0b0e
	length_list[0x0b0e] = -1;

	// Packet: 0x0b0f
	length_list[0x0b0f] = -1;

	// Packet: 0x0b10
	length_list[0x0b10] = 10;

	// Packet: 0x0b11
	length_list[0x0b11] = 4;

	// Packet: 0x0b12
	length_list[0x0b12] = 2;

	// Packet: 0x0b13
	length_list[0x0b13] = 48;

	// Packet: 0x0b14
	length_list[0x0b14] = 2;

	// Packet: 0x0b15
	length_list[0x0b15] = 7;

	// Packet: 0x0b16
	length_list[0x0b16] = 2;

	// Packet: 0x0b17
	length_list[0x0b17] = 3;

	// Packet: 0x0b18
	length_list[0x0b18] = 4;

	// Packet: 0x0b19
	length_list[0x0b19] = 2;

	// Packet: 0x0b1a
	length_list[0x0b1a] = 29;

	// Packet: 0x0b1b
	if (packetver >= 20190213) {
		length_list[0x0b1b] = 2;
	}

	// Packet: 0x0b1c
	if (packetver >= 20190227) {
		length_list[0x0b1c] = 2;
	}

	// Packet: 0x0b1d
	if (packetver >= 20190227) {
		length_list[0x0b1d] = 2;
	}

	// Packet: 0x0b1e
	if (packetver >= 20190619) {
		length_list[0x0b1e] = 14;
	} else if (packetver >= 20190227) {
		length_list[0x0b1e] = 10;
	}

	// Packet: 0x0b1f
	if (packetver >= 20190619) {
		length_list[0x0b1f] = 14;
	} else if (packetver >= 20190306) {
		length_list[0x0b1f] = 10;
	}

	// Packet: 0x0b20
	if (packetver >= 20190403) {
		length_list[0x0b20] = 271;
	}

	// Packet: 0x0b21
	if (packetver >= 20190403) {
		length_list[0x0b21] = 13;
	} else if (packetver >= 20190306) {
		// removed
	} else if (packetver >= 20190227) {
		length_list[0x0b21] = 10;
	}

	// Packet: 0x0b22
	if (packetver >= 20190403) {
		length_list[0x0b22] = 5;
	} else if (packetver >= 20190306) {
		// removed
	} else if (packetver >= 20190227) {
		length_list[0x0b22] = 6;
	}

	// Packet: 0x0b23
	if (packetver >= 20190417) {
		length_list[0x0b23] = 6;
	} else if (packetver >= 20190306) {
		// removed
	} else if (packetver >= 20190227) {
		length_list[0x0b23] = 10;
	}

	// Packet: 0x0b24
	if (packetver >= 20190417) {
		length_list[0x0b24] = 6;
	}

	// Packet: 0x0b25
	if (packetver >= 20190417) {
		length_list[0x0b25] = 6;
	}

	// Packet: 0x0b26
	if (packetver >= 20190522) {
		// removed
	} else if (packetver >= 20190508) {
		length_list[0x0b26] = 16;
	} else if (packetver >= 20190417) {
		length_list[0x0b26] = 18;
	}

	// Packet: 0x0b27
	if (packetver >= 20190508) {
		length_list[0x0b27] = -1;
	}

	// Packet: 0x0b28
	if (packetver >= 20190522) {
		length_list[0x0b28] = 3;
	} else if (packetver >= 20190508) {
		length_list[0x0b28] = 22;
	}

	// Packet: 0x0b29
	if (packetver >= 20190605) {
		// removed
	} else if (packetver >= 20190508) {
		length_list[0x0b29] = 6;
	}

	// Packet: 0x0b2a
	if (packetver >= 20190605) {
		// removed
	} else if (packetver >= 20190522) {
		length_list[0x0b2a] = 40;
	} else if (packetver >= 20190508) {
		length_list[0x0b2a] = 6;
	}

	// Packet: 0x0b2b
	if (packetver >= 20190522) {
		length_list[0x0b2b] = 11;
	}

	// Packet: 0x0b2c
	if (packetver >= 20190522) {
		length_list[0x0b2c] = 3;
	}

	// Packet: 0x0b2d
	if (packetver >= 20190522) {
		length_list[0x0b2d] = 11;
	}

	// Packet: 0x0b2e
	if (packetver >= 20190522) {
		length_list[0x0b2e] = 4;
	}

	// Packet: 0x0b2f
	if (packetver >= 20190529) {
		length_list[0x0b2f] = 73;
	}

	// Packet: 0x0b30
	if (packetver >= 20190529) {
		length_list[0x0b30] = -1;
	}

	// Packet: 0x0b31
	if (packetver >= 20190619) {
		length_list[0x0b31] = 17;
	}

	// Packet: 0x0b32
	if (packetver >= 20190619) {
		length_list[0x0b32] = -1;
	}

	// Packet: 0x0b33
	if (packetver >= 20190619) {
		length_list[0x0b33] = 17;
	}

	// Packet: 0x0b34
	if (packetver >= 20190724) {
		length_list[0x0b34] = 50;
	} else if (packetver >= 20190703) {
		length_list[0x0b34] = 26;
	}

	// Packet: 0x0b35
	if (packetver >= 20190703) {
		length_list[0x0b35] = 3;
	}

	// Packet: 0x0b36
	if (packetver >= 20190717) {
		length_list[0x0b36] = -1;
	}

	// Packet: 0x0b37
	if (packetver >= 20190724) {
		length_list[0x0b37] = -1;
	}

	// Packet: 0x0b38
	if (packetver >= 20191002) {
		// removed
	} else if (packetver >= 20190724) {
		length_list[0x0b38] = -1;
	}

	// Packet: 0x0b39
	if (packetver >= 20190724) {
		length_list[0x0b39] = -1;
	}

	// Packet: 0x0b3a
	if (packetver >= 20191002) {
		// removed
	} else if (packetver >= 20190724) {
		length_list[0x0b3a] = 4;
	}

	// Packet: 0x0b3b
	if (packetver >= 20191002) {
		// removed
	} else if (packetver >= 20190724) {
		length_list[0x0b3b] = 4;
	}

	// Packet: 0x0b3c
	if (packetver >= 20190724) {
		length_list[0x0b3c] = 4;
	}

	// Packet: 0x0b3d
	if (packetver >= 20190724) {
		length_list[0x0b3d] = -1;
	}

	// Packet: 0x0b3e
	if (packetver >= 20190724) {
		length_list[0x0b3e] = -1;
	}

	// Packet: 0x0b3f
	if (packetver >= 20191224) {
		length_list[0x0b3f] = 64;
	} else if (packetver >= 20191016) {
		length_list[0x0b3f] = 63;
	} else if (packetver >= 20190724) {
		length_list[0x0b3f] = 64;
	}

	// Packet: 0x0b40
	if (packetver >= 20190724) {
		length_list[0x0b40] = -1;
	}

	// Packet: 0x0b41
	if (packetver >= 20191224) {
		length_list[0x0b41] = 70;
	} else if (packetver >= 20191016) {
		length_list[0x0b41] = 69;
	} else if (packetver >= 20190724) {
		length_list[0x0b41] = 41;
	}

	// Packet: 0x0b42
	if (packetver >= 20191224) {
		length_list[0x0b42] = 62;
	} else if (packetver >= 20191016) {
		length_list[0x0b42] = 61;
	} else if (packetver >= 20190724) {
		length_list[0x0b42] = 30;
	}

	// Packet: 0x0b43
	if (packetver >= 20191224) {
		length_list[0x0b43] = 48;
	} else if (packetver >= 20190724) {
		length_list[0x0b43] = 47;
	}

	// Packet: 0x0b44
	if (packetver >= 20191224) {
		length_list[0x0b44] = 58;
	} else if (packetver >= 20191016) {
		length_list[0x0b44] = 57;
	} else if (packetver >= 20190724) {
		length_list[0x0b44] = 32;
	}

	// Packet: 0x0b45
	if (packetver >= 20191224) {
		length_list[0x0b45] = 58;
	} else if (packetver >= 20191016) {
		length_list[0x0b45] = 57;
	} else if (packetver >= 20190724) {
		length_list[0x0b45] = 32;
	}

	// Packet: 0x0b46
	if (packetver >= 20190724) {
		length_list[0x0b46] = 10;
	}

	// Packet: 0x0b47
	if (packetver >= 20190724) {
		length_list[0x0b47] = 14;
	}

	// Packet: 0x0b48
	if (packetver >= 20190724) {
		length_list[0x0b48] = 18;
	}

	// Packet: 0x0b49
	if (packetver >= 20190724) {
		length_list[0x0b49] = 4;
	}

	// Packet: 0x0b4a
	if (packetver >= 20190724) {
		length_list[0x0b4a] = 6;
	}

	// Packet: 0x0b4b
	if (packetver >= 20190724) {
		length_list[0x0b4b] = 4;
	}

	// Packet: 0x0b4c
	if (packetver >= 20190724) {
		length_list[0x0b4c] = 2;
	}

	// Packet: 0x0b4d
	if (packetver >= 20190807) {
		length_list[0x0b4d] = -1;
	} else if (packetver >= 20190724) {
		length_list[0x0b4d] = 6;
	}

	// Packet: 0x0b4e
	if (packetver >= 20190807) {
		length_list[0x0b4e] = -1;
	}

	// Packet: 0x0b4f
	if (packetver >= 20190807) {
		length_list[0x0b4f] = 2;
	}

	// Packet: 0x0b50
	if (packetver >= 20190807) {
		length_list[0x0b50] = 2;
	}

	// Packet: 0x0b51
	if (packetver >= 20190807) {
		length_list[0x0b51] = 2;
	}

	// Packet: 0x0b52
	if (packetver >= 20190807) {
		length_list[0x0b52] = 2;
	}

	// Packet: 0x0b53
	if (packetver >= 20190821) {
		length_list[0x0b53] = 52;
	}

	// Packet: 0x0b54
	if (packetver >= 20190821) {
		length_list[0x0b54] = 8;
	}

	// Packet: 0x0b55
	if (packetver >= 20190821) {
		length_list[0x0b55] = -1;
	}

	// Packet: 0x0b56
	if (packetver >= 20190904) {
		length_list[0x0b56] = -1;
	}

	// Packet: 0x0b57
	if (packetver >= 20190904) {
		length_list[0x0b57] = -1;
	}

	// Packet: 0x0b58
	if (packetver >= 20190904) {
		length_list[0x0b58] = 2;
	}

	// Packet: 0x0b59
	if (packetver >= 20191016) {
		length_list[0x0b59] = 4;
	} else if (packetver >= 20190918) {
		length_list[0x0b59] = 2;
	}

	// Packet: 0x0b5a
	if (packetver >= 20191016) {
		length_list[0x0b5a] = -1;
	} else if (packetver >= 20190918) {
		length_list[0x0b5a] = 4;
	}

	// Packet: 0x0b5b
	if (packetver >= 20191016) {
		length_list[0x0b5b] = 14;
	} else if (packetver >= 20190918) {
		length_list[0x0b5b] = -1;
	}

	// Packet: 0x0b5c
	if (packetver >= 20191016) {
		length_list[0x0b5c] = 2;
	} else if (packetver >= 20191002) {
		length_list[0x0b5c] = 14;
	} else if (packetver >= 20190918) {
		length_list[0x0b5c] = 10;
	}

	// Packet: 0x0b5d
	if (packetver >= 20191016) {
		length_list[0x0b5d] = 10;
	} else if (packetver >= 20190918) {
		length_list[0x0b5d] = 2;
	}

	// Packet: 0x0b5e
	if (packetver >= 20191016) {
		length_list[0x0b5e] = 33;
	} else if (packetver >= 20191002) {
		length_list[0x0b5e] = 10;
	}

	// Packet: 0x0b5f
	if (packetver >= 20191030) {
		length_list[0x0b5f] = -1;
	} else if (packetver >= 20191016) {
		// removed
	} else if (packetver >= 20191002) {
		length_list[0x0b5f] = 33;
	}

	// Packet: 0x0b60
	if (packetver >= 20191030) {
		length_list[0x0b60] = -1;
	}

	// Packet: 0x0b61
	if (packetver >= 20191030) {
		length_list[0x0b61] = -1;
	}

	// Packet: 0x0b62
	if (packetver >= 20191030) {
		length_list[0x0b62] = -1;
	}

	// Packet: 0x0b63
	if (packetver >= 20191030) {
		length_list[0x0b63] = -1;
	}

	// Packet: 0x0b64
	if (packetver >= 20191030) {
		length_list[0x0b64] = -1;
	}

	// Packet: 0x0b65
	if (packetver >= 20191106) {
		length_list[0x0b65] = -1;
	}

	// Packet: 0x0b66
	if (packetver >= 20191224) {
		length_list[0x0b66] = 26;
	} else if (packetver >= 20191106) {
		length_list[0x0b66] = 25;
	}

	// Packet: 0x0b67
	if (packetver >= 20191224) {
		length_list[0x0b67] = 33;
	} else if (packetver >= 20191106) {
		length_list[0x0b67] = 32;
	}

	// Packet: 0x0b68
	if (packetver >= 20191120) {
		length_list[0x0b68] = 12;
	}

	// Packet: 0x0b69
	if (packetver >= 20191127) {
		length_list[0x0b69] = 18;
	}

	// Packet: 0x0b6a
	if (packetver >= 20191204) {
		length_list[0x0b6a] = -1;
	}

	// Packet: 0x0b6b
	if (packetver >= 20191218) {
		length_list[0x0b6b] = 14;
	}

	// Packet: 0x0b6c
	if (packetver >= 20191218) {
		length_list[0x0b6c] = 12;
	}

	// Packet: 0x0b6d
	if (packetver >= 20191224) {
		length_list[0x0b6d] = 6;
	}

	// Packet: 0x0b6e
	if (packetver >= 20191224) {
		length_list[0x0b6e] = 14;
	}
	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
