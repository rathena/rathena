/**
 * Pets/PetEmotionTable.js
 *
 * Pet Emotion table
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author MrUnzO
 */

import Emotion from 'DB/EmotionsConst.js';

const petEmotionTable = [
	[
		[
			Emotion.ET_HNG,
			Emotion.ET_HNG,
			Emotion.ET_KIK,
			Emotion.ET_KIK,
			Emotion.ET_FRET,
			Emotion.ET_HUK,
			Emotion.ET_CRY
		],
		[
			Emotion.ET_HNG,
			Emotion.ET_HNG,
			Emotion.ET_KIK,
			Emotion.ET_KIK,
			Emotion.ET_FRET,
			Emotion.ET_FRET,
			Emotion.ET_SWEAT
		],
		[
			Emotion.ET_SCRATCH,
			Emotion.ET_HNG,
			-1,
			Emotion.ET_HNG,
			Emotion.ET_SWEAT,
			Emotion.ET_SCRATCH,
			Emotion.ET_THINK
		],
		[Emotion.ET_OK, -1, -1, Emotion.ET_THINK, Emotion.ET_SCRATCH, Emotion.ET_THINK, Emotion.ET_AHA],
		[Emotion.ET_SMILE, -1, -1, Emotion.ET_SCRATCH, Emotion.ET_OK, Emotion.ET_DELIGHT, Emotion.ET_AHA]
	],
	[
		[
			Emotion.ET_SCRATCH,
			Emotion.ET_HNG,
			Emotion.ET_KIK,
			Emotion.ET_KIK,
			Emotion.ET_HNG,
			Emotion.ET_HNG,
			Emotion.ET_SWEAT
		],
		[Emotion.ET_SCRATCH, Emotion.ET_HNG, -1, Emotion.ET_HNG, Emotion.ET_KIK, Emotion.ET_HNG, Emotion.ET_HNG],
		[Emotion.ET_OK, -1, -1, Emotion.ET_THINK, Emotion.ET_SCRATCH, Emotion.ET_THINK, Emotion.ET_AHA],
		[Emotion.ET_SMILE, -1, -1, Emotion.ET_SCRATCH, Emotion.ET_OK, Emotion.ET_DELIGHT, Emotion.ET_AHA],
		[Emotion.ET_THANKS, -1, -1, Emotion.ET_PROFUSELY_SWEAT, Emotion.ET_THROB, Emotion.ET_DELIGHT, Emotion.ET_BEST]
	],
	[
		[Emotion.ET_HNG, Emotion.ET_HNG, -1, Emotion.ET_HNG, Emotion.ET_KIK, Emotion.ET_SWEAT, Emotion.ET_HNG],
		[Emotion.ET_SCRATCH, -1, -1, Emotion.ET_THINK, Emotion.ET_THINK, Emotion.ET_THINK, Emotion.ET_SCRATCH],
		[Emotion.ET_OK, -1, -1, Emotion.ET_SCRATCH, Emotion.ET_OK, Emotion.ET_DELIGHT, Emotion.ET_AHA],
		[Emotion.ET_SMILE, -1, -1, Emotion.ET_PROFUSELY_SWEAT, Emotion.ET_SMILE, Emotion.ET_DELIGHT, Emotion.ET_BEST],
		[Emotion.ET_THANKS, Emotion.ET_BEST, -1, Emotion.ET_HUK, Emotion.ET_BIGTHROB, Emotion.ET_OK, Emotion.ET_BEST]
	],
	[
		[Emotion.ET_HNG, -1, -1, Emotion.ET_THINK, Emotion.ET_THINK, Emotion.ET_THINK, Emotion.ET_SCRATCH],
		[Emotion.ET_HNG, -1, -1, Emotion.ET_SCRATCH, Emotion.ET_SCRATCH, Emotion.ET_SCRATCH, Emotion.ET_THINK],
		[Emotion.ET_SCRATCH, -1, -1, Emotion.ET_PROFUSELY_SWEAT, Emotion.ET_BEST, Emotion.ET_DELIGHT, Emotion.ET_AHA],
		[
			Emotion.ET_OK,
			Emotion.ET_BEST,
			Emotion.ET_PROFUSELY_SWEAT,
			Emotion.ET_HUK,
			Emotion.ET_THROB,
			Emotion.ET_OK,
			Emotion.ET_BEST
		],
		[
			Emotion.ET_SMILE,
			Emotion.ET_BEST,
			Emotion.ET_HELP,
			Emotion.ET_CRY,
			Emotion.ET_CHUP,
			Emotion.ET_STARE_ABOUT,
			Emotion.ET_SMILE
		]
	],
	[
		[Emotion.ET_HUK, -1, -1, Emotion.ET_SCRATCH, Emotion.ET_SCRATCH, Emotion.ET_SCRATCH, Emotion.ET_MONEY],
		[
			Emotion.ET_HNG,
			-1,
			-1,
			Emotion.ET_PROFUSELY_SWEAT,
			Emotion.ET_SCRATCH,
			Emotion.ET_SURPRISE,
			Emotion.ET_SURPRISE
		],
		[
			Emotion.ET_HNG,
			Emotion.ET_BEST,
			Emotion.ET_PROFUSELY_SWEAT,
			Emotion.ET_HUK,
			Emotion.ET_SMILE,
			Emotion.ET_OK,
			Emotion.ET_BEST
		],
		[
			Emotion.ET_SCRATCH,
			Emotion.ET_BEST,
			Emotion.ET_HELP,
			Emotion.ET_CRY,
			Emotion.ET_THROB,
			Emotion.ET_SMILE,
			Emotion.ET_SMILE
		],
		[
			Emotion.ET_SCRATCH,
			Emotion.ET_BEST,
			Emotion.ET_HELP,
			Emotion.ET_CRY,
			Emotion.ET_CHUP,
			Emotion.ET_SMILE,
			Emotion.ET_CHUP
		]
	]
];

export default petEmotionTable;
