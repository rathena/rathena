/**
 * Engine/SessionStorage.js
 *
 * Session Storage
 * Manage session variables
 *
 * @author Vincent Thibault
 */

/** @typedef {import('Renderer/Entity/Player.js').default} Player */
/** @typedef {import('Renderer/Entity/Entity.js').default} Entity */

export default {
	isTouchDevice: false,
	isRenewal: false,
	TouchTargeting: false,
	AutoTargeting: false,

	FreezeUI: false,

	AuthCode: 0,
	AID: 0,
	GID: 0,
	UserLevel: 0,
	Sex: 0,
	LangType: 0,
	ServerName: null,
	ratesInfo: null,

	/** @type {Player|Entity|null} The entity currently controlled by the client */
	Entity: null,

	AdminList: [],

	underAutoCounter: false,

	moveAction: null,

	// weight and max_weight now live on the player entity (Session.Entity)

	/**
	 * Player money, stored on the player entity.
	 * Kept here as an accessor for the many consumers reading it from the session.
	 */
	get zeny() {
		return this.Entity ? this.Entity.money : 0;
	},

	set zeny(value) {
		if (this.Entity) {
			this.Entity.money = value;
		}
	},

	petId: 0,
	pet: {},

	hasParty: false,
	isPartyLeader: false,

	hasGuild: false,
	guildRight: 0,
	guildName: '',
	isGuildMaster: false,

	Playing: false,
	hasCart: false,
	CartNum: 0,

	homCustomAI: false,
	merCustomAI: false,

	autoFollow: false,
	autoFollowTarget: null,

	ping: {
		pingTime: 0,
		pongTime: 0,
		returned: false,
		value: 0
	},

	serverTick: 0,

	mapState: {
		property: 0,
		type: 0,
		flag: 0,
		isPVPZone: false,
		isAgitZone: false,
		isPVP: false,
		isGVG: false,
		isSiege: false,
		isNoLockOn: false,
		showPVPCounter: false,
		showBFCounter: false,
		isBattleField: false
	},

	requestCashShop: true,

	captchaGetIdOnEntityClick: false,
	captchaGetIdOnFloorClick: false,
	captchaGetIdOnFloorRange: 1,

	Achievement: {
		total_achievements: 0,
		total_points: 0,
		rank: 0,
		current_rank_points: 0,
		next_rank_points: 0,
		list: {}
	}
};
