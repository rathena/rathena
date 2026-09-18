/**
 * Network/Packets/packets2023_len_main.js
 *
 * Network Packet Length
 * Manage packets length
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * Thanks to Andrei Karas (4144)
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

	// Packet: 0x006A
	length_list[0x006a] = 23;

	// Packet: 0x006B
	length_list[0x006b] = -1;

	// Packet: 0x006C
	length_list[0x006c] = 3;

	// Packet: 0x006D
	length_list[0x006d] = 157;

	// Packet: 0x006E
	length_list[0x006e] = 3;

	// Packet: 0x006F
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

	// Packet: 0x007A
	length_list[0x007a] = 58;

	// Packet: 0x007B
	length_list[0x007b] = 60;

	// Packet: 0x007C
	length_list[0x007c] = 44;

	// Packet: 0x007D
	length_list[0x007d] = 2;

	// Packet: 0x007E
	length_list[0x007e] = 46;

	// Packet: 0x007F
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

	// Packet: 0x008A
	length_list[0x008a] = 29;

	// Packet: 0x008B
	length_list[0x008b] = 23;

	// Packet: 0x008C
	length_list[0x008c] = 14;

	// Packet: 0x008D
	length_list[0x008d] = -1;

	// Packet: 0x008E
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

	// Packet: 0x009A
	length_list[0x009a] = -1;

	// Packet: 0x009B
	length_list[0x009b] = 34;

	// Packet: 0x009C
	length_list[0x009c] = 9;

	// Packet: 0x009D
	length_list[0x009d] = 19;

	// Packet: 0x009E
	length_list[0x009e] = 19;

	// Packet: 0x009F
	length_list[0x009f] = 20;

	// Packet: 0x00A0
	length_list[0x00a0] = 33;

	// Packet: 0x00A1
	length_list[0x00a1] = 6;

	// Packet: 0x00A2
	length_list[0x00a2] = 14;

	// Packet: 0x00A3
	length_list[0x00a3] = -1;

	// Packet: 0x00A4
	length_list[0x00a4] = -1;

	// Packet: 0x00A5
	length_list[0x00a5] = -1;

	// Packet: 0x00A6
	length_list[0x00a6] = -1;

	// Packet: 0x00A7
	length_list[0x00a7] = 9;

	// Packet: 0x00A8
	length_list[0x00a8] = 7;

	// Packet: 0x00A9
	length_list[0x00a9] = 6;

	// Packet: 0x00AA
	length_list[0x00aa] = 9;

	// Packet: 0x00AB
	length_list[0x00ab] = 4;

	// Packet: 0x00AC
	length_list[0x00ac] = 7;

	// Packet: 0x00AE
	length_list[0x00ae] = -1;

	// Packet: 0x00AF
	length_list[0x00af] = 6;

	// Packet: 0x00B0
	length_list[0x00b0] = 8;

	// Packet: 0x00B1
	length_list[0x00b1] = 8;

	// Packet: 0x00B2
	length_list[0x00b2] = 3;

	// Packet: 0x00B3
	length_list[0x00b3] = 3;

	// Packet: 0x00B4
	length_list[0x00b4] = -1;

	// Packet: 0x00B5
	length_list[0x00b5] = 6;

	// Packet: 0x00B6
	length_list[0x00b6] = 6;

	// Packet: 0x00B7
	length_list[0x00b7] = -1;

	// Packet: 0x00B8
	length_list[0x00b8] = 7;

	// Packet: 0x00B9
	length_list[0x00b9] = 6;

	// Packet: 0x00BA
	length_list[0x00ba] = 2;

	// Packet: 0x00BB
	length_list[0x00bb] = 5;

	// Packet: 0x00BC
	length_list[0x00bc] = 6;

	// Packet: 0x00BD
	length_list[0x00bd] = 44;

	// Packet: 0x00BE
	length_list[0x00be] = 5;

	// Packet: 0x00BF
	length_list[0x00bf] = 3;

	// Packet: 0x00C0
	length_list[0x00c0] = 7;

	// Packet: 0x00C1
	length_list[0x00c1] = 2;

	// Packet: 0x00C2
	length_list[0x00c2] = 6;

	// Packet: 0x00C3
	length_list[0x00c3] = 8;

	// Packet: 0x00C4
	length_list[0x00c4] = 6;

	// Packet: 0x00C5
	length_list[0x00c5] = 7;

	// Packet: 0x00C6
	length_list[0x00c6] = -1;

	// Packet: 0x00C7
	length_list[0x00c7] = -1;

	// Packet: 0x00C8
	length_list[0x00c8] = -1;

	// Packet: 0x00C9
	length_list[0x00c9] = -1;

	// Packet: 0x00CA
	length_list[0x00ca] = 3;

	// Packet: 0x00CB
	length_list[0x00cb] = 3;

	// Packet: 0x00CC
	length_list[0x00cc] = 6;

	// Packet: 0x00CD
	length_list[0x00cd] = 3;

	// Packet: 0x00CE
	length_list[0x00ce] = 2;

	// Packet: 0x00CF
	length_list[0x00cf] = 27;

	// Packet: 0x00D0
	length_list[0x00d0] = 3;

	// Packet: 0x00D1
	length_list[0x00d1] = 4;

	// Packet: 0x00D2
	length_list[0x00d2] = 4;

	// Packet: 0x00D3
	length_list[0x00d3] = 2;

	// Packet: 0x00D4
	length_list[0x00d4] = -1;

	// Packet: 0x00D5
	length_list[0x00d5] = -1;

	// Packet: 0x00D6
	length_list[0x00d6] = 3;

	// Packet: 0x00D7
	length_list[0x00d7] = -1;

	// Packet: 0x00D8
	length_list[0x00d8] = 6;

	// Packet: 0x00D9
	length_list[0x00d9] = 14;

	// Packet: 0x00DA
	length_list[0x00da] = 3;

	// Packet: 0x00DB
	length_list[0x00db] = -1;

	// Packet: 0x00DC
	length_list[0x00dc] = 28;

	// Packet: 0x00DD
	length_list[0x00dd] = 29;

	// Packet: 0x00DE
	length_list[0x00de] = -1;

	// Packet: 0x00DF
	length_list[0x00df] = -1;

	// Packet: 0x00E0
	length_list[0x00e0] = 30;

	// Packet: 0x00E1
	length_list[0x00e1] = 30;

	// Packet: 0x00E2
	length_list[0x00e2] = 26;

	// Packet: 0x00E3
	length_list[0x00e3] = 2;

	// Packet: 0x00E4
	length_list[0x00e4] = 6;

	// Packet: 0x00E5
	length_list[0x00e5] = 26;

	// Packet: 0x00E6
	length_list[0x00e6] = 3;

	// Packet: 0x00E7
	length_list[0x00e7] = 3;

	// Packet: 0x00E8
	length_list[0x00e8] = 8;

	// Packet: 0x00E9
	length_list[0x00e9] = 29;

	// Packet: 0x00EA
	length_list[0x00ea] = 5;

	// Packet: 0x00EB
	length_list[0x00eb] = 2;

	// Packet: 0x00EC
	length_list[0x00ec] = 3;

	// Packet: 0x00ED
	length_list[0x00ed] = 2;

	// Packet: 0x00EE
	length_list[0x00ee] = 2;

	// Packet: 0x00EF
	length_list[0x00ef] = 2;

	// Packet: 0x00F0
	length_list[0x00f0] = 3;

	// Packet: 0x00F1
	length_list[0x00f1] = 2;

	// Packet: 0x00F2
	length_list[0x00f2] = 6;

	// Packet: 0x00F3
	length_list[0x00f3] = -1;

	// Packet: 0x00F4
	length_list[0x00f4] = 31;

	// Packet: 0x00F5
	length_list[0x00f5] = 11;

	// Packet: 0x00F6
	length_list[0x00f6] = 8;

	// Packet: 0x00F7
	length_list[0x00f7] = 17;

	// Packet: 0x00F8
	length_list[0x00f8] = 2;

	// Packet: 0x00F9
	length_list[0x00f9] = 26;

	// Packet: 0x00FA
	length_list[0x00fa] = 3;

	// Packet: 0x00FB
	length_list[0x00fb] = -1;

	// Packet: 0x00FC
	length_list[0x00fc] = 6;

	// Packet: 0x00FD
	length_list[0x00fd] = 27;

	// Packet: 0x00FE
	length_list[0x00fe] = 30;

	// Packet: 0x00FF
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

	// Packet: 0x010A
	length_list[0x010a] = 6;

	// Packet: 0x010B
	length_list[0x010b] = 6;

	// Packet: 0x010C
	length_list[0x010c] = 6;

	// Packet: 0x010D
	length_list[0x010d] = 2;

	// Packet: 0x010E
	length_list[0x010e] = 11;

	// Packet: 0x010F
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

	// Packet: 0x011A
	length_list[0x011a] = 15;

	// Packet: 0x011B
	length_list[0x011b] = 20;

	// Packet: 0x011C
	length_list[0x011c] = 68;

	// Packet: 0x011D
	length_list[0x011d] = 2;

	// Packet: 0x011E
	length_list[0x011e] = 3;

	// Packet: 0x011F
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

	// Packet: 0x012A
	length_list[0x012a] = 2;

	// Packet: 0x012B
	length_list[0x012b] = 2;

	// Packet: 0x012C
	length_list[0x012c] = 3;

	// Packet: 0x012D
	length_list[0x012d] = 4;

	// Packet: 0x012E
	length_list[0x012e] = 2;

	// Packet: 0x012F
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

	// Packet: 0x013A
	length_list[0x013a] = 4;

	// Packet: 0x013B
	length_list[0x013b] = 4;

	// Packet: 0x013C
	length_list[0x013c] = 4;

	// Packet: 0x013D
	length_list[0x013d] = 6;

	// Packet: 0x013E
	length_list[0x013e] = 24;

	// Packet: 0x013F
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

	// Packet: 0x014A
	length_list[0x014a] = 6;

	// Packet: 0x014B
	length_list[0x014b] = 27;

	// Packet: 0x014C
	length_list[0x014c] = -1;

	// Packet: 0x014D
	length_list[0x014d] = 2;

	// Packet: 0x014E
	length_list[0x014e] = 6;

	// Packet: 0x014F
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

	// Packet: 0x015A
	length_list[0x015a] = 66;

	// Packet: 0x015B
	length_list[0x015b] = 54;

	// Packet: 0x015C
	length_list[0x015c] = 90;

	// Packet: 0x015D
	length_list[0x015d] = 42;

	// Packet: 0x015E
	length_list[0x015e] = 6;

	// Packet: 0x015F
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

	// Packet: 0x016A
	length_list[0x016a] = 30;

	// Packet: 0x016B
	length_list[0x016b] = 10;

	// Packet: 0x016C
	length_list[0x016c] = 43;

	// Packet: 0x016D
	length_list[0x016d] = 14;

	// Packet: 0x016E
	length_list[0x016e] = 186;

	// Packet: 0x016F
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

	// Packet: 0x017A
	length_list[0x017a] = 4;

	// Packet: 0x017B
	length_list[0x017b] = -1;

	// Packet: 0x017C
	length_list[0x017c] = 6;

	// Packet: 0x017D
	length_list[0x017d] = 7;

	// Packet: 0x017E
	length_list[0x017e] = -1;

	// Packet: 0x017F
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

	// Packet: 0x018A
	length_list[0x018a] = 4;

	// Packet: 0x018B
	length_list[0x018b] = 4;

	// Packet: 0x018C
	length_list[0x018c] = 29;

	// Packet: 0x018D
	length_list[0x018d] = -1;

	// Packet: 0x018E
	length_list[0x018e] = 18;

	// Packet: 0x018F
	length_list[0x018f] = 8;

	// Packet: 0x0190
	length_list[0x0190] = 23;

	// Packet: 0x0191
	length_list[0x0191] = 27;

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

	// Packet: 0x019A
	length_list[0x019a] = 14;

	// Packet: 0x019B
	length_list[0x019b] = 10;

	// Packet: 0x019C
	length_list[0x019c] = -1;

	// Packet: 0x019D
	length_list[0x019d] = 6;

	// Packet: 0x019E
	length_list[0x019e] = 2;

	// Packet: 0x019F
	length_list[0x019f] = 6;

	// Packet: 0x01A0
	length_list[0x01a0] = 3;

	// Packet: 0x01A1
	length_list[0x01a1] = 3;

	// Packet: 0x01A2
	length_list[0x01a2] = 37;

	// Packet: 0x01A3
	length_list[0x01a3] = 7;

	// Packet: 0x01A4
	length_list[0x01a4] = 11;

	// Packet: 0x01A5
	length_list[0x01a5] = 26;

	// Packet: 0x01A6
	length_list[0x01a6] = -1;

	// Packet: 0x01A7
	length_list[0x01a7] = 4;

	// Packet: 0x01A8
	length_list[0x01a8] = 4;

	// Packet: 0x01A9
	length_list[0x01a9] = 6;

	// Packet: 0x01AA
	length_list[0x01aa] = 10;

	// Packet: 0x01AB
	length_list[0x01ab] = 12;

	// Packet: 0x01AC
	length_list[0x01ac] = 6;

	// Packet: 0x01AD
	length_list[0x01ad] = -1;

	// Packet: 0x01AE
	length_list[0x01ae] = 6;

	// Packet: 0x01AF
	length_list[0x01af] = 4;

	// Packet: 0x01B0
	length_list[0x01b0] = 11;

	// Packet: 0x01B1
	length_list[0x01b1] = 7;

	// Packet: 0x01B2
	length_list[0x01b2] = -1;

	// Packet: 0x01B3
	length_list[0x01b3] = 67;

	// Packet: 0x01B4
	length_list[0x01b4] = 12;

	// Packet: 0x01B5
	length_list[0x01b5] = 18;

	// Packet: 0x01B6
	length_list[0x01b6] = 114;

	// Packet: 0x01B7
	length_list[0x01b7] = 6;

	// Packet: 0x01B8
	length_list[0x01b8] = 3;

	// Packet: 0x01B9
	length_list[0x01b9] = 6;

	// Packet: 0x01BA
	length_list[0x01ba] = 26;

	// Packet: 0x01BB
	length_list[0x01bb] = 26;

	// Packet: 0x01BC
	length_list[0x01bc] = 26;

	// Packet: 0x01BD
	length_list[0x01bd] = 26;

	// Packet: 0x01BE
	length_list[0x01be] = 2;

	// Packet: 0x01BF
	length_list[0x01bf] = 3;

	// Packet: 0x01C0
	length_list[0x01c0] = 2;

	// Packet: 0x01C1
	length_list[0x01c1] = 14;

	// Packet: 0x01C2
	length_list[0x01c2] = 10;

	// Packet: 0x01C3
	length_list[0x01c3] = -1;

	// Packet: 0x01C4
	length_list[0x01c4] = 32;

	// Packet: 0x01C5
	length_list[0x01c5] = 32;

	// Packet: 0x01C6
	length_list[0x01c6] = 4;

	// Packet: 0x01C7
	length_list[0x01c7] = 2;

	// Packet: 0x01C8
	length_list[0x01c8] = 15;

	// Packet: 0x01C9
	length_list[0x01c9] = 97;

	// Packet: 0x01CA
	length_list[0x01ca] = 3;

	// Packet: 0x01CB
	length_list[0x01cb] = 9;

	// Packet: 0x01CC
	length_list[0x01cc] = 9;

	// Packet: 0x01CD
	length_list[0x01cd] = 30;

	// Packet: 0x01CE
	length_list[0x01ce] = 6;

	// Packet: 0x01CF
	length_list[0x01cf] = 28;

	// Packet: 0x01D0
	length_list[0x01d0] = 8;

	// Packet: 0x01D1
	length_list[0x01d1] = 14;

	// Packet: 0x01D2
	length_list[0x01d2] = 10;

	// Packet: 0x01D3
	length_list[0x01d3] = 35;

	// Packet: 0x01D4
	length_list[0x01d4] = 6;

	// Packet: 0x01D5
	length_list[0x01d5] = -1;

	// Packet: 0x01D6
	length_list[0x01d6] = 4;

	// Packet: 0x01D7
	length_list[0x01d7] = 15;

	// Packet: 0x01D8
	length_list[0x01d8] = 58;

	// Packet: 0x01D9
	length_list[0x01d9] = 57;

	// Packet: 0x01DA
	length_list[0x01da] = 64;

	// Packet: 0x01DB
	length_list[0x01db] = 2;

	// Packet: 0x01DC
	length_list[0x01dc] = -1;

	// Packet: 0x01DD
	length_list[0x01dd] = 47;

	// Packet: 0x01DE
	length_list[0x01de] = 33;

	// Packet: 0x01DF
	length_list[0x01df] = 6;

	// Packet: 0x01E0
	length_list[0x01e0] = 30;

	// Packet: 0x01E1
	length_list[0x01e1] = 8;

	// Packet: 0x01E2
	length_list[0x01e2] = 34;

	// Packet: 0x01E3
	length_list[0x01e3] = 14;

	// Packet: 0x01E4
	length_list[0x01e4] = 2;

	// Packet: 0x01E5
	length_list[0x01e5] = 6;

	// Packet: 0x01E6
	length_list[0x01e6] = 26;

	// Packet: 0x01E7
	length_list[0x01e7] = 2;

	// Packet: 0x01E8
	length_list[0x01e8] = 28;

	// Packet: 0x01E9
	length_list[0x01e9] = 81;

	// Packet: 0x01EA
	length_list[0x01ea] = 6;

	// Packet: 0x01EB
	length_list[0x01eb] = 10;

	// Packet: 0x01EC
	length_list[0x01ec] = 26;

	// Packet: 0x01ED
	length_list[0x01ed] = 2;

	// Packet: 0x01EE
	length_list[0x01ee] = -1;

	// Packet: 0x01EF
	length_list[0x01ef] = -1;

	// Packet: 0x01F0
	length_list[0x01f0] = -1;

	// Packet: 0x01F1
	length_list[0x01f1] = -1;

	// Packet: 0x01F2
	length_list[0x01f2] = 20;

	// Packet: 0x01F3
	length_list[0x01f3] = 10;

	// Packet: 0x01F4
	length_list[0x01f4] = 32;

	// Packet: 0x01F5
	length_list[0x01f5] = 9;

	// Packet: 0x01F6
	length_list[0x01f6] = 34;

	// Packet: 0x01F7
	length_list[0x01f7] = 14;

	// Packet: 0x01F8
	length_list[0x01f8] = 2;

	// Packet: 0x01F9
	length_list[0x01f9] = 6;

	// Packet: 0x01FA
	length_list[0x01fa] = 48;

	// Packet: 0x01FB
	length_list[0x01fb] = 56;

	// Packet: 0x01FC
	length_list[0x01fc] = -1;

	// Packet: 0x01FD
	length_list[0x01fd] = 25;

	// Packet: 0x01FE
	length_list[0x01fe] = 5;

	// Packet: 0x01FF
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

	// Packet: 0x020A
	length_list[0x020a] = 10;

	// Packet: 0x020D
	length_list[0x020d] = -1;

	// Packet: 0x020E
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

	// Packet: 0x021A
	length_list[0x021a] = 282;

	// Packet: 0x021B
	length_list[0x021b] = 10;

	// Packet: 0x021C
	length_list[0x021c] = 10;

	// Packet: 0x021D
	length_list[0x021d] = 6;

	// Packet: 0x021E
	length_list[0x021e] = 6;

	// Packet: 0x021F
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

	// Packet: 0x022A
	length_list[0x022a] = 62;

	// Packet: 0x022B
	length_list[0x022b] = 61;

	// Packet: 0x022C
	length_list[0x022c] = 69;

	// Packet: 0x022D
	length_list[0x022d] = 5;

	// Packet: 0x022E
	length_list[0x022e] = 73;

	// Packet: 0x022F
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

	// Packet: 0x023A
	length_list[0x023a] = 4;

	// Packet: 0x023B
	length_list[0x023b] = 36;

	// Packet: 0x023C
	length_list[0x023c] = 6;

	// Packet: 0x023D
	length_list[0x023d] = 6;

	// Packet: 0x023E
	length_list[0x023e] = 8;

	// Packet: 0x023F
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

	// Packet: 0x024A
	length_list[0x024a] = 70;

	// Packet: 0x024B
	length_list[0x024b] = 4;

	// Packet: 0x024C
	length_list[0x024c] = 8;

	// Packet: 0x024D
	length_list[0x024d] = 12;

	// Packet: 0x024E
	length_list[0x024e] = 6;

	// Packet: 0x024F
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

	// Packet: 0x025A
	length_list[0x025a] = -1;

	// Packet: 0x025B
	length_list[0x025b] = 8;

	// Packet: 0x025C
	length_list[0x025c] = 4;

	// Packet: 0x025D
	length_list[0x025d] = 6;

	// Packet: 0x025E
	length_list[0x025e] = 4;

	// Packet: 0x025F
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

	// Packet: 0x026A
	length_list[0x026a] = 4;

	// Packet: 0x026B
	length_list[0x026b] = 4;

	// Packet: 0x026C
	length_list[0x026c] = 4;

	// Packet: 0x026D
	length_list[0x026d] = 4;

	// Packet: 0x026F
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

	// Packet: 0x027A
	length_list[0x027a] = -1;

	// Packet: 0x027B
	length_list[0x027b] = 14;

	// Packet: 0x027C
	length_list[0x027c] = 60;

	// Packet: 0x027D
	length_list[0x027d] = 62;

	// Packet: 0x027E
	length_list[0x027e] = -1;

	// Packet: 0x027F
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

	// Packet: 0x028A
	length_list[0x028a] = 18;

	// Packet: 0x028B
	length_list[0x028b] = -1;

	// Packet: 0x028C
	length_list[0x028c] = 46;

	// Packet: 0x028D
	length_list[0x028d] = 34;

	// Packet: 0x028E
	length_list[0x028e] = 4;

	// Packet: 0x028F
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

	// Packet: 0x029A
	length_list[0x029a] = 37;

	// Packet: 0x029B
	length_list[0x029b] = 80;

	// Packet: 0x029C
	length_list[0x029c] = 66;

	// Packet: 0x029D
	length_list[0x029d] = -1;

	// Packet: 0x029E
	length_list[0x029e] = 11;

	// Packet: 0x029F
	length_list[0x029f] = 3;

	// Packet: 0x02A2
	length_list[0x02a2] = 8;

	// Packet: 0x02A5
	length_list[0x02a5] = 8;

	// Packet: 0x02A6
	length_list[0x02a6] = -1;

	// Packet: 0x02A7
	length_list[0x02a7] = -1;

	// Packet: 0x02AA
	length_list[0x02aa] = 4;

	// Packet: 0x02AB
	length_list[0x02ab] = 36;

	// Packet: 0x02AC
	length_list[0x02ac] = 6;

	// Packet: 0x02AD
	length_list[0x02ad] = 8;

	// Packet: 0x02B0
	length_list[0x02b0] = 85;

	// Packet: 0x02B1
	length_list[0x02b1] = -1;

	// Packet: 0x02B2
	length_list[0x02b2] = -1;

	// Packet: 0x02B3
	length_list[0x02b3] = 107;

	// Packet: 0x02B4
	length_list[0x02b4] = 6;

	// Packet: 0x02B5
	length_list[0x02b5] = -1;

	// Packet: 0x02B6
	length_list[0x02b6] = 7;

	// Packet: 0x02B7
	length_list[0x02b7] = 7;

	// Packet: 0x02B8
	length_list[0x02b8] = 32;

	// Packet: 0x02B9
	length_list[0x02b9] = 191;

	// Packet: 0x02BA
	length_list[0x02ba] = 11;

	// Packet: 0x02BB
	length_list[0x02bb] = 8;

	// Packet: 0x02BC
	length_list[0x02bc] = 6;

	// Packet: 0x02C1
	length_list[0x02c1] = -1;

	// Packet: 0x02C2
	length_list[0x02c2] = -1;

	// Packet: 0x02C4
	length_list[0x02c4] = 26;

	// Packet: 0x02C5
	length_list[0x02c5] = 30;

	// Packet: 0x02C6
	length_list[0x02c6] = 30;

	// Packet: 0x02C7
	length_list[0x02c7] = 7;

	// Packet: 0x02C8
	length_list[0x02c8] = 3;

	// Packet: 0x02C9
	length_list[0x02c9] = 3;

	// Packet: 0x02CA
	length_list[0x02ca] = 3;

	// Packet: 0x02CB
	length_list[0x02cb] = 65;

	// Packet: 0x02CC
	length_list[0x02cc] = 4;

	// Packet: 0x02CD
	length_list[0x02cd] = 71;

	// Packet: 0x02CE
	length_list[0x02ce] = 10;

	// Packet: 0x02CF
	length_list[0x02cf] = 6;

	// Packet: 0x02D0
	length_list[0x02d0] = -1;

	// Packet: 0x02D1
	length_list[0x02d1] = -1;

	// Packet: 0x02D2
	length_list[0x02d2] = -1;

	// Packet: 0x02D3
	length_list[0x02d3] = 4;

	// Packet: 0x02D4
	length_list[0x02d4] = 39;

	// Packet: 0x02D5
	length_list[0x02d5] = 2;

	// Packet: 0x02D6
	length_list[0x02d6] = 6;

	// Packet: 0x02D7
	length_list[0x02d7] = -1;

	// Packet: 0x02D8
	length_list[0x02d8] = 10;

	// Packet: 0x02D9
	length_list[0x02d9] = 10;

	// Packet: 0x02DA
	length_list[0x02da] = 3;

	// Packet: 0x02DB
	length_list[0x02db] = -1;

	// Packet: 0x02DC
	length_list[0x02dc] = -1;

	// Packet: 0x02DD
	length_list[0x02dd] = 32;

	// Packet: 0x02DE
	length_list[0x02de] = 6;

	// Packet: 0x02DF
	length_list[0x02df] = 36;

	// Packet: 0x02E0
	length_list[0x02e0] = 34;

	// Packet: 0x02E1
	length_list[0x02e1] = 33;

	// Packet: 0x02E2
	length_list[0x02e2] = 20;

	// Packet: 0x02E3
	length_list[0x02e3] = 22;

	// Packet: 0x02E4
	length_list[0x02e4] = 11;

	// Packet: 0x02E5
	length_list[0x02e5] = 9;

	// Packet: 0x02E6
	length_list[0x02e6] = 6;

	// Packet: 0x02E7
	length_list[0x02e7] = -1;

	// Packet: 0x02E8
	length_list[0x02e8] = -1;

	// Packet: 0x02E9
	length_list[0x02e9] = -1;

	// Packet: 0x02EA
	length_list[0x02ea] = -1;

	// Packet: 0x02EB
	length_list[0x02eb] = 13;

	// Packet: 0x02EC
	length_list[0x02ec] = 71;

	// Packet: 0x02ED
	length_list[0x02ed] = 63;

	// Packet: 0x02EE
	length_list[0x02ee] = 64;

	// Packet: 0x02EF
	length_list[0x02ef] = 8;

	// Packet: 0x02F0
	length_list[0x02f0] = 10;

	// Packet: 0x02F1
	length_list[0x02f1] = 2;

	// Packet: 0x02F2
	length_list[0x02f2] = 2;

	// Packet: 0x02F3
	length_list[0x02f3] = -1;

	// Packet: 0x02F4
	length_list[0x02f4] = 3;

	// Packet: 0x02F5
	length_list[0x02f5] = 7;

	// Packet: 0x02F6
	length_list[0x02f6] = 7;

	// Packet: 0x02F7
	length_list[0x02f7] = 47;

	// Packet: 0x035C
	length_list[0x035c] = 2;

	// Packet: 0x035D
	length_list[0x035d] = -1;

	// Packet: 0x035E
	length_list[0x035e] = 2;

	// Packet: 0x035F
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
	length_list[0x0367] = 31;

	// Packet: 0x0368
	length_list[0x0368] = 6;

	// Packet: 0x0369
	length_list[0x0369] = 6;

	// Packet: 0x03DD
	length_list[0x03dd] = 18;

	// Packet: 0x03DE
	length_list[0x03de] = 18;

	// Packet: 0x0436
	length_list[0x0436] = 23;

	// Packet: 0x0437
	length_list[0x0437] = 7;

	// Packet: 0x0438
	length_list[0x0438] = 10;

	// Packet: 0x0439
	length_list[0x0439] = 8;

	// Packet: 0x043D
	length_list[0x043d] = 8;

	// Packet: 0x043E
	length_list[0x043e] = -1;

	// Packet: 0x043F
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

	// Packet: 0x044A
	length_list[0x044a] = 6;

	// Packet: 0x044B
	length_list[0x044b] = 2;

	// Packet: 0x07D7
	length_list[0x07d7] = 8;

	// Packet: 0x07D8
	length_list[0x07d8] = 8;

	// Packet: 0x07D9
	length_list[0x07d9] = 268;

	// Packet: 0x07DA
	length_list[0x07da] = 6;

	// Packet: 0x07DB
	length_list[0x07db] = 8;

	// Packet: 0x07DC
	length_list[0x07dc] = 6;

	// Packet: 0x07DD
	length_list[0x07dd] = 54;

	// Packet: 0x07DE
	length_list[0x07de] = 30;

	// Packet: 0x07DF
	length_list[0x07df] = 54;

	// Packet: 0x07E0
	length_list[0x07e0] = 58;

	// Packet: 0x07E1
	length_list[0x07e1] = 15;

	// Packet: 0x07E2
	length_list[0x07e2] = 8;

	// Packet: 0x07E3
	length_list[0x07e3] = 6;

	// Packet: 0x07E4
	length_list[0x07e4] = -1;

	// Packet: 0x07E5
	length_list[0x07e5] = 4;

	// Packet: 0x07E6
	length_list[0x07e6] = 8;

	// Packet: 0x07E7
	length_list[0x07e7] = 32;

	// Packet: 0x07E8
	length_list[0x07e8] = -1;

	// Packet: 0x07E9
	length_list[0x07e9] = 5;

	// Packet: 0x07EA
	length_list[0x07ea] = 2;

	// Packet: 0x07EB
	length_list[0x07eb] = -1;

	// Packet: 0x07EC
	length_list[0x07ec] = 8;

	// Packet: 0x07ED
	length_list[0x07ed] = 10;

	// Packet: 0x07EE
	length_list[0x07ee] = 6;

	// Packet: 0x07EF
	length_list[0x07ef] = 8;

	// Packet: 0x07F0
	length_list[0x07f0] = 6;

	// Packet: 0x07F1
	length_list[0x07f1] = 18;

	// Packet: 0x07F2
	length_list[0x07f2] = 8;

	// Packet: 0x07F3
	length_list[0x07f3] = 6;

	// Packet: 0x07F4
	length_list[0x07f4] = 3;

	// Packet: 0x07F5
	length_list[0x07f5] = 6;

	// Packet: 0x07F6
	length_list[0x07f6] = 14;

	// Packet: 0x07F7
	length_list[0x07f7] = -1;

	// Packet: 0x07F8
	length_list[0x07f8] = -1;

	// Packet: 0x07F9
	length_list[0x07f9] = -1;

	// Packet: 0x07FA
	length_list[0x07fa] = 8;

	// Packet: 0x07FB
	length_list[0x07fb] = 25;

	// Packet: 0x07FC
	length_list[0x07fc] = 10;

	// Packet: 0x07FD
	length_list[0x07fd] = -1;

	// Packet: 0x07FE
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

	// Packet: 0x080A
	length_list[0x080a] = 18;

	// Packet: 0x080B
	length_list[0x080b] = 6;

	// Packet: 0x080C
	length_list[0x080c] = 2;

	// Packet: 0x080D
	length_list[0x080d] = 3;

	// Packet: 0x080E
	length_list[0x080e] = 14;

	// Packet: 0x080F
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

	// Packet: 0x081A
	length_list[0x081a] = 4;

	// Packet: 0x081B
	length_list[0x081b] = 12;

	// Packet: 0x081C
	length_list[0x081c] = 10;

	// Packet: 0x081D
	length_list[0x081d] = 22;

	// Packet: 0x081E
	length_list[0x081e] = 8;

	// Packet: 0x081F
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

	// Packet: 0x082A
	length_list[0x082a] = 10;

	// Packet: 0x082B
	length_list[0x082b] = 6;

	// Packet: 0x082C
	length_list[0x082c] = 10;

	// Packet: 0x082D
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

	// Packet: 0x083A
	length_list[0x083a] = 5;

	// Packet: 0x083B
	length_list[0x083b] = 2;

	// Packet: 0x083C
	length_list[0x083c] = 14;

	// Packet: 0x083D
	length_list[0x083d] = 6;

	// Packet: 0x083E
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

	// Packet: 0x084A
	length_list[0x084a] = 2;

	// Packet: 0x084B
	length_list[0x084b] = 21;

	// Packet: 0x084C
	length_list[0x084c] = 10;

	// Packet: 0x084D
	length_list[0x084d] = 10;

	// Packet: 0x084E
	length_list[0x084e] = 5;

	// Packet: 0x084F
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

	// Packet: 0x085A
	length_list[0x085a] = 2;

	// Packet: 0x085B
	length_list[0x085b] = 2;

	// Packet: 0x085C
	length_list[0x085c] = 2;

	// Packet: 0x085D
	length_list[0x085d] = 2;

	// Packet: 0x085E
	length_list[0x085e] = 2;

	// Packet: 0x085F
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

	// Packet: 0x086A
	length_list[0x086a] = 2;

	// Packet: 0x086B
	length_list[0x086b] = 2;

	// Packet: 0x086C
	length_list[0x086c] = 2;

	// Packet: 0x086D
	length_list[0x086d] = 2;

	// Packet: 0x086E
	length_list[0x086e] = 2;

	// Packet: 0x086F
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

	// Packet: 0x087A
	length_list[0x087a] = 2;

	// Packet: 0x087B
	length_list[0x087b] = 2;

	// Packet: 0x087C
	length_list[0x087c] = 2;

	// Packet: 0x087D
	length_list[0x087d] = 2;

	// Packet: 0x087E
	length_list[0x087e] = 2;

	// Packet: 0x087F
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

	// Packet: 0x088A
	length_list[0x088a] = 2;

	// Packet: 0x088B
	length_list[0x088b] = 2;

	// Packet: 0x088C
	length_list[0x088c] = 2;

	// Packet: 0x088D
	length_list[0x088d] = 2;

	// Packet: 0x088E
	length_list[0x088e] = 2;

	// Packet: 0x088F
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

	// Packet: 0x089A
	length_list[0x089a] = 2;

	// Packet: 0x089B
	length_list[0x089b] = 2;

	// Packet: 0x089C
	length_list[0x089c] = 2;

	// Packet: 0x089D
	length_list[0x089d] = 2;

	// Packet: 0x089E
	length_list[0x089e] = 2;

	// Packet: 0x089F
	length_list[0x089f] = 2;

	// Packet: 0x08A0
	length_list[0x08a0] = 2;

	// Packet: 0x08A1
	length_list[0x08a1] = 2;

	// Packet: 0x08A2
	length_list[0x08a2] = 2;

	// Packet: 0x08A3
	length_list[0x08a3] = 2;

	// Packet: 0x08A4
	length_list[0x08a4] = 2;

	// Packet: 0x08A5
	length_list[0x08a5] = 2;

	// Packet: 0x08A6
	length_list[0x08a6] = 2;

	// Packet: 0x08A7
	length_list[0x08a7] = 2;

	// Packet: 0x08A8
	length_list[0x08a8] = 2;

	// Packet: 0x08A9
	length_list[0x08a9] = 2;

	// Packet: 0x08AA
	length_list[0x08aa] = 2;

	// Packet: 0x08AB
	length_list[0x08ab] = 2;

	// Packet: 0x08AC
	length_list[0x08ac] = 2;

	// Packet: 0x08AD
	length_list[0x08ad] = 2;

	// Packet: 0x08AF
	length_list[0x08af] = 10;

	// Packet: 0x08B0
	length_list[0x08b0] = 17;

	// Packet: 0x08B1
	length_list[0x08b1] = -1;

	// Packet: 0x08B2
	length_list[0x08b2] = -1;

	// Packet: 0x08B3
	length_list[0x08b3] = -1;

	// Packet: 0x08B4
	length_list[0x08b4] = 2;

	// Packet: 0x08B5
	length_list[0x08b5] = 6;

	// Packet: 0x08B6
	length_list[0x08b6] = 3;

	// Packet: 0x08B8
	length_list[0x08b8] = 10;

	// Packet: 0x08B9
	length_list[0x08b9] = 12;

	// Packet: 0x08BA
	length_list[0x08ba] = 10;

	// Packet: 0x08BB
	length_list[0x08bb] = 8;

	// Packet: 0x08BC
	length_list[0x08bc] = 10;

	// Packet: 0x08BD
	length_list[0x08bd] = 8;

	// Packet: 0x08BE
	length_list[0x08be] = 14;

	// Packet: 0x08BF
	length_list[0x08bf] = 8;

	// Packet: 0x08C0
	length_list[0x08c0] = -1;

	// Packet: 0x08C1
	length_list[0x08c1] = 2;

	// Packet: 0x08C2
	length_list[0x08c2] = 2;

	// Packet: 0x08C3
	length_list[0x08c3] = 10;

	// Packet: 0x08C4
	length_list[0x08c4] = 8;

	// Packet: 0x08C5
	length_list[0x08c5] = 6;

	// Packet: 0x08C6
	length_list[0x08c6] = 4;

	// Packet: 0x08C7
	length_list[0x08c7] = -1;

	// Packet: 0x08C8
	length_list[0x08c8] = 34;

	// Packet: 0x08C9
	length_list[0x08c9] = 2;

	// Packet: 0x08CA
	length_list[0x08ca] = -1;

	// Packet: 0x08CB
	length_list[0x08cb] = -1;

	// Packet: 0x08CC
	length_list[0x08cc] = 109;

	// Packet: 0x08CD
	length_list[0x08cd] = 10;

	// Packet: 0x08CE
	length_list[0x08ce] = 2;

	// Packet: 0x08CF
	length_list[0x08cf] = 10;

	// Packet: 0x08D0
	length_list[0x08d0] = 9;

	// Packet: 0x08D1
	length_list[0x08d1] = 7;

	// Packet: 0x08D2
	length_list[0x08d2] = 10;

	// Packet: 0x08D3
	length_list[0x08d3] = 10;

	// Packet: 0x08D4
	length_list[0x08d4] = 8;

	// Packet: 0x08D5
	length_list[0x08d5] = -1;

	// Packet: 0x08D6
	length_list[0x08d6] = 6;

	// Packet: 0x08D7
	length_list[0x08d7] = 28;

	// Packet: 0x08D8
	length_list[0x08d8] = 27;

	// Packet: 0x08D9
	length_list[0x08d9] = 30;

	// Packet: 0x08DA
	length_list[0x08da] = 26;

	// Packet: 0x08DB
	length_list[0x08db] = 27;

	// Packet: 0x08DC
	length_list[0x08dc] = 26;

	// Packet: 0x08DD
	length_list[0x08dd] = 27;

	// Packet: 0x08DE
	length_list[0x08de] = 27;

	// Packet: 0x08DF
	length_list[0x08df] = 50;

	// Packet: 0x08E0
	length_list[0x08e0] = 51;

	// Packet: 0x08E1
	length_list[0x08e1] = 51;

	// Packet: 0x08E2
	length_list[0x08e2] = 27;

	// Packet: 0x08E3
	length_list[0x08e3] = 157;

	// Packet: 0x08E4
	length_list[0x08e4] = 6;

	// Packet: 0x08FC
	length_list[0x08fc] = 30;

	// Packet: 0x08FD
	length_list[0x08fd] = 6;

	// Packet: 0x08FE
	length_list[0x08fe] = -1;

	// Packet: 0x08FF
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

	// Packet: 0x090A
	length_list[0x090a] = 26;

	// Packet: 0x090D
	length_list[0x090d] = -1;

	// Packet: 0x090E
	length_list[0x090e] = 2;

	// Packet: 0x090F
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

	// Packet: 0x091A
	length_list[0x091a] = 2;

	// Packet: 0x091B
	length_list[0x091b] = 2;

	// Packet: 0x091C
	length_list[0x091c] = 2;

	// Packet: 0x091D
	length_list[0x091d] = 2;

	// Packet: 0x091E
	length_list[0x091e] = 2;

	// Packet: 0x091F
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

	// Packet: 0x092A
	length_list[0x092a] = 2;

	// Packet: 0x092B
	length_list[0x092b] = 2;

	// Packet: 0x092C
	length_list[0x092c] = 2;

	// Packet: 0x092D
	length_list[0x092d] = 2;

	// Packet: 0x092E
	length_list[0x092e] = 2;

	// Packet: 0x092F
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

	// Packet: 0x093A
	length_list[0x093a] = 2;

	// Packet: 0x093B
	length_list[0x093b] = 2;

	// Packet: 0x093C
	length_list[0x093c] = 2;

	// Packet: 0x093D
	length_list[0x093d] = 2;

	// Packet: 0x093E
	length_list[0x093e] = 2;

	// Packet: 0x093F
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

	// Packet: 0x094A
	length_list[0x094a] = 2;

	// Packet: 0x094B
	length_list[0x094b] = 2;

	// Packet: 0x094C
	length_list[0x094c] = 2;

	// Packet: 0x094D
	length_list[0x094d] = 2;

	// Packet: 0x094E
	length_list[0x094e] = 2;

	// Packet: 0x094F
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

	// Packet: 0x095A
	length_list[0x095a] = 2;

	// Packet: 0x095B
	length_list[0x095b] = 2;

	// Packet: 0x095C
	length_list[0x095c] = 2;

	// Packet: 0x095D
	length_list[0x095d] = 2;

	// Packet: 0x095E
	length_list[0x095e] = 2;

	// Packet: 0x095F
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

	// Packet: 0x096A
	length_list[0x096a] = 2;

	// Packet: 0x096B
	length_list[0x096b] = 4;

	// Packet: 0x096C
	length_list[0x096c] = 6;

	// Packet: 0x096D
	length_list[0x096d] = -1;

	// Packet: 0x096E
	length_list[0x096e] = -1;

	// Packet: 0x096F
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

	// Packet: 0x097A
	length_list[0x097a] = -1;

	// Packet: 0x097B
	length_list[0x097b] = -1;

	// Packet: 0x097C
	length_list[0x097c] = 4;

	// Packet: 0x097D
	length_list[0x097d] = 288;

	// Packet: 0x097E
	length_list[0x097e] = 12;

	// Packet: 0x097F
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

	// Packet: 0x098A
	length_list[0x098a] = -1;

	// Packet: 0x098B
	length_list[0x098b] = 2;

	// Packet: 0x098C
	length_list[0x098c] = 4;

	// Packet: 0x098D
	length_list[0x098d] = -1;

	// Packet: 0x098E
	length_list[0x098e] = -1;

	// Packet: 0x098F
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

	// Packet: 0x099A
	length_list[0x099a] = 9;

	// Packet: 0x099B
	length_list[0x099b] = 8;

	// Packet: 0x099C
	length_list[0x099c] = 6;

	// Packet: 0x099D
	length_list[0x099d] = -1;

	// Packet: 0x099E
	length_list[0x099e] = 12;

	// Packet: 0x099F
	length_list[0x099f] = -1;

	// Packet: 0x09A0
	length_list[0x09a0] = 6;

	// Packet: 0x09A1
	length_list[0x09a1] = 2;

	// Packet: 0x09A2
	length_list[0x09a2] = 6;

	// Packet: 0x09A3
	length_list[0x09a3] = -1;

	// Packet: 0x09A4
	length_list[0x09a4] = 18;

	// Packet: 0x09A5
	length_list[0x09a5] = 7;

	// Packet: 0x09A6
	length_list[0x09a6] = 12;

	// Packet: 0x09A7
	length_list[0x09a7] = 10;

	// Packet: 0x09A8
	length_list[0x09a8] = 16;

	// Packet: 0x09A9
	length_list[0x09a9] = 10;

	// Packet: 0x09AA
	length_list[0x09aa] = 16;

	// Packet: 0x09AB
	length_list[0x09ab] = 6;

	// Packet: 0x09AC
	length_list[0x09ac] = -1;

	// Packet: 0x09AD
	length_list[0x09ad] = 12;

	// Packet: 0x09AE
	length_list[0x09ae] = 19;

	// Packet: 0x09AF
	length_list[0x09af] = 4;

	// Packet: 0x09B0
	length_list[0x09b0] = 10;

	// Packet: 0x09B1
	length_list[0x09b1] = 4;

	// Packet: 0x09B2
	length_list[0x09b2] = 10;

	// Packet: 0x09B3
	length_list[0x09b3] = 6;

	// Packet: 0x09B4
	length_list[0x09b4] = 6;

	// Packet: 0x09B5
	length_list[0x09b5] = 2;

	// Packet: 0x09B6
	length_list[0x09b6] = 6;

	// Packet: 0x09B7
	length_list[0x09b7] = 4;

	// Packet: 0x09B8
	length_list[0x09b8] = 6;

	// Packet: 0x09B9
	length_list[0x09b9] = 4;

	// Packet: 0x09BA
	length_list[0x09ba] = 2;

	// Packet: 0x09BB
	length_list[0x09bb] = 6;

	// Packet: 0x09BC
	length_list[0x09bc] = 6;

	// Packet: 0x09BD
	length_list[0x09bd] = 2;

	// Packet: 0x09BE
	length_list[0x09be] = 2;

	// Packet: 0x09BF
	length_list[0x09bf] = 4;

	// Packet: 0x09C1
	length_list[0x09c1] = 10;

	// Packet: 0x09C2
	length_list[0x09c2] = -1;

	// Packet: 0x09C3
	length_list[0x09c3] = 10;

	// Packet: 0x09C4
	length_list[0x09c4] = 10;

	// Packet: 0x09C5
	length_list[0x09c5] = 1042;

	// Packet: 0x09C6
	length_list[0x09c6] = -1;

	// Packet: 0x09C7
	length_list[0x09c7] = 18;

	// Packet: 0x09C8
	length_list[0x09c8] = -1;

	// Packet: 0x09C9
	length_list[0x09c9] = -1;

	// Packet: 0x09CA
	length_list[0x09ca] = -1;

	// Packet: 0x09CB
	length_list[0x09cb] = 17;

	// Packet: 0x09CC
	length_list[0x09cc] = -1;

	// Packet: 0x09CD
	length_list[0x09cd] = 8;

	// Packet: 0x09CE
	length_list[0x09ce] = 102;

	// Packet: 0x09CF
	length_list[0x09cf] = -1;

	// Packet: 0x09D0
	length_list[0x09d0] = -1;

	// Packet: 0x09D1
	length_list[0x09d1] = 14;

	// Packet: 0x09D2
	length_list[0x09d2] = -1;

	// Packet: 0x09D3
	length_list[0x09d3] = -1;

	// Packet: 0x09D4
	length_list[0x09d4] = 2;

	// Packet: 0x09D5
	length_list[0x09d5] = -1;

	// Packet: 0x09D6
	length_list[0x09d6] = -1;

	// Packet: 0x09D7
	length_list[0x09d7] = -1;

	// Packet: 0x09D8
	length_list[0x09d8] = 2;

	// Packet: 0x09D9
	length_list[0x09d9] = 4;

	// Packet: 0x09DA
	length_list[0x09da] = -1;

	// Packet: 0x09DB
	length_list[0x09db] = -1;

	// Packet: 0x09DC
	length_list[0x09dc] = -1;

	// Packet: 0x09DD
	length_list[0x09dd] = -1;

	// Packet: 0x09DE
	length_list[0x09de] = -1;

	// Packet: 0x09DF
	length_list[0x09df] = 7;

	// Packet: 0x09E0
	length_list[0x09e0] = -1;

	// Packet: 0x09E1
	length_list[0x09e1] = 8;

	// Packet: 0x09E2
	length_list[0x09e2] = 8;

	// Packet: 0x09E3
	length_list[0x09e3] = 8;

	// Packet: 0x09E4
	length_list[0x09e4] = 8;

	// Packet: 0x09E5
	length_list[0x09e5] = 18;

	// Packet: 0x09E6
	length_list[0x09e6] = 24;

	// Packet: 0x09E7
	length_list[0x09e7] = 3;

	// Packet: 0x09E8
	length_list[0x09e8] = 11;

	// Packet: 0x09E9
	length_list[0x09e9] = 2;

	// Packet: 0x09EA
	length_list[0x09ea] = 11;

	// Packet: 0x09EB
	length_list[0x09eb] = -1;

	// Packet: 0x09EC
	length_list[0x09ec] = -1;

	// Packet: 0x09ED
	length_list[0x09ed] = 3;

	// Packet: 0x09EE
	length_list[0x09ee] = 11;

	// Packet: 0x09EF
	length_list[0x09ef] = 11;

	// Packet: 0x09F0
	length_list[0x09f0] = -1;

	// Packet: 0x09F1
	length_list[0x09f1] = 11;

	// Packet: 0x09F2
	length_list[0x09f2] = 12;

	// Packet: 0x09F3
	length_list[0x09f3] = 11;

	// Packet: 0x09F4
	length_list[0x09f4] = 12;

	// Packet: 0x09F5
	length_list[0x09f5] = 11;

	// Packet: 0x09F6
	length_list[0x09f6] = 11;

	// Packet: 0x09F7
	length_list[0x09f7] = 77;

	// Packet: 0x09F8
	length_list[0x09f8] = -1;

	// Packet: 0x09F9
	length_list[0x09f9] = 143;

	// Packet: 0x09FA
	length_list[0x09fa] = -1;

	// Packet: 0x09FB
	length_list[0x09fb] = -1;

	// Packet: 0x09FC
	length_list[0x09fc] = 6;

	// Packet: 0x09FD
	length_list[0x09fd] = -1;

	// Packet: 0x09FE
	length_list[0x09fe] = -1;

	// Packet: 0x09FF
	length_list[0x09ff] = -1;

	// Packet: 0x0A00
	length_list[0x0a00] = 269;

	// Packet: 0x0A01
	length_list[0x0a01] = 3;

	// Packet: 0x0A02
	length_list[0x0a02] = 4;

	// Packet: 0x0A03
	length_list[0x0a03] = 2;

	// Packet: 0x0A04
	length_list[0x0a04] = 6;

	// Packet: 0x0A05
	length_list[0x0a05] = 63;

	// Packet: 0x0A06
	length_list[0x0a06] = 6;

	// Packet: 0x0A07
	length_list[0x0a07] = 9;

	// Packet: 0x0A08
	length_list[0x0a08] = 26;

	// Packet: 0x0A09
	length_list[0x0a09] = 55;

	// Packet: 0x0A0A
	length_list[0x0a0a] = 57;

	// Packet: 0x0A0B
	length_list[0x0a0b] = 57;

	// Packet: 0x0A0C
	length_list[0x0a0c] = 66;

	// Packet: 0x0A0D
	length_list[0x0a0d] = -1;

	// Packet: 0x0A0E
	length_list[0x0a0e] = 14;

	// Packet: 0x0A0F
	length_list[0x0a0f] = -1;

	// Packet: 0x0A10
	length_list[0x0a10] = -1;

	// Packet: 0x0A11
	length_list[0x0a11] = -1;

	// Packet: 0x0A12
	length_list[0x0a12] = 27;

	// Packet: 0x0A13
	length_list[0x0a13] = 26;

	// Packet: 0x0A14
	length_list[0x0a14] = 10;

	// Packet: 0x0A15
	length_list[0x0a15] = 12;

	// Packet: 0x0A16
	length_list[0x0a16] = 26;

	// Packet: 0x0A17
	length_list[0x0a17] = 6;

	// Packet: 0x0A18
	length_list[0x0a18] = 14;

	// Packet: 0x0A19
	length_list[0x0a19] = 2;

	// Packet: 0x0A1A
	length_list[0x0a1a] = 25;

	// Packet: 0x0A1B
	length_list[0x0a1b] = 2;

	// Packet: 0x0A1C
	length_list[0x0a1c] = -1;

	// Packet: 0x0A1D
	length_list[0x0a1d] = 2;

	// Packet: 0x0A1E
	length_list[0x0a1e] = 3;

	// Packet: 0x0A1F
	length_list[0x0a1f] = 2;

	// Packet: 0x0A20
	length_list[0x0a20] = 23;

	// Packet: 0x0A21
	length_list[0x0a21] = 3;

	// Packet: 0x0A22
	length_list[0x0a22] = 7;

	// Packet: 0x0A23
	length_list[0x0a23] = -1;

	// Packet: 0x0A24
	length_list[0x0a24] = 66;

	// Packet: 0x0A25
	length_list[0x0a25] = 6;

	// Packet: 0x0A26
	length_list[0x0a26] = 7;

	// Packet: 0x0A27
	length_list[0x0a27] = 8;

	// Packet: 0x0A28
	length_list[0x0a28] = 3;

	// Packet: 0x0A29
	length_list[0x0a29] = 6;

	// Packet: 0x0A2A
	length_list[0x0a2a] = 6;

	// Packet: 0x0A2B
	length_list[0x0a2b] = 14;

	// Packet: 0x0A2C
	length_list[0x0a2c] = 12;

	// Packet: 0x0A2D
	length_list[0x0a2d] = -1;

	// Packet: 0x0A2E
	length_list[0x0a2e] = 6;

	// Packet: 0x0A2F
	length_list[0x0a2f] = 7;

	// Packet: 0x0A30
	length_list[0x0a30] = 106;

	// Packet: 0x0A31
	length_list[0x0a31] = -1;

	// Packet: 0x0A32
	length_list[0x0a32] = 2;

	// Packet: 0x0A33
	length_list[0x0a33] = 7;

	// Packet: 0x0A34
	length_list[0x0a34] = 6;

	// Packet: 0x0A35
	length_list[0x0a35] = 4;

	// Packet: 0x0A36
	length_list[0x0a36] = 7;

	// Packet: 0x0A37
	length_list[0x0a37] = 69;

	// Packet: 0x0A38
	length_list[0x0a38] = 3;

	// Packet: 0x0A39
	length_list[0x0a39] = 36;

	// Packet: 0x0A3A
	length_list[0x0a3a] = 12;

	// Packet: 0x0A3B
	length_list[0x0a3b] = -1;

	// Packet: 0x0A3C
	length_list[0x0a3c] = -1;

	// Packet: 0x0A3D
	length_list[0x0a3d] = 20;

	// Packet: 0x0A3E
	length_list[0x0a3e] = -1;

	// Packet: 0x0A3F
	length_list[0x0a3f] = 11;

	// Packet: 0x0A40
	length_list[0x0a40] = 11;

	// Packet: 0x0A41
	length_list[0x0a41] = 18;

	// Packet: 0x0A42
	length_list[0x0a42] = 43;

	// Packet: 0x0A43
	length_list[0x0a43] = 85;

	// Packet: 0x0A44
	length_list[0x0a44] = -1;

	// Packet: 0x0A46
	length_list[0x0a46] = 14;

	// Packet: 0x0A47
	length_list[0x0a47] = 3;

	// Packet: 0x0A48
	length_list[0x0a48] = 2;

	// Packet: 0x0A49
	length_list[0x0a49] = 22;

	// Packet: 0x0A4A
	length_list[0x0a4a] = 6;

	// Packet: 0x0A4B
	length_list[0x0a4b] = 22;

	// Packet: 0x0A4C
	length_list[0x0a4c] = 28;

	// Packet: 0x0A4D
	length_list[0x0a4d] = -1;

	// Packet: 0x0A4E
	length_list[0x0a4e] = 6;

	// Packet: 0x0A4F
	length_list[0x0a4f] = -1;

	// Packet: 0x0A50
	length_list[0x0a50] = 4;

	// Packet: 0x0A51
	length_list[0x0a51] = 34;

	// Packet: 0x0A52
	length_list[0x0a52] = 20;

	// Packet: 0x0A53
	length_list[0x0a53] = 10;

	// Packet: 0x0A54
	length_list[0x0a54] = -1;

	// Packet: 0x0A55
	length_list[0x0a55] = 2;

	// Packet: 0x0A56
	length_list[0x0a56] = 6;

	// Packet: 0x0A57
	length_list[0x0a57] = 6;

	// Packet: 0x0A58
	length_list[0x0a58] = 8;

	// Packet: 0x0A59
	length_list[0x0a59] = -1;

	// Packet: 0x0A5A
	length_list[0x0a5a] = 2;

	// Packet: 0x0A5B
	length_list[0x0a5b] = 7;

	// Packet: 0x0A5C
	length_list[0x0a5c] = 18;

	// Packet: 0x0A5D
	length_list[0x0a5d] = 6;

	// Packet: 0x0A5E
	length_list[0x0a5e] = 26;

	// Packet: 0x0A5F
	length_list[0x0a5f] = 8;

	// Packet: 0x0A60
	length_list[0x0a60] = 3;

	// Packet: 0x0A68
	length_list[0x0a68] = 3;

	// Packet: 0x0A69
	length_list[0x0a69] = 6;

	// Packet: 0x0A6A
	length_list[0x0a6a] = 12;

	// Packet: 0x0A6B
	length_list[0x0a6b] = -1;

	// Packet: 0x0A6C
	length_list[0x0a6c] = 7;

	// Packet: 0x0A6D
	length_list[0x0a6d] = -1;

	// Packet: 0x0A6E
	length_list[0x0a6e] = -1;

	// Packet: 0x0A6F
	length_list[0x0a6f] = -1;

	// Packet: 0x0A70
	length_list[0x0a70] = 2;

	// Packet: 0x0A71
	length_list[0x0a71] = -1;

	// Packet: 0x0A72
	length_list[0x0a72] = 61;

	// Packet: 0x0A73
	length_list[0x0a73] = 2;

	// Packet: 0x0A74
	length_list[0x0a74] = 8;

	// Packet: 0x0A76
	length_list[0x0a76] = 80;

	// Packet: 0x0A77
	length_list[0x0a77] = 15;

	// Packet: 0x0A78
	length_list[0x0a78] = 15;

	// Packet: 0x0A79
	length_list[0x0a79] = -1;

	// Packet: 0x0A7B
	length_list[0x0a7b] = -1;

	// Packet: 0x0A7C
	length_list[0x0a7c] = -1;

	// Packet: 0x0A7D
	length_list[0x0a7d] = -1;

	// Packet: 0x0A7E
	length_list[0x0a7e] = -1;

	// Packet: 0x0A7F
	length_list[0x0a7f] = -1;

	// Packet: 0x0A80
	length_list[0x0a80] = 6;

	// Packet: 0x0A81
	length_list[0x0a81] = 4;

	// Packet: 0x0A82
	length_list[0x0a82] = 46;

	// Packet: 0x0A83
	length_list[0x0a83] = 46;

	// Packet: 0x0A84
	length_list[0x0a84] = 94;

	// Packet: 0x0A85
	length_list[0x0a85] = 82;

	// Packet: 0x0A86
	length_list[0x0a86] = -1;

	// Packet: 0x0A87
	length_list[0x0a87] = -1;

	// Packet: 0x0A88
	length_list[0x0a88] = 2;

	// Packet: 0x0A89
	length_list[0x0a89] = 61;

	// Packet: 0x0A8A
	length_list[0x0a8a] = 6;

	// Packet: 0x0A8B
	length_list[0x0a8b] = 2;

	// Packet: 0x0A8C
	length_list[0x0a8c] = 2;

	// Packet: 0x0A8D
	length_list[0x0a8d] = -1;

	// Packet: 0x0A8E
	length_list[0x0a8e] = 2;

	// Packet: 0x0A8F
	length_list[0x0a8f] = 2;

	// Packet: 0x0A90
	length_list[0x0a90] = 3;

	// Packet: 0x0A91
	length_list[0x0a91] = -1;

	// Packet: 0x0A92
	length_list[0x0a92] = -1;

	// Packet: 0x0A93
	length_list[0x0a93] = 3;

	// Packet: 0x0A94
	length_list[0x0a94] = 2;

	// Packet: 0x0A95
	length_list[0x0a95] = 4;

	// Packet: 0x0A96
	length_list[0x0a96] = 61;

	// Packet: 0x0A97
	length_list[0x0a97] = 8;

	// Packet: 0x0A98
	length_list[0x0a98] = 10;

	// Packet: 0x0A99
	length_list[0x0a99] = 4;

	// Packet: 0x0A9A
	length_list[0x0a9a] = 10;

	// Packet: 0x0A9B
	length_list[0x0a9b] = -1;

	// Packet: 0x0A9C
	length_list[0x0a9c] = 2;

	// Packet: 0x0A9D
	length_list[0x0a9d] = 4;

	// Packet: 0x0A9E
	length_list[0x0a9e] = 2;

	// Packet: 0x0A9F
	length_list[0x0a9f] = 2;

	// Packet: 0x0AA0
	length_list[0x0aa0] = 2;

	// Packet: 0x0AA1
	length_list[0x0aa1] = 4;

	// Packet: 0x0AA2
	length_list[0x0aa2] = -1;

	// Packet: 0x0AA3
	length_list[0x0aa3] = 9;

	// Packet: 0x0AA4
	length_list[0x0aa4] = 2;

	// Packet: 0x0AA5
	length_list[0x0aa5] = -1;

	// Packet: 0x0AA6
	length_list[0x0aa6] = 36;

	// Packet: 0x0AA7
	length_list[0x0aa7] = 6;

	// Packet: 0x0AA8
	length_list[0x0aa8] = 5;

	// Packet: 0x0AA9
	length_list[0x0aa9] = -1;

	// Packet: 0x0AAA
	length_list[0x0aaa] = -1;

	// Packet: 0x0AAB
	length_list[0x0aab] = -1;

	// Packet: 0x0AAC
	length_list[0x0aac] = 69;

	// Packet: 0x0AAD
	length_list[0x0aad] = 51;

	// Packet: 0x0AAE
	length_list[0x0aae] = 2;

	// Packet: 0x0AAF
	length_list[0x0aaf] = 6;

	// Packet: 0x0AB0
	length_list[0x0ab0] = 6;

	// Packet: 0x0AB1
	length_list[0x0ab1] = 14;

	// Packet: 0x0AB2
	length_list[0x0ab2] = 7;

	// Packet: 0x0AB3
	length_list[0x0ab3] = 19;

	// Packet: 0x0AB4
	length_list[0x0ab4] = 6;

	// Packet: 0x0AB5
	length_list[0x0ab5] = 2;

	// Packet: 0x0AB6
	length_list[0x0ab6] = 8;

	// Packet: 0x0AB7
	length_list[0x0ab7] = 4;

	// Packet: 0x0AB8
	length_list[0x0ab8] = 2;

	// Packet: 0x0AB9
	length_list[0x0ab9] = 47;

	// Packet: 0x0ABA
	length_list[0x0aba] = 2;

	// Packet: 0x0ABB
	length_list[0x0abb] = 2;

	// Packet: 0x0ABC
	length_list[0x0abc] = -1;

	// Packet: 0x0ABD
	length_list[0x0abd] = 10;

	// Packet: 0x0ABE
	length_list[0x0abe] = -1;

	// Packet: 0x0ABF
	length_list[0x0abf] = -1;

	// Packet: 0x0AC0
	length_list[0x0ac0] = 26;

	// Packet: 0x0AC1
	length_list[0x0ac1] = 26;

	// Packet: 0x0AC2
	length_list[0x0ac2] = -1;

	// Packet: 0x0AC3
	length_list[0x0ac3] = 2;

	// Packet: 0x0AC4
	length_list[0x0ac4] = -1;

	// Packet: 0x0AC5
	length_list[0x0ac5] = 156;

	// Packet: 0x0AC6
	length_list[0x0ac6] = 156;

	// Packet: 0x0AC7
	length_list[0x0ac7] = 156;

	// Packet: 0x0AC8
	length_list[0x0ac8] = 2;

	// Packet: 0x0AC9
	length_list[0x0ac9] = -1;

	// Packet: 0x0ACA
	length_list[0x0aca] = 3;

	// Packet: 0x0ACB
	length_list[0x0acb] = 12;

	// Packet: 0x0ACC
	length_list[0x0acc] = 18;

	// Packet: 0x0ACD
	length_list[0x0acd] = 23;

	// Packet: 0x0ACE
	length_list[0x0ace] = 4;

	// Packet: 0x0ACF
	length_list[0x0acf] = 68;

	// Packet: 0x0AD0
	length_list[0x0ad0] = 11;

	// Packet: 0x0AD1
	length_list[0x0ad1] = -1;

	// Packet: 0x0AD2
	length_list[0x0ad2] = 30;

	// Packet: 0x0AD3
	length_list[0x0ad3] = -1;

	// Packet: 0x0AD4
	length_list[0x0ad4] = -1;

	// Packet: 0x0AD5
	length_list[0x0ad5] = 2;

	// Packet: 0x0AD6
	length_list[0x0ad6] = 2;

	// Packet: 0x0AD7
	length_list[0x0ad7] = 8;

	// Packet: 0x0AD8
	length_list[0x0ad8] = 8;

	// Packet: 0x0AD9
	length_list[0x0ad9] = -1;

	// Packet: 0x0ADA
	length_list[0x0ada] = 32;

	// Packet: 0x0ADB
	length_list[0x0adb] = -1;

	// Packet: 0x0ADC
	length_list[0x0adc] = 6;

	// Packet: 0x0ADD
	length_list[0x0add] = 24;

	// Packet: 0x0ADE
	length_list[0x0ade] = 6;

	// Packet: 0x0ADF
	length_list[0x0adf] = 58;

	// Packet: 0x0AE0
	length_list[0x0ae0] = 30;

	// Packet: 0x0AE1
	length_list[0x0ae1] = 28;

	// Packet: 0x0AE2
	length_list[0x0ae2] = 7;

	// Packet: 0x0AE3
	length_list[0x0ae3] = -1;

	// Packet: 0x0AE4
	length_list[0x0ae4] = 89;

	// Packet: 0x0AE5
	length_list[0x0ae5] = -1;

	// Packet: 0x0AE6
	length_list[0x0ae6] = 10;

	// Packet: 0x0AE7
	length_list[0x0ae7] = 38;

	// Packet: 0x0AE8
	length_list[0x0ae8] = 2;

	// Packet: 0x0AE9
	length_list[0x0ae9] = 13;

	// Packet: 0x0AEC
	length_list[0x0aec] = 2;

	// Packet: 0x0AED
	length_list[0x0aed] = 2;

	// Packet: 0x0AEE
	length_list[0x0aee] = 2;

	// Packet: 0x0AEF
	length_list[0x0aef] = 2;

	// Packet: 0x0AF0
	length_list[0x0af0] = 10;

	// Packet: 0x0AF1
	length_list[0x0af1] = 102;

	// Packet: 0x0AF2
	length_list[0x0af2] = 40;

	// Packet: 0x0AF3
	length_list[0x0af3] = -1;

	// Packet: 0x0AF4
	length_list[0x0af4] = 11;

	// Packet: 0x0AF5
	length_list[0x0af5] = 3;

	// Packet: 0x0AF6
	length_list[0x0af6] = 88;

	// Packet: 0x0AF7
	length_list[0x0af7] = 32;

	// Packet: 0x0AF8
	length_list[0x0af8] = 11;

	// Packet: 0x0AF9
	length_list[0x0af9] = 6;

	// Packet: 0x0AFA
	length_list[0x0afa] = 58;

	// Packet: 0x0AFB
	length_list[0x0afb] = -1;

	// Packet: 0x0AFC
	length_list[0x0afc] = 16;

	// Packet: 0x0AFD
	length_list[0x0afd] = -1;

	// Packet: 0x0AFE
	length_list[0x0afe] = -1;

	// Packet: 0x0AFF
	length_list[0x0aff] = -1;

	// Packet: 0x0B00
	length_list[0x0b00] = 8;

	// Packet: 0x0B01
	length_list[0x0b01] = 56;

	// Packet: 0x0B02
	length_list[0x0b02] = 26;

	// Packet: 0x0B03
	length_list[0x0b03] = -1;

	// Packet: 0x0B04
	length_list[0x0b04] = 190;

	// Packet: 0x0B05
	length_list[0x0b05] = 63;

	// Packet: 0x0B07
	length_list[0x0b07] = -1;

	// Packet: 0x0B08
	length_list[0x0b08] = -1;

	// Packet: 0x0B09
	length_list[0x0b09] = -1;

	// Packet: 0x0B0A
	length_list[0x0b0a] = -1;

	// Packet: 0x0B0B
	length_list[0x0b0b] = 4;

	// Packet: 0x0B0C
	length_list[0x0b0c] = 155;

	// Packet: 0x0B0D
	length_list[0x0b0d] = 10;

	// Packet: 0x0B0E
	length_list[0x0b0e] = -1;

	// Packet: 0x0B0F
	length_list[0x0b0f] = -1;

	// Packet: 0x0B10
	length_list[0x0b10] = 10;

	// Packet: 0x0B11
	length_list[0x0b11] = 4;

	// Packet: 0x0B12
	length_list[0x0b12] = 2;

	// Packet: 0x0B13
	length_list[0x0b13] = 48;

	// Packet: 0x0B14
	length_list[0x0b14] = 2;

	// Packet: 0x0B15
	length_list[0x0b15] = 7;

	// Packet: 0x0B16
	length_list[0x0b16] = 2;

	// Packet: 0x0B17
	length_list[0x0b17] = 3;

	// Packet: 0x0B18
	length_list[0x0b18] = 4;

	// Packet: 0x0B19
	length_list[0x0b19] = 2;

	// Packet: 0x0B1A
	length_list[0x0b1a] = 29;

	// Packet: 0x0B1B
	length_list[0x0b1b] = 2;

	// Packet: 0x0B1C
	length_list[0x0b1c] = 2;

	// Packet: 0x0B1D
	length_list[0x0b1d] = 2;

	// Packet: 0x0B1E
	length_list[0x0b1e] = 14;

	// Packet: 0x0B1F
	length_list[0x0b1f] = 14;

	// Packet: 0x0B20
	length_list[0x0b20] = 271;

	// Packet: 0x0B21
	length_list[0x0b21] = 13;

	// Packet: 0x0B22
	length_list[0x0b22] = 5;

	// Packet: 0x0B23
	length_list[0x0b23] = 6;

	// Packet: 0x0B24
	length_list[0x0b24] = 6;

	// Packet: 0x0B25
	length_list[0x0b25] = 6;

	// Packet: 0x0B27
	length_list[0x0b27] = -1;

	// Packet: 0x0B28
	length_list[0x0b28] = 3;

	// Packet: 0x0B2B
	length_list[0x0b2b] = 11;

	// Packet: 0x0B2C
	length_list[0x0b2c] = 3;

	// Packet: 0x0B2D
	length_list[0x0b2d] = 11;

	// Packet: 0x0B2E
	length_list[0x0b2e] = 4;

	// Packet: 0x0B2F
	length_list[0x0b2f] = 73;

	// Packet: 0x0B30
	length_list[0x0b30] = -1;

	// Packet: 0x0B31
	length_list[0x0b31] = 17;

	// Packet: 0x0B32
	length_list[0x0b32] = -1;

	// Packet: 0x0B33
	length_list[0x0b33] = 17;

	// Packet: 0x0B34
	length_list[0x0b34] = 50;

	// Packet: 0x0B35
	length_list[0x0b35] = 3;

	// Packet: 0x0B36
	length_list[0x0b36] = -1;

	// Packet: 0x0B37
	length_list[0x0b37] = -1;

	// Packet: 0x0B39
	length_list[0x0b39] = -1;

	// Packet: 0x0B3C
	length_list[0x0b3c] = 4;

	// Packet: 0x0B3D
	length_list[0x0b3d] = -1;

	// Packet: 0x0B3E
	length_list[0x0b3e] = -1;

	// Packet: 0x0B3F
	length_list[0x0b3f] = 64;

	// Packet: 0x0B40
	length_list[0x0b40] = -1;

	// Packet: 0x0B41
	length_list[0x0b41] = 70;

	// Packet: 0x0B42
	length_list[0x0b42] = 62;

	// Packet: 0x0B43
	length_list[0x0b43] = 48;

	// Packet: 0x0B44
	length_list[0x0b44] = 58;

	// Packet: 0x0B45
	length_list[0x0b45] = 58;

	// Packet: 0x0B46
	length_list[0x0b46] = 10;

	// Packet: 0x0B47
	length_list[0x0b47] = 14;

	// Packet: 0x0B48
	length_list[0x0b48] = 18;

	// Packet: 0x0B49
	length_list[0x0b49] = 4;

	// Packet: 0x0B4A
	length_list[0x0b4a] = 6;

	// Packet: 0x0B4B
	length_list[0x0b4b] = 4;

	// Packet: 0x0B4C
	length_list[0x0b4c] = 2;

	// Packet: 0x0B4D
	length_list[0x0b4d] = -1;

	// Packet: 0x0B4E
	length_list[0x0b4e] = -1;

	// Packet: 0x0B4F
	length_list[0x0b4f] = 2;

	// Packet: 0x0B50
	length_list[0x0b50] = 2;

	// Packet: 0x0B51
	length_list[0x0b51] = 2;

	// Packet: 0x0B52
	length_list[0x0b52] = 2;

	// Packet: 0x0B53
	length_list[0x0b53] = 52;

	// Packet: 0x0B54
	length_list[0x0b54] = 8;

	// Packet: 0x0B55
	length_list[0x0b55] = -1;

	// Packet: 0x0B56
	length_list[0x0b56] = -1;

	// Packet: 0x0B57
	length_list[0x0b57] = -1;

	// Packet: 0x0B58
	length_list[0x0b58] = 2;

	// Packet: 0x0B59
	length_list[0x0b59] = 4;

	// Packet: 0x0B5A
	length_list[0x0b5a] = -1;

	// Packet: 0x0B5B
	length_list[0x0b5b] = 14;

	// Packet: 0x0B5C
	length_list[0x0b5c] = 2;

	// Packet: 0x0B5D
	length_list[0x0b5d] = 10;

	// Packet: 0x0B5E
	length_list[0x0b5e] = 33;

	// Packet: 0x0B5F
	length_list[0x0b5f] = -1;

	// Packet: 0x0B60
	length_list[0x0b60] = -1;

	// Packet: 0x0B61
	length_list[0x0b61] = -1;

	// Packet: 0x0B62
	length_list[0x0b62] = -1;

	// Packet: 0x0B63
	length_list[0x0b63] = -1;

	// Packet: 0x0B64
	length_list[0x0b64] = -1;

	// Packet: 0x0B65
	length_list[0x0b65] = -1;

	// Packet: 0x0B66
	length_list[0x0b66] = 26;

	// Packet: 0x0B67
	length_list[0x0b67] = 33;

	// Packet: 0x0B68
	length_list[0x0b68] = 12;

	// Packet: 0x0B69
	length_list[0x0b69] = 18;

	// Packet: 0x0B6A
	length_list[0x0b6a] = -1;

	// Packet: 0x0B6B
	length_list[0x0b6b] = 14;

	// Packet: 0x0B6C
	length_list[0x0b6c] = 12;

	// Packet: 0x0B6D
	length_list[0x0b6d] = 6;

	// Packet: 0x0B6E
	length_list[0x0b6e] = 14;

	// Packet: 0x0B6F
	length_list[0x0b6f] = 177;

	// Packet: 0x0B70
	length_list[0x0b70] = -1;

	// Packet: 0x0B71
	length_list[0x0b71] = 177;

	// Packet: 0x0B72
	length_list[0x0b72] = -1;

	// Packet: 0x0B73
	length_list[0x0b73] = 8;

	// Packet: 0x0B74
	length_list[0x0b74] = 1026;

	// Packet: 0x0B75
	length_list[0x0b75] = 1026;

	// Packet: 0x0B76
	length_list[0x0b76] = 77;

	// Packet: 0x0B77
	length_list[0x0b77] = -1;

	// Packet: 0x0B78
	length_list[0x0b78] = -1;

	// Packet: 0x0B79
	length_list[0x0b79] = -1;

	// Packet: 0x0B7A
	length_list[0x0b7a] = -1;

	// Packet: 0x0B7B
	length_list[0x0b7b] = 118;

	// Packet: 0x0B7C
	length_list[0x0b7c] = -1;

	// Packet: 0x0B7D
	length_list[0x0b7d] = -1;

	// Packet: 0x0B7E
	length_list[0x0b7e] = 60;

	// Packet: 0x0B7F
	length_list[0x0b7f] = 10;

	// Packet: 0x0B80
	length_list[0x0b80] = 10;

	// Packet: 0x0B8C
	length_list[0x0b8c] = -1;

	// Packet: 0x0B8D
	length_list[0x0b8d] = -1;

	// Packet: 0x0B8E
	length_list[0x0b8e] = 18;

	// Packet: 0x0B8F
	length_list[0x0b8f] = 6;

	// Packet: 0x0B90
	length_list[0x0b90] = 2;

	// Packet: 0x0B91
	length_list[0x0b91] = 8;

	// Packet: 0x0B92
	length_list[0x0b92] = 5;

	// Packet: 0x0B93
	length_list[0x0b93] = 12;

	// Packet: 0x0B94
	length_list[0x0b94] = 14;

	// Packet: 0x0B95
	length_list[0x0b95] = -1;

	// Packet: 0x0B96
	length_list[0x0b96] = 26;

	// Packet: 0x0B97
	length_list[0x0b97] = 27;

	// Packet: 0x0B98
	length_list[0x0b98] = 6;

	// Packet: 0x0B99
	length_list[0x0b99] = 10;

	// Packet: 0x0B9A
	length_list[0x0b9a] = 11;

	// Packet: 0x0B9B
	length_list[0x0b9b] = 12;

	// Packet: 0x0B9C
	length_list[0x0b9c] = 16;

	// Packet: 0x0B9D
	length_list[0x0b9d] = 14;

	// Packet: 0x0B9E
	length_list[0x0b9e] = 12;

	// Packet: 0x0B9F
	length_list[0x0b9f] = 10;

	// Packet: 0x0BA0
	length_list[0x0ba0] = 2;

	// Packet: 0x0BA1
	length_list[0x0ba1] = 3;

	// Packet: 0x0BA2
	length_list[0x0ba2] = 10;

	// Packet: 0x0BA3
	length_list[0x0ba3] = 10;

	// Packet: 0x0BA4
	length_list[0x0ba4] = 85;

	// Packet: 0x0BA5
	length_list[0x0ba5] = 12;

	// Packet: 0x0BA6
	length_list[0x0ba6] = -1;

	// Packet: 0x0BA7
	length_list[0x0ba7] = -1;

	// Packet: 0x0BA8
	length_list[0x0ba8] = 7;

	// Packet: 0x0BA9
	length_list[0x0ba9] = -1;

	// Packet: 0x0BAA
	length_list[0x0baa] = 22;

	// Packet: 0x0BAB
	length_list[0x0bab] = 22;

	// Packet: 0x0BAC
	length_list[0x0bac] = 22;

	// Packet: 0x0BAD
	length_list[0x0bad] = 2;

	// Packet: 0x0BAE
	length_list[0x0bae] = 3;

	// Packet: 0x0BAF
	length_list[0x0baf] = 16;

	// Packet: 0x0BB0
	length_list[0x0bb0] = 9;

	// Packet: 0x0BB1
	length_list[0x0bb1] = 3;

	// Packet: 0x0BB2
	length_list[0x0bb2] = 2;

	// Packet: 0x0BB3
	length_list[0x0bb3] = -1;

	// Packet: 0x0BB4
	length_list[0x0bb4] = 31;

	// Packet: 0x0BB5
	length_list[0x0bb5] = 10;

	// Packet: 0x0BB6
	length_list[0x0bb6] = 18;

	// Packet: 0x0BB7
	length_list[0x0bb7] = -1;

	// Packet: 0x0BB8
	length_list[0x0bb8] = 16;

	// Packet: 0x0BB9
	length_list[0x0bb9] = 27;

	// Packet: 0x0BBA
	length_list[0x0bba] = -1;

	// Packet: 0x0BBB
	length_list[0x0bbb] = -1;

	// Packet: 0x0BBC
	length_list[0x0bbc] = 22;

	// Packet: 0x0BBD
	length_list[0x0bbd] = 6;

	// Packet: 0x0BBE
	length_list[0x0bbe] = 6;

	// Packet: 0x0BBF
	length_list[0x0bbf] = 12;

	// Packet: 0x0BC0
	length_list[0x0bc0] = 3;

	// Packet: 0x0BC1
	length_list[0x0bc1] = -1;

	// Packet: 0x0BC2
	length_list[0x0bc2] = 5;

	// Packet: 0x0BC3
	length_list[0x0bc3] = 10;

	// Packet: 0x0BC4
	length_list[0x0bc4] = 2;

	// Packet: 0x0BC5
	length_list[0x0bc5] = 7;

	// Packet: 0x0BC6
	length_list[0x0bc6] = 9;

	// Packet: 0x0BC7
	length_list[0x0bc7] = 6;

	// Packet: 0x0BC8
	length_list[0x0bc8] = -1;

	// Packet: 0x0BC9
	length_list[0x0bc9] = 10;

	// Packet: 0x0BCA
	length_list[0x0bca] = -1;

	// Packet: 0x0BCB
	length_list[0x0bcb] = 4;

	// Packet: 0x0BCC
	length_list[0x0bcc] = -1;

	// Packet: 0x0BCD
	length_list[0x0bcd] = -1;

	// Packet: 0x0BCE
	length_list[0x0bce] = 8;

	// Packet: 0x0BCF
	length_list[0x0bcf] = 9;

	// Packet: 0x0BD0
	length_list[0x0bd0] = 8;

	// Packet: 0x0BD1
	length_list[0x0bd1] = 13;

	// Packet: 0x0BD2
	length_list[0x0bd2] = 8;

	// Packet: 0x0BD3
	length_list[0x0bd3] = 13;

	// Packet: 0x0BD4
	length_list[0x0bd4] = 2;

	// Packet: 0x0BD5
	length_list[0x0bd5] = 9;

	// Packet: 0x0BD6
	length_list[0x0bd6] = 8;

	// Packet: 0x0BD7
	length_list[0x0bd7] = 9;

	// Packet: 0x0BD8
	length_list[0x0bd8] = 14;

	// Packet: 0x0BD9
	length_list[0x0bd9] = 51;

	// Packet: 0x0BDA
	length_list[0x0bda] = -1;

	// Packet: 0x0BDB
	length_list[0x0bdb] = 19;

	// Packet: 0x0BDC
	length_list[0x0bdc] = 18;

	// Packet: 0x0BDD
	length_list[0x0bdd] = -1;

	// Packet: 0x0BDE
	length_list[0x0bde] = -1;

	// Packet: 0x0BDF
	length_list[0x0bdf] = 3;

	// Packet: 0x0BE0
	length_list[0x0be0] = 3;

	// Packet: 0x0BE1
	length_list[0x0be1] = 3;

	// Packet: 0x0BE2
	length_list[0x0be2] = 137;

	// Packet: 0x0BE3
	length_list[0x0be3] = 34;

	// Packet: 0x0BE4
	length_list[0x0be4] = 14;

	// Packet: 0x0BE5
	length_list[0x0be5] = 8;

	// Packet: 0x0BE6
	length_list[0x0be6] = 8;

	// Packet: 0x0BE7
	length_list[0x0be7] = 4;

	// Packet: 0x0BE8
	length_list[0x0be8] = 4;

	// Packet: 0x0BE9
	length_list[0x0be9] = 6;

	// Packet: 0x0BEA
	length_list[0x0bea] = 10;

	// Packet: 0x0BEB
	length_list[0x0beb] = 7;

	// Packet: 0x0BEC
	length_list[0x0bec] = 7;

	// Packet: 0x0BED
	length_list[0x0bed] = 9;

	// Packet: 0x0BEE
	length_list[0x0bee] = 5;

	// Packet: 0x0BEF
	length_list[0x0bef] = -1;

	// Packet: 0x0BF0
	length_list[0x0bf0] = 14;

	// Packet: 0x0BF1
	length_list[0x0bf1] = 18;

	// Packet: 0x0BF2
	length_list[0x0bf2] = 13;

	// Packet: 0x0BF3
	length_list[0x0bf3] = -1;

	// Packet: 0x0BF4
	length_list[0x0bf4] = -1;

	// Packet: 0x0BF5
	length_list[0x0bf5] = 6;

	// Packet: 0x0BF6
	length_list[0x0bf6] = -1;

	// Packet: 0x0BF7
	length_list[0x0bf7] = -1;

	return length_list;
}

/**
 * Export
 */
export default {
	init: init
};
