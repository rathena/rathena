/**
 * Renderer/Entity/Player.js
 *
 * Player class — represents the locally controlled character.
 * Extends Entity to allow instanceof checks and future player-only methods.
 *
 * All player-specific data fields (joblevel, money, weight, stats, exp, etc.)
 * are declared on Entity.prototype so that Entity.set() can auto-map them
 * from server packets via its Entity.prototype.hasOwnProperty() whitelist gate.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import Entity from 'Renderer/Entity/Entity.js';

/**
 * Player class
 *
 * Marker subclass of Entity for the local player character.
 * Session.Entity is an instance of this class while playing.
 *
 * @class Player
 * @extends {Entity}
 */
class Player extends Entity {}

/**
 * Export
 */
export default Player;
