/**
 * Network/Packets/packets2012_len_main.js
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
	length_list[0x006d] = 146;

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
	if (packetver >= 20121227) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20121128) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0202] = 10;
	} else if (packetver >= 20121017) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0202] = -1;
	} else if (packetver >= 20120925) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120814) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0202] = 12;
	} else if (packetver >= 20120724) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120710) {
		length_list[0x0202] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120515) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0202] = 90;
	} else if (packetver >= 20120503) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120424) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120120) {
		length_list[0x0202] = 12;
	} else if (packetver >= 20120117) {
		length_list[0x0202] = 5;
	} else if (packetver >= 20120110) {
		length_list[0x0202] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0202] = 5;
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
	if (packetver >= 20121107) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120911) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x022d] = 8;
	} else if (packetver >= 20120830) {
		length_list[0x022d] = 12;
	} else if (packetver >= 20120724) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120626) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120508) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120424) {
		length_list[0x022d] = 12;
	} else if (packetver >= 20120417) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120410) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120228) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x022d] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x022d] = 19;
	} else if (packetver >= 20120131) {
		length_list[0x022d] = 6;
	} else if (packetver >= 20120110) {
		length_list[0x022d] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x022d] = 19;
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
	if (packetver >= 20121024) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120925) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120724) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120626) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120529) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x023b] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x023b] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120424) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120214) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x023b] = 26;
	} else if (packetver >= 20120120) {
		length_list[0x023b] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x023b] = 4;
	} else if (packetver >= 20120110) {
		length_list[0x023b] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x023b] = 26;
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
	if (packetver >= 20121107) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120925) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120822) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0281] = 5;
	} else if (packetver >= 20120808) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120724) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120626) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120604) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120508) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120424) {
		length_list[0x0281] = 26;
	} else if (packetver >= 20120417) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120404) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0281] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120214) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0281] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0281] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x0281] = 36;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121227) {
		length_list[0x02c4] = 19;
	} else if (packetver >= 20120716) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x02c4] = 10;
	} else if (packetver >= 20120515) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x02c4] = 26;
	} else if (packetver >= 20120410) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x02c4] = 26;
	} else if (packetver >= 20120314) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x02c4] = 6;
	} else if (packetver >= 20120131) {
		length_list[0x02c4] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x02c4] = 8;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121227) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20121212) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x035f] = -1;
	} else if (packetver >= 20121107) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x035f] = 5;
	} else if (packetver >= 20121024) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x035f] = 8;
	} else if (packetver >= 20120716) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120424) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120328) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x035f] = 6;
	} else if (packetver >= 20120117) {
		length_list[0x035f] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x035f] = 6;
	}

	// Packet: 0x0360
	if (packetver >= 20121227) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20121212) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0360] = 5;
	} else if (packetver >= 20121114) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20121024) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20120724) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120424) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120328) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x0360] = 90;
	} else if (packetver >= 20120221) {
		length_list[0x0360] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0360] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x0360] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0360] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0360] = 6;
	}

	// Packet: 0x0361
	if (packetver >= 20121227) {
		length_list[0x0361] = 36;
	} else if (packetver >= 20121212) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20121114) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0361] = 36;
	} else if (packetver >= 20120925) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120822) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20120724) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120626) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120604) {
		length_list[0x0361] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0361] = 10;
	} else if (packetver >= 20120508) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120424) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0361] = 5;
	} else if (packetver >= 20120131) {
		length_list[0x0361] = -1;
	} else if (packetver >= 20120120) {
		length_list[0x0361] = 19;
	} else if (packetver >= 20120117) {
		length_list[0x0361] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x0361] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x0361] = 5;
	}

	// Packet: 0x0362
	if (packetver >= 20121227) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0362] = 10;
	} else if (packetver >= 20121212) {
		length_list[0x0362] = -1;
	} else if (packetver >= 20121205) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0362] = 19;
	} else if (packetver >= 20121107) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120724) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0362] = 18;
	} else if (packetver >= 20120626) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120508) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120424) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0362] = 8;
	} else if (packetver >= 20120328) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0362] = 6;
	} else if (packetver >= 20120131) {
		length_list[0x0362] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0362] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0362] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x0362] = 6;
	}

	// Packet: 0x0363
	if (packetver >= 20121227) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0363] = 6;
	} else if (packetver >= 20121205) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0363] = 26;
	} else if (packetver >= 20121107) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120925) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120814) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120724) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120710) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0363] = 19;
	} else if (packetver >= 20120612) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120604) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120515) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0363] = -1;
	} else if (packetver >= 20120503) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120424) {
		length_list[0x0363] = 18;
	} else if (packetver >= 20120417) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120328) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120228) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0363] = 18;
	} else if (packetver >= 20120214) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0363] = 8;
	} else if (packetver >= 20120131) {
		length_list[0x0363] = 12;
	} else if (packetver >= 20120120) {
		length_list[0x0363] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0363] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0363] = 8;
	}

	// Packet: 0x0364
	if (packetver >= 20121121) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0364] = 36;
	} else if (packetver >= 20121107) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120925) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120814) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120801) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20120716) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120710) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120604) {
		length_list[0x0364] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120522) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20120508) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120424) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120410) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20120328) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120314) {
		length_list[0x0364] = 19;
	} else if (packetver >= 20120214) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0364] = 8;
	} else if (packetver >= 20120120) {
		length_list[0x0364] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0364] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0364] = 8;
	}

	// Packet: 0x0365
	if (packetver >= 20121121) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0365] = 5;
	} else if (packetver >= 20121107) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20121031) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20121024) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0365] = -1;
	} else if (packetver >= 20120925) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120911) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0365] = 8;
	} else if (packetver >= 20120830) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0365] = 26;
	} else if (packetver >= 20120814) {
		length_list[0x0365] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120724) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120710) {
		length_list[0x0365] = 26;
	} else if (packetver >= 20120702) {
		length_list[0x0365] = 8;
	} else if (packetver >= 20120626) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120604) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120508) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120424) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120328) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120214) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0365] = 18;
	} else if (packetver >= 20120131) {
		length_list[0x0365] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0365] = 6;
	} else if (packetver >= 20120117) {
		length_list[0x0365] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x0365] = 18;
	}

	// Packet: 0x0366
	if (packetver >= 20121227) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20121107) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20121024) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120925) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120905) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120814) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0366] = 19;
	} else if (packetver >= 20120724) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120702) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0366] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120604) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120522) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20120508) {
		length_list[0x0366] = 26;
	} else if (packetver >= 20120503) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120424) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120404) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20120328) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120228) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120214) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0366] = 90;
	} else if (packetver >= 20120120) {
		length_list[0x0366] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0366] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0366] = 90;
	}

	// Packet: 0x0367
	if (packetver >= 20120716) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0367] = 8;
	} else if (packetver >= 20120314) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0367] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0367] = 8;
	} else if (packetver >= 20120120) {
		length_list[0x0367] = 10;
	} else if (packetver >= 20120117) {
		length_list[0x0367] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x0367] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0367] = 2;
	}

	// Packet: 0x0368
	if (packetver >= 20121227) {
		length_list[0x0368] = -1;
	} else if (packetver >= 20121218) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20121212) {
		length_list[0x0368] = 26;
	} else if (packetver >= 20121107) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20121024) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120724) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120328) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120131) {
		length_list[0x0368] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0368] = 6;
	} else if (packetver >= 20120117) {
		length_list[0x0368] = 7;
	} else if (packetver >= 20120103) {
		length_list[0x0368] = 6;
	}

	// Packet: 0x0369
	if (packetver >= 20121227) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20121212) {
		length_list[0x0369] = 26;
	} else if (packetver >= 20121107) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20121024) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120925) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120905) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120814) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120724) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120626) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120604) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120522) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0369] = 26;
	} else if (packetver >= 20120503) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120424) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120328) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120314) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0369] = 26;
	} else if (packetver >= 20120228) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120214) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0369] = 7;
	} else if (packetver >= 20120117) {
		length_list[0x0369] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0369] = 7;
	}

	// Packet: 0x03dd
	length_list[0x03dd] = 18;

	// Packet: 0x03de
	length_list[0x03de] = 18;

	// Packet: 0x0436
	if (packetver >= 20121031) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0436] = 5;
	} else if (packetver >= 20120925) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120905) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0436] = 26;
	} else if (packetver >= 20120814) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120724) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120710) {
		length_list[0x0436] = 7;
	} else if (packetver >= 20120626) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120604) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120508) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120424) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120328) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120214) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0436] = 4;
	} else if (packetver >= 20120131) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0436] = 5;
	} else if (packetver >= 20120117) {
		length_list[0x0436] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0436] = 4;
	}

	// Packet: 0x0437
	if (packetver >= 20121227) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20121218) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20121128) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20121107) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20121024) {
		length_list[0x0437] = 10;
	} else if (packetver >= 20121017) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120925) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120905) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120822) {
		length_list[0x0437] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120724) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120626) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120604) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120515) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120424) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120328) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120228) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0437] = 5;
	} else if (packetver >= 20120120) {
		length_list[0x0437] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0437] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0437] = 5;
	}

	// Packet: 0x0438
	if (packetver >= 20121227) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20121107) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20121024) {
		length_list[0x0438] = 12;
	} else if (packetver >= 20121017) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120925) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120911) {
		length_list[0x0438] = 6;
	} else if (packetver >= 20120822) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0438] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x0438] = 90;
	} else if (packetver >= 20120801) {
		length_list[0x0438] = 18;
	} else if (packetver >= 20120724) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120626) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120604) {
		length_list[0x0438] = 19;
	} else if (packetver >= 20120529) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120515) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120424) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120328) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120228) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120214) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0438] = 10;
	} else if (packetver >= 20120117) {
		length_list[0x0438] = 2;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121107) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20121010) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x07e4] = 4;
	} else if (packetver >= 20120919) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120724) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x07e4] = 10;
	} else if (packetver >= 20120529) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120508) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120424) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x07e4] = 90;
	} else if (packetver >= 20120328) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x07e4] = 6;
	} else if (packetver >= 20120117) {
		length_list[0x07e4] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x07e4] = 19;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121205) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x07ec] = 26;
	} else if (packetver >= 20121107) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120925) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120822) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x07ec] = 12;
	} else if (packetver >= 20120808) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120724) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120626) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120604) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120508) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120424) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120328) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120307) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x07ec] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x07ec] = 8;
	} else if (packetver >= 20120131) {
		length_list[0x07ec] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x07ec] = 26;
	} else if (packetver >= 20120117) {
		length_list[0x07ec] = 36;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121205) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0802] = 36;
	} else if (packetver >= 20121107) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120925) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120905) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0802] = 36;
	} else if (packetver >= 20120814) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120724) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120710) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0802] = -1;
	} else if (packetver >= 20120626) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120604) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120508) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120424) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120328) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120214) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0802] = 26;
	} else if (packetver >= 20120131) {
		length_list[0x0802] = 90;
	} else if (packetver >= 20120120) {
		length_list[0x0802] = 2;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121227) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20121107) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20121024) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120925) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120905) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120814) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120724) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120710) {
		length_list[0x0811] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120604) {
		length_list[0x0811] = 12;
	} else if (packetver >= 20120522) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120424) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120328) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120228) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120214) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0811] = -1;
	} else if (packetver >= 20120131) {
		length_list[0x0811] = 8;
	} else if (packetver >= 20120120) {
		length_list[0x0811] = 6;
	} else if (packetver >= 20120117) {
		length_list[0x0811] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0811] = -1;
	}

	// Packet: 0x0812
	length_list[0x0812] = 8;

	// Packet: 0x0813
	length_list[0x0813] = -1;

	// Packet: 0x0814
	length_list[0x0814] = 86;

	// Packet: 0x0815
	if (packetver >= 20121227) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20121212) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0815] = 6;
	} else if (packetver >= 20121121) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0815] = 90;
	} else if (packetver >= 20121107) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20121024) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0815] = 8;
	} else if (packetver >= 20121010) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0815] = 12;
	} else if (packetver >= 20120919) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120905) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120822) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0815] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120801) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120710) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0815] = 26;
	} else if (packetver >= 20120626) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120604) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120424) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120328) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120228) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120214) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0815] = 36;
	} else if (packetver >= 20120131) {
		length_list[0x0815] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0815] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x0815] = -1;
	}

	// Packet: 0x0816
	length_list[0x0816] = 6;

	// Packet: 0x0817
	if (packetver >= 20121218) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0817] = 10;
	} else if (packetver >= 20121031) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0817] = 7;
	} else if (packetver >= 20121017) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0817] = 6;
	} else if (packetver >= 20120911) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0817] = 36;
	} else if (packetver >= 20120522) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0817] = -1;
	} else if (packetver >= 20120508) {
		length_list[0x0817] = 4;
	} else if (packetver >= 20120503) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0817] = 8;
	} else if (packetver >= 20120214) {
		length_list[0x0817] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0817] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0817] = 2;
	}

	// Packet: 0x0818
	length_list[0x0818] = -1;

	// Packet: 0x0819
	if (packetver >= 20121227) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20121107) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20121024) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20121010) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0819] = 19;
	} else if (packetver >= 20120919) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120911) {
		length_list[0x0819] = 8;
	} else if (packetver >= 20120905) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120814) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120724) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120702) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0819] = 6;
	} else if (packetver >= 20120618) {
		length_list[0x0819] = 36;
	} else if (packetver >= 20120612) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120604) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120424) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120328) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120228) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120214) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0819] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0819] = 2;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121010) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0835] = 90;
	} else if (packetver >= 20120905) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0835] = 8;
	} else if (packetver >= 20120801) {
		length_list[0x0835] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0835] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x0835] = 2;
	}

	// Packet: 0x0836
	length_list[0x0836] = -1;

	// Packet: 0x0837
	length_list[0x0837] = 3;

	// Packet: 0x0838
	if (packetver >= 20121227) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20121121) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0838] = 26;
	} else if (packetver >= 20121107) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20121024) {
		length_list[0x0838] = 18;
	} else if (packetver >= 20121017) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20121010) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0838] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120905) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0838] = 5;
	} else if (packetver >= 20120814) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0838] = 26;
	} else if (packetver >= 20120724) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120702) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0838] = -1;
	} else if (packetver >= 20120612) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120604) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120515) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120424) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120328) {
		length_list[0x0838] = 8;
	} else if (packetver >= 20120307) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120228) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120214) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x0838] = 12;
	} else if (packetver >= 20120131) {
		length_list[0x0838] = 5;
	} else if (packetver >= 20120120) {
		length_list[0x0838] = 26;
	} else if (packetver >= 20120117) {
		length_list[0x0838] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x0838] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0838] = 12;
	}

	// Packet: 0x0839
	length_list[0x0839] = 66;

	// Packet: 0x083a
	length_list[0x083a] = 5;

	// Packet: 0x083b
	length_list[0x083b] = 2;

	// Packet: 0x083c
	if (packetver >= 20121227) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x083c] = 6;
	} else if (packetver >= 20121107) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20121024) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120925) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120905) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120814) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120724) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120626) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120604) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120515) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120424) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120328) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120228) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120214) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x083c] = 10;
	} else if (packetver >= 20120117) {
		length_list[0x083c] = 2;
	} else if (packetver >= 20120103) {
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
	if (packetver >= 20121107) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x085a] = 26;
	} else if (packetver >= 20121024) {
		length_list[0x085a] = 8;
	} else if (packetver >= 20120830) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x085a] = 5;
	} else if (packetver >= 20120710) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x085a] = 7;
	} else if (packetver >= 20120522) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x085a] = 8;
	} else if (packetver >= 20120307) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x085a] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x085a] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x085a] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x085a] = 2;
	}

	// Packet: 0x085b
	if (packetver >= 20120716) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x085b] = 10;
	} else if (packetver >= 20120410) {
		length_list[0x085b] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x085b] = 4;
	} else if (packetver >= 20120103) {
		length_list[0x085b] = 2;
	}

	// Packet: 0x085c
	if (packetver >= 20121218) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x085c] = 7;
	} else if (packetver >= 20121114) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x085c] = -1;
	} else if (packetver >= 20120830) {
		length_list[0x085c] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x085c] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x085c] = 2;
	}

	// Packet: 0x085d
	if (packetver >= 20121010) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x085d] = 5;
	} else if (packetver >= 20120911) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x085d] = 10;
	} else if (packetver >= 20120830) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x085d] = 10;
	} else if (packetver >= 20120808) {
		length_list[0x085d] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x085d] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x085d] = 2;
	}

	// Packet: 0x085e
	if (packetver >= 20120822) {
		length_list[0x085e] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x085e] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x085e] = 2;
	}

	// Packet: 0x085f
	if (packetver >= 20120814) {
		length_list[0x085f] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x085f] = 36;
	} else if (packetver >= 20120702) {
		length_list[0x085f] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x085f] = 5;
	} else if (packetver >= 20120612) {
		length_list[0x085f] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x085f] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x085f] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x085f] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x085f] = 2;
	}

	// Packet: 0x0860
	if (packetver >= 20121227) {
		length_list[0x0860] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x0860] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0860] = 36;
	} else if (packetver >= 20120103) {
		length_list[0x0860] = 2;
	}

	// Packet: 0x0861
	if (packetver >= 20121205) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0861] = 18;
	} else if (packetver >= 20121114) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0861] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0861] = 19;
	} else if (packetver >= 20120822) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0861] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0861] = 8;
	} else if (packetver >= 20120612) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0861] = 18;
	} else if (packetver >= 20120529) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0861] = 18;
	} else if (packetver >= 20120404) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0861] = -1;
	} else if (packetver >= 20120314) {
		length_list[0x0861] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0861] = 36;
	} else if (packetver >= 20120103) {
		length_list[0x0861] = 2;
	}

	// Packet: 0x0862
	if (packetver >= 20121121) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0862] = -1;
	} else if (packetver >= 20120822) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0862] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0862] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0862] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x0862] = 2;
	}

	// Packet: 0x0863
	if (packetver >= 20121212) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0863] = 6;
	} else if (packetver >= 20121128) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0863] = -1;
	} else if (packetver >= 20120710) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0863] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0863] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0863] = 26;
	} else if (packetver >= 20120314) {
		length_list[0x0863] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0863] = 5;
	} else if (packetver >= 20120228) {
		length_list[0x0863] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x0863] = 2;
	}

	// Packet: 0x0864
	if (packetver >= 20121212) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0864] = 18;
	} else if (packetver >= 20120410) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0864] = 18;
	} else if (packetver >= 20120320) {
		length_list[0x0864] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0864] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0864] = 2;
	}

	// Packet: 0x0865
	if (packetver >= 20121114) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0865] = 19;
	} else if (packetver >= 20120822) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0865] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0865] = 5;
	} else if (packetver >= 20120314) {
		length_list[0x0865] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0865] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0865] = 2;
	}

	// Packet: 0x0866
	if (packetver >= 20120801) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0866] = 4;
	} else if (packetver >= 20120702) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0866] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0866] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x0866] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0866] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0866] = 2;
	}

	// Packet: 0x0867
	if (packetver >= 20121227) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0867] = 5;
	} else if (packetver >= 20121205) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0867] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0867] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x0867] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0867] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0867] = 2;
	}

	// Packet: 0x0868
	if (packetver >= 20121031) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0868] = -1;
	} else if (packetver >= 20121017) {
		length_list[0x0868] = 8;
	} else if (packetver >= 20120919) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0868] = 6;
	} else if (packetver >= 20120822) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0868] = 7;
	} else if (packetver >= 20120702) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0868] = 5;
	} else if (packetver >= 20120515) {
		length_list[0x0868] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0868] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0868] = 2;
	}

	// Packet: 0x0869
	if (packetver >= 20121017) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0869] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x0869] = 8;
	} else if (packetver >= 20120702) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0869] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0869] = 10;
	} else if (packetver >= 20120529) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0869] = 19;
	} else if (packetver >= 20120515) {
		length_list[0x0869] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x0869] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0869] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0869] = 2;
	}

	// Packet: 0x086a
	if (packetver >= 20121128) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x086a] = 90;
	} else if (packetver >= 20120808) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x086a] = -1;
	} else if (packetver >= 20120604) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x086a] = 36;
	} else if (packetver >= 20120522) {
		length_list[0x086a] = -1;
	} else if (packetver >= 20120410) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x086a] = 5;
	} else if (packetver >= 20120314) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x086a] = 19;
	} else if (packetver >= 20120221) {
		length_list[0x086a] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x086a] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x086a] = 2;
	}

	// Packet: 0x086b
	if (packetver >= 20121024) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x086b] = 6;
	} else if (packetver >= 20120830) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x086b] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x086b] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x086b] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x086b] = 2;
	}

	// Packet: 0x086c
	if (packetver >= 20121212) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x086c] = -1;
	} else if (packetver >= 20121128) {
		length_list[0x086c] = 10;
	} else if (packetver >= 20121121) {
		length_list[0x086c] = -1;
	} else if (packetver >= 20121114) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x086c] = 5;
	} else if (packetver >= 20121017) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x086c] = 8;
	} else if (packetver >= 20120417) {
		length_list[0x086c] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x086c] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x086c] = 2;
	}

	// Packet: 0x086d
	if (packetver >= 20121128) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x086d] = 6;
	} else if (packetver >= 20121114) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x086d] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x086d] = 18;
	} else if (packetver >= 20120612) {
		length_list[0x086d] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x086d] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x086d] = 2;
	}

	// Packet: 0x086e
	if (packetver >= 20121205) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x086e] = 4;
	} else if (packetver >= 20120830) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x086e] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x086e] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x086e] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x086e] = 2;
	}

	// Packet: 0x086f
	if (packetver >= 20121128) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x086f] = 8;
	} else if (packetver >= 20120919) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x086f] = 5;
	} else if (packetver >= 20120905) {
		length_list[0x086f] = 6;
	} else if (packetver >= 20120822) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x086f] = 36;
	} else if (packetver >= 20120702) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x086f] = 26;
	} else if (packetver >= 20120612) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x086f] = 5;
	} else if (packetver >= 20120328) {
		length_list[0x086f] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x086f] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x086f] = 2;
	}

	// Packet: 0x0870
	if (packetver >= 20121227) {
		length_list[0x0870] = 12;
	} else if (packetver >= 20121212) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0870] = -1;
	} else if (packetver >= 20120314) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0870] = -1;
	} else if (packetver >= 20120228) {
		length_list[0x0870] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0870] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0870] = 2;
	}

	// Packet: 0x0871
	if (packetver >= 20121107) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0871] = 5;
	} else if (packetver >= 20120702) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0871] = 6;
	} else if (packetver >= 20120612) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0871] = 36;
	} else if (packetver >= 20120503) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0871] = 10;
	} else if (packetver >= 20120417) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0871] = 5;
	} else if (packetver >= 20120307) {
		length_list[0x0871] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0871] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x0871] = 2;
	}

	// Packet: 0x0872
	if (packetver >= 20121227) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20121218) {
		length_list[0x0872] = 4;
	} else if (packetver >= 20121205) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0872] = 10;
	} else if (packetver >= 20121121) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20121031) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0872] = 26;
	} else if (packetver >= 20120822) {
		length_list[0x0872] = -1;
	} else if (packetver >= 20120801) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0872] = -1;
	} else if (packetver >= 20120702) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0872] = 6;
	} else if (packetver >= 20120221) {
		length_list[0x0872] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0872] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x0872] = 2;
	}

	// Packet: 0x0873
	if (packetver >= 20121114) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0873] = 8;
	} else if (packetver >= 20121010) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0873] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0873] = 10;
	} else if (packetver >= 20120417) {
		length_list[0x0873] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0873] = 4;
	} else if (packetver >= 20120103) {
		length_list[0x0873] = 2;
	}

	// Packet: 0x0874
	if (packetver >= 20121017) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0874] = 8;
	} else if (packetver >= 20120702) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0874] = 5;
	} else if (packetver >= 20120503) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0874] = 90;
	} else if (packetver >= 20120404) {
		length_list[0x0874] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0874] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x0874] = 2;
	}

	// Packet: 0x0875
	if (packetver >= 20121212) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0875] = -1;
	} else if (packetver >= 20121128) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0875] = 8;
	} else if (packetver >= 20121031) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0875] = 4;
	} else if (packetver >= 20120822) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0875] = 5;
	} else if (packetver >= 20120808) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0875] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0875] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0875] = 4;
	} else if (packetver >= 20120307) {
		length_list[0x0875] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0875] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0875] = 2;
	}

	// Packet: 0x0876
	if (packetver >= 20121121) {
		length_list[0x0876] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0876] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x0876] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0876] = 5;
	} else if (packetver >= 20120404) {
		length_list[0x0876] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0876] = 36;
	} else if (packetver >= 20120103) {
		length_list[0x0876] = 2;
	}

	// Packet: 0x0877
	if (packetver >= 20121218) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0877] = 6;
	} else if (packetver >= 20121107) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0877] = 19;
	} else if (packetver >= 20120919) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0877] = 26;
	} else if (packetver >= 20120905) {
		length_list[0x0877] = -1;
	} else if (packetver >= 20120801) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0877] = -1;
	} else if (packetver >= 20120716) {
		length_list[0x0877] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0877] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0877] = 2;
	}

	// Packet: 0x0878
	if (packetver >= 20121212) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0878] = 90;
	} else if (packetver >= 20121114) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0878] = 26;
	} else if (packetver >= 20120911) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0878] = 5;
	} else if (packetver >= 20120716) {
		length_list[0x0878] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0878] = -1;
	} else if (packetver >= 20120702) {
		length_list[0x0878] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x0878] = 2;
	}

	// Packet: 0x0879
	if (packetver >= 20121121) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0879] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0879] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0879] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0879] = 18;
	} else if (packetver >= 20120103) {
		length_list[0x0879] = 2;
	}

	// Packet: 0x087a
	if (packetver >= 20121227) {
		length_list[0x087a] = 7;
	} else if (packetver >= 20121218) {
		length_list[0x087a] = 5;
	} else if (packetver >= 20121031) {
		length_list[0x087a] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x087a] = 8;
	} else if (packetver >= 20121010) {
		length_list[0x087a] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x087a] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x087a] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x087a] = 8;
	} else if (packetver >= 20120905) {
		length_list[0x087a] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x087a] = 2;
	}

	// Packet: 0x087b
	if (packetver >= 20121227) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x087b] = 26;
	} else if (packetver >= 20121031) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x087b] = -1;
	} else if (packetver >= 20120911) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x087b] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x087b] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x087b] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x087b] = 2;
	}

	// Packet: 0x087c
	if (packetver >= 20120830) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x087c] = 90;
	} else if (packetver >= 20120801) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x087c] = 18;
	} else if (packetver >= 20120604) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x087c] = 26;
	} else if (packetver >= 20120522) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x087c] = 5;
	} else if (packetver >= 20120328) {
		length_list[0x087c] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x087c] = 36;
	} else if (packetver >= 20120103) {
		length_list[0x087c] = 2;
	}

	// Packet: 0x087d
	if (packetver >= 20121218) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x087d] = 10;
	} else if (packetver >= 20121017) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x087d] = 90;
	} else if (packetver >= 20120830) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x087d] = 6;
	} else if (packetver >= 20120612) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x087d] = 6;
	} else if (packetver >= 20120522) {
		length_list[0x087d] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x087d] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x087d] = 2;
	}

	// Packet: 0x087e
	if (packetver >= 20121031) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x087e] = 10;
	} else if (packetver >= 20121010) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x087e] = 26;
	} else if (packetver >= 20120911) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x087e] = 8;
	} else if (packetver >= 20120529) {
		length_list[0x087e] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x087e] = 5;
	} else if (packetver >= 20120515) {
		length_list[0x087e] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x087e] = 2;
	}

	// Packet: 0x087f
	if (packetver >= 20121218) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x087f] = 12;
	} else if (packetver >= 20121114) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x087f] = 5;
	} else if (packetver >= 20120822) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x087f] = 26;
	} else if (packetver >= 20120716) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x087f] = 5;
	} else if (packetver >= 20120529) {
		length_list[0x087f] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x087f] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x087f] = 2;
	}

	// Packet: 0x0880
	if (packetver >= 20121128) {
		length_list[0x0880] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0880] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0880] = 2;
	}

	// Packet: 0x0881
	if (packetver >= 20120702) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0881] = 6;
	} else if (packetver >= 20120221) {
		length_list[0x0881] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0881] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x0881] = 2;
	}

	// Packet: 0x0882
	if (packetver >= 20121031) {
		length_list[0x0882] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0882] = 36;
	} else if (packetver >= 20120822) {
		length_list[0x0882] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0882] = 18;
	} else if (packetver >= 20120515) {
		length_list[0x0882] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0882] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x0882] = 2;
	}

	// Packet: 0x0883
	if (packetver >= 20121017) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0883] = 5;
	} else if (packetver >= 20120710) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0883] = -1;
	} else if (packetver >= 20120404) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0883] = 90;
	} else if (packetver >= 20120307) {
		length_list[0x0883] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0883] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0883] = 2;
	}

	// Packet: 0x0884
	if (packetver >= 20120515) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0884] = 36;
	} else if (packetver >= 20120417) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0884] = 6;
	} else if (packetver >= 20120314) {
		length_list[0x0884] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0884] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0884] = 2;
	}

	// Packet: 0x0885
	if (packetver >= 20120830) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0885] = 4;
	} else if (packetver >= 20120612) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0885] = 26;
	} else if (packetver >= 20120515) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0885] = 6;
	} else if (packetver >= 20120417) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0885] = 5;
	} else if (packetver >= 20120320) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0885] = 5;
	} else if (packetver >= 20120307) {
		length_list[0x0885] = 7;
	} else if (packetver >= 20120228) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0885] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x0885] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0885] = 26;
	} else if (packetver >= 20120120) {
		length_list[0x0885] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x0885] = 18;
	} else if (packetver >= 20120103) {
		length_list[0x0885] = 2;
	}

	// Packet: 0x0886
	if (packetver >= 20121218) {
		length_list[0x0886] = 8;
	} else if (packetver >= 20120710) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0886] = 36;
	} else if (packetver >= 20120417) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0886] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0886] = 8;
	} else if (packetver >= 20120120) {
		length_list[0x0886] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0886] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0886] = 2;
	}

	// Packet: 0x0887
	if (packetver >= 20121024) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0887] = -1;
	} else if (packetver >= 20121010) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0887] = -1;
	} else if (packetver >= 20120911) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0887] = 19;
	} else if (packetver >= 20120830) {
		length_list[0x0887] = 10;
	} else if (packetver >= 20120801) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0887] = 5;
	} else if (packetver >= 20120314) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0887] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0887] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0887] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x0887] = 4;
	} else if (packetver >= 20120117) {
		length_list[0x0887] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0887] = 2;
	}

	// Packet: 0x0888
	if (packetver >= 20121227) {
		length_list[0x0888] = 6;
	} else if (packetver >= 20121212) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0888] = 36;
	} else if (packetver >= 20121128) {
		length_list[0x0888] = 5;
	} else if (packetver >= 20121031) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0888] = 5;
	} else if (packetver >= 20121017) {
		length_list[0x0888] = 18;
	} else if (packetver >= 20121010) {
		length_list[0x0888] = 4;
	} else if (packetver >= 20120905) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0888] = 8;
	} else if (packetver >= 20120801) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0888] = 5;
	} else if (packetver >= 20120228) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0888] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0888] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0888] = 18;
	} else if (packetver >= 20120103) {
		length_list[0x0888] = 2;
	}

	// Packet: 0x0889
	if (packetver >= 20121227) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0889] = -1;
	} else if (packetver >= 20121212) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0889] = 6;
	} else if (packetver >= 20121031) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0889] = 6;
	} else if (packetver >= 20120710) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0889] = 90;
	} else if (packetver >= 20120417) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0889] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0889] = 7;
	} else if (packetver >= 20120307) {
		length_list[0x0889] = 10;
	} else if (packetver >= 20120131) {
		length_list[0x0889] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0889] = 36;
	} else if (packetver >= 20120117) {
		length_list[0x0889] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0889] = 2;
	}

	// Packet: 0x088a
	if (packetver >= 20121031) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x088a] = 6;
	} else if (packetver >= 20121010) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x088a] = 26;
	} else if (packetver >= 20120919) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x088a] = 6;
	} else if (packetver >= 20120830) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x088a] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x088a] = 8;
	} else if (packetver >= 20120307) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x088a] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x088a] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x088a] = 19;
	} else if (packetver >= 20120103) {
		length_list[0x088a] = 2;
	}

	// Packet: 0x088b
	if (packetver >= 20121212) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x088b] = 4;
	} else if (packetver >= 20121128) {
		length_list[0x088b] = -1;
	} else if (packetver >= 20120822) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x088b] = 26;
	} else if (packetver >= 20120808) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x088b] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x088b] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x088b] = 18;
	} else if (packetver >= 20120103) {
		length_list[0x088b] = 2;
	}

	// Packet: 0x088c
	if (packetver >= 20120320) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x088c] = 4;
	} else if (packetver >= 20120221) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x088c] = 26;
	} else if (packetver >= 20120120) {
		length_list[0x088c] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x088c] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x088c] = 2;
	}

	// Packet: 0x088d
	if (packetver >= 20121010) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x088d] = 10;
	} else if (packetver >= 20120822) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x088d] = -1;
	} else if (packetver >= 20120716) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x088d] = 5;
	} else if (packetver >= 20120529) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x088d] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x088d] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x088d] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x088d] = -1;
	} else if (packetver >= 20120120) {
		length_list[0x088d] = 18;
	} else if (packetver >= 20120103) {
		length_list[0x088d] = 2;
	}

	// Packet: 0x088e
	if (packetver >= 20121227) {
		length_list[0x088e] = 5;
	} else if (packetver >= 20120801) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x088e] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x088e] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x088e] = 5;
	} else if (packetver >= 20120404) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x088e] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x088e] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x088e] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x088e] = 2;
	}

	// Packet: 0x088f
	if (packetver >= 20121114) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x088f] = -1;
	} else if (packetver >= 20121017) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x088f] = 26;
	} else if (packetver >= 20120925) {
		length_list[0x088f] = -1;
	} else if (packetver >= 20120919) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x088f] = 26;
	} else if (packetver >= 20120814) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x088f] = 5;
	} else if (packetver >= 20120508) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x088f] = 36;
	} else if (packetver >= 20120131) {
		length_list[0x088f] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x088f] = 5;
	} else if (packetver >= 20120117) {
		length_list[0x088f] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x088f] = 2;
	}

	// Packet: 0x0890
	if (packetver >= 20121227) {
		length_list[0x0890] = 10;
	} else if (packetver >= 20121218) {
		length_list[0x0890] = 8;
	} else if (packetver >= 20121114) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0890] = 10;
	} else if (packetver >= 20120905) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0890] = 4;
	} else if (packetver >= 20120808) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0890] = 7;
	} else if (packetver >= 20120529) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0890] = -1;
	} else if (packetver >= 20120314) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0890] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0890] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0890] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0890] = 8;
	} else if (packetver >= 20120120) {
		length_list[0x0890] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0890] = 2;
	}

	// Packet: 0x0891
	if (packetver >= 20121114) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0891] = 8;
	} else if (packetver >= 20121017) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0891] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0891] = 26;
	} else if (packetver >= 20120529) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0891] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x0891] = -1;
	} else if (packetver >= 20120417) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0891] = 6;
	} else if (packetver >= 20120307) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0891] = 36;
	} else if (packetver >= 20120131) {
		length_list[0x0891] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0891] = 8;
	} else if (packetver >= 20120117) {
		length_list[0x0891] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x0891] = 2;
	}

	// Packet: 0x0892
	if (packetver >= 20121128) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0892] = -1;
	} else if (packetver >= 20120702) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0892] = 36;
	} else if (packetver >= 20120120) {
		length_list[0x0892] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x0892] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0892] = 2;
	}

	// Packet: 0x0893
	if (packetver >= 20121218) {
		length_list[0x0893] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0893] = 6;
	} else if (packetver >= 20120716) {
		length_list[0x0893] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0893] = 36;
	} else if (packetver >= 20120103) {
		length_list[0x0893] = 2;
	}

	// Packet: 0x0894
	if (packetver >= 20121227) {
		length_list[0x0894] = 26;
	} else if (packetver >= 20121205) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0894] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0894] = -1;
	} else if (packetver >= 20120131) {
		length_list[0x0894] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0894] = 10;
	} else if (packetver >= 20120103) {
		length_list[0x0894] = 2;
	}

	// Packet: 0x0895
	if (packetver >= 20121218) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0895] = 6;
	} else if (packetver >= 20121205) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0895] = 6;
	} else if (packetver >= 20120911) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0895] = 5;
	} else if (packetver >= 20120822) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0895] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0895] = 19;
	} else if (packetver >= 20120131) {
		length_list[0x0895] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x0895] = 7;
	} else if (packetver >= 20120103) {
		length_list[0x0895] = 2;
	}

	// Packet: 0x0896
	if (packetver >= 20121227) {
		length_list[0x0896] = 6;
	} else if (packetver >= 20121114) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0896] = 6;
	} else if (packetver >= 20121017) {
		length_list[0x0896] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0896] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x0896] = 2;
	}

	// Packet: 0x0897
	if (packetver >= 20121024) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0897] = 8;
	} else if (packetver >= 20120911) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0897] = 6;
	} else if (packetver >= 20120716) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0897] = -1;
	} else if (packetver >= 20120702) {
		length_list[0x0897] = 26;
	} else if (packetver >= 20120612) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0897] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0897] = 26;
	} else if (packetver >= 20120117) {
		length_list[0x0897] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x0897] = 5;
	} else if (packetver >= 20120103) {
		length_list[0x0897] = 2;
	}

	// Packet: 0x0898
	if (packetver >= 20121121) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0898] = 7;
	} else if (packetver >= 20121107) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x0898] = 36;
	} else if (packetver >= 20121017) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0898] = 36;
	} else if (packetver >= 20120207) {
		length_list[0x0898] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0898] = 4;
	} else if (packetver >= 20120103) {
		length_list[0x0898] = 2;
	}

	// Packet: 0x0899
	if (packetver >= 20121128) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0899] = 26;
	} else if (packetver >= 20121114) {
		length_list[0x0899] = 5;
	} else if (packetver >= 20120830) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0899] = 26;
	} else if (packetver >= 20120716) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0899] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0899] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0899] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x0899] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x0899] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x0899] = 2;
	}

	// Packet: 0x089a
	if (packetver >= 20121212) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x089a] = 26;
	} else if (packetver >= 20121128) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x089a] = 18;
	} else if (packetver >= 20121114) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x089a] = 7;
	} else if (packetver >= 20121024) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x089a] = 26;
	} else if (packetver >= 20120522) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x089a] = 36;
	} else if (packetver >= 20120508) {
		length_list[0x089a] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x089a] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x089a] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x089a] = 2;
	}

	// Packet: 0x089b
	if (packetver >= 20121114) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x089b] = -1;
	} else if (packetver >= 20120503) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x089b] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x089b] = -1;
	} else if (packetver >= 20120117) {
		length_list[0x089b] = 2;
	} else if (packetver >= 20120110) {
		length_list[0x089b] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x089b] = 2;
	}

	// Packet: 0x089c
	if (packetver >= 20121031) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x089c] = 26;
	} else if (packetver >= 20120808) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x089c] = 90;
	} else if (packetver >= 20120417) {
		length_list[0x089c] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x089c] = 26;
	} else if (packetver >= 20120404) {
		length_list[0x089c] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x089c] = 2;
	}

	// Packet: 0x089d
	if (packetver >= 20121212) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x089d] = 12;
	} else if (packetver >= 20121010) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x089d] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x089d] = 6;
	} else if (packetver >= 20120307) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x089d] = -1;
	} else if (packetver >= 20120120) {
		length_list[0x089d] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x089d] = 90;
	} else if (packetver >= 20120103) {
		length_list[0x089d] = 2;
	}

	// Packet: 0x089e
	if (packetver >= 20120710) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x089e] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x089e] = 26;
	} else if (packetver >= 20120120) {
		length_list[0x089e] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x089e] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x089e] = 2;
	}

	// Packet: 0x089f
	if (packetver >= 20120830) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x089f] = 19;
	} else if (packetver >= 20120808) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x089f] = 5;
	} else if (packetver >= 20120724) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x089f] = 36;
	} else if (packetver >= 20120710) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x089f] = 6;
	} else if (packetver >= 20120221) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x089f] = 36;
	} else if (packetver >= 20120207) {
		length_list[0x089f] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x089f] = 36;
	} else if (packetver >= 20120120) {
		length_list[0x089f] = -1;
	} else if (packetver >= 20120103) {
		length_list[0x089f] = 2;
	}

	// Packet: 0x08a0
	if (packetver >= 20120911) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x08a0] = 4;
	} else if (packetver >= 20120710) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x08a0] = 8;
	} else if (packetver >= 20120626) {
		length_list[0x08a0] = 26;
	} else if (packetver >= 20120503) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x08a0] = 5;
	} else if (packetver >= 20120404) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x08a0] = 18;
	} else if (packetver >= 20120320) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x08a0] = -1;
	} else if (packetver >= 20120131) {
		length_list[0x08a0] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x08a0] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x08a0] = 2;
	}

	// Packet: 0x08a1
	if (packetver >= 20121017) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x08a1] = 7;
	} else if (packetver >= 20120801) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x08a1] = 6;
	} else if (packetver >= 20120131) {
		length_list[0x08a1] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x08a1] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x08a1] = 2;
	}

	// Packet: 0x08a2
	if (packetver >= 20120808) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x08a2] = 26;
	} else if (packetver >= 20120612) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x08a2] = 7;
	} else if (packetver >= 20120522) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x08a2] = 90;
	} else if (packetver >= 20120417) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x08a2] = 8;
	} else if (packetver >= 20120307) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x08a2] = 4;
	} else if (packetver >= 20120207) {
		length_list[0x08a2] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x08a2] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x08a2] = 2;
	}

	// Packet: 0x08a3
	if (packetver >= 20121218) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x08a3] = 19;
	} else if (packetver >= 20120716) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x08a3] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x08a3] = 10;
	} else if (packetver >= 20120503) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x08a3] = -1;
	} else if (packetver >= 20120410) {
		length_list[0x08a3] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x08a3] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x08a3] = 2;
	}

	// Packet: 0x08a4
	if (packetver >= 20121010) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x08a4] = 7;
	} else if (packetver >= 20120919) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x08a4] = 18;
	} else if (packetver >= 20120702) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x08a4] = 19;
	} else if (packetver >= 20120503) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x08a4] = 6;
	} else if (packetver >= 20120131) {
		length_list[0x08a4] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x08a4] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x08a4] = 2;
	}

	// Packet: 0x08a5
	if (packetver >= 20120919) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x08a5] = -1;
	} else if (packetver >= 20120830) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x08a5] = 8;
	} else if (packetver >= 20120522) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x08a5] = 6;
	} else if (packetver >= 20120221) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x08a5] = 4;
	} else if (packetver >= 20120131) {
		length_list[0x08a5] = 2;
	} else if (packetver >= 20120120) {
		length_list[0x08a5] = 90;
	} else if (packetver >= 20120103) {
		length_list[0x08a5] = 2;
	}

	// Packet: 0x08a6
	if (packetver >= 20121227) {
		length_list[0x08a6] = 6;
	} else if (packetver >= 20121024) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x08a6] = 5;
	} else if (packetver >= 20121010) {
		length_list[0x08a6] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x08a6] = 19;
	} else if (packetver >= 20120905) {
		length_list[0x08a6] = 26;
	} else if (packetver >= 20120716) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x08a6] = -1;
	} else if (packetver >= 20120417) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x08a6] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x08a6] = 19;
	} else if (packetver >= 20120120) {
		length_list[0x08a6] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x08a6] = 19;
	} else if (packetver >= 20120103) {
		length_list[0x08a6] = 2;
	}

	// Packet: 0x08a7
	if (packetver >= 20121218) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x08a7] = 5;
	} else if (packetver >= 20121128) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x08a7] = -1;
	} else if (packetver >= 20120919) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x08a7] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x08a7] = 18;
	} else if (packetver >= 20120221) {
		length_list[0x08a7] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x08a7] = 6;
	} else if (packetver >= 20120103) {
		length_list[0x08a7] = 2;
	}

	// Packet: 0x08a8
	if (packetver >= 20121205) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x08a8] = -1;
	} else if (packetver >= 20120822) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x08a8] = 4;
	} else if (packetver >= 20120716) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x08a8] = 19;
	} else if (packetver >= 20120522) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x08a8] = 19;
	} else if (packetver >= 20120424) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x08a8] = 36;
	} else if (packetver >= 20120320) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x08a8] = 36;
	} else if (packetver >= 20120307) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x08a8] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x08a8] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x08a8] = 12;
	} else if (packetver >= 20120103) {
		length_list[0x08a8] = 2;
	}

	// Packet: 0x08a9
	if (packetver >= 20121218) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x08a9] = 36;
	} else if (packetver >= 20121121) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x08a9] = 6;
	} else if (packetver >= 20121031) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x08a9] = -1;
	} else if (packetver >= 20121017) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x08a9] = -1;
	} else if (packetver >= 20120801) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x08a9] = 19;
	} else if (packetver >= 20120529) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x08a9] = 6;
	} else if (packetver >= 20120515) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x08a9] = 19;
	} else if (packetver >= 20120503) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x08a9] = 7;
	} else if (packetver >= 20120120) {
		length_list[0x08a9] = 2;
	} else if (packetver >= 20120117) {
		length_list[0x08a9] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x08a9] = 2;
	}

	// Packet: 0x08aa
	if (packetver >= 20121227) {
		length_list[0x08aa] = 18;
	} else if (packetver >= 20121218) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x08aa] = 4;
	} else if (packetver >= 20121128) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x08aa] = 7;
	} else if (packetver >= 20121031) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x08aa] = 5;
	} else if (packetver >= 20120808) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x08aa] = 5;
	} else if (packetver >= 20120716) {
		length_list[0x08aa] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x08aa] = 8;
	} else if (packetver >= 20120103) {
		length_list[0x08aa] = 2;
	}

	// Packet: 0x08ab
	if (packetver >= 20121121) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x08ab] = 5;
	} else if (packetver >= 20120830) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x08ab] = 7;
	} else if (packetver >= 20120808) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x08ab] = 8;
	} else if (packetver >= 20120724) {
		length_list[0x08ab] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x08ab] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x08ab] = 5;
	} else if (packetver >= 20120110) {
		length_list[0x08ab] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x08ab] = 36;
	}

	// Packet: 0x08ac
	if (packetver >= 20121218) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x08ac] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x08ac] = 7;
	} else if (packetver >= 20120515) {
		length_list[0x08ac] = 5;
	} else if (packetver >= 20120328) {
		length_list[0x08ac] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x08ac] = 26;
	} else if (packetver >= 20120103) {
		length_list[0x08ac] = 2;
	}

	// Packet: 0x08ad
	if (packetver >= 20121212) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x08ad] = 6;
	} else if (packetver >= 20121128) {
		length_list[0x08ad] = 5;
	} else if (packetver >= 20121114) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x08ad] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x08ad] = -1;
	} else if (packetver >= 20120522) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x08ad] = 10;
	} else if (packetver >= 20120404) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x08ad] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20120131) {
		length_list[0x08ad] = 6;
	} else if (packetver >= 20120120) {
		length_list[0x08ad] = 2;
	} else if (packetver >= 20120103) {
		length_list[0x08ad] = 6;
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
	if (packetver >= 20120307) {
		length_list[0x08e2] = 27;
	} else if (packetver >= 20120103) {
		length_list[0x08e2] = 25;
	}

	// Packet: 0x08e3
	length_list[0x08e3] = 146;

	// Packet: 0x08e4
	length_list[0x08e4] = 6;

	// Packet: 0x08e5
	length_list[0x08e5] = 41;

	// Packet: 0x08e6
	length_list[0x08e6] = 4;

	// Packet: 0x08e7
	length_list[0x08e7] = 10;

	// Packet: 0x08e8
	length_list[0x08e8] = -1;

	// Packet: 0x08e9
	length_list[0x08e9] = 2;

	// Packet: 0x08ea
	length_list[0x08ea] = 4;

	// Packet: 0x08eb
	length_list[0x08eb] = 39;

	// Packet: 0x08ec
	length_list[0x08ec] = 73;

	// Packet: 0x08ed
	length_list[0x08ed] = 43;

	// Packet: 0x08ee
	length_list[0x08ee] = 6;

	// Packet: 0x08ef
	length_list[0x08ef] = 6;

	// Packet: 0x08f0
	length_list[0x08f0] = 6;

	// Packet: 0x08f1
	length_list[0x08f1] = 6;

	// Packet: 0x08f2
	length_list[0x08f2] = 36;

	// Packet: 0x08f3
	length_list[0x08f3] = -1;

	// Packet: 0x08f4
	length_list[0x08f4] = 6;

	// Packet: 0x08f5
	length_list[0x08f5] = -1;

	// Packet: 0x08f6
	length_list[0x08f6] = 22;

	// Packet: 0x08f7
	length_list[0x08f7] = 3;

	// Packet: 0x08f8
	length_list[0x08f8] = 7;

	// Packet: 0x08f9
	length_list[0x08f9] = 6;

	// Packet: 0x08fa
	length_list[0x08fa] = 6;

	// Packet: 0x08fb
	length_list[0x08fb] = 6;

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

	// Packet: 0x0909
	length_list[0x0909] = 6;

	// Packet: 0x090a
	length_list[0x090a] = 26;

	// Packet: 0x090b
	length_list[0x090b] = 30;

	// Packet: 0x090c
	length_list[0x090c] = 30;

	// Packet: 0x090d
	length_list[0x090d] = -1;

	// Packet: 0x090e
	length_list[0x090e] = 2;

	// Packet: 0x090f
	if (packetver >= 20120131) {
		length_list[0x090f] = -1;
	}

	// Packet: 0x0910
	if (packetver >= 20120120) {
		length_list[0x0910] = 10;
	}

	// Packet: 0x0911
	if (packetver >= 20120120) {
		length_list[0x0911] = 30;
	}

	// Packet: 0x0912
	if (packetver >= 20120120) {
		length_list[0x0912] = 10;
	}

	// Packet: 0x0913
	if (packetver >= 20120120) {
		length_list[0x0913] = 30;
	}

	// Packet: 0x0914
	if (packetver >= 20120131) {
		length_list[0x0914] = -1;
	}

	// Packet: 0x0915
	if (packetver >= 20120131) {
		length_list[0x0915] = -1;
	}

	// Packet: 0x0916
	if (packetver >= 20120131) {
		length_list[0x0916] = 26;
	}

	// Packet: 0x0917
	if (packetver >= 20121218) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0917] = 8;
	} else if (packetver >= 20120911) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0917] = -1;
	} else if (packetver >= 20120830) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0917] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0917] = 36;
	} else if (packetver >= 20120307) {
		length_list[0x0917] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0917] = 7;
	} else if (packetver >= 20120207) {
		length_list[0x0917] = 2;
	}

	// Packet: 0x0918
	if (packetver >= 20121024) {
		length_list[0x0918] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0918] = 36;
	} else if (packetver >= 20120911) {
		length_list[0x0918] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0918] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x0918] = 2;
	}

	// Packet: 0x0919
	if (packetver >= 20120604) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x0919] = 5;
	} else if (packetver >= 20120307) {
		length_list[0x0919] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0919] = 19;
	} else if (packetver >= 20120221) {
		length_list[0x0919] = 4;
	} else if (packetver >= 20120207) {
		length_list[0x0919] = 2;
	}

	// Packet: 0x091a
	if (packetver >= 20121227) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x091a] = 8;
	} else if (packetver >= 20121212) {
		length_list[0x091a] = -1;
	} else if (packetver >= 20121128) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x091a] = 6;
	} else if (packetver >= 20121114) {
		length_list[0x091a] = 8;
	} else if (packetver >= 20120911) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x091a] = 6;
	} else if (packetver >= 20120522) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x091a] = 6;
	} else if (packetver >= 20120404) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x091a] = -1;
	} else if (packetver >= 20120221) {
		length_list[0x091a] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x091a] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x091a] = 2;
	}

	// Packet: 0x091b
	if (packetver >= 20121205) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x091b] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x091b] = -1;
	} else if (packetver >= 20120404) {
		length_list[0x091b] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x091b] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x091b] = 2;
	}

	// Packet: 0x091c
	if (packetver >= 20121227) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x091c] = 19;
	} else if (packetver >= 20121212) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x091c] = 8;
	} else if (packetver >= 20121128) {
		length_list[0x091c] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x091c] = 5;
	} else if (packetver >= 20120417) {
		length_list[0x091c] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x091c] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x091c] = 2;
	}

	// Packet: 0x091d
	if (packetver >= 20120905) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x091d] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x091d] = -1;
	} else if (packetver >= 20120612) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x091d] = -1;
	} else if (packetver >= 20120417) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x091d] = 18;
	} else if (packetver >= 20120404) {
		length_list[0x091d] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x091d] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x091d] = 2;
	}

	// Packet: 0x091e
	if (packetver >= 20121205) {
		length_list[0x091e] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x091e] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x091e] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x091e] = 12;
	} else if (packetver >= 20120207) {
		length_list[0x091e] = 2;
	}

	// Packet: 0x091f
	if (packetver >= 20121227) {
		length_list[0x091f] = 4;
	} else if (packetver >= 20121205) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x091f] = 90;
	} else if (packetver >= 20120822) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x091f] = 19;
	} else if (packetver >= 20120801) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x091f] = 6;
	} else if (packetver >= 20120522) {
		length_list[0x091f] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x091f] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x091f] = 2;
	}

	// Packet: 0x0920
	if (packetver >= 20121227) {
		length_list[0x0920] = 90;
	} else if (packetver >= 20121218) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0920] = 5;
	} else if (packetver >= 20121024) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0920] = 4;
	} else if (packetver >= 20120822) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0920] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0920] = 7;
	} else if (packetver >= 20120404) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0920] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x0920] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0920] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0920] = 2;
	}

	// Packet: 0x0921
	if (packetver >= 20121212) {
		length_list[0x0921] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0921] = 19;
	} else if (packetver >= 20121128) {
		length_list[0x0921] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0921] = 10;
	} else if (packetver >= 20120221) {
		length_list[0x0921] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0921] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x0921] = 2;
	}

	// Packet: 0x0922
	if (packetver >= 20121205) {
		length_list[0x0922] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0922] = 8;
	} else if (packetver >= 20121114) {
		length_list[0x0922] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0922] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x0922] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0922] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x0922] = 2;
	}

	// Packet: 0x0923
	if (packetver >= 20121010) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0923] = 6;
	} else if (packetver >= 20120522) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0923] = 7;
	} else if (packetver >= 20120503) {
		length_list[0x0923] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0923] = 19;
	} else if (packetver >= 20120207) {
		length_list[0x0923] = 2;
	}

	// Packet: 0x0924
	if (packetver >= 20120612) {
		length_list[0x0924] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0924] = 90;
	} else if (packetver >= 20120207) {
		length_list[0x0924] = 2;
	}

	// Packet: 0x0925
	if (packetver >= 20121017) {
		length_list[0x0925] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0925] = 12;
	} else if (packetver >= 20120307) {
		length_list[0x0925] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0925] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x0925] = 2;
	}

	// Packet: 0x0926
	if (packetver >= 20120905) {
		length_list[0x0926] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0926] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x0926] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0926] = 6;
	} else if (packetver >= 20120314) {
		length_list[0x0926] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0926] = 18;
	} else if (packetver >= 20120207) {
		length_list[0x0926] = 2;
	}

	// Packet: 0x0927
	if (packetver >= 20121128) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0927] = 19;
	} else if (packetver >= 20121010) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0927] = 6;
	} else if (packetver >= 20120911) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0927] = 18;
	} else if (packetver >= 20120830) {
		length_list[0x0927] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0927] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0927] = 2;
	}

	// Packet: 0x0928
	if (packetver >= 20120801) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0928] = -1;
	} else if (packetver >= 20120307) {
		length_list[0x0928] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0928] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x0928] = 2;
	}

	// Packet: 0x0929
	if (packetver >= 20121128) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0929] = 4;
	} else if (packetver >= 20120822) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0929] = 8;
	} else if (packetver >= 20120801) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0929] = 12;
	} else if (packetver >= 20120314) {
		length_list[0x0929] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0929] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x0929] = 2;
	}

	// Packet: 0x092a
	if (packetver >= 20120808) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x092a] = 4;
	} else if (packetver >= 20120320) {
		length_list[0x092a] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x092a] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x092a] = 2;
	}

	// Packet: 0x092b
	if (packetver >= 20121107) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20121031) {
		length_list[0x092b] = 4;
	} else if (packetver >= 20120801) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x092b] = 90;
	} else if (packetver >= 20120522) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x092b] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x092b] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x092b] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x092b] = 2;
	}

	// Packet: 0x092c
	if (packetver >= 20120710) {
		length_list[0x092c] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x092c] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x092c] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x092c] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x092c] = 2;
	}

	// Packet: 0x092d
	if (packetver >= 20120716) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x092d] = 12;
	} else if (packetver >= 20120307) {
		length_list[0x092d] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x092d] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x092d] = 2;
	}

	// Packet: 0x092e
	if (packetver >= 20120808) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x092e] = 6;
	} else if (packetver >= 20120612) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x092e] = -1;
	} else if (packetver >= 20120410) {
		length_list[0x092e] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x092e] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x092e] = 2;
	}

	// Packet: 0x092f
	if (packetver >= 20121205) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x092f] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x092f] = 4;
	} else if (packetver >= 20120612) {
		length_list[0x092f] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x092f] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x092f] = 2;
	}

	// Packet: 0x0930
	if (packetver >= 20121121) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0930] = 4;
	} else if (packetver >= 20120919) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0930] = -1;
	} else if (packetver >= 20120808) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0930] = 6;
	} else if (packetver >= 20120529) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0930] = 4;
	} else if (packetver >= 20120228) {
		length_list[0x0930] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0930] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x0930] = 2;
	}

	// Packet: 0x0931
	if (packetver >= 20121205) {
		length_list[0x0931] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0931] = 7;
	} else if (packetver >= 20121031) {
		length_list[0x0931] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0931] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x0931] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0931] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0931] = 2;
	}

	// Packet: 0x0932
	if (packetver >= 20121227) {
		length_list[0x0932] = 5;
	} else if (packetver >= 20121218) {
		length_list[0x0932] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0932] = 18;
	} else if (packetver >= 20121010) {
		length_list[0x0932] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0932] = 6;
	} else if (packetver >= 20120822) {
		length_list[0x0932] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0932] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x0932] = 2;
	}

	// Packet: 0x0933
	if (packetver >= 20121227) {
		length_list[0x0933] = -1;
	} else if (packetver >= 20120307) {
		length_list[0x0933] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0933] = 18;
	} else if (packetver >= 20120221) {
		length_list[0x0933] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0933] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0933] = 2;
	}

	// Packet: 0x0934
	if (packetver >= 20121114) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0934] = 12;
	} else if (packetver >= 20121017) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0934] = 6;
	} else if (packetver >= 20120905) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0934] = 5;
	} else if (packetver >= 20120822) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0934] = 10;
	} else if (packetver >= 20120808) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0934] = 10;
	} else if (packetver >= 20120404) {
		length_list[0x0934] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0934] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0934] = 2;
	}

	// Packet: 0x0935
	if (packetver >= 20121227) {
		length_list[0x0935] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0935] = 26;
	} else if (packetver >= 20121121) {
		length_list[0x0935] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0935] = 10;
	} else if (packetver >= 20120830) {
		length_list[0x0935] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0935] = 5;
	} else if (packetver >= 20120702) {
		length_list[0x0935] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0935] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x0935] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0935] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x0935] = 2;
	}

	// Packet: 0x0936
	if (packetver >= 20120716) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0936] = 8;
	} else if (packetver >= 20120702) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0936] = 12;
	} else if (packetver >= 20120612) {
		length_list[0x0936] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0936] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0936] = 2;
	}

	// Packet: 0x0937
	if (packetver >= 20120830) {
		length_list[0x0937] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0937] = 10;
	} else if (packetver >= 20120307) {
		length_list[0x0937] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0937] = 26;
	} else if (packetver >= 20120221) {
		length_list[0x0937] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0937] = 2;
	}

	// Packet: 0x0938
	if (packetver >= 20121128) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0938] = 12;
	} else if (packetver >= 20121114) {
		length_list[0x0938] = -1;
	} else if (packetver >= 20120801) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0938] = 10;
	} else if (packetver >= 20120702) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0938] = -1;
	} else if (packetver >= 20120417) {
		length_list[0x0938] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0938] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0938] = 2;
	}

	// Packet: 0x0939
	if (packetver >= 20120207) {
		length_list[0x0939] = 2;
	}

	// Packet: 0x093a
	if (packetver >= 20121212) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x093a] = 6;
	} else if (packetver >= 20121031) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x093a] = 8;
	} else if (packetver >= 20120702) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x093a] = 90;
	} else if (packetver >= 20120604) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x093a] = 19;
	} else if (packetver >= 20120320) {
		length_list[0x093a] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x093a] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x093a] = 2;
	}

	// Packet: 0x093b
	if (packetver >= 20121121) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x093b] = 12;
	} else if (packetver >= 20121107) {
		length_list[0x093b] = 10;
	} else if (packetver >= 20120314) {
		length_list[0x093b] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x093b] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x093b] = 2;
	}

	// Packet: 0x093c
	if (packetver >= 20121121) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x093c] = 8;
	} else if (packetver >= 20121017) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x093c] = 8;
	} else if (packetver >= 20120911) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x093c] = 12;
	} else if (packetver >= 20120808) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x093c] = 26;
	} else if (packetver >= 20120724) {
		length_list[0x093c] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x093c] = 5;
	} else if (packetver >= 20120228) {
		length_list[0x093c] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x093c] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x093c] = 2;
	}

	// Packet: 0x093d
	if (packetver >= 20120814) {
		length_list[0x093d] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x093d] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x093d] = 2;
	}

	// Packet: 0x093e
	if (packetver >= 20121212) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x093e] = 8;
	} else if (packetver >= 20120919) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x093e] = 12;
	} else if (packetver >= 20120529) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x093e] = 90;
	} else if (packetver >= 20120404) {
		length_list[0x093e] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x093e] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x093e] = 2;
	}

	// Packet: 0x093f
	if (packetver >= 20121212) {
		length_list[0x093f] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x093f] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x093f] = 2;
	}

	// Packet: 0x0940
	if (packetver >= 20121227) {
		length_list[0x0940] = 10;
	} else if (packetver >= 20120830) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0940] = 12;
	} else if (packetver >= 20120814) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x0940] = 10;
	} else if (packetver >= 20120626) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20120618) {
		length_list[0x0940] = -1;
	} else if (packetver >= 20120515) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x0940] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0940] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0940] = 2;
	}

	// Packet: 0x0941
	if (packetver >= 20121121) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0941] = 26;
	} else if (packetver >= 20120822) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0941] = 90;
	} else if (packetver >= 20120808) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0941] = 8;
	} else if (packetver >= 20120529) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0941] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0941] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0941] = 26;
	} else if (packetver >= 20120307) {
		length_list[0x0941] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0941] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0941] = 2;
	}

	// Packet: 0x0942
	if (packetver >= 20121031) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0942] = -1;
	} else if (packetver >= 20121017) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0942] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x0942] = 36;
	} else if (packetver >= 20120307) {
		length_list[0x0942] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0942] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x0942] = 2;
	}

	// Packet: 0x0943
	if (packetver >= 20120830) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0943] = 8;
	} else if (packetver >= 20120612) {
		length_list[0x0943] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0943] = 4;
	} else if (packetver >= 20120207) {
		length_list[0x0943] = 2;
	}

	// Packet: 0x0944
	if (packetver >= 20121121) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0944] = -1;
	} else if (packetver >= 20120911) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0944] = 26;
	} else if (packetver >= 20120710) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0944] = 5;
	} else if (packetver >= 20120522) {
		length_list[0x0944] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0944] = 18;
	} else if (packetver >= 20120207) {
		length_list[0x0944] = 2;
	}

	// Packet: 0x0945
	if (packetver >= 20121205) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0945] = 5;
	} else if (packetver >= 20121114) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0945] = 90;
	} else if (packetver >= 20120911) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0945] = -1;
	} else if (packetver >= 20120830) {
		length_list[0x0945] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0945] = 7;
	} else if (packetver >= 20120417) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0945] = -1;
	} else if (packetver >= 20120328) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0945] = 19;
	} else if (packetver >= 20120307) {
		length_list[0x0945] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0945] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0945] = 2;
	}

	// Packet: 0x0946
	if (packetver >= 20121227) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0946] = 36;
	} else if (packetver >= 20121212) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0946] = 10;
	} else if (packetver >= 20121128) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0946] = 5;
	} else if (packetver >= 20121017) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0946] = 19;
	} else if (packetver >= 20120808) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0946] = -1;
	} else if (packetver >= 20120320) {
		length_list[0x0946] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0946] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0946] = 2;
	}

	// Packet: 0x0947
	if (packetver >= 20121227) {
		length_list[0x0947] = 6;
	} else if (packetver >= 20121218) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0947] = 5;
	} else if (packetver >= 20121031) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0947] = 6;
	} else if (packetver >= 20121017) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0947] = 10;
	} else if (packetver >= 20120522) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0947] = 10;
	} else if (packetver >= 20120503) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0947] = 4;
	} else if (packetver >= 20120410) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0947] = 36;
	} else if (packetver >= 20120307) {
		length_list[0x0947] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0947] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0947] = 2;
	}

	// Packet: 0x0948
	if (packetver >= 20121010) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0948] = 18;
	} else if (packetver >= 20120919) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0948] = 10;
	} else if (packetver >= 20120503) {
		length_list[0x0948] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0948] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0948] = 2;
	}

	// Packet: 0x0949
	if (packetver >= 20121121) {
		length_list[0x0949] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0949] = 18;
	} else if (packetver >= 20120919) {
		length_list[0x0949] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0949] = 4;
	} else if (packetver >= 20120207) {
		length_list[0x0949] = 2;
	}

	// Packet: 0x094a
	if (packetver >= 20121121) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x094a] = 10;
	} else if (packetver >= 20120710) {
		length_list[0x094a] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x094a] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x094a] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x094a] = 2;
	}

	// Packet: 0x094b
	if (packetver >= 20121212) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x094b] = 6;
	} else if (packetver >= 20121121) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x094b] = 19;
	} else if (packetver >= 20120919) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x094b] = -1;
	} else if (packetver >= 20120905) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x094b] = 18;
	} else if (packetver >= 20120716) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x094b] = 5;
	} else if (packetver >= 20120522) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x094b] = 5;
	} else if (packetver >= 20120417) {
		length_list[0x094b] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x094b] = 19;
	} else if (packetver >= 20120207) {
		length_list[0x094b] = 2;
	}

	// Packet: 0x094c
	if (packetver >= 20121017) {
		length_list[0x094c] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x094c] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x094c] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x094c] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x094c] = 2;
	}

	// Packet: 0x094d
	if (packetver >= 20121128) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x094d] = 8;
	} else if (packetver >= 20121114) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x094d] = 5;
	} else if (packetver >= 20120830) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x094d] = 36;
	} else if (packetver >= 20120529) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x094d] = 26;
	} else if (packetver >= 20120515) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20120508) {
		length_list[0x094d] = 6;
	} else if (packetver >= 20120307) {
		length_list[0x094d] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x094d] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x094d] = 2;
	}

	// Packet: 0x094e
	if (packetver >= 20120830) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x094e] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x094e] = 8;
	} else if (packetver >= 20120716) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x094e] = 26;
	} else if (packetver >= 20120612) {
		length_list[0x094e] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x094e] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x094e] = 2;
	}

	// Packet: 0x094f
	if (packetver >= 20121121) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x094f] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x094f] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x094f] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x094f] = 2;
	}

	// Packet: 0x0950
	if (packetver >= 20121205) {
		length_list[0x0950] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0950] = 12;
	} else if (packetver >= 20121121) {
		length_list[0x0950] = 5;
	} else if (packetver >= 20121010) {
		length_list[0x0950] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0950] = 5;
	} else if (packetver >= 20120911) {
		length_list[0x0950] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0950] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0950] = 2;
	}

	// Packet: 0x0951
	if (packetver >= 20121218) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0951] = 8;
	} else if (packetver >= 20120702) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0951] = 6;
	} else if (packetver >= 20120404) {
		length_list[0x0951] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0951] = 7;
	} else if (packetver >= 20120207) {
		length_list[0x0951] = 2;
	}

	// Packet: 0x0952
	if (packetver >= 20121128) {
		length_list[0x0952] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0952] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x0952] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0952] = 90;
	} else if (packetver >= 20120221) {
		length_list[0x0952] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0952] = 19;
	} else if (packetver >= 20120207) {
		length_list[0x0952] = 2;
	}

	// Packet: 0x0953
	if (packetver >= 20121227) {
		length_list[0x0953] = 8;
	} else if (packetver >= 20121218) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0953] = 6;
	} else if (packetver >= 20121205) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0953] = 6;
	} else if (packetver >= 20120830) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0953] = 18;
	} else if (packetver >= 20120716) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0953] = 4;
	} else if (packetver >= 20120702) {
		length_list[0x0953] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x0953] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0953] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x0953] = 2;
	}

	// Packet: 0x0954
	if (packetver >= 20121212) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0954] = 7;
	} else if (packetver >= 20121128) {
		length_list[0x0954] = 8;
	} else if (packetver >= 20121121) {
		length_list[0x0954] = 36;
	} else if (packetver >= 20121031) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x0954] = 90;
	} else if (packetver >= 20120618) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x0954] = 36;
	} else if (packetver >= 20120404) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0954] = 6;
	} else if (packetver >= 20120307) {
		length_list[0x0954] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0954] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0954] = 2;
	}

	// Packet: 0x0955
	if (packetver >= 20120830) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x0955] = -1;
	} else if (packetver >= 20120503) {
		length_list[0x0955] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0955] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0955] = 2;
	}

	// Packet: 0x0956
	if (packetver >= 20121010) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0956] = 8;
	} else if (packetver >= 20120919) {
		length_list[0x0956] = 36;
	} else if (packetver >= 20120716) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0956] = -1;
	} else if (packetver >= 20120529) {
		length_list[0x0956] = 2;
	} else if (packetver >= 20120522) {
		length_list[0x0956] = 6;
	} else if (packetver >= 20120207) {
		length_list[0x0956] = 2;
	}

	// Packet: 0x0957
	if (packetver >= 20121227) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20121218) {
		length_list[0x0957] = 18;
	} else if (packetver >= 20121212) {
		length_list[0x0957] = 90;
	} else if (packetver >= 20121128) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0957] = 26;
	} else if (packetver >= 20121017) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0957] = 5;
	} else if (packetver >= 20120801) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0957] = 36;
	} else if (packetver >= 20120522) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0957] = 6;
	} else if (packetver >= 20120221) {
		length_list[0x0957] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0957] = 90;
	} else if (packetver >= 20120207) {
		length_list[0x0957] = 2;
	}

	// Packet: 0x0958
	if (packetver >= 20121227) {
		length_list[0x0958] = 8;
	} else if (packetver >= 20121128) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20121121) {
		length_list[0x0958] = 6;
	} else if (packetver >= 20121010) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0958] = 6;
	} else if (packetver >= 20120919) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0958] = 10;
	} else if (packetver >= 20120410) {
		length_list[0x0958] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x0958] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0958] = 2;
	}

	// Packet: 0x0959
	if (packetver >= 20121212) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0959] = 26;
	} else if (packetver >= 20121017) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0959] = -1;
	} else if (packetver >= 20120919) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0959] = 7;
	} else if (packetver >= 20120905) {
		length_list[0x0959] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x0959] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0959] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x0959] = 2;
	}

	// Packet: 0x095a
	if (packetver >= 20120207) {
		length_list[0x095a] = 2;
	}

	// Packet: 0x095b
	if (packetver >= 20121218) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x095b] = 8;
	} else if (packetver >= 20120830) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20120822) {
		length_list[0x095b] = 6;
	} else if (packetver >= 20120612) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x095b] = 8;
	} else if (packetver >= 20120503) {
		length_list[0x095b] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x095b] = -1;
	} else if (packetver >= 20120207) {
		length_list[0x095b] = 2;
	}

	// Packet: 0x095c
	if (packetver >= 20121121) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x095c] = -1;
	} else if (packetver >= 20121017) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x095c] = 5;
	} else if (packetver >= 20120314) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x095c] = 4;
	} else if (packetver >= 20120228) {
		length_list[0x095c] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x095c] = 5;
	} else if (packetver >= 20120214) {
		length_list[0x095c] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x095c] = 2;
	}

	// Packet: 0x095d
	if (packetver >= 20121227) {
		length_list[0x095d] = -1;
	} else if (packetver >= 20121031) {
		length_list[0x095d] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x095d] = 26;
	} else if (packetver >= 20120710) {
		length_list[0x095d] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x095d] = 36;
	} else if (packetver >= 20120404) {
		length_list[0x095d] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x095d] = 26;
	} else if (packetver >= 20120207) {
		length_list[0x095d] = 2;
	}

	// Packet: 0x095e
	if (packetver >= 20121031) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20121024) {
		length_list[0x095e] = 6;
	} else if (packetver >= 20121010) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x095e] = 10;
	} else if (packetver >= 20120307) {
		length_list[0x095e] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x095e] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x095e] = 2;
	}

	// Packet: 0x095f
	if (packetver >= 20120207) {
		length_list[0x095f] = 2;
	}

	// Packet: 0x0960
	if (packetver >= 20121205) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20121128) {
		length_list[0x0960] = -1;
	} else if (packetver >= 20121114) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0960] = 6;
	} else if (packetver >= 20121024) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0960] = 5;
	} else if (packetver >= 20120911) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0960] = 90;
	} else if (packetver >= 20120716) {
		length_list[0x0960] = 2;
	} else if (packetver >= 20120710) {
		length_list[0x0960] = 90;
	} else if (packetver >= 20120702) {
		length_list[0x0960] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x0960] = 2;
	}

	// Packet: 0x0961
	if (packetver >= 20120710) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0961] = 4;
	} else if (packetver >= 20120417) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20120410) {
		length_list[0x0961] = 36;
	} else if (packetver >= 20120307) {
		length_list[0x0961] = 2;
	} else if (packetver >= 20120228) {
		length_list[0x0961] = 10;
	} else if (packetver >= 20120207) {
		length_list[0x0961] = 2;
	}

	// Packet: 0x0962
	if (packetver >= 20121227) {
		length_list[0x0962] = 26;
	} else if (packetver >= 20121024) {
		length_list[0x0962] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0962] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x0962] = 2;
	} else if (packetver >= 20120626) {
		length_list[0x0962] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0962] = 2;
	}

	// Packet: 0x0963
	if (packetver >= 20121114) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0963] = 4;
	} else if (packetver >= 20121010) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120925) {
		length_list[0x0963] = 8;
	} else if (packetver >= 20120919) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0963] = 6;
	} else if (packetver >= 20120801) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0963] = 6;
	} else if (packetver >= 20120612) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120604) {
		length_list[0x0963] = 8;
	} else if (packetver >= 20120404) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0963] = 6;
	} else if (packetver >= 20120314) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x0963] = 8;
	} else if (packetver >= 20120221) {
		length_list[0x0963] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0963] = 7;
	} else if (packetver >= 20120207) {
		length_list[0x0963] = 2;
	}

	// Packet: 0x0964
	if (packetver >= 20121017) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0964] = 26;
	} else if (packetver >= 20120905) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x0964] = 6;
	} else if (packetver >= 20120808) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20120801) {
		length_list[0x0964] = 12;
	} else if (packetver >= 20120522) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0964] = 6;
	} else if (packetver >= 20120320) {
		length_list[0x0964] = 2;
	} else if (packetver >= 20120314) {
		length_list[0x0964] = 18;
	} else if (packetver >= 20120207) {
		length_list[0x0964] = 2;
	}

	// Packet: 0x0965
	if (packetver >= 20121218) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0965] = -1;
	} else if (packetver >= 20121121) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x0965] = 6;
	} else if (packetver >= 20121031) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x0965] = 19;
	} else if (packetver >= 20120801) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0965] = 6;
	} else if (packetver >= 20120503) {
		length_list[0x0965] = 2;
	} else if (packetver >= 20120424) {
		length_list[0x0965] = 8;
	} else if (packetver >= 20120207) {
		length_list[0x0965] = 2;
	}

	// Packet: 0x0966
	if (packetver >= 20121218) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x0966] = -1;
	} else if (packetver >= 20121205) {
		length_list[0x0966] = 8;
	} else if (packetver >= 20121114) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20121107) {
		length_list[0x0966] = 18;
	} else if (packetver >= 20120919) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0966] = 8;
	} else if (packetver >= 20120801) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20120724) {
		length_list[0x0966] = 8;
	} else if (packetver >= 20120522) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x0966] = 4;
	} else if (packetver >= 20120328) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20120320) {
		length_list[0x0966] = 5;
	} else if (packetver >= 20120221) {
		length_list[0x0966] = 2;
	} else if (packetver >= 20120214) {
		length_list[0x0966] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x0966] = 2;
	}

	// Packet: 0x0967
	if (packetver >= 20120911) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20120905) {
		length_list[0x0967] = 7;
	} else if (packetver >= 20120822) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0967] = -1;
	} else if (packetver >= 20120404) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20120328) {
		length_list[0x0967] = 10;
	} else if (packetver >= 20120228) {
		length_list[0x0967] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0967] = 36;
	} else if (packetver >= 20120207) {
		length_list[0x0967] = 2;
	}

	// Packet: 0x0968
	if (packetver >= 20121212) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20121205) {
		length_list[0x0968] = 5;
	} else if (packetver >= 20120919) {
		length_list[0x0968] = 2;
	} else if (packetver >= 20120911) {
		length_list[0x0968] = 5;
	} else if (packetver >= 20120207) {
		length_list[0x0968] = 2;
	}

	// Packet: 0x0969
	if (packetver >= 20121017) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20121010) {
		length_list[0x0969] = 18;
	} else if (packetver >= 20120822) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20120814) {
		length_list[0x0969] = 5;
	} else if (packetver >= 20120228) {
		length_list[0x0969] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x0969] = 19;
	} else if (packetver >= 20120207) {
		length_list[0x0969] = 2;
	}

	// Packet: 0x096a
	if (packetver >= 20121227) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20121212) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20121121) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20121114) {
		length_list[0x096a] = 8;
	} else if (packetver >= 20121107) {
		length_list[0x096a] = 26;
	} else if (packetver >= 20121031) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20121024) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20121017) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120925) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120919) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120911) {
		length_list[0x096a] = 36;
	} else if (packetver >= 20120905) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120830) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120814) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120808) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120724) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120716) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120626) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120612) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120604) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120529) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120522) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120515) {
		length_list[0x096a] = -1;
	} else if (packetver >= 20120508) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120503) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120424) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120417) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120410) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120404) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120328) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120307) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120228) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120221) {
		length_list[0x096a] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x096a] = 2;
	} else if (packetver >= 20120207) {
		length_list[0x096a] = 6;
	}

	// Packet: 0x096b
	if (packetver >= 20120404) {
		length_list[0x096b] = 4;
	} else if (packetver >= 20120207) {
		length_list[0x096b] = 3;
	}

	// Packet: 0x096c
	if (packetver >= 20120207) {
		length_list[0x096c] = 6;
	}

	// Packet: 0x096d
	if (packetver >= 20120228) {
		length_list[0x096d] = -1;
	} else if (packetver >= 20120214) {
		length_list[0x096d] = 2;
	}

	// Packet: 0x096e
	if (packetver >= 20120228) {
		length_list[0x096e] = -1;
	} else if (packetver >= 20120221) {
		length_list[0x096e] = 6;
	} else if (packetver >= 20120214) {
		length_list[0x096e] = 10;
	}

	// Packet: 0x096f
	if (packetver >= 20120221) {
		length_list[0x096f] = 7;
	} else if (packetver >= 20120214) {
		length_list[0x096f] = 3;
	}

	// Packet: 0x0970
	if (packetver >= 20120214) {
		length_list[0x0970] = 31;
	}

	// Packet: 0x0971
	if (packetver >= 20120214) {
		length_list[0x0971] = 6;
	}

	// Packet: 0x0972
	if (packetver >= 20120214) {
		length_list[0x0972] = -1;
	}

	// Packet: 0x0973
	if (packetver >= 20120214) {
		length_list[0x0973] = 7;
	}

	// Packet: 0x0974
	if (packetver >= 20120221) {
		length_list[0x0974] = 2;
	}

	// Packet: 0x0975
	if (packetver >= 20120221) {
		length_list[0x0975] = -1;
	}

	// Packet: 0x0976
	if (packetver >= 20120221) {
		length_list[0x0976] = -1;
	}

	// Packet: 0x0977
	if (packetver >= 20120228) {
		length_list[0x0977] = 14;
	} else if (packetver >= 20120221) {
		length_list[0x0977] = 15;
	}

	// Packet: 0x0978
	if (packetver >= 20120328) {
		length_list[0x0978] = 6;
	}

	// Packet: 0x0979
	if (packetver >= 20120328) {
		length_list[0x0979] = 50;
	}

	// Packet: 0x097a
	if (packetver >= 20120410) {
		length_list[0x097a] = -1;
	}

	// Packet: 0x097b
	if (packetver >= 20120417) {
		length_list[0x097b] = -1;
	}

	// Packet: 0x097c
	if (packetver >= 20120503) {
		length_list[0x097c] = 4;
	}

	// Packet: 0x097d
	if (packetver >= 20120503) {
		length_list[0x097d] = 288;
	}

	// Packet: 0x097e
	if (packetver >= 20120503) {
		length_list[0x097e] = 12;
	}

	// Packet: 0x097f
	if (packetver >= 20120503) {
		length_list[0x097f] = -1;
	}

	// Packet: 0x0980
	if (packetver >= 20120503) {
		length_list[0x0980] = 7;
	}

	// Packet: 0x0981
	if (packetver >= 20120515) {
		length_list[0x0981] = -1;
	}

	// Packet: 0x0982
	if (packetver >= 20120522) {
		length_list[0x0982] = 7;
	} else if (packetver >= 20120515) {
		length_list[0x0982] = 3;
	}

	// Packet: 0x0983
	if (packetver >= 20120529) {
		length_list[0x0983] = 29;
	}

	// Packet: 0x0984
	if (packetver >= 20120529) {
		length_list[0x0984] = 28;
	}

	// Packet: 0x0985
	if (packetver >= 20120604) {
		length_list[0x0985] = -1;
	}

	// Packet: 0x0986
	if (packetver >= 20120626) {
		length_list[0x0986] = 10;
	} else if (packetver >= 20120612) {
		length_list[0x0986] = 6;
	}

	// Packet: 0x0987
	if (packetver >= 20120702) {
		length_list[0x0987] = 63;
	}

	// Packet: 0x0988
	if (packetver >= 20120716) {
		length_list[0x0988] = 6;
	} else if (packetver >= 20120702) {
		length_list[0x0988] = 2;
	}

	// Packet: 0x0989
	if (packetver >= 20120716) {
		length_list[0x0989] = 2;
	} else if (packetver >= 20120702) {
		length_list[0x0989] = 6;
	}

	// Packet: 0x098a
	if (packetver >= 20120702) {
		length_list[0x098a] = -1;
	}

	// Packet: 0x098b
	if (packetver >= 20120716) {
		length_list[0x098b] = 2;
	}

	// Packet: 0x098c
	if (packetver >= 20120716) {
		length_list[0x098c] = 4;
	}

	// Packet: 0x098d
	if (packetver >= 20120716) {
		length_list[0x098d] = -1;
	}

	// Packet: 0x098e
	if (packetver >= 20120716) {
		length_list[0x098e] = -1;
	}

	// Packet: 0x098f
	if (packetver >= 20120724) {
		length_list[0x098f] = -1;
	}

	// Packet: 0x0990
	if (packetver >= 20120925) {
		length_list[0x0990] = 31;
	}

	// Packet: 0x0991
	if (packetver >= 20120925) {
		length_list[0x0991] = -1;
	}

	// Packet: 0x0992
	if (packetver >= 20120925) {
		length_list[0x0992] = -1;
	}

	// Packet: 0x0993
	if (packetver >= 20120925) {
		length_list[0x0993] = -1;
	}

	// Packet: 0x0994
	if (packetver >= 20120925) {
		length_list[0x0994] = -1;
	}

	// Packet: 0x0995
	if (packetver >= 20120925) {
		length_list[0x0995] = -1;
	}

	// Packet: 0x0996
	if (packetver >= 20120925) {
		length_list[0x0996] = -1;
	}

	// Packet: 0x0997
	if (packetver >= 20120925) {
		length_list[0x0997] = -1;
	}

	// Packet: 0x0998
	if (packetver >= 20120925) {
		length_list[0x0998] = 8;
	}

	// Packet: 0x0999
	if (packetver >= 20120925) {
		length_list[0x0999] = 11;
	}

	// Packet: 0x099a
	if (packetver >= 20120925) {
		length_list[0x099a] = 9;
	}

	// Packet: 0x099b
	if (packetver >= 20121010) {
		length_list[0x099b] = 8;
	}

	// Packet: 0x099c
	if (packetver >= 20121024) {
		length_list[0x099c] = 6;
	}

	// Packet: 0x099d
	if (packetver >= 20121024) {
		length_list[0x099d] = -1;
	}

	// Packet: 0x099e
	if (packetver >= 20121128) {
		length_list[0x099e] = 12;
	}

	// Packet: 0x099f
	if (packetver >= 20121212) {
		length_list[0x099f] = -1;
	}

	// Packet: 0x09a0
	if (packetver >= 20121212) {
		length_list[0x09a0] = 6;
	}

	// Packet: 0x09a1
	if (packetver >= 20121212) {
		length_list[0x09a1] = 2;
	}

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
