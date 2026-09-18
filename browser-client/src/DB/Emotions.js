/**
 * DB/Emotions.js
 *
 * List all emotions
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/// Enum Emotions
/// [ <index in sprite>,  <index in interface> { <high priority shortcut>, <other shortcut>}]

const list = [
	// ET_SURPRISE = 0
	[0, 0, '!'],

	// ET_QUESTION = 1
	[1, 1, '?'],

	// ET_DELIGHT = 2
	[2, 2, 'ho', 'delight', 'rlQma', '¤¡'],

	// ET_THROB = 3
	[3, 3, 'lv', 'heart', 'gkxm'],

	// ET_SWEAT = 4
	[5, 5, 'swt', 'sweat', 'Eka'],

	// ET_AHA = 5
	[6, 6, 'ic', 'aha', 'dkgk'],

	// ET_FRET = 6
	[7, 7, 'an', 'fret', 'Wkwmd'],

	// ET_ANGER = 7
	[8, 8, 'ag', 'ghk', 'anger'],

	// ET_MONEY = 8
	[9, 9, '$', 'money', 'ehs'],

	// ET_THINK = 9
	[10, 10, '...'],

	// ET_SCISSOR = 10
	[12, 12, 'scissors', 'rkdnl', 'gawi'],

	// ET_ROCK = 11
	[11, 11, 'rock', 'wnajr', 'bawi', 'qkdnl', '¹ÙÀ§'],

	// ET_WRAP = 12
	[13, 13, 'paper', 'qh', 'bo'],

	// ET_FLAG = 13
	[14, -1],

	// ET_BIGTHROB = 14
	[4, 4, 'lv2'],

	// ET_THANKS = 15
	[15, 14, 'thx', '¤£'],

	// ET_KEK = 16
	[16, 15, 'wah'],

	// ET_SORRY = 17
	[17, 16, 'sry', 'sorry'],

	// ET_SMILE = 18
	[18, 17, 'heh', 'ÇÏÇÏ', '¤·', 'smile'],

	// ET_PROFUSELY_SWEAT = 19
	[19, 18, 'swt2'],

	// ET_SCRATCH = 20
	[20, 19, 'hmm'],

	// ET_BEST = 21
	[21, 20, 'no1', '¿øÃò', '¤º'],

	// ET_STARE_ABOUT = 22
	[22, 21, '??'],

	// ET_HUK = 23
	[23, 22, 'omg', '¤¾'],

	// ET_O = 24
	[24, 23, 'oh', 'o'],

	// ET_X = 25
	[25, 24, 'X', 'x'],

	// ET_HELP = 26
	[26, 25, 'hlp', '¤º¤º', 'help'],

	// ET_GO = 27
	[27, 26, 'go'],

	// ET_CRY = 28
	[28, 27, 'sob'],

	// ET_KIK = 29
	[29, 28, 'gg', 'Å±', '¤»', 'ÄÉÄÉ'],

	// ET_CHUP = 30
	[30, 29, 'kis'],

	// ET_CHUPCHUP = 31
	[31, 30, 'kis2'],

	// ET_HNG = 32
	[32, 31, 'pif'],

	// ET_OK = 33
	[33, 32, 'ok', '²ô´ö'],

	// ET_CHAT_PROHIBIT = 34
	[1000, -1],

	// ET_INDONESIA_FLAG = 35
	[34, -1],

	// ET_STARE = 36
	[35, 33, 'bzz', 'e1', '¤¹'],

	// ET_HUNGRY = 37
	[36, 34, 'rice', 'e2'],

	// ET_COOL = 38
	[37, 35, 'awsm', 'e3'],

	// ET_MERONG = 39
	[38, 36, 'meh', 'e4'],

	// ET_SHY = 40
	[39, 37, 'shy', 'e5', 'ºÎ'],

	// ET_GOODBOY = 41
	[40, 38, 'pat', 'e6'],

	// ET_SPTIME = 42
	[41, 39, 'mp', '¿¥', 'e7'],

	// ET_SEXY = 43
	[42, 40, 'slur', 'e8'],

	// ET_COMEON = 44
	[43, 41, 'com', 'e9'],

	// ET_SLEEPY = 45
	[44, 42, 'yawn', '¤¸', 'e10'],

	// ET_CONGRATULATION = 46
	[45, 43, 'grat', '¤º¤»', 'e11', 'grat'],

	// ET_HPTIME = 47
	[46, 44, 'hp', 'ÇÇ', 'e12'],

	// ET_PH_FLAG = 48
	[47, -1],

	// ET_MY_FLAG = 49
	[48, -1],

	// ET_SI_FLAG = 50
	[49, -1],

	// ET_BR_FLAG = 51
	[50, -1],

	// ET_SPARK = 52
	[51, 45, 'fsh', 'e13'],

	// ET_CONFUSE = 53
	[52, 46, 'spin', 'e14'],

	// ET_OHNO = 54
	[53, 47, 'sigh', 'ÇÑ¼û', 'e15'],

	// ET_HUM = 55
	[54, 48, 'dum', 'e16'],

	// ET_BLABLA = 56
	[55, 49, 'crwd', '½Ã²ø½Ã²ø', 'e17'],

	// ET_OTL = 57
	[56, 50, 'desp', 'otl', 'e18'],

	// ET_DICE1 = 58
	[57, -1, 'dice', 'e19'],

	// ET_DICE2 = 59
	[58, -1],

	// ET_DICE3 = 60
	[59, -1],

	// ET_DICE4 = 61
	[60, -1],

	// ET_DICE5 = 62
	[61, -1],

	// ET_DICE6 = 63
	[62, -1],

	// ET_INDIA_FLAG = 64
	[63, -1],

	// ET_LUV = 65
	[64, 51, 'love', 'e20'],

	// ET_FLAG8 = 66
	[65, -1],

	// ET_FLAG9 = 67
	[66, -1],

	// ET_MOBILE = 68
	[67, 52, 'mobile', 'e21'],

	// ET_MAIL = 69
	[68, 53, 'mail', 'e22'],

	// ET_ANTENNA0 = 70
	[69, -1, 'antenna0', 'e23'],

	// ET_ANTENNA1 = 71
	[70, 54, 'antenna1', 'e24'],

	// ET_ANTENNA2 = 72
	[71, 55, 'antenna2', 'e25'],

	// ET_ANTENNA3 = 73
	[72, 56, 'antenna3', 'e26'],

	// ET_HUM2 = 74
	[73, 57, 'hum', 'Èì', 'e27'],

	// ET_ABS = 75
	[74, 58, 'abs', '¸Û', 'e28'],

	// ET_OOPS = 76
	[75, 59, 'oops', '¿ó', 'e29'],

	// ET_SPIT = 77
	[76, 60, 'spit', 'À¡', 'e30'],

	// ET_ENE = 78
	[77, 61, 'ene', 'e31', 'Å»·Â'],

	// ET_PANIC = 79
	[78, 62, 'panic', 'e32', 'øÈ²'],

	// ET_WHISP = 80
	[79, 63, 'whisp', 'e33', 'À§½ºÆÛ'],

	// ET_!QUEST = 81 / 0
	[80, -1],

	// ET_?QUEST = 82 / 1
	[81, -1],

	// ET_!JOB = 83 / 2
	[82, -1],

	// ET_?JOB = 84 / 3
	[83, -1],

	// ET_!EVENT = 85 / 4
	[84, -1],

	// ET_?EVENT = 86 / 5
	[85, -1],

	// ET_??? = 87 / 6
	[86, -1]
];

let i, j, count, size;
const commands = {};
const names = {};
const indexes = {};
const order = {};

for (i = 0, count = list.length; i < count; ++i) {
	size = list[i].length;
	for (j = 2; j < size; ++j) {
		commands[list[i][j]] = i;
	}

	if (size > 0) {
		indexes[i] = list[i][0];
	}

	if (size > 1 && list[i][1] > -1) {
		order[list[i][1]] = list[i][0];
	}

	if (size > 2) {
		names[list[i][0]] = list[i][2];
	}
}

/**
 * Exports
 */
export default {
	commands: commands,
	names: names,
	indexes: indexes,
	order: order
};
