/**
 * Network/Packets/packets2006_len_main.js
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
	if (packetver >= 20061030) {
		length_list[0x006d] = 110;
	} else if (packetver >= 20060102) {
		length_list[0x006d] = 108;
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
	if (packetver >= 20060327) {
		length_list[0x0072] = 25;
	} else if (packetver >= 20060102) {
		length_list[0x0072] = 20;
	}

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
	length_list[0x0078] = 54;

	// Packet: 0x0079
	length_list[0x0079] = 53;

	// Packet: 0x007a
	length_list[0x007a] = 58;

	// Packet: 0x007b
	length_list[0x007b] = 60;

	// Packet: 0x007c
	length_list[0x007c] = 41;

	// Packet: 0x007d
	length_list[0x007d] = 2;

	// Packet: 0x007e
	if (packetver >= 20060327) {
		length_list[0x007e] = 102;
	} else if (packetver >= 20060102) {
		length_list[0x007e] = 103;
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
	if (packetver >= 20060327) {
		length_list[0x0085] = 11;
	} else if (packetver >= 20060102) {
		length_list[0x0085] = 13;
	}

	// Packet: 0x0086
	length_list[0x0086] = 16;

	// Packet: 0x0087
	length_list[0x0087] = 12;

	// Packet: 0x0088
	length_list[0x0088] = 10;

	// Packet: 0x0089
	if (packetver >= 20060327) {
		length_list[0x0089] = 8;
	} else if (packetver >= 20060102) {
		length_list[0x0089] = 10;
	}

	// Packet: 0x008a
	length_list[0x008a] = 29;

	// Packet: 0x008b
	length_list[0x008b] = 23;

	// Packet: 0x008c
	if (packetver >= 20060327) {
		length_list[0x008c] = 11;
	} else if (packetver >= 20060102) {
		length_list[0x008c] = 15;
	}

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
	if (packetver >= 20060327) {
		length_list[0x0094] = 14;
	} else if (packetver >= 20060102) {
		length_list[0x0094] = 16;
	}

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
	if (packetver >= 20060327) {
		length_list[0x009b] = 26;
	} else if (packetver >= 20060102) {
		length_list[0x009b] = 33;
	}

	// Packet: 0x009c
	length_list[0x009c] = 9;

	// Packet: 0x009d
	length_list[0x009d] = 17;

	// Packet: 0x009e
	length_list[0x009e] = 17;

	// Packet: 0x009f
	if (packetver >= 20060327) {
		length_list[0x009f] = 14;
	} else if (packetver >= 20060102) {
		length_list[0x009f] = 17;
	}

	// Packet: 0x00a0
	length_list[0x00a0] = 23;

	// Packet: 0x00a1
	length_list[0x00a1] = 6;

	// Packet: 0x00a2
	if (packetver >= 20060327) {
		length_list[0x00a2] = 15;
	} else if (packetver >= 20060102) {
		length_list[0x00a2] = 12;
	}

	// Packet: 0x00a3
	length_list[0x00a3] = -1;

	// Packet: 0x00a4
	length_list[0x00a4] = -1;

	// Packet: 0x00a5
	length_list[0x00a5] = -1;

	// Packet: 0x00a6
	length_list[0x00a6] = -1;

	// Packet: 0x00a7
	if (packetver >= 20060327) {
		length_list[0x00a7] = 8;
	} else if (packetver >= 20060102) {
		length_list[0x00a7] = 6;
	}

	// Packet: 0x00a8
	length_list[0x00a8] = 7;

	// Packet: 0x00a9
	length_list[0x00a9] = 6;

	// Packet: 0x00aa
	length_list[0x00aa] = 7;

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
	if (packetver >= 20060327) {
		length_list[0x00f5] = 8;
	} else if (packetver >= 20060102) {
		length_list[0x00f5] = 10;
	}

	// Packet: 0x00f6
	length_list[0x00f6] = 8;

	// Packet: 0x00f7
	if (packetver >= 20060327) {
		length_list[0x00f7] = 22;
	} else if (packetver >= 20060102) {
		length_list[0x00f7] = 26;
	}

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
	if (packetver >= 20060327) {
		length_list[0x0113] = 22;
	} else if (packetver >= 20060102) {
		length_list[0x0113] = 23;
	}

	// Packet: 0x0114
	length_list[0x0114] = 31;

	// Packet: 0x0115
	length_list[0x0115] = 35;

	// Packet: 0x0116
	if (packetver >= 20060327) {
		length_list[0x0116] = 10;
	} else if (packetver >= 20060102) {
		length_list[0x0116] = 14;
	}

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
	length_list[0x0158] = -1;

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
	if (packetver >= 20060327) {
		length_list[0x0190] = 19;
	} else if (packetver >= 20060102) {
		length_list[0x0190] = 9;
	}

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
	length_list[0x01a2] = 35;

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
	length_list[0x01fd] = 4;

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
	length_list[0x022c] = 64;

	// Packet: 0x022d
	length_list[0x022d] = 5;

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
	length_list[0x023b] = 36;

	// Packet: 0x023c
	length_list[0x023c] = 6;

	// Packet: 0x023d
	length_list[0x023d] = 6;

	// Packet: 0x023e
	if (packetver >= 20061207) {
		length_list[0x023e] = 8;
	} else if (packetver >= 20060102) {
		length_list[0x023e] = 4;
	}

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
	if (packetver >= 20060109) {
		length_list[0x0261] = 11;
	}

	// Packet: 0x0262
	if (packetver >= 20060109) {
		length_list[0x0262] = 11;
	}

	// Packet: 0x0263
	if (packetver >= 20060109) {
		length_list[0x0263] = 11;
	}

	// Packet: 0x0264
	if (packetver >= 20060109) {
		length_list[0x0264] = 20;
	}

	// Packet: 0x0265
	if (packetver >= 20060109) {
		length_list[0x0265] = 20;
	}

	// Packet: 0x0266
	if (packetver >= 20060109) {
		length_list[0x0266] = 30;
	}

	// Packet: 0x0267
	if (packetver >= 20060109) {
		length_list[0x0267] = 4;
	}

	// Packet: 0x0268
	if (packetver >= 20060109) {
		length_list[0x0268] = 4;
	}

	// Packet: 0x0269
	if (packetver >= 20060109) {
		length_list[0x0269] = 4;
	}

	// Packet: 0x026a
	if (packetver >= 20060109) {
		length_list[0x026a] = 4;
	}

	// Packet: 0x026b
	if (packetver >= 20060109) {
		length_list[0x026b] = 4;
	}

	// Packet: 0x026c
	if (packetver >= 20060109) {
		length_list[0x026c] = 4;
	}

	// Packet: 0x026d
	if (packetver >= 20060109) {
		length_list[0x026d] = 4;
	}

	// Packet: 0x026f
	if (packetver >= 20060109) {
		length_list[0x026f] = 2;
	}

	// Packet: 0x0270
	if (packetver >= 20060109) {
		length_list[0x0270] = 2;
	}

	// Packet: 0x0271
	if (packetver >= 20060126) {
		length_list[0x0271] = 40;
	} else if (packetver >= 20060109) {
		length_list[0x0271] = 38;
	}

	// Packet: 0x0272
	if (packetver >= 20060109) {
		length_list[0x0272] = 44;
	}

	// Packet: 0x0273
	if (packetver >= 20060313) {
		length_list[0x0273] = 30;
	} else if (packetver >= 20060306) {
		length_list[0x0273] = 6;
	}

	// Packet: 0x0274
	if (packetver >= 20060306) {
		length_list[0x0274] = 8;
	}

	// Packet: 0x0277
	if (packetver >= 20060424) {
		length_list[0x0277] = 84;
	}

	// Packet: 0x0278
	if (packetver >= 20060607) {
		length_list[0x0278] = 2;
	}

	// Packet: 0x0279
	if (packetver >= 20060607) {
		length_list[0x0279] = 2;
	}

	// Packet: 0x027a
	if (packetver >= 20060607) {
		length_list[0x027a] = -1;
	}

	// Packet: 0x027b
	if (packetver >= 20060626) {
		length_list[0x027b] = 14;
	}

	// Packet: 0x027c
	if (packetver >= 20060626) {
		length_list[0x027c] = 60;
	}

	// Packet: 0x027d
	if (packetver >= 20060703) {
		length_list[0x027d] = 62;
	}

	// Packet: 0x027e
	if (packetver >= 20060710) {
		length_list[0x027e] = -1;
	}

	// Packet: 0x027f
	if (packetver >= 20060821) {
		length_list[0x027f] = 8;
	} else if (packetver >= 20060731) {
		length_list[0x027f] = 4;
	}

	// Packet: 0x0280
	if (packetver >= 20060918) {
		length_list[0x0280] = 12;
	} else if (packetver >= 20060904) {
		length_list[0x0280] = 10;
	}

	// Packet: 0x0281
	if (packetver >= 20060918) {
		length_list[0x0281] = 4;
	} else if (packetver >= 20060904) {
		length_list[0x0281] = 2;
	}

	// Packet: 0x0282
	if (packetver >= 20060918) {
		length_list[0x0282] = 284;
	} else if (packetver >= 20060904) {
		length_list[0x0282] = 282;
	}

	// Packet: 0x0283
	if (packetver >= 20060904) {
		length_list[0x0283] = 6;
	}

	// Packet: 0x0284
	if (packetver >= 20060911) {
		length_list[0x0284] = 14;
	}

	// Packet: 0x0285
	if (packetver >= 20060918) {
		length_list[0x0285] = 6;
	}

	// Packet: 0x0286
	if (packetver >= 20060918) {
		length_list[0x0286] = 4;
	}

	// Packet: 0x028a
	if (packetver >= 20060927) {
		length_list[0x028a] = 18;
	}

	// Packet: 0x028b
	if (packetver >= 20061030) {
		length_list[0x028b] = -1;
	}

	// Packet: 0x028c
	if (packetver >= 20061030) {
		length_list[0x028c] = 46;
	}

	// Packet: 0x028d
	if (packetver >= 20061030) {
		length_list[0x028d] = 34;
	}

	// Packet: 0x028e
	if (packetver >= 20061030) {
		length_list[0x028e] = 4;
	}

	// Packet: 0x028f
	if (packetver >= 20061030) {
		length_list[0x028f] = 6;
	}

	// Packet: 0x0290
	if (packetver >= 20061030) {
		length_list[0x0290] = 4;
	}

	// Packet: 0x0291
	if (packetver >= 20061030) {
		length_list[0x0291] = 4;
	}

	// Packet: 0x0292
	if (packetver >= 20061030) {
		length_list[0x0292] = 2;
	}

	// Packet: 0x0293
	if (packetver >= 20061204) {
		length_list[0x0293] = 70;
	} else if (packetver >= 20061030) {
		length_list[0x0293] = 66;
	}

	// Packet: 0x0294
	if (packetver >= 20061204) {
		length_list[0x0294] = 10;
	}

	// Packet: 0x0295
	if (packetver >= 20061218) {
		length_list[0x0295] = -1;
	} else if (packetver >= 20061211) {
		length_list[0x0295] = 4;
	}

	// Packet: 0x0296
	if (packetver >= 20061218) {
		length_list[0x0296] = -1;
	} else if (packetver >= 20061211) {
		length_list[0x0296] = 4;
	}

	// Packet: 0x0297
	if (packetver >= 20061218) {
		length_list[0x0297] = -1;
	} else if (packetver >= 20061211) {
		length_list[0x0297] = 4;
	}

	// Packet: 0x0298
	if (packetver >= 20061211) {
		length_list[0x0298] = 8;
	}

	// Packet: 0x0299
	if (packetver >= 20061211) {
		length_list[0x0299] = 6;
	}

	// Packet: 0x029a
	if (packetver >= 20061218) {
		length_list[0x029a] = 27;
	}

	// Packet: 0x029b
	if (packetver >= 20061226) {
		length_list[0x029b] = 70;
	}

	// Packet: 0x029c
	if (packetver >= 20061226) {
		length_list[0x029c] = 66;
	}

	// Packet: 0x029d
	if (packetver >= 20061226) {
		length_list[0x029d] = -1;
	}

	// Packet: 0x029e
	if (packetver >= 20061226) {
		length_list[0x029e] = 11;
	}

	// Packet: 0x029f
	if (packetver >= 20061226) {
		length_list[0x029f] = 3;
	}

	// Packet: 0x02a0
	if (packetver >= 20061226) {
		length_list[0x02a0] = 10;
	}

	// Packet: 0x02a1
	if (packetver >= 20061226) {
		length_list[0x02a1] = 4;
	}

	// Packet: 0x02a2
	if (packetver >= 20061226) {
		length_list[0x02a2] = 8;
	}

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
