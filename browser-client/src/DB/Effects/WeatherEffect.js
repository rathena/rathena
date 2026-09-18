/**
 * DB/Effects/WeatherEffect.js
 *
 * Weather DB
 *
 * This file is part of ROBrowser, Ragnarok Online in the Web Browser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

const Weather = {};

// Sky and clouds features
Weather.sky = {};
Weather.effects = {};

// Blue sky and white clouds
Weather.sky['airplane.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['airplane_01.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['gonryun.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['gon_dun02.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['himinn.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['ra_temsky.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['rwc01.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['sch_gld.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['valkyrie.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };
Weather.sky['yuno.rsw'] = { skyColor: [0.4, 0.6, 0.8, 1.0], cloudColor: [1.0, 1.0, 1.0] };

// Extras
Weather.sky['5@tower.rsw'] = { skyColor: [0.2, 0.0, 0.2, 1.0], cloudColor: [1.0, 0.7, 0.7] };
Weather.sky['thana_boss.rsw'] = { skyColor: [0.88, 0.83, 0.76, 1.0], cloudColor: [0.37, 0.0, 0.0] };

// TODO: add others effect
Weather.effects['xmas.rsw'] = { weather: 'snow' };
Weather.effects['comodo.rsw'] = { weather: 'fireworks' };
Weather.effects['einbroch.rsw'] = { weather: 'cloud3' };
//Weather.effects['payon.rsw'] = { weather: 'rain' };

/**
 * Export
 */
export default Weather;
