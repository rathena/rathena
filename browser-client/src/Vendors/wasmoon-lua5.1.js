/**
 * ⚠️ IMPORTANT — DO NOT REPLACE / DO NOT ADD REMOTE WASM LOADING
 *
 * This is a modified build of wasmoon (Lua 5.1 WASM runtime).
 *
 * Changes from upstream:  
 * - Remote/URL-based WASM resolution was removed  
 * - fetch / XHR remote loading paths were removed  
 * - locateFile is kept, but only resolves the locally bundled liblua5.1.wasm  
 * - liblua5.1.wasm is ALWAYS loaded locally from the bundled build  
 *
 * Why this was done:
 * - Avoid Vite "has been externalized for browser compatibility" warnings
 * - Prevent dynamic runtime fetch of the WASM file
 * - Guarantee offline support
 * - Avoid third-party/CDN dependency
 * - Ensure deterministic bundling inside the application
 *
 * Because of this:
 * - DO NOT change the current customWasmUri handling (must stay local via `url`) 
 * - DO NOT reintroduce remote fetch/XHR wasm loading  
 * - DO NOT switch back to npm wasmoon build
 *
 * The WASM must remain statically bundled and locally resolved.
 *
 * This file intentionally diverges from upstream wasmoon.
 */

import lodash from 'lodash';

'use strict';
var exports = {};

	const _documentCurrentScript = typeof document !== 'undefined' ? document.currentScript : null;
	function _interopNamespaceDefault(e) {
		const n = Object.create(null);
		if (e) {
			Object.keys(e).forEach(function (k) {
				if (k !== 'default') {
					var d = Object.getOwnPropertyDescriptor(e, k);
					Object.defineProperty(n, k, d.get ? d : {
						enumerable: true,
						get: function () { return e[k]; }
					});
				}
			});
		}
		n.default = e;
		return Object.freeze(n);
	}

	var lodash__namespace = /*#__PURE__*/_interopNamespaceDefault(lodash);

	class JsType {
		static create(name, is) {
			return new JsType(name, is);
		}
		static decorate(target) {
			const jsType = new JsType();
			jsType.value = target;
			return jsType;
		}
		constructor(name, is) {
			this._priority = 0;
			this._name = null;
			this._name = name !== null && name !== void 0 ? name : null;
			this.match = is;
		}
		_pushMetaTable(thread) {
			if (this._gc) {
				thread.luaApi.lua_pushcfunction(thread.address, this._gc);
				thread.luaApi.lua_setfield(thread.address, -2, '__gc');
			}
			if (this._call) {
				thread.luaApi.lua_pushcfunction(thread.address, this._call);
				thread.luaApi.lua_setfield(thread.address, -2, '__call');
			}
			const ops = [
				'add',
				'sub',
				'mul',
				'div',
				'mod',
				'pow',
				'unm',
				'concat',
				'len',
				'eq',
				'lt',
				'le',
				'index',
				'newindex',
				'metatable',
				'tostring',
			];
			for (const op of ops) {
				if (this[`_${op}`]) {
					const target = this[`_${op}`];
					if (typeof target === 'number') {
						thread.luaApi.lua_pushcfunction(thread.address, target);
					}
					else {
						thread.pushValue(this[`_${op}`]);
					}
					thread.luaApi.lua_setfield(thread.address, -2, `__${op}`);
				}
			}
		}
		bind(thread) {
			thread.luaApi.luaL_newmetatable(thread.address, this._name);
			thread.luaApi.lua_pushstring(thread.address, this._name);
			thread.luaApi.lua_setfield(thread.address, -2, '__name');
			this._pushMetaTable(thread);
			thread.luaApi.lua_pop(thread.address, 1);
			thread.bindType(this);
		}
		push(pushMethod) {
			this._push = pushMethod;
			return this;
		}
		priority(priority) {
			this._priority = priority;
			return this;
		}
		name(name) {
			this._name = name;
			return this;
		}
		gc(funcPointer) {
			this._gc = funcPointer;
			return this;
		}
		call(funcPointer) {
			this._call = funcPointer;
			return this;
		}
		metatable(value) {
			this._metatable = value;
			return this;
		}
		tostring(value) {
			this._tostring = value;
			return this;
		}
		index(funcPointer) {
			this._index = funcPointer;
			return this;
		}
		newindex(funcPointer) {
			this._newindex = funcPointer;
			return this;
		}
		operation(op, func) {
			const ops = {
				'+': '_add',
				'add': '_add',
				'-': '_sub',
				'sub': '_sub',
				'*': '_mul',
				'mul': '_mul',
				'/': '_div',
				'div': '_div',
				'%': '_mod',
				'mod': '_mod',
				'^': '_pow',
				'pow': '_pow',
				'unm': '_unm',
				'..': '_concat',
				'contact': '_concat',
				'#': '_len',
				'len': '_len',
				'==': '_eq',
				'eq': '_eq',
				'<': '_lt',
				'lt': '_lt',
				'<=': '_le',
				'le': '_le',
			};
			const metakey = ops[op];
			if (!metakey) {
				throw new Error(`Invalid operation: ${op}`);
			}
			this[metakey] = func;
			return this;
		}
	}

	const PointerSize = 4;
	const LUA_MULTRET = -1;
	const LUA_REGISTRYINDEX = -10000;
	const LUA_ENVIRONINDEX = -10001;
	const LUA_GLOBALSINDEX = -10002;
	const LUA_IDSIZE = 60;
	exports.LuaReturn = void 0;
	(function (LuaReturn) {
		LuaReturn[LuaReturn["Ok"] = 0] = "Ok";
		LuaReturn[LuaReturn["Yield"] = 1] = "Yield";
		LuaReturn[LuaReturn["ErrorRun"] = 2] = "ErrorRun";
		LuaReturn[LuaReturn["ErrorSyntax"] = 3] = "ErrorSyntax";
		LuaReturn[LuaReturn["ErrorMem"] = 4] = "ErrorMem";
		LuaReturn[LuaReturn["ErrorErr"] = 5] = "ErrorErr";
		LuaReturn[LuaReturn["ErrorFile"] = 6] = "ErrorFile";
	})(exports.LuaReturn || (exports.LuaReturn = {}));
	exports.LuaType = void 0;
	(function (LuaType) {
		LuaType[LuaType["None"] = -1] = "None";
		LuaType[LuaType["Nil"] = 0] = "Nil";
		LuaType[LuaType["Boolean"] = 1] = "Boolean";
		LuaType[LuaType["LightUserdata"] = 2] = "LightUserdata";
		LuaType[LuaType["Number"] = 3] = "Number";
		LuaType[LuaType["String"] = 4] = "String";
		LuaType[LuaType["Table"] = 5] = "Table";
		LuaType[LuaType["Function"] = 6] = "Function";
		LuaType[LuaType["Userdata"] = 7] = "Userdata";
		LuaType[LuaType["Thread"] = 8] = "Thread";
	})(exports.LuaType || (exports.LuaType = {}));
	exports.LuaEventCodes = void 0;
	(function (LuaEventCodes) {
		LuaEventCodes[LuaEventCodes["Call"] = 0] = "Call";
		LuaEventCodes[LuaEventCodes["Ret"] = 1] = "Ret";
		LuaEventCodes[LuaEventCodes["Line"] = 2] = "Line";
		LuaEventCodes[LuaEventCodes["Count"] = 3] = "Count";
		LuaEventCodes[LuaEventCodes["TailCall"] = 4] = "TailCall";
	})(exports.LuaEventCodes || (exports.LuaEventCodes = {}));
	exports.LuaEventMasks = void 0;
	(function (LuaEventMasks) {
		LuaEventMasks[LuaEventMasks["Call"] = 1] = "Call";
		LuaEventMasks[LuaEventMasks["Ret"] = 2] = "Ret";
		LuaEventMasks[LuaEventMasks["Line"] = 4] = "Line";
		LuaEventMasks[LuaEventMasks["Count"] = 8] = "Count";
	})(exports.LuaEventMasks || (exports.LuaEventMasks = {}));
	exports.LuaLibraries = void 0;
	(function (LuaLibraries) {
		LuaLibraries["Base"] = "_G";
		LuaLibraries["Coroutine"] = "coroutine";
		LuaLibraries["Table"] = "table";
		LuaLibraries["IO"] = "io";
		LuaLibraries["OS"] = "os";
		LuaLibraries["String"] = "string";
		LuaLibraries["UTF8"] = "utf8";
		LuaLibraries["Math"] = "math";
		LuaLibraries["Debug"] = "debug";
		LuaLibraries["Package"] = "package";
	})(exports.LuaLibraries || (exports.LuaLibraries = {}));
	class LuaTimeoutError extends Error {
	}

	class LuaDebug {
		constructor(pointer, module, message) {
			this.tracebacks = [];
			this.pointer = pointer;
			this.module = module;
			this.message = message;
		}
		read() {
			let baseIndex = this.pointer >> 2;
			const event = this.module.HEAPU32.at(baseIndex++);
			const name = this.readStructString(baseIndex++);
			const namewhat = this.readStructString(baseIndex++);
			const what = this.readStructString(baseIndex++);
			const source = this.readStructString(baseIndex++);
			const currentline = this.module.HEAPU32.at(baseIndex++);
			const nups = this.module.HEAPU32.at(baseIndex++);
			const linedefined = this.module.HEAPU32.at(baseIndex++);
			const lastlinedefined = this.module.HEAPU32.at(baseIndex++);
			const short_src = this.module.UTF8ToString(baseIndex << 2, 60);
			const i_ci = this.module.HEAPU32.at(baseIndex + LUA_IDSIZE / 4);
			const traceback = {
				event,
				name: name || '?',
				namewhat,
				what,
				source,
				currentline: currentline === 0xffffffff ? '?' : currentline,
				nups,
				linedefined: linedefined === 0xffffffff ? '?' : linedefined,
				lastlinedefined: lastlinedefined === 0xffffffff ? '?' : lastlinedefined,
				short_src,
				i_ci,
			};
			this.tracebacks.push(traceback);
		}
		getMessage() {
			var _a, _b;
			const result = [];
			result.push(`${((_a = this.tracebacks[0]) === null || _a === void 0 ? void 0 : _a.short_src) || '?'}:${((_b = this.tracebacks[0]) === null || _b === void 0 ? void 0 : _b.currentline) || '?'}: ${this.message}`);
			if (this.tracebacks.length) {
				result.push('stack traceback:');
			}
			for (const traceback of this.tracebacks) {
				result.push(`    at ${traceback.name} (${traceback.short_src}:${traceback.currentline}) (${traceback.what}:${traceback.namewhat})`);
			}
			return result.join('\n');
		}
		getTracebacks() {
			return this.tracebacks;
		}
		readStructString(structPointer) {
			return this.module.UTF8ToString(this.module.HEAPU32.at(structPointer));
		}
	}
	LuaDebug.structSize = 100;

	exports.DictType = void 0;
	(function (DictType) {
		DictType[DictType["Array"] = 0] = "Array";
		DictType[DictType["Object"] = 1] = "Object";
		DictType[DictType["Map"] = 2] = "Map";
	})(exports.DictType || (exports.DictType = {}));
	const detectDuplicateKeys = (keys) => {
		const set = new Set();
		for (const _key of keys) {
			const key = String(_key);
			if (set.has(key)) {
				return true;
			}
			set.add(key);
		}
		return false;
	};
	const detectDictType = (keys) => {
		if (keys.some((key) => !['number', 'string'].includes(typeof key)) || detectDuplicateKeys(keys)) {
			return exports.DictType.Map;
		}
		else if (keys.some((key) => typeof key === 'number')) {
			return exports.DictType.Array;
		}
		else {
			return exports.DictType.Object;
		}
	};
	const mapTransform = (map, options = {}) => {
		if (!options.refs) {
			options.refs = new Map();
		}
		if (options.refs.has(map)) {
			return options.refs.get(map);
		}
		const keys = [...map.keys()];
		const dictType = options.dictType === undefined ? detectDictType(keys) : options.dictType;
		if (dictType === exports.DictType.Map) {
			return map;
		}
		const result = dictType === exports.DictType.Array ? [] : {};
		options.refs.set(map, result);
		for (const key of keys) {
			const value = map.get(key);
			if (value instanceof Map) {
				result[key] = mapTransform(value, options);
				continue;
			}
			result[key] = value;
		}
		return result;
	};

	/**
	 * Echos the value of a value. Trys to print the value out
	 * in the best way possible given the different types.
	 *
	 * @param {Object} obj The object to print out.
	 * @param {Object} opts Optional options object that alters the output.
	 */
	/* legacy: obj, showHidden, depth, colors*/
	function inspect(obj, opts) {
		// default options
		var ctx = {
			seen: [],
			stylize: stylizeNoColor
		};
		// legacy...
		if (arguments.length >= 3) ctx.depth = arguments[2];
		if (arguments.length >= 4) ctx.colors = arguments[3];
		if (isBoolean(opts)) {
			// legacy...
			ctx.showHidden = opts;
		} else if (opts) {
			// got an "options" object
			_extend(ctx, opts);
		}
		// set default options
		if (isUndefined(ctx.showHidden)) ctx.showHidden = false;
		if (isUndefined(ctx.depth)) ctx.depth = 2;
		if (isUndefined(ctx.colors)) ctx.colors = false;
		if (isUndefined(ctx.customInspect)) ctx.customInspect = true;
		if (ctx.colors) ctx.stylize = stylizeWithColor;
		return formatValue(ctx, obj, ctx.depth);
	}

	// http://en.wikipedia.org/wiki/ANSI_escape_code#graphics
	inspect.colors = {
		'bold': [1, 22],
		'italic': [3, 23],
		'underline': [4, 24],
		'inverse': [7, 27],
		'white': [37, 39],
		'grey': [90, 39],
		'black': [30, 39],
		'blue': [34, 39],
		'cyan': [36, 39],
		'green': [32, 39],
		'magenta': [35, 39],
		'red': [31, 39],
		'yellow': [33, 39]
	};

	// Don't use 'blue' not visible on cmd.exe
	inspect.styles = {
		'special': 'cyan',
		'number': 'yellow',
		'boolean': 'yellow',
		'undefined': 'grey',
		'null': 'bold',
		'string': 'green',
		'date': 'magenta',
		// "name": intentionally not styling
		'regexp': 'red'
	};


	function stylizeWithColor(str, styleType) {
		var style = inspect.styles[styleType];

		if (style) {
			return '\u001b[' + inspect.colors[style][0] + 'm' + str +
				'\u001b[' + inspect.colors[style][1] + 'm';
		} else {
			return str;
		}
	}


	function stylizeNoColor(str, styleType) {
		return str;
	}


	function arrayToHash(array) {
		var hash = {};

		array.forEach(function (val, idx) {
			hash[val] = true;
		});

		return hash;
	}


	function formatValue(ctx, value, recurseTimes) {
		// Provide a hook for user-specified inspect functions.
		// Check that value is an object with an inspect function on it
		if (ctx.customInspect &&
			value &&
			isFunction(value.inspect) &&
			// Filter out the util module, it's inspect function is special
			value.inspect !== inspect &&
			// Also filter out any prototype objects using the circular check.
			!(value.constructor && value.constructor.prototype === value)) {
			var ret = value.inspect(recurseTimes, ctx);
			if (!isString(ret)) {
				ret = formatValue(ctx, ret, recurseTimes);
			}
			return ret;
		}

		// Primitive types cannot have properties
		var primitive = formatPrimitive(ctx, value);
		if (primitive) {
			return primitive;
		}

		// Look up the keys of the object.
		var keys = Object.keys(value);
		var visibleKeys = arrayToHash(keys);

		if (ctx.showHidden) {
			keys = Object.getOwnPropertyNames(value);
		}

		// IE doesn't make error fields non-enumerable
		// http://msdn.microsoft.com/en-us/library/ie/dww52sbt(v=vs.94).aspx
		if (isError(value)
			&& (keys.indexOf('message') >= 0 || keys.indexOf('description') >= 0)) {
			return formatError(value);
		}

		// Some type of object without properties can be shortcutted.
		if (keys.length === 0) {
			if (isFunction(value)) {
				var name = value.name ? ': ' + value.name : '';
				return ctx.stylize('[Function' + name + ']', 'special');
			}
			if (isRegExp(value)) {
				return ctx.stylize(RegExp.prototype.toString.call(value), 'regexp');
			}
			if (isDate(value)) {
				return ctx.stylize(Date.prototype.toString.call(value), 'date');
			}
			if (isError(value)) {
				return formatError(value);
			}
		}

		var base = '', array = false, braces = ['{', '}'];

		// Make Array say that they are Array
		if (isArray(value)) {
			array = true;
			braces = ['[', ']'];
		}

		// Make functions say that they are functions
		if (isFunction(value)) {
			var n = value.name ? ': ' + value.name : '';
			base = ' [Function' + n + ']';
		}

		// Make RegExps say that they are RegExps
		if (isRegExp(value)) {
			base = ' ' + RegExp.prototype.toString.call(value);
		}

		// Make dates with properties first say the date
		if (isDate(value)) {
			base = ' ' + Date.prototype.toUTCString.call(value);
		}

		// Make error with message first say the error
		if (isError(value)) {
			base = ' ' + formatError(value);
		}

		if (keys.length === 0 && (!array || value.length == 0)) {
			return braces[0] + base + braces[1];
		}

		if (recurseTimes < 0) {
			if (isRegExp(value)) {
				return ctx.stylize(RegExp.prototype.toString.call(value), 'regexp');
			} else {
				return ctx.stylize('[Object]', 'special');
			}
		}

		ctx.seen.push(value);

		var output;
		if (array) {
			output = formatArray(ctx, value, recurseTimes, visibleKeys, keys);
		} else {
			output = keys.map(function (key) {
				return formatProperty(ctx, value, recurseTimes, visibleKeys, key, array);
			});
		}

		ctx.seen.pop();

		return reduceToSingleString(output, base, braces);
	}


	function formatPrimitive(ctx, value) {
		if (isUndefined(value))
			return ctx.stylize('undefined', 'undefined');
		if (isString(value)) {
			var simple = '\'' + JSON.stringify(value).replace(/^"|"$/g, '')
				.replace(/'/g, "\\'")
				.replace(/\\"/g, '"') + '\'';
			return ctx.stylize(simple, 'string');
		}
		if (isNumber(value))
			return ctx.stylize('' + value, 'number');
		if (isBoolean(value))
			return ctx.stylize('' + value, 'boolean');
		// For some reason typeof null is "object", so special case here.
		if (isNull(value))
			return ctx.stylize('null', 'null');
	}


	function formatError(value) {
		return '[' + Error.prototype.toString.call(value) + ']';
	}


	function formatArray(ctx, value, recurseTimes, visibleKeys, keys) {
		var output = [];
		for (var i = 0, l = value.length; i < l; ++i) {
			if (hasOwnProperty(value, String(i))) {
				output.push(formatProperty(ctx, value, recurseTimes, visibleKeys,
					String(i), true));
			} else {
				output.push('');
			}
		}
		keys.forEach(function (key) {
			if (!key.match(/^\d+$/)) {
				output.push(formatProperty(ctx, value, recurseTimes, visibleKeys,
					key, true));
			}
		});
		return output;
	}


	function formatProperty(ctx, value, recurseTimes, visibleKeys, key, array) {
		var name, str, desc;
		desc = Object.getOwnPropertyDescriptor(value, key) || { value: value[key] };
		if (desc.get) {
			if (desc.set) {
				str = ctx.stylize('[Getter/Setter]', 'special');
			} else {
				str = ctx.stylize('[Getter]', 'special');
			}
		} else {
			if (desc.set) {
				str = ctx.stylize('[Setter]', 'special');
			}
		}
		if (!hasOwnProperty(visibleKeys, key)) {
			name = '[' + key + ']';
		}
		if (!str) {
			if (ctx.seen.indexOf(desc.value) < 0) {
				if (isNull(recurseTimes)) {
					str = formatValue(ctx, desc.value, null);
				} else {
					str = formatValue(ctx, desc.value, recurseTimes - 1);
				}
				if (str.indexOf('\n') > -1) {
					if (array) {
						str = str.split('\n').map(function (line) {
							return '  ' + line;
						}).join('\n').substr(2);
					} else {
						str = '\n' + str.split('\n').map(function (line) {
							return '   ' + line;
						}).join('\n');
					}
				}
			} else {
				str = ctx.stylize('[Circular]', 'special');
			}
		}
		if (isUndefined(name)) {
			if (array && key.match(/^\d+$/)) {
				return str;
			}
			name = JSON.stringify('' + key);
			if (name.match(/^"([a-zA-Z_][a-zA-Z_0-9]*)"$/)) {
				name = name.substr(1, name.length - 2);
				name = ctx.stylize(name, 'name');
			} else {
				name = name.replace(/'/g, "\\'")
					.replace(/\\"/g, '"')
					.replace(/(^"|"$)/g, "'");
				name = ctx.stylize(name, 'string');
			}
		}

		return name + ': ' + str;
	}


	function reduceToSingleString(output, base, braces) {
		var length = output.reduce(function (prev, cur) {
			if (cur.indexOf('\n') >= 0);
			return prev + cur.replace(/\u001b\[\d\d?m/g, '').length + 1;
		}, 0);

		if (length > 60) {
			return braces[0] +
				(base === '' ? '' : base + '\n ') +
				' ' +
				output.join(',\n  ') +
				' ' +
				braces[1];
		}

		return braces[0] + base + ' ' + output.join(', ') + ' ' + braces[1];
	}


	// NOTE: These type checking functions intentionally don't use `instanceof`
	// because it is fragile and can be easily faked with `Object.create()`.
	function isArray(ar) {
		return Array.isArray(ar);
	}

	function isBoolean(arg) {
		return typeof arg === 'boolean';
	}

	function isNull(arg) {
		return arg === null;
	}

	function isNumber(arg) {
		return typeof arg === 'number';
	}

	function isString(arg) {
		return typeof arg === 'string';
	}

	function isUndefined(arg) {
		return arg === void 0;
	}

	function isRegExp(re) {
		return isObject(re) && objectToString(re) === '[object RegExp]';
	}

	function isObject(arg) {
		return typeof arg === 'object' && arg !== null;
	}

	function isDate(d) {
		return isObject(d) && objectToString(d) === '[object Date]';
	}

	function isError(e) {
		return isObject(e) &&
			(objectToString(e) === '[object Error]' || e instanceof Error);
	}

	function isFunction(arg) {
		return typeof arg === 'function';
	}

	function objectToString(o) {
		return Object.prototype.toString.call(o);
	}

	function _extend(origin, add) {
		// Don't do anything if add isn't an object
		if (!add || !isObject(add)) return origin;

		var keys = Object.keys(add);
		var i = keys.length;
		while (i--) {
			origin[keys[i]] = add[keys[i]];
		}
		return origin;
	}
	function hasOwnProperty(obj, prop) {
		return Object.prototype.hasOwnProperty.call(obj, prop);
	}

	class LuaTable {
		constructor(thread, ref, pointer) {
			this.alive = true;
			this.thread = thread;
			this.ref = ref;
			this.pointer = pointer;
		}
		[inspect.custom]() {
			return this.toString();
		}
		$get(key) {
			if (!this.alive) {
				throw new Error('table is destroyed');
			}
			return this.getTableValue(key);
		}
		$set(key, value) {
			return this.setTableValue(key, value);
		}
		$istable() {
			return true;
		}
		$getRef() {
			return this.ref;
		}
		$detach(dictType) {
			this.thread.luaApi.lua_rawgeti(this.thread.address, LUA_REGISTRYINDEX, this.ref);
			let map = this.detachTable(-1);
			this.thread.pop();
			map = mapTransform(map, { dictType: dictType !== null && dictType !== void 0 ? dictType : exports.DictType.Map });
			return map;
		}
		$isAlive() {
			return this.alive;
		}
		$destroy() {
			this.thread.luaApi.luaL_unref(this.thread.address, LUA_REGISTRYINDEX, this.ref);
			this.thread.luaApi.pointerRefs.delete(this.pointer);
			this.alive = false;
		}
		toString() {
			return `[LuaTable 0x${this.pointer.toString(16)} *${this.ref} ${this.alive ? 'Alive' : 'Destroyed'}]`;
		}
		getTableValue(key) {
			this.thread.luaApi.lua_rawgeti(this.thread.address, LUA_REGISTRYINDEX, this.ref);
			this.thread.pushValue(key);
			this.thread.luaApi.lua_gettable(this.thread.address, -2);
			const result = this.thread.getValue(-1);
			this.thread.pop(2);
			return result;
		}
		setTableValue(key, value) {
			this.thread.luaApi.lua_rawgeti(this.thread.address, LUA_REGISTRYINDEX, this.ref);
			this.thread.pushValue(key);
			key = this.thread.getValue(-1);
			this.thread.pushValue(value);
			value = this.thread.getValue(-1);
			this.thread.luaApi.lua_settable(this.thread.address, -3);
			this.thread.pop(1);
			return true;
		}
		detachTable(index, refs) {
			index = this.thread.luaApi.lua_absindex(this.thread.address, index);
			if (!refs) {
				refs = new Map();
			}
			const pointer = this.thread.luaApi.lua_topointer(this.thread.address, index);
			if (refs.has(pointer)) {
				return refs.get(pointer);
			}
			const result = new Map();
			refs.set(pointer, result);
			this.thread.luaApi.lua_pushnil(this.thread.address);
			while (this.thread.luaApi.lua_next(this.thread.address, index) !== 0) {
				const key = this.thread.getType(-2) === exports.LuaType.Table ? this.detachTable(-2, refs) : this.thread.getValue(-2);
				const value = this.thread.getType(-1) === exports.LuaType.Table ? this.detachTable(-1, refs) : this.thread.getValue(-1);
				result.set(key, value);
				this.thread.pop();
			}
			return result;
		}
	}
	const getTable = (thread, index) => {
		const pointer = thread.luaApi.lua_topointer(thread.address, index);
		if (thread.luaApi.pointerRefs.has(pointer)) {
			return thread.luaApi.pointerRefs.get(pointer).proxy;
		}
		const ref = thread.luaApi.luaL_ref(thread.address, LUA_REGISTRYINDEX);
		thread.luaApi.lua_rawgeti(thread.address, LUA_REGISTRYINDEX, ref);
		const table = new LuaTable(thread, ref, pointer);
		const { proxy, revoke } = Proxy.revocable(table, {
			get: (target, key) => {
				if (!target.$isAlive() && key !== 'toString') {
					throw new Error(`${target.toString()} is destroyed`);
				}
				if (target[key]) {
					return target[key].bind(target);
				}
				if (key === Symbol.toStringTag) {
					return () => 'LuaTable';
				}
				if (typeof key === 'symbol') {
					return undefined;
				}
				return target.$get(key);
			},
			set: (target, key, value) => {
				if (!target.$isAlive()) {
					throw new Error(`${target.toString()} is destroyed`);
				}
				return target.$set(key, value);
			},
		});
		thread.luaApi.pointerRefs.set(pointer, { proxy, revoke });
		return proxy;
	};

	class MultiReturn extends Array {
	}

	const INSTRUCTION_HOOK_COUNT = 1000;
	class LuaThread {
		constructor(luaApi, address, parent) {
			this.closed = false;
			this.types = [];
			this.luaApi = luaApi;
			this.address = address;
			this.parent = parent;
			this.types = (parent === null || parent === void 0 ? void 0 : parent.types) || [];
		}
		newThread() {
			const address = this.luaApi.lua_newthread(this.address);
			if (!address) {
				throw new Error('lua_newthread returned a null pointer');
			}
			return new LuaThread(this.luaApi, address, this.parent || this);
		}
		bindType(type) {
			this.types.unshift(type);
			this.types.sort((a, b) => b._priority - a._priority);
		}
		getTop() {
			return this.luaApi.lua_gettop(this.address);
		}
		setTop(index) {
			this.luaApi.lua_settop(this.address, index);
		}
		remove(index) {
			return this.luaApi.lua_remove(this.address, index);
		}
		getType(index) {
			return this.luaApi.lua_type(this.address, index);
		}
		isBasicValue(index) {
			const type = this.luaApi.lua_type(this.address, index);
			return [exports.LuaType.Nil, exports.LuaType.Boolean, exports.LuaType.Number, exports.LuaType.String].includes(type);
		}
		pushBasicValue(target, options) {
			if (target === undefined || target === null) {
				this.luaApi.lua_pushnil(this.address);
			}
			else if (typeof target === 'number') {
				if (Number.isInteger(target)) {
					this.luaApi.lua_pushinteger(this.address, target);
				}
				else {
					this.luaApi.lua_pushnumber(this.address, target);
				}
			}
			else if (typeof target === 'string') {
				this.luaApi.lua_pushstring(this.address, target);
			}
			else if (typeof target === 'boolean') {
				this.luaApi.lua_pushboolean(this.address, target ? 1 : 0);
			}
			else if (lodash__namespace.isPlainObject(target) || lodash__namespace.isArray(target)) {
				this.pushTable(target, options);
			}
			else {
				return false;
			}
			return true;
		}
		pushValue(target, options = {}) {
			const startTop = this.getTop();
			if (target instanceof JsType) {
				if (target._push) {
					target._push({ thread: this, target: target.value, options });
				}
				else {
					if (!this.pushBasicValue(target.value, options)) {
						const ref = this.luaApi.ref(target.value);
						const luaPointer = this.luaApi.lua_newuserdata(this.address, PointerSize);
						this.luaApi.module.setValue(luaPointer, ref, '*');
					}
					this.luaApi.lua_createtable(this.address, 0, 0);
					target._pushMetaTable(this);
					this.luaApi.lua_setmetatable(this.address, -2);
				}
			}
			else if (target instanceof LuaThread) {
				const isMain = this.luaApi.lua_pushthread(target.address) === 1;
				if (!isMain) {
					this.luaApi.lua_xmove(target.address, this.address, 1);
				}
				return;
			}
			else if (target && target.$istable) {
				const ref = target.$getRef();
				this.luaApi.lua_rawgeti(this.address, LUA_REGISTRYINDEX, ref);
				return;
			}
			else {
				if (!this.pushBasicValue(target, options)) {
					const type = this.types.find((t) => t.match(target));
					if (type._push) {
						type._push({ thread: this, target, options, type });
					}
					else {
						const ref = this.luaApi.ref(target);
						const luaPointer = this.luaApi.lua_newuserdata(this.address, PointerSize);
						this.luaApi.module.setValue(luaPointer, ref, '*');
						this.luaApi.luaL_getmetatable(this.address, type === null || type === void 0 ? void 0 : type._name);
						this.luaApi.lua_setmetatable(this.address, -2);
					}
				}
			}
			if (this.getTop() !== startTop + 1) {
				throw new Error(`pushValue expected stack size ${startTop + 1}, got ${this.getTop()}`);
			}
		}
		pushTable(object, options = {}) {
			if (!options.refs) {
				options.refs = new Map();
			}
			else {
				const ref = options.refs.get(object);
				if (ref) {
					this.luaApi.lua_rawgeti(this.address, LUA_REGISTRYINDEX, ref);
					return;
				}
			}
			const arrIndexs = [];
			const recIndexs = [];
			if (lodash__namespace.isArray(object)) {
				const keys = Object.keys(object);
				keys.forEach((key) => {
					if (!isNaN(Number(key))) {
						arrIndexs.push(Number(key));
					}
					else {
						recIndexs.push(key);
					}
				});
			}
			else if (lodash__namespace.isPlainObject(object)) {
				recIndexs.push(...Object.keys(object));
			}
			this.luaApi.lua_createtable(this.address, arrIndexs.length, recIndexs.length);
			const ref = this.luaApi.luaL_ref(this.address, LUA_REGISTRYINDEX);
			this.luaApi.lua_rawgeti(this.address, LUA_REGISTRYINDEX, ref);
			options.refs.set(object, ref);
			try {
				for (const key of arrIndexs) {
					this.pushValue(key, options);
					this.pushValue(object[key], options);
					this.luaApi.lua_settable(this.address, -3);
				}
				for (const key of recIndexs) {
					this.pushValue(key, options);
					this.pushValue(object[key], options);
					this.luaApi.lua_settable(this.address, -3);
				}
			}
			finally {
				const registerRefs = options.refs.values();
				for (const ref of registerRefs) {
					this.luaApi.luaL_unref(this.address, LUA_REGISTRYINDEX, ref);
				}
			}
		}
		getValue(index, options = {}) {
			var _a;
			index = this.luaApi.lua_absindex(this.address, index);
			const type = (_a = options.type) !== null && _a !== void 0 ? _a : this.luaApi.lua_type(this.address, index);
			if (type === exports.LuaType.None) {
				return undefined;
			}
			else if (type === exports.LuaType.Nil) {
				return null;
			}
			else if (type === exports.LuaType.Number) {
				return this.luaApi.lua_tonumber(this.address, index);
			}
			else if (type === exports.LuaType.String) {
				return this.luaApi.lua_tolstring(this.address, index, null);
			}
			else if (type === exports.LuaType.Boolean) {
				return Boolean(this.luaApi.lua_toboolean(this.address, index));
			}
			else if (type === exports.LuaType.Thread) {
				return this.stateToThread(this.luaApi.lua_tothread(this.address, index));
			}
			else if (type === exports.LuaType.Table) {
				return this.getTable(index);
			}
			else if (type === exports.LuaType.Function) {
				return this.getFunction(index);
			}
			else if (type === exports.LuaType.Userdata) {
				const userdata = this.luaApi.lua_touserdata(this.address, index);
				const ref = this.luaApi.module.getValue(userdata, '*');
				return this.luaApi.getRef(ref);
			}
		}
		getFunction(index) {
			this.luaApi.lua_pushvalue(this.address, index);
			const funcRef = this.luaApi.luaL_ref(this.address, LUA_REGISTRYINDEX);
			const pointer = this.luaApi.lua_topointer(this.address, index);
			if (this.luaApi.pointerRefs.has(pointer)) {
				return this.luaApi.pointerRefs.get(pointer);
			}
			const func = (...args) => {
				if (!func.$isAlive()) {
					throw new Error('Tried to call a function that has been destroyed');
				}
				if (this.isClosed()) {
					console.warn('Tried to call a function after closing lua state');
					return;
				}
				const thread = this.newThread();
				thread.luaApi.lua_rawgeti(thread.address, LUA_REGISTRYINDEX, funcRef);
				try {
					for (const arg of args) {
						thread.pushValue(arg);
					}
					const status = thread.luaApi.lua_pcall(thread.address, args.length, 1, 0);
					if (status === exports.LuaReturn.Yield) {
						throw new Error('cannot yield in callbacks from javascript');
					}
					thread.assertOk(status);
					if (thread.getTop() > 0) {
						return thread.getValue(-1);
					}
					return undefined;
				}
				finally {
					thread.close();
					this.pop();
				}
			};
			func.$isAlive = () => true;
			func.$destroy = () => {
				this.luaApi.luaL_unref(this.address, LUA_REGISTRYINDEX, funcRef);
				this.luaApi.pointerRefs.delete(pointer);
				func.$isAlive = () => false;
			};
			this.luaApi.pointerRefs.set(pointer, func);
			return func;
		}
		getTable(index) {
			return getTable(this, index);
		}
		stateToThread(L) {
			var _a;
			return L === ((_a = this.parent) === null || _a === void 0 ? void 0 : _a.address) ? this.parent : new LuaThread(this.luaApi, L, this.parent || this);
		}
		pop(count = 1) {
			this.luaApi.lua_pop(this.address, count);
		}
		loadString(luaCode, name) {
			const size = this.luaApi.module.lengthBytesUTF8(luaCode);
			const pointerSize = size + 1;
			const bufferPointer = this.luaApi.module._malloc(pointerSize);
			try {
				this.luaApi.module.stringToUTF8(luaCode, bufferPointer, pointerSize);
				this.assertOk(this.luaApi.luaL_loadbuffer(this.address, bufferPointer, size, name !== null && name !== void 0 ? name : bufferPointer));
			}
			finally {
				this.luaApi.module._free(bufferPointer);
			}
		}
		loadFile(filename) {
			this.assertOk(this.luaApi.luaL_loadfilex(this.address, filename, null));
		}
		call(name, ...args) {
			const type = this.luaApi.lua_getglobal(this.address, name);
			if (type !== exports.LuaType.Function) {
				throw new Error(`A function of type '${type}' was pushed, expected is ${exports.LuaType.Function}`);
			}
			for (const arg of args) {
				this.pushValue(arg);
			}
			const base = this.getTop() - args.length - 1;
			this.luaApi.lua_call(this.address, args.length, LUA_MULTRET);
			return this.getStackValues(base);
		}
		getStackValues(start = 0) {
			const returns = this.getTop() - start;
			const returnValues = new MultiReturn(returns);
			for (let i = 0; i < returns; i++) {
				returnValues[i] = this.getValue(start + i + 1);
			}
			return returnValues;
		}
		async run(argCount = 0, options) {
			const originalTimeout = this.timeout;
			try {
				if ((options === null || options === void 0 ? void 0 : options.timeout) !== undefined) {
					this.setTimeout(Date.now() + options.timeout);
				}
				let resumeResult = this.resume(argCount);
				while (resumeResult.result === exports.LuaReturn.Yield) {
					if (this.timeout && Date.now() > this.timeout) {
						if (resumeResult.resultCount > 0) {
							this.pop(resumeResult.resultCount);
						}
						throw new Error(`thread timeout exceeded`);
					}
					if (resumeResult.resultCount > 0) {
						const lastValue = this.getValue(-1);
						this.pop(resumeResult.resultCount);
						if (lastValue === Promise.resolve(lastValue)) {
							await lastValue;
						}
						else {
							await new Promise((resolve) => setImmediate(resolve));
						}
					}
					else {
						await new Promise((resolve) => setImmediate(resolve));
					}
					resumeResult = this.resume(0);
				}
				this.assertOk(resumeResult.result);
				return this.getStackValues();
			}
			finally {
				if ((options === null || options === void 0 ? void 0 : options.timeout) !== undefined) {
					this.setTimeout(originalTimeout);
				}
			}
		}
		runSync(argCount = 0) {
			const base = this.getTop() - argCount - 1;
			this.assertOk(this.luaApi.lua_pcall(this.address, argCount, LUA_MULTRET, 0));
			return this.getStackValues(base);
		}
		isClosed() {
			var _a;
			return !this.address || this.closed || Boolean((_a = this.parent) === null || _a === void 0 ? void 0 : _a.isClosed());
		}
		close() {
			if (this.isClosed()) {
				return;
			}
			if (this.hookFunctionPointer) {
				this.luaApi.module.removeFunction(this.hookFunctionPointer);
			}
			this.closed = true;
		}
		resetThread() {
			this.luaApi.lua_close(this.address);
		}
		resume(argCount = 0) {
			const luaResult = this.luaApi.lua_resume(this.address, argCount);
			return {
				result: luaResult,
				resultCount: this.getTop(),
			};
		}
		setTimeout(timeout) {
			if (timeout && timeout > 0) {
				if (!this.hookFunctionPointer) {
					this.hookFunctionPointer = this.luaApi.module.addFunction(() => {
						if (Date.now() > timeout) {
							this.pushValue(new Error(`thread timeout exceeded`));
							this.luaApi.lua_error(this.address);
						}
					}, 'vii');
				}
				this.luaApi.lua_sethook(this.address, this.hookFunctionPointer, exports.LuaEventMasks.Count, INSTRUCTION_HOOK_COUNT);
				this.timeout = timeout;
			}
			else if (this.hookFunctionPointer) {
				this.hookFunctionPointer = undefined;
				this.timeout = undefined;
				this.luaApi.lua_sethook(this.address, null, 0, 0);
			}
		}
		getTimeout() {
			return this.timeout;
		}
		assertOk(result) {
			if (result !== exports.LuaReturn.Ok && result !== exports.LuaReturn.Yield) {
				const resultString = exports.LuaReturn[result];
				let error = new Error(`Lua Error(${resultString}/${result})`);
				if (this.getTop() > 0) {
					if (result === exports.LuaReturn.ErrorMem) {
						error.message = this.luaApi.lua_tolstring(this.address, -1, null);
					}
					else {
						const luaError = this.getValue(-1);
						if (luaError instanceof Error) {
							error = luaError;
						}
						else {
							error.message = new TextDecoder().decode(this.indexToString(-1));
						}
					}
				}
				if (result !== exports.LuaReturn.ErrorMem) {
					const pointer = this.luaApi.module._malloc(LuaDebug.structSize);
					try {
						let level = 0;
						const luaDebug = new LuaDebug(pointer, this.luaApi.module, error.message);
						while (this.luaApi.lua_getstack(this.address, level, pointer)) {
							this.luaApi.lua_getinfo(this.address, 'nSlu', pointer);
							luaDebug.read();
							level++;
						}
						error.message = luaDebug.getMessage();
					}
					catch (err) {
						console.warn('Error in generate stack trace', err);
					}
					finally {
						this.luaApi.module._free(pointer);
					}
				}
				throw error;
			}
		}
		getGlobalPointer(name) {
			this.luaApi.lua_getglobal(this.address, name);
			const pointer = this.getPointer(-1);
			this.pop();
			return pointer;
		}
		getPointer(index) {
			return this.luaApi.lua_topointer(this.address, index);
		}
		indexToString(index) {
			const str = this.luaApi.luaL_tolstring(this.address, index);
			this.pop();
			return str;
		}
		dumpStack(log = console.log) {
			const top = this.getTop();
			for (let i = 1; i <= top; i++) {
				const typename = this.luaApi.luaL_typename(this.address, i);
				const pointer = this.getPointer(i).toString();
				const name = this.indexToString(i);
				log(i, typename, pointer, name);
			}
		}
		dumpTypes(log = console.log) {
			for (const type of this.types) {
				log(type._name, type._priority);
			}
		}
	}

	class LuaGlobal extends LuaThread {
		constructor(cmodule, shouldTraceAllocations) {
			if (shouldTraceAllocations) {
				const memoryStats = { memoryUsed: 0 };
				const allocatorFunctionPointer = cmodule.module.addFunction((_userData, pointer, oldSize, newSize) => {
					if (newSize === 0) {
						if (pointer) {
							memoryStats.memoryUsed -= oldSize;
							cmodule.module._free(pointer);
						}
						return 0;
					}
					const endMemoryDelta = pointer ? newSize - oldSize : newSize;
					const endMemory = memoryStats.memoryUsed + endMemoryDelta;
					if (newSize > oldSize && memoryStats.memoryMax && endMemory > memoryStats.memoryMax) {
						return 0;
					}
					const reallocated = cmodule.module._realloc(pointer, newSize);
					if (reallocated) {
						memoryStats.memoryUsed = endMemory;
					}
					return reallocated;
				}, 'iiiii');
				const address = cmodule.lua_newstate(allocatorFunctionPointer, null);
				if (!address) {
					cmodule.module.removeFunction(allocatorFunctionPointer);
					throw new Error('lua_newstate returned a null pointer');
				}
				super(cmodule, address);
				this.memoryStats = memoryStats;
				this.allocatorFunctionPointer = allocatorFunctionPointer;
			}
			else {
				const address = cmodule.luaL_newstate();
				super(cmodule, address);
			}
			if (this.isClosed()) {
				throw new Error('Global state could not be created (probably due to lack of memory)');
			}
		}
		close() {
			if (this.isClosed()) {
				return;
			}
			super.close();
			this.luaApi.lua_close(this.address);
			if (this.allocatorFunctionPointer) {
				this.luaApi.module.removeFunction(this.allocatorFunctionPointer);
			}
		}
		loadLibrary(library) {
			switch (library) {
				case exports.LuaLibraries.Base:
					this.luaApi.luaopen_base(this.address);
					break;
				case exports.LuaLibraries.Table:
					this.luaApi.luaopen_table(this.address);
					break;
				case exports.LuaLibraries.IO:
					this.luaApi.luaopen_io(this.address);
					break;
				case exports.LuaLibraries.OS:
					this.luaApi.luaopen_os(this.address);
					break;
				case exports.LuaLibraries.String:
					this.luaApi.luaopen_string(this.address);
					break;
				case exports.LuaLibraries.Math:
					this.luaApi.luaopen_math(this.address);
					break;
				case exports.LuaLibraries.Debug:
					this.luaApi.luaopen_debug(this.address);
					break;
				case exports.LuaLibraries.Package:
					this.luaApi.luaopen_package(this.address);
					break;
			}
			this.luaApi.lua_setglobal(this.address, library);
		}
		get(name) {
			const type = this.luaApi.lua_getglobal(this.address, name);
			const value = this.getValue(-1, { type });
			this.pop();
			return value;
		}
		set(name, value, options) {
			this.pushValue(value, options);
			this.luaApi.lua_setglobal(this.address, name);
		}
		getMemoryUsed() {
			return this.getMemoryStatsRef().memoryUsed;
		}
		getMemoryMax() {
			return this.getMemoryStatsRef().memoryMax;
		}
		setMemoryMax(max) {
			this.getMemoryStatsRef().memoryMax = max;
		}
		getMemoryStatsRef() {
			if (!this.memoryStats) {
				throw new Error('Memory allocations is not being traced, please build engine with { traceAllocations: true }');
			}
			return this.memoryStats;
		}
	}

	var initWasmModule = (() => {
		var _scriptDir = (typeof document === 'undefined' && typeof location === 'undefined' ? "" : typeof document === 'undefined' ? location.href : (_documentCurrentScript && _documentCurrentScript.src || new URL('index.js', document.baseURI).href));

		return (
			async function (moduleArg = {}) {

				var e = moduleArg, aa, ba; e.ready = new Promise((a, b) => { aa = a; ba = b; });
				"_malloc _free _realloc _luaL_error _luaL_typerror _luaL_argerror _luaL_addlstring _luaL_addstring _luaL_addvalue _luaL_checkany _luaL_checkinteger _luaL_checklstring _luaL_checknumber _luaL_checkoption _luaL_checkstack _luaL_checktype _luaL_checkudata _luaL_loadbuffer _luaL_loadfile _luaL_loadstring _luaL_buffinit _luaL_callmeta _luaL_getmetafield _luaL_gsub _luaL_newmetatable _luaL_newstate _luaL_openlibs _luaL_optinteger _luaL_optlstring _luaL_optnumber _luaL_prepbuffer _luaL_pushresult _luaL_register _luaL_ref _luaL_unref _luaL_where _lua_atpanic _lua_call _lua_checkstack _lua_close _lua_concat _lua_cpcall _lua_createtable _lua_dump _lua_equal _lua_error _lua_gc _lua_getallocf _lua_getfenv _lua_getfield _lua_gethook _lua_gethookcount _lua_gethookmask _lua_getinfo _lua_getlocal _lua_getmetatable _lua_getstack _lua_gettable _lua_gettop _lua_getupvalue _lua_insert _lua_iscfunction _lua_isnumber _lua_isstring _lua_isuserdata _lua_lessthan _lua_load _lua_newstate _lua_newthread _lua_newuserdata _lua_next _lua_objlen _lua_pcall _lua_pushboolean _lua_pushcclosure _lua_pushfstring _lua_pushinteger _lua_pushlightuserdata _lua_pushlstring _lua_pushnil _lua_pushnumber _lua_pushstring _lua_pushthread _lua_pushvalue _lua_pushvfstring _lua_rawequal _lua_rawget _lua_rawgeti _lua_rawset _lua_rawseti _lua_remove _lua_replace _lua_resume _lua_setallocf _lua_setfenv _lua_setfield _lua_sethook _lua_setlocal _lua_setmetatable _lua_settable _lua_settop _lua_setupvalue _lua_status _lua_toboolean _lua_tocfunction _lua_tointeger _lua_tolstring _lua_tonumber _lua_topointer _lua_tothread _lua_touserdata _lua_type _lua_typename _lua_xmove _lua_yield _luaopen_base _luaopen_table _luaopen_io _luaopen_os _luaopen_string _luaopen_math _luaopen_debug _luaopen_package _memory ___indirect_function_table onRuntimeInitialized".split(" ").forEach(a => {
					Object.getOwnPropertyDescriptor(e.ready,
						a) || Object.defineProperty(e.ready, a, { get: () => g("You are getting " + a + " on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js"), set: () => g("You are setting " + a + " on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js") });
				});
				var ca = Object.assign({}, e), da = "./this.program", ea = (a, b) => { throw b; }, fa = "object" == typeof window, l = "function" == typeof importScripts, n = "object" == typeof process && "object" == typeof process.versions && "string" == typeof process.versions.node, ha = !fa && !n && !l; if (e.ENVIRONMENT) throw Error("Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)"); var r = "", ia, ja, ka;
				if (n) {
					if ("undefined" == typeof process || !process.release || "node" !== process.release.name) throw Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)"); var la = process.versions.node, ma = la.split(".").slice(0, 3); ma = 1E4 * ma[0] + 100 * ma[1] + 1 * ma[2].split("-")[0]; if (16E4 > ma) throw Error("This emscripten-generated code requires node v16.0.0 (detected v" + la + ")"); const { createRequire: a } =
						await Promise.resolve({ createRequire: () => () => {} }); var require$1 = a((typeof document === 'undefined' && typeof location === 'undefined' ? { pathToFileURL: () => ({ href: "" }) }.pathToFileURL(__filename).href : typeof document === 'undefined' ? location.href : (_documentCurrentScript && _documentCurrentScript.src || new URL('index.js', document.baseURI).href))), fs = require$1("fs"), na = require$1("path"); l ? r = na.dirname(r) + "/" : r = { fileURLToPath: () => "" }.fileURLToPath(new URL("./", (typeof document === 'undefined' && typeof location === 'undefined' ? { pathToFileURL: () => ({ href: "" }) }.pathToFileURL(__filename).href : typeof document === 'undefined' ? location.href : (_documentCurrentScript && _documentCurrentScript.src || new URL('index.js', document.baseURI).href)))); ia = (b, c) => { b = oa(b) ? new URL(b) : na.normalize(b); return fs.readFileSync(b, c ? void 0 : "utf8") }; ka = b => { b = ia(b, !0); b.buffer || (b = new Uint8Array(b)); u(b.buffer); return b }; ja = (b, c, d, f = !0) => { b = oa(b) ? new URL(b) : na.normalize(b); fs.readFile(b, f ? void 0 : "utf8", (h, m) => { h ? d(h) : c(f ? m.buffer : m); }); }; !e.thisProgram &&
							1 < process.argv.length && (da = process.argv[1].replace(/\\/g, "/")); process.argv.slice(2); ea = (b, c) => { process.exitCode = b; throw c; };
				} else if (ha) {
					if ("object" == typeof process && "function" === typeof require$1 || "object" == typeof window || "function" == typeof importScripts) throw Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)"); "undefined" != typeof read && (ia = read); ka = a => {
						if ("function" ==
							typeof readbuffer) return new Uint8Array(readbuffer(a)); a = read(a, "binary"); u("object" == typeof a); return a
					}; ja = (a, b) => { setTimeout(() => b(ka(a))); }; "undefined" == typeof clearTimeout && (globalThis.clearTimeout = () => { }); "undefined" == typeof setTimeout && (globalThis.setTimeout = a => "function" == typeof a ? a() : g()); "function" == typeof quit && (ea = (a, b) => { setTimeout(() => { if (!(b instanceof pa)) { let c = b; b && "object" == typeof b && b.stack && (c = [b, b.stack]); z(`exiting due to exception: ${c}`); } quit(a); }); throw b; }); "undefined" != typeof print &&
						("undefined" == typeof console && (console = {}), console.log = print, console.warn = console.error = "undefined" != typeof printErr ? printErr : print);
				} else if (fa || l) {
					l ? r = self.location.href : "undefined" != typeof document && document.currentScript && (r = document.currentScript.src); _scriptDir && (r = _scriptDir); r.startsWith("blob:") ? r = "" : r = r.substr(0, r.replace(/[?#].*/, "").lastIndexOf("/") + 1); if ("object" != typeof window && "function" != typeof importScripts) throw Error("not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)");
					ia = a => { var b = new XMLHttpRequest; b.open("GET", a, !1); b.send(null); return b.responseText }; l && (ka = a => { var b = new XMLHttpRequest; b.open("GET", a, !1); b.responseType = "arraybuffer"; b.send(null); return new Uint8Array(b.response) }); ja = (a, b, c) => { var d = new XMLHttpRequest; d.open("GET", a, !0); d.responseType = "arraybuffer"; d.onload = () => { 200 == d.status || 0 == d.status && d.response ? b(d.response) : c(); }; d.onerror = c; d.send(null); };
				} else throw Error("environment detection error"); var qa = console.log.bind(console), z = console.error.bind(console);
				Object.assign(e, ca); ca = null; C("ENVIRONMENT"); C("GL_MAX_TEXTURE_IMAGE_UNITS"); C("SDL_canPlayWithWebAudio"); C("SDL_numSimultaneouslyQueuedBuffers"); C("INITIAL_MEMORY"); C("wasmMemory"); C("arguments"); C("buffer"); C("canvas"); C("doNotCaptureKeyboard"); C("dynamicLibraries"); C("elementPointerLock"); C("extraStackTrace"); C("forcedAspectRatio"); C("instantiateWasm"); C("keyboardListeningElement"); C("freePreloadedMediaOnUse"); C("loadSplitModule"); C("logReadFiles"); C("mainScriptUrlOrBlob"); C("mem"); C("monitorRunDependencies");
				C("noExitRuntime"); C("noInitialRun"); C("onAbort"); C("onCustomMessage"); C("onExit"); C("onFree"); C("onFullScreen"); C("onMalloc"); C("onRealloc"); C("onRuntimeInitialized"); C("postMainLoop"); C("postRun"); C("preInit"); C("preMainLoop"); C("preinitializedWebGLContext"); C("memoryInitializerRequest"); C("preloadPlugins"); C("print"); C("printErr"); C("quit"); C("setStatus"); C("statusMessage"); C("stderr"); C("stdin"); C("stdout"); C("thisProgram"); C("wasm"); C("wasmBinary"); C("websocket"); C("fetchSettings");
				D("arguments", "arguments_"); D("thisProgram", "thisProgram"); D("quit", "quit_"); u("undefined" == typeof e.memoryInitializerPrefixURL, "Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead"); u("undefined" == typeof e.pthreadMainPrefixURL, "Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead"); u("undefined" == typeof e.cdInitializerPrefixURL, "Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead");
				u("undefined" == typeof e.filePackagePrefixURL, "Module.filePackagePrefixURL option was removed, use Module.locateFile instead"); u("undefined" == typeof e.read, "Module.read option was removed (modify read_ in JS)"); u("undefined" == typeof e.readAsync, "Module.readAsync option was removed (modify readAsync in JS)"); u("undefined" == typeof e.readBinary, "Module.readBinary option was removed (modify readBinary in JS)"); u("undefined" == typeof e.setWindowTitle, "Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)");
				u("undefined" == typeof e.TOTAL_MEMORY, "Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY"); D("asm", "wasmExports"); D("read", "read_"); D("readAsync", "readAsync"); D("readBinary", "readBinary"); D("setWindowTitle", "setWindowTitle"); u(!ha, "shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable."); D("wasmBinary", "wasmBinary"); "object" != typeof WebAssembly && g("no native wasm support detected"); var ra, sa = !1;
				function u(a, b) { a || g("Assertion failed" + (b ? ": " + b : "")); } var E, ta, ua, F, G, va, wa; function xa() { var a = ra.buffer; e.HEAP8 = E = new Int8Array(a); e.HEAP16 = ua = new Int16Array(a); e.HEAPU8 = ta = new Uint8Array(a); e.HEAPU16 = new Uint16Array(a); e.HEAP32 = F = new Int32Array(a); e.HEAPU32 = G = new Uint32Array(a); e.HEAPF32 = va = new Float32Array(a); e.HEAPF64 = wa = new Float64Array(a); } u(!e.STACK_SIZE, "STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time");
				u("undefined" != typeof Int32Array && "undefined" !== typeof Float64Array && void 0 != Int32Array.prototype.subarray && void 0 != Int32Array.prototype.set, "JS engine does not provide full typed array support"); u(!e.wasmMemory, "Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally"); u(!e.INITIAL_MEMORY, "Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically");
				function ya() { if (!sa) { var a = za(); 0 == a && (a += 4); var b = G[a >> 2], c = G[a + 4 >> 2]; 34821223 == b && 2310721022 == c || g(`Stack overflow! Stack cookie has been overwritten at ${Aa(a)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${Aa(c)} ${Aa(b)}`); 1668509029 != G[0] && g("Runtime error: The application has corrupted its heap memory area (address zero)!"); } } var Ba = new Int16Array(1), Ca = new Int8Array(Ba.buffer); Ba[0] = 25459;
				if (115 !== Ca[0] || 99 !== Ca[1]) throw "Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)"; var Da = [], Ea = [], Fa = [], Ga = !1; u(Math.imul, "This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill"); u(Math.fround, "This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill"); u(Math.clz32, "This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill");
				u(Math.trunc, "This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill"); var Ha = 0, H = null, Ia = null, Ja = {}; function Ka(a) { for (var b = a; ;) { if (!Ja[a]) return a; a = b + Math.random(); } }
				function La(a) { Ha++; a ? (u(!Ja[a]), Ja[a] = 1, null === H && "undefined" != typeof setInterval && (H = setInterval(() => { if (sa) clearInterval(H), H = null; else { var b = !1, c; for (c in Ja) b || (b = !0, z("still waiting on run dependencies:")), z(`dependency: ${c}`); b && z("(end of list)"); } }, 6E4))) : z("warning: run dependency added without ID"); } function Ma(a) { Ha--; a ? (u(Ja[a]), delete Ja[a]) : z("warning: run dependency removed without ID"); 0 == Ha && (null !== H && (clearInterval(H), H = null), Ia && (a = Ia, Ia = null, a())); }
				function g(a) { a = "Aborted(" + a + ")"; z(a); sa = !0; a = new WebAssembly.RuntimeError(a); ba(a); throw a; } var Na = a => a.startsWith("data:application/octet-stream;base64,"), oa = a => a.startsWith("file://"); function J(a) { return (...b) => { u(Ga, `native function \`${a}\` called before runtime initialization`); var c = Oa[a]; u(c, `exported native function \`${a}\` not found`); return c(...b) } } var L; if (e.locateFile) { if (L = "liblua5.1.wasm", !Na(L)) { var Pa = L; L = e.locateFile ? e.locateFile(Pa, r) : r + Pa; } } else L = (new URL("liblua5.1.wasm", (typeof document === 'undefined' && typeof location === 'undefined' ? "" : typeof document === 'undefined' ? location.href : (_documentCurrentScript && _documentCurrentScript.src || new URL('index.js', document.baseURI).href)))).href;
				function Qa(a) { if (ka) return ka(a); throw "both async and sync fetching of the wasm failed"; } function Ra(a) { if (fa || l) { if ("function" == typeof fetch && !oa(a)) return fetch(a, { credentials: "same-origin" }).then(b => { if (!b.ok) throw `failed to load wasm binary file at '${a}'`; return b.arrayBuffer() }).catch(() => Qa(a)); if (ja) return new Promise((b, c) => { ja(a, d => b(new Uint8Array(d)), c); }) } return Promise.resolve().then(() => Qa(a)) }
				function Sa(a, b, c) { return Ra(a).then(d => WebAssembly.instantiate(d, b)).then(c, d => { z(`failed to asynchronously prepare wasm: ${d}`); oa(L) && z(`warning: Loading from a file URI (${L}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`); g(d); }) }
				function Ta(a, b) { var c = L; return "function" != typeof WebAssembly.instantiateStreaming || Na(c) || oa(c) || n || "function" != typeof fetch ? Sa(c, a, b) : fetch(c, { credentials: "same-origin" }).then(d => WebAssembly.instantiateStreaming(d, a).then(b, function (f) { z(`wasm streaming compile failed: ${f}`); z("falling back to ArrayBuffer instantiation"); return Sa(c, a, b) })) } var M, Ua;
				function D(a, b) { Object.getOwnPropertyDescriptor(e, a) || Object.defineProperty(e, a, { configurable: !0, get() { g(`\`Module.${a}\` has been replaced by \`${b}\`` + " (the initial value can be provided on Module, but after startup the value is only looked for on a local variable of that name)"); } }); } function C(a) { Object.getOwnPropertyDescriptor(e, a) && g(`\`Module.${a}\` was supplied but \`${a}\` not included in INCOMING_MODULE_JS_API`); }
				function Va(a) { return "FS_createPath" === a || "FS_createDataFile" === a || "FS_createPreloadedFile" === a || "FS_unlink" === a || "addRunDependency" === a || "FS_createLazyFile" === a || "FS_createDevice" === a || "removeRunDependency" === a } function Wa(a, b) { "undefined" !== typeof globalThis && Object.defineProperty(globalThis, a, { configurable: !0, get() { N(`\`${a}\` is not longer defined by emscripten. ${b}`); } }); } Wa("buffer", "Please use HEAP8.buffer or wasmMemory.buffer"); Wa("asm", "Please use wasmExports instead");
				function Xa(a) { Object.getOwnPropertyDescriptor(e, a) || Object.defineProperty(e, a, { configurable: !0, get() { var b = `'${a}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`; Va(a) && (b += ". Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you"); g(b); } }); } function pa(a) { this.name = "ExitStatus"; this.message = `Program terminated with exit(${a})`; this.status = a; }
				var Aa = a => { u("number" === typeof a); return "0x" + (a >>> 0).toString(16).padStart(8, "0") }, N = a => { N.la || (N.la = {}); N.la[a] || (N.la[a] = 1, n && (a = "warning: " + a), z(a)); }, Ya = (a, b) => { for (var c = 0, d = a.length - 1; 0 <= d; d--) { var f = a[d]; "." === f ? a.splice(d, 1) : ".." === f ? (a.splice(d, 1), c++) : c && (a.splice(d, 1), c--); } if (b) for (; c; c--)a.unshift(".."); return a }, Q = a => { var b = "/" === a.charAt(0), c = "/" === a.substr(-1); (a = Ya(a.split("/").filter(d => !!d), !b).join("/")) || b || (a = "."); a && c && (a += "/"); return (b ? "/" : "") + a }, Za = a => {
					var b = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/.exec(a).slice(1);
					a = b[0]; b = b[1]; if (!a && !b) return "."; b &&= b.substr(0, b.length - 1); return a + b
				}, $a = a => { if ("/" === a) return "/"; a = Q(a); a = a.replace(/\/$/, ""); var b = a.lastIndexOf("/"); return -1 === b ? a : a.substr(b + 1) }, ab = (a, b) => Q(a + "/" + b), bb = () => { if ("object" == typeof crypto && "function" == typeof crypto.getRandomValues) return c => crypto.getRandomValues(c); if (n) try { var a = require$1("crypto"); if (a.randomFillSync) return c => a.randomFillSync(c); var b = a.randomBytes; return c => (c.set(b(c.byteLength)), c) } catch (c) { } g("no cryptographic support found for randomDevice. consider polyfilling it if you want to use something insecure like Math.random(), e.g. put this in a --pre-js: var crypto = { getRandomValues: (array) => { for (var i = 0; i < array.length; i++) array[i] = (Math.random()*256)|0 } };"); },
					cb = a => (cb = bb())(a), db = (...a) => { for (var b = "", c = !1, d = a.length - 1; -1 <= d && !c; d--) { c = 0 <= d ? a[d] : R.cwd(); if ("string" != typeof c) throw new TypeError("Arguments to path.resolve must be strings"); if (!c) return ""; b = c + "/" + b; c = "/" === c.charAt(0); } b = Ya(b.split("/").filter(f => !!f), !c).join("/"); return (c ? "/" : "") + b || "." }, eb = (a, b) => {
						function c(m) { for (var p = 0; p < m.length && "" === m[p]; p++); for (var y = m.length - 1; 0 <= y && "" === m[y]; y--); return p > y ? [] : m.slice(p, y - p + 1) } a = db(a).substr(1); b = db(b).substr(1); a = c(a.split("/")); b = c(b.split("/"));
						for (var d = Math.min(a.length, b.length), f = d, h = 0; h < d; h++)if (a[h] !== b[h]) { f = h; break } d = []; for (h = f; h < a.length; h++)d.push(".."); d = d.concat(b.slice(f)); return d.join("/")
					}, gb = "undefined" != typeof TextDecoder ? new TextDecoder("utf8") : void 0, hb = (a, b, c, mustReturnUint8Array) => {
						var d = b + c; for (c = b; a[c] && !(c >= d);)++c;
						if (a.buffer && c > 0 && mustReturnUint8Array) return new Uint8Array(a.buffer, b, c - b);
						if (16 < c - b && a.buffer && gb) return gb.decode(a.subarray(b, c)); for (d = ""; b < c;) {
							var f = a[b++]; if (f & 128) {
								var h = a[b++] & 63; if (192 == (f & 224)) d += String.fromCharCode((f & 31) << 6 | h); else {
									var m = a[b++] & 63; 224 == (f & 240) ? f = (f & 15) << 12 | h << 6 | m : (240 !=
										(f & 248) && N("Invalid UTF-8 leading byte " + Aa(f) + " encountered when deserializing a UTF-8 string in wasm memory to a JS string!"), f = (f & 7) << 18 | h << 12 | m << 6 | a[b++] & 63); 65536 > f ? d += String.fromCharCode(f) : (f -= 65536, d += String.fromCharCode(55296 | f >> 10, 56320 | f & 1023));
								}
							} else d += String.fromCharCode(f);
						} return d
					}, ib = [],
					jb = a => {
						if (typeof a !== 'string') return new TextDecoder().decode(a).length;
						for (var b = 0, c = 0; c < a.length; ++c) { var d = a.charCodeAt(c); 127 >= d ? b++ : 2047 >= d ? b += 2 : 55296 <= d && 57343 >= d ? (b += 4, ++c) : b += 3; } return b
					},
					kb = (a, b, c, d) => {
						if (typeof a !== 'string') a = new TextDecoder().decode(a);
						u("string" === typeof a, `stringToUTF8Array expects a string (got ${typeof a})`);
						if (!(0 < d)) return 0; var f = c; d = c + d - 1; for (var h = 0; h < a.length; ++h) {
							var m = a.charCodeAt(h); if (55296 <= m && 57343 >= m) { var p = a.charCodeAt(++h); m = 65536 + ((m & 1023) << 10) | p & 1023; } if (127 >= m) { if (c >= d) break; b[c++] = m; } else {
								if (2047 >= m) { if (c + 1 >= d) break; b[c++] = 192 | m >> 6; } else {
									if (65535 >= m) { if (c + 2 >= d) break; b[c++] = 224 | m >> 12; } else {
										if (c + 3 >= d) break; 1114111 < m && N("Invalid Unicode code point " + Aa(m) + " encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).");
										b[c++] = 240 | m >> 18; b[c++] = 128 | m >> 12 & 63;
									} b[c++] = 128 | m >> 6 & 63;
								} b[c++] = 128 | m & 63;
							}
						} b[c] = 0; return c - f
					}; function lb(a, b) { var c = Array(jb(a) + 1); a = kb(a, c, 0, c.length); b && (c.length = a); return c } var mb = []; function nb(a, b) { mb[a] = { input: [], output: [], K: b }; ob(a, pb); }
				var pb = {
					open(a) { var b = mb[a.node.rdev]; if (!b) throw new R.g(43); a.tty = b; a.seekable = !1; }, close(a) { a.tty.K.fsync(a.tty); }, fsync(a) { a.tty.K.fsync(a.tty); }, read(a, b, c, d) { if (!a.tty || !a.tty.K.sa) throw new R.g(60); for (var f = 0, h = 0; h < d; h++) { try { var m = a.tty.K.sa(a.tty); } catch (p) { throw new R.g(29); } if (void 0 === m && 0 === f) throw new R.g(6); if (null === m || void 0 === m) break; f++; b[c + h] = m; } f && (a.node.timestamp = Date.now()); return f }, write(a, b, c, d) {
						if (!a.tty || !a.tty.K.ia) throw new R.g(60); try {
							for (var f = 0; f < d; f++)a.tty.K.ia(a.tty,
								b[c + f]);
						} catch (h) { throw new R.g(29); } d && (a.node.timestamp = Date.now()); return f
					}
				}, qb = {
					sa() {
						a: {
							if (!ib.length) {
								var a = null; if (n) { var b = Buffer.alloc(256), c = 0, d = process.stdin.fd; try { c = fs.readSync(d, b); } catch (f) { if (f.toString().includes("EOF")) c = 0; else throw f; } 0 < c ? a = b.slice(0, c).toString("utf-8") : a = null; } else "undefined" != typeof window && "function" == typeof window.prompt ? (a = window.prompt("Input: "), null !== a && (a += "\n")) : "function" == typeof readline && (a = readline(), null !== a && (a += "\n")); if (!a) { a = null; break a } ib = lb(a,
									!0);
							} a = ib.shift();
						} return a
					}, ia(a, b) { null === b || 10 === b ? (qa(hb(a.output, 0)), a.output = []) : 0 != b && a.output.push(b); }, fsync(a) { a.output && 0 < a.output.length && (qa(hb(a.output, 0)), a.output = []); }, Ja() { return { ab: 25856, cb: 5, $a: 191, bb: 35387, Za: [3, 28, 127, 21, 4, 0, 1, 0, 17, 19, 26, 0, 18, 15, 23, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] } }, Ka() { return 0 }, La() { return [24, 80] }
				}, rb = { ia(a, b) { null === b || 10 === b ? (z(hb(a.output, 0)), a.output = []) : 0 != b && a.output.push(b); }, fsync(a) { a.output && 0 < a.output.length && (z(hb(a.output, 0)), a.output = []); } }, sb =
						() => { g("internal error: mmapAlloc called but `emscripten_builtin_memalign` native symbol not exported"); }; function tb(a, b) { var c = a.m ? a.m.length : 0; c >= b || (b = Math.max(b, c * (1048576 > c ? 2 : 1.125) >>> 0), 0 != c && (b = Math.max(b, 256)), c = a.m, a.m = new Uint8Array(b), 0 < a.o && a.m.set(c.subarray(0, a.o), 0)); }
				var S = {
					G: null, s() { return S.createNode(null, "/", 16895, 0) }, createNode(a, b, c, d) {
						if (24576 === (c & 61440) || R.isFIFO(c)) throw new R.g(63); S.G || (S.G = { dir: { node: { C: S.h.C, v: S.h.v, lookup: S.h.lookup, J: S.h.J, rename: S.h.rename, unlink: S.h.unlink, rmdir: S.h.rmdir, readdir: S.h.readdir, symlink: S.h.symlink }, stream: { D: S.l.D } }, file: { node: { C: S.h.C, v: S.h.v }, stream: { D: S.l.D, read: S.l.read, write: S.l.write, T: S.l.T, S: S.l.S, V: S.l.V } }, link: { node: { C: S.h.C, v: S.h.v, readlink: S.h.readlink }, stream: {} }, oa: { node: { C: S.h.C, v: S.h.v }, stream: R.Ea } });
						c = R.createNode(a, b, c, d); T(c.mode) ? (c.h = S.G.dir.node, c.l = S.G.dir.stream, c.m = {}) : R.isFile(c.mode) ? (c.h = S.G.file.node, c.l = S.G.file.stream, c.o = 0, c.m = null) : 40960 === (c.mode & 61440) ? (c.h = S.G.link.node, c.l = S.G.link.stream) : 8192 === (c.mode & 61440) && (c.h = S.G.oa.node, c.l = S.G.oa.stream); c.timestamp = Date.now(); a && (a.m[b] = c, a.timestamp = c.timestamp); return c
					}, lb(a) { return a.m ? a.m.subarray ? a.m.subarray(0, a.o) : new Uint8Array(a.m) : new Uint8Array(0) }, h: {
						C(a) {
							var b = {}; b.dev = 8192 === (a.mode & 61440) ? a.id : 1; b.ino = a.id; b.mode =
								a.mode; b.nlink = 1; b.uid = 0; b.gid = 0; b.rdev = a.rdev; T(a.mode) ? b.size = 4096 : R.isFile(a.mode) ? b.size = a.o : 40960 === (a.mode & 61440) ? b.size = a.link.length : b.size = 0; b.atime = new Date(a.timestamp); b.mtime = new Date(a.timestamp); b.ctime = new Date(a.timestamp); b.Ca = 4096; b.blocks = Math.ceil(b.size / b.Ca); return b
						}, v(a, b) {
							void 0 !== b.mode && (a.mode = b.mode); void 0 !== b.timestamp && (a.timestamp = b.timestamp); if (void 0 !== b.size && (b = b.size, a.o != b)) if (0 == b) a.m = null, a.o = 0; else {
								var c = a.m; a.m = new Uint8Array(b); c && a.m.set(c.subarray(0,
									Math.min(b, a.o))); a.o = b;
							}
						}, lookup() { throw R.da[44]; }, J(a, b, c, d) { return S.createNode(a, b, c, d) }, rename(a, b, c) { if (T(a.mode)) { try { var d = U(b, c); } catch (h) { } if (d) for (var f in d.m) throw new R.g(55); } delete a.parent.m[a.name]; a.parent.timestamp = Date.now(); a.name = c; b.m[c] = a; b.timestamp = a.parent.timestamp; a.parent = b; }, unlink(a, b) { delete a.m[b]; a.timestamp = Date.now(); }, rmdir(a, b) { var c = U(a, b), d; for (d in c.m) throw new R.g(55); delete a.m[b]; a.timestamp = Date.now(); }, readdir(a) {
							var b = [".", ".."], c; for (c of Object.keys(a.m)) b.push(c);
							return b
						}, symlink(a, b, c) { a = S.createNode(a, b, 41471, 0); a.link = c; return a }, readlink(a) { if (40960 !== (a.mode & 61440)) throw new R.g(28); return a.link }
					}, l: {
						read(a, b, c, d, f) { var h = a.node.m; if (f >= a.node.o) return 0; a = Math.min(a.node.o - f, d); u(0 <= a); if (8 < a && h.subarray) b.set(h.subarray(f, f + a), c); else for (d = 0; d < a; d++)b[c + d] = h[f + d]; return a }, write(a, b, c, d, f, h) {
							u(!(b instanceof ArrayBuffer)); b.buffer === E.buffer && (h = !1); if (!d) return 0; a = a.node; a.timestamp = Date.now(); if (b.subarray && (!a.m || a.m.subarray)) {
								if (h) return u(0 ===
									f, "canOwn must imply no weird position inside the file"), a.m = b.subarray(c, c + d), a.o = d; if (0 === a.o && 0 === f) return a.m = b.slice(c, c + d), a.o = d; if (f + d <= a.o) return a.m.set(b.subarray(c, c + d), f), d
							} tb(a, f + d); if (a.m.subarray && b.subarray) a.m.set(b.subarray(c, c + d), f); else for (h = 0; h < d; h++)a.m[f + h] = b[c + h]; a.o = Math.max(a.o, f + d); return d
						}, D(a, b, c) { 1 === c ? b += a.position : 2 === c && R.isFile(a.node.mode) && (b += a.node.o); if (0 > b) throw new R.g(28); return b }, T(a, b, c) { tb(a.node, b + c); a.node.o = Math.max(a.node.o, b + c); }, S(a, b, c, d, f) {
							if (!R.isFile(a.node.mode)) throw new R.g(43);
							a = a.node.m; if (f & 2 || a.buffer !== E.buffer) { if (0 < c || c + b < a.length) a.subarray ? a = a.subarray(c, c + b) : a = Array.prototype.slice.call(a, c, c + b); c = !0; b = sb(); if (!b) throw new R.g(48); E.set(a, b); } else c = !1, b = a.byteOffset; return { Sa: b, Ba: c }
						}, V(a, b, c, d) { S.l.write(a, b, 0, d, c, !1); return 0 }
					}
				}, ub = (a, b, c) => { var d = Ka(`al ${a}`); ja(a, f => { u(f, `Loading data file "${a}" failed (no arrayBuffer).`); b(new Uint8Array(f)); d && Ma(d); }, () => { if (c) c(); else throw `Loading data file "${a}" failed.`; }); d && La(d); }, vb = [], wb = (a, b, c, d) => {
					"undefined" !=
						typeof Browser && Browser.R(); var f = !1; vb.forEach(h => { !f && h.canHandle(b) && (h.handle(a, b, c, d), f = !0); }); return f
				}, xb = (a, b) => { var c = 0; a && (c |= 365); b && (c |= 146); return c }, yb = {
					0: "Success", 1: "Arg list too long", 2: "Permission denied", 3: "Address already in use", 4: "Address not available", 5: "Address family not supported by protocol family", 6: "No more processes", 7: "Socket already connected", 8: "Bad file number", 9: "Trying to read unreadable message", 10: "Mount device busy", 11: "Operation canceled", 12: "No children", 13: "Connection aborted",
					14: "Connection refused", 15: "Connection reset by peer", 16: "File locking deadlock error", 17: "Destination address required", 18: "Math arg out of domain of func", 19: "Quota exceeded", 20: "File exists", 21: "Bad address", 22: "File too large", 23: "Host is unreachable", 24: "Identifier removed", 25: "Illegal byte sequence", 26: "Connection already in progress", 27: "Interrupted system call", 28: "Invalid argument", 29: "I/O error", 30: "Socket is already connected", 31: "Is a directory", 32: "Too many symbolic links", 33: "Too many open files",
					34: "Too many links", 35: "Message too long", 36: "Multihop attempted", 37: "File or path name too long", 38: "Network interface is not configured", 39: "Connection reset by network", 40: "Network is unreachable", 41: "Too many open files in system", 42: "No buffer space available", 43: "No such device", 44: "No such file or directory", 45: "Exec format error", 46: "No record locks available", 47: "The link has been severed", 48: "Not enough core", 49: "No message of desired type", 50: "Protocol not available", 51: "No space left on device",
					52: "Function not implemented", 53: "Socket is not connected", 54: "Not a directory", 55: "Directory not empty", 56: "State not recoverable", 57: "Socket operation on non-socket", 59: "Not a typewriter", 60: "No such device or address", 61: "Value too large for defined data type", 62: "Previous owner died", 63: "Not super-user", 64: "Broken pipe", 65: "Protocol error", 66: "Unknown protocol", 67: "Protocol wrong type for socket", 68: "Math result not representable", 69: "Read only file system", 70: "Illegal seek", 71: "No such process",
					72: "Stale file handle", 73: "Connection timed out", 74: "Text file busy", 75: "Cross-device link", 100: "Device not a stream", 101: "Bad font file fmt", 102: "Invalid slot", 103: "Invalid request code", 104: "No anode", 105: "Block device required", 106: "Channel number out of range", 107: "Level 3 halted", 108: "Level 3 reset", 109: "Link number out of range", 110: "Protocol driver not attached", 111: "No CSI structure available", 112: "Level 2 halted", 113: "Invalid exchange", 114: "Invalid request descriptor", 115: "Exchange full",
					116: "No data (for no delay io)", 117: "Timer expired", 118: "Out of streams resources", 119: "Machine is not on the network", 120: "Package not installed", 121: "The object is remote", 122: "Advertise error", 123: "Srmount error", 124: "Communication error on send", 125: "Cross mount point (not really error)", 126: "Given log. name not unique", 127: "f.d. invalid for this operation", 128: "Remote address changed", 129: "Can   access a needed shared lib", 130: "Accessing a corrupted shared lib", 131: ".lib section in a.out corrupted",
					132: "Attempting to link in too many libs", 133: "Attempting to exec a shared library", 135: "Streams pipe error", 136: "Too many users", 137: "Socket type not supported", 138: "Not supported", 139: "Protocol family not supported", 140: "Can't send after socket shutdown", 141: "Too many references", 142: "Host is down", 148: "No medium (in tape drive)", 156: "Level 2 not synchronized"
				}, zb = {
					EPERM: 63, ENOENT: 44, ESRCH: 71, EINTR: 27, EIO: 29, ENXIO: 60, E2BIG: 1, ENOEXEC: 45, EBADF: 8, ECHILD: 12, EAGAIN: 6, EWOULDBLOCK: 6, ENOMEM: 48, EACCES: 2,
					EFAULT: 21, ENOTBLK: 105, EBUSY: 10, EEXIST: 20, EXDEV: 75, ENODEV: 43, ENOTDIR: 54, EISDIR: 31, EINVAL: 28, ENFILE: 41, EMFILE: 33, ENOTTY: 59, ETXTBSY: 74, EFBIG: 22, ENOSPC: 51, ESPIPE: 70, EROFS: 69, EMLINK: 34, EPIPE: 64, EDOM: 18, ERANGE: 68, ENOMSG: 49, EIDRM: 24, ECHRNG: 106, EL2NSYNC: 156, EL3HLT: 107, EL3RST: 108, ELNRNG: 109, EUNATCH: 110, ENOCSI: 111, EL2HLT: 112, EDEADLK: 16, ENOLCK: 46, EBADE: 113, EBADR: 114, EXFULL: 115, ENOANO: 104, EBADRQC: 103, EBADSLT: 102, EDEADLOCK: 16, EBFONT: 101, ENOSTR: 100, ENODATA: 116, ETIME: 117, ENOSR: 118, ENONET: 119, ENOPKG: 120, EREMOTE: 121,
					ENOLINK: 47, EADV: 122, ESRMNT: 123, ECOMM: 124, EPROTO: 65, EMULTIHOP: 36, EDOTDOT: 125, EBADMSG: 9, ENOTUNIQ: 126, EBADFD: 127, EREMCHG: 128, ELIBACC: 129, ELIBBAD: 130, ELIBSCN: 131, ELIBMAX: 132, ELIBEXEC: 133, ENOSYS: 52, ENOTEMPTY: 55, ENAMETOOLONG: 37, ELOOP: 32, EOPNOTSUPP: 138, EPFNOSUPPORT: 139, ECONNRESET: 15, ENOBUFS: 42, EAFNOSUPPORT: 5, EPROTOTYPE: 67, ENOTSOCK: 57, ENOPROTOOPT: 50, ESHUTDOWN: 140, ECONNREFUSED: 14, EADDRINUSE: 3, ECONNABORTED: 13, ENETUNREACH: 40, ENETDOWN: 38, ETIMEDOUT: 73, EHOSTDOWN: 142, EHOSTUNREACH: 23, EINPROGRESS: 26, EALREADY: 7, EDESTADDRREQ: 17,
					EMSGSIZE: 35, EPROTONOSUPPORT: 66, ESOCKTNOSUPPORT: 137, EADDRNOTAVAIL: 4, ENETRESET: 39, EISCONN: 30, ENOTCONN: 53, ETOOMANYREFS: 141, EUSERS: 136, EDQUOT: 19, ESTALE: 72, ENOTSUP: 138, ENOMEDIUM: 148, EILSEQ: 25, EOVERFLOW: 61, ECANCELED: 11, ENOTRECOVERABLE: 56, EOWNERDEAD: 62, ESTRPIPE: 135
				}; function ob(a, b) { R.qa[a] = { l: b }; } function T(a) { return 16384 === (a & 61440) }
				function U(a, b) { var c = T(a.mode) ? (c = Ab(a, "x")) ? c : a.h.lookup ? 0 : 2 : 54; if (c) throw new R.g(c); for (c = R.F[Bb(a.id, b)]; c; c = c.N) { var d = c.name; if (c.parent.id === a.id && d === b) return c } return R.lookup(a, b) }
				function V(a, b = {}) { a = db(a); if (!a) return { path: "", node: null }; b = Object.assign({ ba: !0, ka: 0 }, b); if (8 < b.ka) throw new R.g(32); a = a.split("/").filter(m => !!m); for (var c = R.root, d = "/", f = 0; f < a.length; f++) { var h = f === a.length - 1; if (h && b.parent) break; c = U(c, a[f]); d = Q(d + "/" + a[f]); c.A && (!h || h && b.ba) && (c = c.A.root); if (!h || b.B) for (h = 0; 40960 === (c.mode & 61440);)if (c = R.readlink(d), d = db(Za(d), c), c = V(d, { ka: b.ka + 1 }).node, 40 < h++) throw new R.g(32); } return { path: d, node: c } }
				function Cb(a) { for (var b; ;) { if (R.Z(a)) return a = a.s.va, b ? "/" !== a[a.length - 1] ? `${a}/${b}` : a + b : a; b = b ? `${a.name}/${b}` : a.name; a = a.parent; } } function Bb(a, b) { for (var c = 0, d = 0; d < b.length; d++)c = (c << 5) - c + b.charCodeAt(d) | 0; return (a + c >>> 0) % R.F.length } function Db(a) { var b = Bb(a.parent.id, a.name); a.N = R.F[b]; R.F[b] = a; } function Eb(a) { var b = Bb(a.parent.id, a.name); if (R.F[b] === a) R.F[b] = a.N; else for (b = R.F[b]; b;) { if (b.N === a) { b.N = a.N; break } b = b.N; } } function Fb(a) { var b = ["r", "w", "rw"][a & 3]; a & 512 && (b += "w"); return b }
				function Ab(a, b) { if (R.ua) return 0; if (!b.includes("r") || a.mode & 292) { if (b.includes("w") && !(a.mode & 146) || b.includes("x") && !(a.mode & 73)) return 2 } else return 2; return 0 } function Gb(a, b) { try { return U(a, b), 20 } catch (c) { } return Ab(a, "wx") } function Hb(a, b, c) { try { var d = U(a, b); } catch (f) { return f.u } if (a = Ab(a, "wx")) return a; if (c) { if (!T(d.mode)) return 54; if (R.Z(d) || Cb(d) === R.cwd()) return 10 } else if (T(d.mode)) return 31; return 0 } function Ib() { for (var a = 0; a <= R.ya; a++)if (!R.streams[a]) return a; throw new R.g(33); }
				function W(a) { a = R.ra(a); if (!a) throw new R.g(8); return a } function Jb(a, b = -1) { R.X || (R.X = function () { this.I = {}; }, R.X.prototype = {}, Object.defineProperties(R.X.prototype, { object: { get() { return this.node }, set(c) { this.node = c; } }, flags: { get() { return this.I.flags }, set(c) { this.I.flags = c; } }, position: { get() { return this.I.position }, set(c) { this.I.position = c; } } })); a = Object.assign(new R.X, a); -1 == b && (b = Ib()); a.fd = b; return R.streams[b] = a }
				function Kb(a) { var b = []; for (a = [a]; a.length;) { var c = a.pop(); b.push(c); a.push(...c.U); } return b } function Lb(a, b, c) { "undefined" == typeof c && (c = b, b = 438); return R.J(a, b | 8192, c) }
				function Mb(a, b) { try { var c = V(a, { B: !b }); a = c.path; } catch (f) { } var d = { Z: !1, exists: !1, error: 0, name: null, path: null, object: null, Pa: !1, Ra: null, Qa: null }; try { c = V(a, { parent: !0 }), d.Pa = !0, d.Ra = c.path, d.Qa = c.node, d.name = $a(a), c = V(a, { B: !b }), d.exists = !0, d.path = c.path, d.object = c.node, d.name = c.node.name, d.Z = "/" === c.path; } catch (f) { d.error = f.u; } return d } function Nb(a, b, c, d) { a = "string" == typeof a ? a : Cb(a); b = Q(a + "/" + b); return R.create(b, xb(c, d)) }
				function Ob(a) { if (!(a.Ma || a.Na || a.link || a.m)) { if ("undefined" != typeof XMLHttpRequest) throw Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread."); if (ia) try { a.m = lb(ia(a.url), !0), a.o = a.m.length; } catch (b) { throw new R.g(29); } else throw Error("Cannot load without read() or XMLHttpRequest."); } }
				var R = {
					root: null, U: [], qa: {}, streams: [], Oa: 1, F: null, pa: "/", Y: !1, ua: !0, g: class extends Error { constructor(a) { super(yb[a]); this.name = "ErrnoError"; this.u = a; for (var b in zb) if (zb[b] === a) { this.code = b; break } } }, da: {}, Ga: null, W: 0, createNode(a, b, c, d) { u("object" == typeof a); a = new R.xa(a, b, c, d); Db(a); return a }, Z(a) { return a === a.parent }, isFile(a) { return 32768 === (a & 61440) }, isFIFO(a) { return 4096 === (a & 61440) }, isSocket(a) { return 49152 === (a & 49152) }, ya: 4096, ra: a => R.streams[a], Ea: {
						open(a) { a.l = R.Ha(a.node.rdev).l; a.l.open?.(a); },
						D() { throw new R.g(70); }
					}, ha: a => a >> 8, nb: a => a & 255, M: (a, b) => a << 8 | b, Ha: a => R.qa[a], wa(a, b) { function c(m) { u(0 < R.W); R.W--; return b(m) } function d(m) { if (m) { if (!d.Fa) return d.Fa = !0, c(m) } else ++h >= f.length && c(null); } "function" == typeof a && (b = a, a = !1); R.W++; 1 < R.W && z(`warning: ${R.W} FS.syncfs operations in flight at once, probably just doing extra work`); var f = Kb(R.root.s), h = 0; f.forEach(m => { if (!m.type.wa) return d(null); m.type.wa(m, a, d); }); }, s(a, b, c) {
						if ("string" == typeof a) throw a; var d = "/" === c; if (d && R.root) throw new R.g(10);
						if (!d && c) { var f = V(c, { ba: !1 }); c = f.path; f = f.node; if (f.A) throw new R.g(10); if (!T(f.mode)) throw new R.g(54); } b = { type: a, rb: b, va: c, U: [] }; a = a.s(b); a.s = b; b.root = a; d ? R.root = a : f && (f.A = b, f.s && f.s.U.push(b)); return a
					}, xb(a) { a = V(a, { ba: !1 }); if (!a.node.A) throw new R.g(28); a = a.node; var b = a.A, c = Kb(b); Object.keys(R.F).forEach(d => { for (d = R.F[d]; d;) { var f = d.N; c.includes(d.s) && Eb(d); d = f; } }); a.A = null; b = a.s.U.indexOf(b); u(-1 !== b); a.s.U.splice(b, 1); }, lookup(a, b) { return a.h.lookup(a, b) }, J(a, b, c) {
						var d = V(a, { parent: !0 }).node;
						a = $a(a); if (!a || "." === a || ".." === a) throw new R.g(28); var f = Gb(d, a); if (f) throw new R.g(f); if (!d.h.J) throw new R.g(63); return d.h.J(d, a, b, c)
					}, create(a, b) { return R.J(a, (void 0 !== b ? b : 438) & 4095 | 32768, 0) }, mkdir(a, b) { return R.J(a, (void 0 !== b ? b : 511) & 1023 | 16384, 0) }, ob(a, b) { a = a.split("/"); for (var c = "", d = 0; d < a.length; ++d)if (a[d]) { c += "/" + a[d]; try { R.mkdir(c, b); } catch (f) { if (20 != f.u) throw f; } } }, symlink(a, b) {
						if (!db(a)) throw new R.g(44); var c = V(b, { parent: !0 }).node; if (!c) throw new R.g(44); b = $a(b); var d = Gb(c, b); if (d) throw new R.g(d);
						if (!c.h.symlink) throw new R.g(63); return c.h.symlink(c, b, a)
					}, rename(a, b) {
						var c = Za(a), d = Za(b), f = $a(a), h = $a(b); var m = V(a, { parent: !0 }); var p = m.node; m = V(b, { parent: !0 }); m = m.node; if (!p || !m) throw new R.g(44); if (p.s !== m.s) throw new R.g(75); var y = U(p, f); a = eb(a, d); if ("." !== a.charAt(0)) throw new R.g(28); a = eb(b, c); if ("." !== a.charAt(0)) throw new R.g(55); try { var q = U(m, h); } catch (x) { } if (y !== q) {
							b = T(y.mode); if (f = Hb(p, f, b)) throw new R.g(f); if (f = q ? Hb(m, h, b) : Gb(m, h)) throw new R.g(f); if (!p.h.rename) throw new R.g(63); if (y.A ||
								q && q.A) throw new R.g(10); if (m !== p && (f = Ab(p, "w"))) throw new R.g(f); Eb(y); try { p.h.rename(y, m, h); } catch (x) { throw x; } finally { Db(y); }
						}
					}, rmdir(a) { var b = V(a, { parent: !0 }).node; a = $a(a); var c = U(b, a), d = Hb(b, a, !0); if (d) throw new R.g(d); if (!b.h.rmdir) throw new R.g(63); if (c.A) throw new R.g(10); b.h.rmdir(b, a); Eb(c); }, readdir(a) { a = V(a, { B: !0 }).node; if (!a.h.readdir) throw new R.g(54); return a.h.readdir(a) }, unlink(a) {
						var b = V(a, { parent: !0 }).node; if (!b) throw new R.g(44); a = $a(a); var c = U(b, a), d = Hb(b, a, !1); if (d) throw new R.g(d);
						if (!b.h.unlink) throw new R.g(63); if (c.A) throw new R.g(10); b.h.unlink(b, a); Eb(c);
					}, readlink(a) { a = V(a).node; if (!a) throw new R.g(44); if (!a.h.readlink) throw new R.g(28); return db(Cb(a.parent), a.h.readlink(a)) }, stat(a, b) { a = V(a, { B: !b }).node; if (!a) throw new R.g(44); if (!a.h.C) throw new R.g(63); return a.h.C(a) }, lstat(a) { return R.stat(a, !0) }, chmod(a, b, c) { a = "string" == typeof a ? V(a, { B: !c }).node : a; if (!a.h.v) throw new R.g(63); a.h.v(a, { mode: b & 4095 | a.mode & -4096, timestamp: Date.now() }); }, lchmod(a, b) { R.chmod(a, b, !0); },
					fchmod(a, b) { a = W(a); R.chmod(a.node, b); }, chown(a, b, c, d) { a = "string" == typeof a ? V(a, { B: !d }).node : a; if (!a.h.v) throw new R.g(63); a.h.v(a, { timestamp: Date.now() }); }, lchown(a, b, c) { R.chown(a, b, c, !0); }, fchown(a, b, c) { a = W(a); R.chown(a.node, b, c); }, truncate(a, b) { if (0 > b) throw new R.g(28); a = "string" == typeof a ? V(a, { B: !0 }).node : a; if (!a.h.v) throw new R.g(63); if (T(a.mode)) throw new R.g(31); if (!R.isFile(a.mode)) throw new R.g(28); var c = Ab(a, "w"); if (c) throw new R.g(c); a.h.v(a, { size: b, timestamp: Date.now() }); }, kb(a, b) {
						a = W(a); if (0 ===
							(a.flags & 2097155)) throw new R.g(28); R.truncate(a.node, b);
					}, yb(a, b, c) { a = V(a, { B: !0 }).node; a.h.v(a, { timestamp: Math.max(b, c) }); }, open(a, b, c) {
						if ("" === a) throw new R.g(44); if ("string" == typeof b) { var d = { r: 0, "r+": 2, w: 577, "w+": 578, a: 1089, "a+": 1090 }[b]; if ("undefined" == typeof d) throw Error(`Unknown file open mode: ${b}`); b = d; } c = b & 64 ? ("undefined" == typeof c ? 438 : c) & 4095 | 32768 : 0; if ("object" == typeof a) var f = a; else { a = Q(a); try { f = V(a, { B: !(b & 131072) }).node; } catch (h) { } } d = !1; if (b & 64) if (f) { if (b & 128) throw new R.g(20); } else f =
							R.J(a, c, 0), d = !0; if (!f) throw new R.g(44); 8192 === (f.mode & 61440) && (b &= -513); if (b & 65536 && !T(f.mode)) throw new R.g(54); if (!d && (c = f ? 40960 === (f.mode & 61440) ? 32 : T(f.mode) && ("r" !== Fb(b) || b & 512) ? 31 : Ab(f, Fb(b)) : 44)) throw new R.g(c); b & 512 && !d && R.truncate(f, 0); b &= -131713; f = Jb({ node: f, path: Cb(f), flags: b, seekable: !0, position: 0, l: f.l, Xa: [], error: !1 }); f.l.open && f.l.open(f); !e.logReadFiles || b & 1 || (R.ja || (R.ja = {}), a in R.ja || (R.ja[a] = 1)); return f
					}, close(a) {
						if (null === a.fd) throw new R.g(8); a.ea && (a.ea = null); try {
							a.l.close &&
								a.l.close(a);
						} catch (b) { throw b; } finally { R.streams[a.fd] = null; } a.fd = null;
					}, D(a, b, c) { if (null === a.fd) throw new R.g(8); if (!a.seekable || !a.l.D) throw new R.g(70); if (0 != c && 1 != c && 2 != c) throw new R.g(28); a.position = a.l.D(a, b, c); a.Xa = []; return a.position }, read(a, b, c, d, f) {
						u(0 <= c); if (0 > d || 0 > f) throw new R.g(28); if (null === a.fd) throw new R.g(8); if (1 === (a.flags & 2097155)) throw new R.g(8); if (T(a.node.mode)) throw new R.g(31); if (!a.l.read) throw new R.g(28); var h = "undefined" != typeof f; if (!h) f = a.position; else if (!a.seekable) throw new R.g(70);
						b = a.l.read(a, b, c, d, f); h || (a.position += b); return b
					}, write(a, b, c, d, f, h) { u(0 <= c); if (0 > d || 0 > f) throw new R.g(28); if (null === a.fd) throw new R.g(8); if (0 === (a.flags & 2097155)) throw new R.g(8); if (T(a.node.mode)) throw new R.g(31); if (!a.l.write) throw new R.g(28); a.seekable && a.flags & 1024 && R.D(a, 0, 2); var m = "undefined" != typeof f; if (!m) f = a.position; else if (!a.seekable) throw new R.g(70); b = a.l.write(a, b, c, d, f, h); m || (a.position += b); return b }, T(a, b, c) {
						if (null === a.fd) throw new R.g(8); if (0 > b || 0 >= c) throw new R.g(28); if (0 ===
							(a.flags & 2097155)) throw new R.g(8); if (!R.isFile(a.node.mode) && !T(a.node.mode)) throw new R.g(43); if (!a.l.T) throw new R.g(138); a.l.T(a, b, c);
					}, S(a, b, c, d, f) { if (0 !== (d & 2) && 0 === (f & 2) && 2 !== (a.flags & 2097155)) throw new R.g(2); if (1 === (a.flags & 2097155)) throw new R.g(2); if (!a.l.S) throw new R.g(43); return a.l.S(a, b, c, d, f) }, V(a, b, c, d, f) { u(0 <= c); return a.l.V ? a.l.V(a, b, c, d, f) : 0 }, qb: () => 0, fa(a, b, c) { if (!a.l.fa) throw new R.g(59); return a.l.fa(a, b, c) }, readFile(a, b = {}) {
						b.flags = b.flags || 0; b.encoding = b.encoding || "binary";
						if ("utf8" !== b.encoding && "binary" !== b.encoding) throw Error(`Invalid encoding type "${b.encoding}"`); var c, d = R.open(a, b.flags); a = R.stat(a).size; var f = new Uint8Array(a); R.read(d, f, 0, a, 0); "utf8" === b.encoding ? c = hb(f, 0) : "binary" === b.encoding && (c = f); R.close(d); return c
					}, writeFile(a, b, c = {}) {
						c.flags = c.flags || 577; a = R.open(a, c.flags, c.mode); if ("string" == typeof b) { var d = new Uint8Array(jb(b) + 1); b = kb(b, d, 0, d.length); R.write(a, d, 0, b, void 0, c.Da); } else if (ArrayBuffer.isView(b)) R.write(a, b, 0, b.byteLength, void 0, c.Da);
						else throw Error("Unsupported data type"); R.close(a);
					}, cwd: () => R.pa, chdir(a) { a = V(a, { B: !0 }); if (null === a.node) throw new R.g(44); if (!T(a.node.mode)) throw new R.g(54); var b = Ab(a.node, "x"); if (b) throw new R.g(b); R.pa = a.path; }, R(a, b, c) {
						u(!R.R.Y, "FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)"); R.R.Y = !0; e.stdin = a || e.stdin; e.stdout = b || e.stdout; e.stderr = c || e.stderr; e.stdin ? R.L("/dev",
							"stdin", e.stdin) : R.symlink("/dev/tty", "/dev/stdin"); e.stdout ? R.L("/dev", "stdout", null, e.stdout) : R.symlink("/dev/tty", "/dev/stdout"); e.stderr ? R.L("/dev", "stderr", null, e.stderr) : R.symlink("/dev/tty1", "/dev/stderr"); a = R.open("/dev/stdin", 0); b = R.open("/dev/stdout", 1); c = R.open("/dev/stderr", 1); u(0 === a.fd, `invalid handle for stdin (${a.fd})`); u(1 === b.fd, `invalid handle for stdout (${b.fd})`); u(2 === c.fd, `invalid handle for stderr (${c.fd})`);
					}, sb() {
						R.R.Y = !1; Pb(0); for (var a = 0; a < R.streams.length; a++) {
							var b =
								R.streams[a]; b && R.close(b);
						}
					}, jb(a, b) { a = Mb(a, b); return a.exists ? a.object : null }, hb(a, b) { a = "string" == typeof a ? a : Cb(a); for (b = b.split("/").reverse(); b.length;) { var c = b.pop(); if (c) { var d = Q(a + "/" + c); try { R.mkdir(d); } catch (f) { } a = d; } } return d }, L(a, b, c, d) {
						a = ab("string" == typeof a ? a : Cb(a), b); b = xb(!!c, !!d); R.L.ha || (R.L.ha = 64); var f = R.M(R.L.ha++, 0); ob(f, {
							open(h) { h.seekable = !1; }, close() { d?.buffer?.length && d(10); }, read(h, m, p, y) {
								for (var q = 0, x = 0; x < y; x++) {
									try { var t = c(); } catch (B) { throw new R.g(29); } if (void 0 === t && 0 === q) throw new R.g(6);
									if (null === t || void 0 === t) break; q++; m[p + x] = t;
								} q && (h.node.timestamp = Date.now()); return q
							}, write(h, m, p, y) { for (var q = 0; q < y; q++)try { d(m[p + q]); } catch (x) { throw new R.g(29); } y && (h.node.timestamp = Date.now()); return q }
						}); return Lb(a, b, f)
					}, fb(a, b, c, d, f) {
						function h() { this.ga = !1; this.I = []; } function m(t, B, k, w, v) { t = t.node.m; if (v >= t.length) return 0; w = Math.min(t.length - v, w); u(0 <= w); if (t.slice) for (var A = 0; A < w; A++)B[k + A] = t[v + A]; else for (A = 0; A < w; A++)B[k + A] = t.get(v + A); return w } h.prototype.get = function (t) {
							if (!(t > this.length -
								1 || 0 > t)) { var B = t % this.chunkSize; return this.ta(t / this.chunkSize | 0)[B] }
						}; h.prototype.Ia = function (t) { this.ta = t; }; h.prototype.na = function () {
							var t = new XMLHttpRequest; t.open("HEAD", c, !1); t.send(null); if (!(200 <= t.status && 300 > t.status || 304 === t.status)) throw Error("Couldn't load " + c + ". Status: " + t.status); var B = Number(t.getResponseHeader("Content-length")), k, w = (k = t.getResponseHeader("Accept-Ranges")) && "bytes" === k; t = (k = t.getResponseHeader("Content-Encoding")) && "gzip" === k; var v = 1048576; w || (v = B); var A = this; A.Ia(I => {
								var O = I * v, P = (I + 1) * v - 1; P = Math.min(P, B - 1); if ("undefined" == typeof A.I[I]) {
									var fb = A.I; if (O > P) throw Error("invalid range (" + O + ", " + P + ") or no bytes requested!"); if (P > B - 1) throw Error("only " + B + " bytes available! programmer error!"); var K = new XMLHttpRequest; K.open("GET", c, !1); B !== v && K.setRequestHeader("Range", "bytes=" + O + "-" + P); K.responseType = "arraybuffer"; K.overrideMimeType && K.overrideMimeType("text/plain; charset=x-user-defined"); K.send(null); if (!(200 <= K.status && 300 > K.status || 304 === K.status)) throw Error("Couldn't load " +
										c + ". Status: " + K.status); O = void 0 !== K.response ? new Uint8Array(K.response || []) : lb(K.responseText || "", !0); fb[I] = O;
								} if ("undefined" == typeof A.I[I]) throw Error("doXHR failed!"); return A.I[I]
							}); if (t || !B) v = B = 1, v = B = this.ta(0).length, qa("LazyFiles on gzip forces download of the whole file when length is accessed"); this.Aa = B; this.za = v; this.ga = !0;
						}; if ("undefined" != typeof XMLHttpRequest) {
							if (!l) throw "Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";
							var p = new h; Object.defineProperties(p, { length: { get: function () { this.ga || this.na(); return this.Aa } }, chunkSize: { get: function () { this.ga || this.na(); return this.za } } }); var y = void 0;
						} else y = c, p = void 0; var q = Nb(a, b, d, f); p ? q.m = p : y && (q.m = null, q.url = y); Object.defineProperties(q, { o: { get: function () { return this.m.length } } }); var x = {}; Object.keys(q.l).forEach(t => { var B = q.l[t]; x[t] = (...k) => { Ob(q); return B(...k) }; }); x.read = (t, B, k, w, v) => { Ob(q); return m(t, B, k, w, v) }; x.S = (t, B, k) => {
							Ob(q); var w = sb(); if (!w) throw new R.g(48);
							m(t, E, w, B, k); return { Sa: w, Ba: !0 }
						}; q.l = x; return q
					}, Ya() { g("FS.absolutePath has been removed; use PATH_FS.resolve instead"); }, eb() { g("FS.createFolder has been removed; use FS.mkdir instead"); }, gb() { g("FS.createLink has been removed; use FS.symlink instead"); }, mb() { g("FS.joinPath has been removed; use PATH.join instead"); }, pb() { g("FS.mmapAlloc has been replaced by the top level function mmapAlloc"); }, vb() { g("FS.standardizePath has been removed; use PATH.normalize instead"); }
				}, X = (a, b, mustReturnUint8Array) => {
					u("number" == typeof a,
						`UTF8ToString expects a number (got ${typeof a})`); return a ? hb(ta, a, b, mustReturnUint8Array) : ""
				}; function Qb(a, b) { if ("/" === b.charAt(0)) return b; a = -100 === a ? R.cwd() : W(a).path; if (0 == b.length) throw new R.g(44); return Q(a + "/" + b) } var Rb = void 0; function Y() { u(void 0 != Rb); var a = F[+Rb >> 2]; Rb += 4; return a }
				var Sb = (a, b, c) => { u("number" == typeof c, "stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!"); return kb(a, ta, b, c) }, Tb = (a, b) => { u(a == a >>> 0 || a == (a | 0)); u(b === (b | 0)); return b + 2097152 >>> 0 < 4194305 - !!a ? (a >>> 0) + 4294967296 * b : NaN }, Ub = a => 0 === a % 4 && (0 !== a % 100 || 0 === a % 400), Vb = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335], Wb = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334], Yb = a => { var b = jb(a) + 1, c = Xb(b); c && Sb(a, c, b); return c }, Zb = {}, ac = () => {
					if (!$b) {
						var a =
							{ USER: "web_user", LOGNAME: "web_user", PATH: "/", PWD: "/", HOME: "/home/web_user", LANG: ("object" == typeof navigator && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8", _: da || "./this.program" }, b; for (b in Zb) void 0 === Zb[b] ? delete a[b] : a[b] = Zb[b]; var c = []; for (b in a) c.push(`${b}=${a[b]}`); $b = c;
					} return $b
				}, $b, bc = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31], cc = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31], dc = (a, b) => {
					u(0 <= a.length, "writeArrayToMemory array must have a length (should be an array or typed array)");
					E.set(a, b);
				}, ec = [], Z, fc = a => { var b = ec[a]; b || (a >= ec.length && (ec.length = a + 1), ec[a] = b = Z.get(a)); u(Z.get(a) == b, "JavaScript-side Wasm function table mirror is out of date!"); return b }, gc = a => { var b = e["_" + a]; u(b, "Cannot call unknown function " + a + ", make sure it is exported"); return b }, hc, ic = []; function jc(a, b, c, d) { a ||= this; this.parent = a; this.s = a.s; this.A = null; this.id = R.Oa++; this.name = b; this.mode = c; this.h = {}; this.l = {}; this.rdev = d; }
				Object.defineProperties(jc.prototype, { read: { get: function () { return 365 === (this.mode & 365) }, set: function (a) { a ? this.mode |= 365 : this.mode &= -366; } }, write: { get: function () { return 146 === (this.mode & 146) }, set: function (a) { a ? this.mode |= 146 : this.mode &= -147; } }, Na: { get: function () { return T(this.mode) } }, Ma: { get: function () { return 8192 === (this.mode & 61440) } } }); R.xa = jc;
				R.ib = (a, b, c, d, f, h, m, p, y, q) => { function x(k) { function w(v) { q?.(); if (!p) { var A = a, I = b; A && (A = "string" == typeof A ? A : Cb(A), I = b ? Q(A + "/" + b) : A); A = xb(d, f); I = R.create(I, A); if (v) { if ("string" == typeof v) { for (var O = Array(v.length), P = 0, fb = v.length; P < fb; ++P)O[P] = v.charCodeAt(P); v = O; } R.chmod(I, A | 146); O = R.open(I, 577); R.write(O, v, 0, v.length, 0, y); R.close(O); R.chmod(I, A); } } h?.(); Ma(B); } wb(k, t, w, () => { m?.(); Ma(B); }) || w(k); } var t = b ? db(Q(a + "/" + b)) : a, B = Ka(`cp ${t}`); La(B); "string" == typeof c ? ub(c, x, m) : x(c); };
				[44].forEach(a => { R.da[a] = new R.g(a); R.da[a].stack = "<generic error, no stack>"; }); R.F = Array(4096); R.s(S, {}, "/"); R.mkdir("/tmp"); R.mkdir("/home"); R.mkdir("/home/web_user"); (function () { R.mkdir("/dev"); ob(R.M(1, 3), { read: () => 0, write: (d, f, h, m) => m }); Lb("/dev/null", R.M(1, 3)); nb(R.M(5, 0), qb); nb(R.M(6, 0), rb); Lb("/dev/tty", R.M(5, 0)); Lb("/dev/tty1", R.M(6, 0)); var a = new Uint8Array(1024), b = 0, c = () => { 0 === b && (b = cb(a).byteLength); return a[--b] }; R.L("/dev", "random", c); R.L("/dev", "urandom", c); R.mkdir("/dev/shm"); R.mkdir("/dev/shm/tmp"); })();
				(function () { R.mkdir("/proc"); var a = R.mkdir("/proc/self"); R.mkdir("/proc/self/fd"); R.s({ s() { var b = R.createNode(a, "fd", 16895, 73); b.h = { lookup(c, d) { var f = W(+d); c = { parent: null, s: { va: "fake" }, h: { readlink: () => f.path } }; return c.parent = c } }; return b } }, {}, "/proc/self/fd"); })(); R.Ga = { MEMFS: S };
				var nc = {
					__syscall_dup3: function (a, b, c) { try { var d = W(a); u(!c); if (d.fd === b) return -28; var f = R.ra(b); f && R.close(f); return Jb(d, b).fd } catch (h) { if ("undefined" == typeof R || "ErrnoError" !== h.name) throw h; return -h.u } }, __syscall_fcntl64: function (a, b, c) {
						Rb = c; try { var d = W(a); switch (b) { case 0: var f = Y(); if (0 > f) break; for (; R.streams[f];)f++; return Jb(d, f).fd; case 1: case 2: return 0; case 3: return d.flags; case 4: return f = Y(), d.flags |= f, 0; case 12: return f = Y(), ua[f + 0 >> 1] = 2, 0; case 13: case 14: return 0 }return -28 } catch (h) {
							if ("undefined" ==
								typeof R || "ErrnoError" !== h.name) throw h; return -h.u
						}
					}, __syscall_ioctl: function (a, b, c) {
						Rb = c; try {
							var d = W(a); switch (b) {
								case 21509: return d.tty ? 0 : -59; case 21505: if (!d.tty) return -59; if (d.tty.K.Ja) { a = [3, 28, 127, 21, 4, 0, 1, 0, 17, 19, 26, 0, 18, 15, 23, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]; var f = Y(); F[f >> 2] = 25856; F[f + 4 >> 2] = 5; F[f + 8 >> 2] = 191; F[f + 12 >> 2] = 35387; for (var h = 0; 32 > h; h++)E[f + h + 17] = a[h] || 0; } return 0; case 21510: case 21511: case 21512: return d.tty ? 0 : -59; case 21506: case 21507: case 21508: if (!d.tty) return -59; if (d.tty.K.Ka) for (f =
									Y(), a = [], h = 0; 32 > h; h++)a.push(E[f + h + 17]); return 0; case 21519: if (!d.tty) return -59; f = Y(); return F[f >> 2] = 0; case 21520: return d.tty ? -28 : -59; case 21531: return f = Y(), R.fa(d, b, f); case 21523: if (!d.tty) return -59; d.tty.K.La && (h = [24, 80], f = Y(), ua[f >> 1] = h[0], ua[f + 2 >> 1] = h[1]); return 0; case 21524: return d.tty ? 0 : -59; case 21515: return d.tty ? 0 : -59; default: return -28
							}
						} catch (m) { if ("undefined" == typeof R || "ErrnoError" !== m.name) throw m; return -m.u }
					}, __syscall_openat: function (a, b, c, d) {
						Rb = d; try {
							b = X(b); b = Qb(a, b); var f = d ? Y() :
								0; return R.open(b, c, f).fd
						} catch (h) { if ("undefined" == typeof R || "ErrnoError" !== h.name) throw h; return -h.u }
					}, __syscall_readlinkat: function (a, b, c, d) { try { b = X(b); b = Qb(a, b); if (0 >= d) return -28; var f = R.readlink(b), h = Math.min(d, jb(f)), m = E[c + h]; Sb(f, c, d + 1); E[c + h] = m; return h } catch (p) { if ("undefined" == typeof R || "ErrnoError" !== p.name) throw p; return -p.u } }, __syscall_renameat: function (a, b, c, d) {
						try { return b = X(b), d = X(d), b = Qb(a, b), d = Qb(c, d), R.rename(b, d), 0 } catch (f) {
							if ("undefined" == typeof R || "ErrnoError" !== f.name) throw f;
							return -f.u
						}
					}, __syscall_rmdir: function (a) { try { return a = X(a), R.rmdir(a), 0 } catch (b) { if ("undefined" == typeof R || "ErrnoError" !== b.name) throw b; return -b.u } }, __syscall_unlinkat: function (a, b, c) { try { return b = X(b), b = Qb(a, b), 0 === c ? R.unlink(b) : 512 === c ? R.rmdir(b) : g("Invalid flags passed to unlinkat"), 0 } catch (d) { if ("undefined" == typeof R || "ErrnoError" !== d.name) throw d; return -d.u } }, _emscripten_get_now_is_monotonic: () => 1, _emscripten_system: a => {
						if (n) {
							if (!a) return 1; a = X(a); if (!a.length) return 0; a = require$1("child_process").ub(a,
								[], { tb: !0, stdio: "inherit" }); var b = (c, d) => c << 8 | d; return null === a.status ? b(0, (c => { switch (c) { case "SIGHUP": return 1; case "SIGQUIT": return 3; case "SIGFPE": return 8; case "SIGKILL": return 9; case "SIGALRM": return 14; case "SIGTERM": return 15 }return 2 })(a.signal)) : a.status << 8 | 0
						} return a ? -52 : 0
					}, _emscripten_throw_longjmp: () => { throw Infinity; }, _gmtime_js: function (a, b, c) {
						a = Tb(a, b); a = new Date(1E3 * a); F[c >> 2] = a.getUTCSeconds(); F[c + 4 >> 2] = a.getUTCMinutes(); F[c + 8 >> 2] = a.getUTCHours(); F[c + 12 >> 2] = a.getUTCDate(); F[c + 16 >>
							2] = a.getUTCMonth(); F[c + 20 >> 2] = a.getUTCFullYear() - 1900; F[c + 24 >> 2] = a.getUTCDay(); F[c + 28 >> 2] = (a.getTime() - Date.UTC(a.getUTCFullYear(), 0, 1, 0, 0, 0, 0)) / 864E5 | 0;
					}, _localtime_js: function (a, b, c) {
						a = Tb(a, b); a = new Date(1E3 * a); F[c >> 2] = a.getSeconds(); F[c + 4 >> 2] = a.getMinutes(); F[c + 8 >> 2] = a.getHours(); F[c + 12 >> 2] = a.getDate(); F[c + 16 >> 2] = a.getMonth(); F[c + 20 >> 2] = a.getFullYear() - 1900; F[c + 24 >> 2] = a.getDay(); F[c + 28 >> 2] = (Ub(a.getFullYear()) ? Vb : Wb)[a.getMonth()] + a.getDate() - 1 | 0; F[c + 36 >> 2] = -(60 * a.getTimezoneOffset()); b = (new Date(a.getFullYear(),
							6, 1)).getTimezoneOffset(); var d = (new Date(a.getFullYear(), 0, 1)).getTimezoneOffset(); F[c + 32 >> 2] = (b != d && a.getTimezoneOffset() == Math.min(d, b)) | 0;
					}, _mktime_js: function (a) {
						var b = new Date(F[a + 20 >> 2] + 1900, F[a + 16 >> 2], F[a + 12 >> 2], F[a + 8 >> 2], F[a + 4 >> 2], F[a >> 2], 0), c = F[a + 32 >> 2], d = b.getTimezoneOffset(), f = (new Date(b.getFullYear(), 6, 1)).getTimezoneOffset(), h = (new Date(b.getFullYear(), 0, 1)).getTimezoneOffset(), m = Math.min(h, f); 0 > c ? F[a + 32 >> 2] = Number(f != h && m == d) : 0 < c != (m == d) && (f = Math.max(h, f), b.setTime(b.getTime() + 6E4 *
							((0 < c ? m : f) - d))); F[a + 24 >> 2] = b.getDay(); F[a + 28 >> 2] = (Ub(b.getFullYear()) ? Vb : Wb)[b.getMonth()] + b.getDate() - 1 | 0; F[a >> 2] = b.getSeconds(); F[a + 4 >> 2] = b.getMinutes(); F[a + 8 >> 2] = b.getHours(); F[a + 12 >> 2] = b.getDate(); F[a + 16 >> 2] = b.getMonth(); F[a + 20 >> 2] = b.getYear(); a = b.getTime(); a = isNaN(a) ? -1 : a / 1E3; return kc((M = a, 1 <= +Math.abs(M) ? 0 < M ? +Math.floor(M / 4294967296) >>> 0 : ~~+Math.ceil((M - +(~~M >>> 0)) / 4294967296) >>> 0 : 0)), a >>> 0
					}, _tzset_js: (a, b, c) => {
						function d(y) { return (y = y.toTimeString().match(/\(([A-Za-z ]+)\)$/)) ? y[1] : "GMT" }
						var f = (new Date).getFullYear(), h = new Date(f, 0, 1), m = new Date(f, 6, 1); f = h.getTimezoneOffset(); var p = m.getTimezoneOffset(); G[a >> 2] = 60 * Math.max(f, p); F[b >> 2] = Number(f != p); a = d(h); b = d(m); a = Yb(a); b = Yb(b); p < f ? (G[c >> 2] = a, G[c + 4 >> 2] = b) : (G[c >> 2] = b, G[c + 4 >> 2] = a);
					}, emscripten_date_now: () => Date.now(), emscripten_get_now: () => performance.now(), emscripten_memcpy_js: (a, b, c) => ta.copyWithin(a, b, b + c), emscripten_resize_heap: a => {
						var b = ta.length; a >>>= 0; u(a > b); if (2147483648 < a) return z(`Cannot enlarge memory, requested ${a} bytes, but the limit is ${2147483648} bytes!`),
							!1; for (var c = 1; 4 >= c; c *= 2) { var d = b * (1 + .2 / c); d = Math.min(d, a + 100663296); var f = Math; d = Math.max(a, d); f = f.min.call(f, 2147483648, d + (65536 - d % 65536) % 65536); a: { d = f; var h = ra.buffer, m = (d - h.byteLength + 65535) / 65536; try { ra.grow(m); xa(); var p = 1; break a } catch (y) { z(`growMemory: Attempted to grow heap from ${h.byteLength} bytes to ${d} bytes, but got error: ${y}`); } p = void 0; } if (p) return !0 } z(`Failed to grow the heap from ${b} bytes to ${f} bytes, not enough memory!`); return !1
					}, environ_get: (a, b) => {
						var c = 0; ac().forEach((d,
							f) => { var h = b + c; f = G[a + 4 * f >> 2] = h; for (h = 0; h < d.length; ++h)u(d.charCodeAt(h) === (d.charCodeAt(h) & 255)), E[f++] = d.charCodeAt(h); E[f] = 0; c += d.length + 1; }); return 0
					}, environ_sizes_get: (a, b) => { var c = ac(); G[a >> 2] = c.length; var d = 0; c.forEach(f => d += f.length + 1); G[b >> 2] = d; return 0 }, exit: a => { lc(); sa = !0; ea(a, new pa(a)); }, fd_close: function (a) { try { var b = W(a); R.close(b); return 0 } catch (c) { if ("undefined" == typeof R || "ErrnoError" !== c.name) throw c; return c.u } }, fd_read: function (a, b, c, d) {
						try {
							a: {
								var f = W(a); a = b; for (var h, m = b = 0; m < c; m++) {
									var p =
										G[a >> 2], y = G[a + 4 >> 2]; a += 8; var q = R.read(f, E, p, y, h); if (0 > q) { var x = -1; break a } b += q; if (q < y) break; "undefined" !== typeof h && (h += q);
								} x = b;
							} G[d >> 2] = x; return 0
						} catch (t) { if ("undefined" == typeof R || "ErrnoError" !== t.name) throw t; return t.u }
					}, fd_seek: function (a, b, c, d, f) {
						b = Tb(b, c); try {
							if (isNaN(b)) return 61; var h = W(a); R.D(h, b, d); Ua = [h.position >>> 0, (M = h.position, 1 <= +Math.abs(M) ? 0 < M ? +Math.floor(M / 4294967296) >>> 0 : ~~+Math.ceil((M - +(~~M >>> 0)) / 4294967296) >>> 0 : 0)]; F[f >> 2] = Ua[0]; F[f + 4 >> 2] = Ua[1]; h.ea && 0 === b && 0 === d && (h.ea = null);
							return 0
						} catch (m) { if ("undefined" == typeof R || "ErrnoError" !== m.name) throw m; return m.u }
					}, fd_write: function (a, b, c, d) { try { a: { var f = W(a); a = b; for (var h, m = b = 0; m < c; m++) { var p = G[a >> 2], y = G[a + 4 >> 2]; a += 8; var q = R.write(f, E, p, y, h); if (0 > q) { var x = -1; break a } b += q; "undefined" !== typeof h && (h += q); } x = b; } G[d >> 2] = x; return 0 } catch (t) { if ("undefined" == typeof R || "ErrnoError" !== t.name) throw t; return t.u } }, invoke_vii: mc, strftime: (a, b, c, d) => {
						function f(k, w, v) { for (k = "number" == typeof k ? k.toString() : k || ""; k.length < w;)k = v[0] + k; return k }
						function h(k, w) { return f(k, w, "0") } function m(k, w) { function v(I) { return 0 > I ? -1 : 0 < I ? 1 : 0 } var A; 0 === (A = v(k.getFullYear() - w.getFullYear())) && 0 === (A = v(k.getMonth() - w.getMonth())) && (A = v(k.getDate() - w.getDate())); return A } function p(k) {
							switch (k.getDay()) {
								case 0: return new Date(k.getFullYear() - 1, 11, 29); case 1: return k; case 2: return new Date(k.getFullYear(), 0, 3); case 3: return new Date(k.getFullYear(), 0, 2); case 4: return new Date(k.getFullYear(), 0, 1); case 5: return new Date(k.getFullYear() - 1, 11, 31); case 6: return new Date(k.getFullYear() -
									1, 11, 30)
							}
						} function y(k) { var w = k.O; for (k = new Date((new Date(k.P + 1900, 0, 1)).getTime()); 0 < w;) { var v = k.getMonth(), A = (Ub(k.getFullYear()) ? bc : cc)[v]; if (w > A - k.getDate()) w -= A - k.getDate() + 1, k.setDate(1), 11 > v ? k.setMonth(v + 1) : (k.setMonth(0), k.setFullYear(k.getFullYear() + 1)); else { k.setDate(k.getDate() + w); break } } v = new Date(k.getFullYear() + 1, 0, 4); w = p(new Date(k.getFullYear(), 0, 4)); v = p(v); return 0 >= m(w, k) ? 0 >= m(v, k) ? k.getFullYear() + 1 : k.getFullYear() : k.getFullYear() - 1 } var q = G[d + 40 >> 2]; d = {
							Va: F[d >> 2], Ua: F[d + 4 >> 2], $: F[d +
								8 >> 2], ma: F[d + 12 >> 2], aa: F[d + 16 >> 2], P: F[d + 20 >> 2], H: F[d + 24 >> 2], O: F[d + 28 >> 2], wb: F[d + 32 >> 2], Ta: F[d + 36 >> 2], Wa: q ? X(q) : ""
						}; c = X(c); q = { "%c": "%a %b %d %H:%M:%S %Y", "%D": "%m/%d/%y", "%F": "%Y-%m-%d", "%h": "%b", "%r": "%I:%M:%S %p", "%R": "%H:%M", "%T": "%H:%M:%S", "%x": "%m/%d/%y", "%X": "%H:%M:%S", "%Ec": "%c", "%EC": "%C", "%Ex": "%m/%d/%y", "%EX": "%H:%M:%S", "%Ey": "%y", "%EY": "%Y", "%Od": "%d", "%Oe": "%e", "%OH": "%H", "%OI": "%I", "%Om": "%m", "%OM": "%M", "%OS": "%S", "%Ou": "%u", "%OU": "%U", "%OV": "%V", "%Ow": "%w", "%OW": "%W", "%Oy": "%y" }; for (var x in q) c =
							c.replace(new RegExp(x, "g"), q[x]); var t = "Sunday Monday Tuesday Wednesday Thursday Friday Saturday".split(" "), B = "January February March April May June July August September October November December".split(" "); q = {
								"%a": k => t[k.H].substring(0, 3), "%A": k => t[k.H], "%b": k => B[k.aa].substring(0, 3), "%B": k => B[k.aa], "%C": k => h((k.P + 1900) / 100 | 0, 2), "%d": k => h(k.ma, 2), "%e": k => f(k.ma, 2, " "), "%g": k => y(k).toString().substring(2), "%G": y, "%H": k => h(k.$, 2), "%I": k => { k = k.$; 0 == k ? k = 12 : 12 < k && (k -= 12); return h(k, 2) }, "%j": k => {
									for (var w =
										0, v = 0; v <= k.aa - 1; w += (Ub(k.P + 1900) ? bc : cc)[v++]); return h(k.ma + w, 3)
								}, "%m": k => h(k.aa + 1, 2), "%M": k => h(k.Ua, 2), "%n": () => "\n", "%p": k => 0 <= k.$ && 12 > k.$ ? "AM" : "PM", "%S": k => h(k.Va, 2), "%t": () => "\t", "%u": k => k.H || 7, "%U": k => h(Math.floor((k.O + 7 - k.H) / 7), 2), "%V": k => { var w = Math.floor((k.O + 7 - (k.H + 6) % 7) / 7); 2 >= (k.H + 371 - k.O - 2) % 7 && w++; if (w) 53 == w && (v = (k.H + 371 - k.O) % 7, 4 == v || 3 == v && Ub(k.P) || (w = 1)); else { w = 52; var v = (k.H + 7 - k.O - 1) % 7; (4 == v || 5 == v && Ub(k.P % 400 - 1)) && w++; } return h(w, 2) }, "%w": k => k.H, "%W": k => h(Math.floor((k.O + 7 - (k.H + 6) % 7) / 7),
									2), "%y": k => (k.P + 1900).toString().substring(2), "%Y": k => k.P + 1900, "%z": k => { k = k.Ta; var w = 0 <= k; k = Math.abs(k) / 60; return (w ? "+" : "-") + String("0000" + (k / 60 * 100 + k % 60)).slice(-4) }, "%Z": k => k.Wa, "%%": () => "%"
							}; c = c.replace(/%%/g, "\x00\x00"); for (x in q) c.includes(x) && (c = c.replace(new RegExp(x, "g"), q[x](d))); c = c.replace(/\0\0/g, "%"); x = lb(c, !1); if (x.length > b) return 0; dc(x, a); return x.length - 1
					}
				}, Oa = function () {
					var a = { env: nc, wasi_snapshot_preview1: nc }; La("wasm-instantiate"); var b = e; Ta(a, function (c) {
						u(e === b, "the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?");
						b = null; Oa = c.instance.exports; ra = Oa.memory; u(ra, "memory not found in wasm exports"); xa(); Z = Oa.__indirect_function_table; u(Z, "table not found in wasm exports"); Ea.unshift(Oa.__wasm_call_ctors); Ma("wasm-instantiate");
					}).catch(ba); return {}
				}(); e._lua_checkstack = J("lua_checkstack"); e._lua_xmove = J("lua_xmove"); e._lua_atpanic = J("lua_atpanic"); e._lua_newthread = J("lua_newthread"); e._lua_gettop = J("lua_gettop"); e._lua_settop = J("lua_settop"); e._lua_remove = J("lua_remove"); e._lua_insert = J("lua_insert");
				e._lua_replace = J("lua_replace"); e._lua_pushvalue = J("lua_pushvalue"); e._lua_type = J("lua_type"); e._lua_typename = J("lua_typename"); e._lua_iscfunction = J("lua_iscfunction"); e._lua_isnumber = J("lua_isnumber"); e._lua_isstring = J("lua_isstring"); e._lua_isuserdata = J("lua_isuserdata"); e._lua_rawequal = J("lua_rawequal"); e._lua_equal = J("lua_equal"); e._lua_lessthan = J("lua_lessthan"); e._lua_tonumber = J("lua_tonumber"); e._lua_tointeger = J("lua_tointeger"); e._lua_toboolean = J("lua_toboolean"); e._lua_tolstring = J("lua_tolstring");
				e._lua_objlen = J("lua_objlen"); e._lua_tocfunction = J("lua_tocfunction"); e._lua_touserdata = J("lua_touserdata"); e._lua_tothread = J("lua_tothread"); e._lua_topointer = J("lua_topointer"); e._lua_pushnil = J("lua_pushnil"); e._lua_pushnumber = J("lua_pushnumber"); e._lua_pushinteger = J("lua_pushinteger"); e._lua_pushlstring = J("lua_pushlstring"); e._lua_pushstring = J("lua_pushstring"); e._lua_pushvfstring = J("lua_pushvfstring"); e._lua_pushfstring = J("lua_pushfstring"); e._lua_pushcclosure = J("lua_pushcclosure");
				e._lua_pushboolean = J("lua_pushboolean"); e._lua_pushlightuserdata = J("lua_pushlightuserdata"); e._lua_pushthread = J("lua_pushthread"); e._lua_gettable = J("lua_gettable"); e._lua_getfield = J("lua_getfield"); e._lua_rawget = J("lua_rawget"); e._lua_rawgeti = J("lua_rawgeti"); e._lua_createtable = J("lua_createtable"); e._lua_getmetatable = J("lua_getmetatable"); e._lua_getfenv = J("lua_getfenv"); e._lua_settable = J("lua_settable"); e._lua_setfield = J("lua_setfield"); e._lua_rawset = J("lua_rawset"); e._lua_rawseti = J("lua_rawseti");
				e._lua_setmetatable = J("lua_setmetatable"); e._lua_setfenv = J("lua_setfenv"); e._lua_call = J("lua_call"); e._lua_pcall = J("lua_pcall"); e._lua_cpcall = J("lua_cpcall"); e._lua_load = J("lua_load"); e._lua_dump = J("lua_dump"); e._lua_status = J("lua_status"); e._lua_gc = J("lua_gc"); e._lua_error = J("lua_error"); e._lua_next = J("lua_next"); e._lua_concat = J("lua_concat"); e._lua_getallocf = J("lua_getallocf"); e._lua_setallocf = J("lua_setallocf"); e._lua_newuserdata = J("lua_newuserdata"); e._lua_getupvalue = J("lua_getupvalue");
				e._lua_setupvalue = J("lua_setupvalue"); e._luaL_argerror = J("luaL_argerror"); e._lua_getstack = J("lua_getstack"); e._luaL_error = J("luaL_error"); e._lua_getinfo = J("lua_getinfo"); e._luaL_typerror = J("luaL_typerror"); e._luaL_where = J("luaL_where"); e._luaL_checkoption = J("luaL_checkoption"); e._luaL_optlstring = J("luaL_optlstring"); e._luaL_checklstring = J("luaL_checklstring"); e._luaL_newmetatable = J("luaL_newmetatable"); e._luaL_checkudata = J("luaL_checkudata"); e._luaL_checkstack = J("luaL_checkstack");
				e._luaL_checktype = J("luaL_checktype"); e._luaL_checkany = J("luaL_checkany"); e._luaL_checknumber = J("luaL_checknumber"); e._luaL_optnumber = J("luaL_optnumber"); e._luaL_checkinteger = J("luaL_checkinteger"); e._luaL_optinteger = J("luaL_optinteger"); e._luaL_getmetafield = J("luaL_getmetafield"); e._luaL_callmeta = J("luaL_callmeta"); e._luaL_register = J("luaL_register"); e._luaL_gsub = J("luaL_gsub"); e._luaL_prepbuffer = J("luaL_prepbuffer"); e._luaL_buffinit = J("luaL_buffinit"); e._luaL_addlstring = J("luaL_addlstring");
				e._luaL_addstring = J("luaL_addstring"); e._luaL_pushresult = J("luaL_pushresult"); e._luaL_addvalue = J("luaL_addvalue"); e._luaL_ref = J("luaL_ref"); e._luaL_unref = J("luaL_unref"); e._luaL_loadfile = J("luaL_loadfile"); e._luaL_loadbuffer = J("luaL_loadbuffer"); e._luaL_loadstring = J("luaL_loadstring"); e._luaL_newstate = J("luaL_newstate"); e._lua_newstate = J("lua_newstate"); e._free = J("free"); e._realloc = J("realloc"); e._luaopen_base = J("luaopen_base"); e._lua_yield = J("lua_yield"); e._lua_resume = J("lua_resume");
				e._luaopen_debug = J("luaopen_debug"); e._lua_gethookmask = J("lua_gethookmask"); e._lua_gethook = J("lua_gethook"); e._lua_gethookcount = J("lua_gethookcount"); e._lua_getlocal = J("lua_getlocal"); e._lua_sethook = J("lua_sethook"); e._lua_setlocal = J("lua_setlocal"); var Xb = e._malloc = J("malloc"), kc = J("setTempRet0"); e._luaL_openlibs = J("luaL_openlibs"); e._luaopen_package = J("luaopen_package"); e._luaopen_table = J("luaopen_table"); e._luaopen_io = J("luaopen_io"); e._luaopen_os = J("luaopen_os"); e._luaopen_string = J("luaopen_string");
				e._luaopen_math = J("luaopen_math"); var Pb = J("fflush"); e._lua_close = J("lua_close"); var oc = J("setThrew"), pc = () => (pc = Oa.emscripten_stack_init)(), za = () => (za = Oa.emscripten_stack_get_end)(), qc = J("stackSave"), rc = J("stackRestore"), sc = J("stackAlloc"); e.dynCall_jiji = J("dynCall_jiji"); function mc(a, b, c) { var d = qc(); try { fc(a)(b, c); } catch (f) { rc(d); if (f !== f + 0) throw f; oc(1, 0); } } e.ENV = Zb;
				e.ccall = (a, b, c, d) => { var f = { string: q => { var x = 0; if (null !== q && void 0 !== q && 0 !== q) { x = jb(q) + 1; var t = sc(x); Sb(q, t, x); x = t; } return x }, array: q => { var x = sc(q.length); dc(q, x); return x } }; a = gc(a); var h = [], m = 0; u("array" !== b, 'Return type should not be "array".'); if (d) for (var p = 0; p < d.length; p++) { var y = f[c[p]]; y ? (0 === m && (m = qc()), h[p] = y(d[p])) : h[p] = d[p]; } c = a(...h); return c = function (q) { 0 !== m && rc(m); return "string" === b ? X(q, undefined, true) : "boolean" === b ? !!q : q }(c) };
				e.addFunction = (a, b) => {
					u("undefined" != typeof a); if (!hc) { hc = new WeakMap; var c = Z.length; if (hc) for (var d = 0; d < 0 + c; d++) { var f = fc(d); f && hc.set(f, d); } } if (c = hc.get(a) || 0) return c; if (ic.length) c = ic.pop(); else { try { Z.grow(1); } catch (p) { if (!(p instanceof RangeError)) throw p; throw "Unable to grow wasm table. Set ALLOW_TABLE_GROWTH."; } c = Z.length - 1; } try { d = c, Z.set(d, a), ec[d] = Z.get(d); } catch (p) {
						if (!(p instanceof TypeError)) throw p; u("undefined" != typeof b, "Missing signature argument to addFunction: " + a); u(!b.includes("j"),
							"i64 not permitted in function signatures when WASM_BIGINT is disabled"); if ("function" == typeof WebAssembly.Function) { d = WebAssembly.Function; u(!b.includes("j"), "i64 not permitted in function signatures when WASM_BIGINT is disabled"); f = { i: "i32", j: "i64", f: "f32", d: "f64", e: "externref", p: "i32" }; for (var h = { parameters: [], results: "v" == b[0] ? [] : [f[b[0]]] }, m = 1; m < b.length; ++m)u(b[m] in f, "invalid signature char: " + b[m]), h.parameters.push(f[b[m]]); b = new d(h, a); } else {
								d = [1]; f = b.slice(0, 1); b = b.slice(1); h = {
									i: 127, p: 127,
									j: 126, f: 125, d: 124, e: 111
								}; d.push(96); m = b.length; u(16384 > m); 128 > m ? d.push(m) : d.push(m % 128 | 128, m >> 7); for (m = 0; m < b.length; ++m)u(b[m] in h, "invalid signature char: " + b[m]), d.push(h[b[m]]); "v" == f ? d.push(0) : d.push(1, h[f]); b = [0, 97, 115, 109, 1, 0, 0, 0, 1]; f = d.length; u(16384 > f); 128 > f ? b.push(f) : b.push(f % 128 | 128, f >> 7); b.push(...d); b.push(2, 7, 1, 1, 101, 1, 102, 0, 0, 7, 5, 1, 1, 102, 0, 0); b = new WebAssembly.Module(new Uint8Array(b)); b = (new WebAssembly.Instance(b, { e: { f: a } })).exports.f;
							} d = c; Z.set(d, b); ec[d] = Z.get(d);
					} hc.set(a, c); return c
				};
				e.removeFunction = a => { hc.delete(fc(a)); Z.set(a, null); ec[a] = Z.get(a); ic.push(a); }; e.setValue = function (a, b, c = "i8") { c.endsWith("*") && (c = "*"); switch (c) { case "i1": E[a] = b; break; case "i8": E[a] = b; break; case "i16": ua[a >> 1] = b; break; case "i32": F[a >> 2] = b; break; case "i64": g("to do setValue(i64) use WASM_BIGINT"); case "float": va[a >> 2] = b; break; case "double": wa[a >> 3] = b; break; case "*": G[a >> 2] = b; break; default: g(`invalid type for setValue: ${c}`); } };
				e.getValue = function (a, b = "i8") { b.endsWith("*") && (b = "*"); switch (b) { case "i1": return E[a]; case "i8": return E[a]; case "i16": return ua[a >> 1]; case "i32": return F[a >> 2]; case "i64": g("to do getValue(i64) use WASM_BIGINT"); case "float": return va[a >> 2]; case "double": return wa[a >> 3]; case "*": return G[a >> 2]; default: g(`invalid type for getValue: ${b}`); } }; e.UTF8ToString = X; e.stringToUTF8 = Sb; e.lengthBytesUTF8 = jb; e.stringToNewUTF8 = Yb; e.FS = R;
				"writeI53ToI64 writeI53ToI64Clamped writeI53ToI64Signaling writeI53ToU64Clamped writeI53ToU64Signaling readI53FromI64 readI53FromU64 convertI32PairToI53 convertU32PairToI53 inetPton4 inetNtop4 inetPton6 inetNtop6 readSockaddr writeSockaddr getCallstack emscriptenLog convertPCtoSourceLocation readEmAsmArgs jstoi_q listenOnce autoResumeAudioContext dynCallLegacy getDynCaller dynCall handleException runtimeKeepalivePush runtimeKeepalivePop callUserCallback maybeExit asmjsMangle HandleAllocator getNativeTypeSize STACK_SIZE STACK_ALIGN POINTER_SIZE ASSERTIONS cwrap reallyNegative unSign strLen reSign formatString intArrayToString AsciiToString UTF16ToString stringToUTF16 lengthBytesUTF16 UTF32ToString stringToUTF32 lengthBytesUTF32 registerKeyEventCallback maybeCStringToJsString findEventTarget getBoundingClientRect fillMouseEventData registerMouseEventCallback registerWheelEventCallback registerUiEventCallback registerFocusEventCallback fillDeviceOrientationEventData registerDeviceOrientationEventCallback fillDeviceMotionEventData registerDeviceMotionEventCallback screenOrientation fillOrientationChangeEventData registerOrientationChangeEventCallback fillFullscreenChangeEventData registerFullscreenChangeEventCallback JSEvents_requestFullscreen JSEvents_resizeCanvasForFullscreen registerRestoreOldStyle hideEverythingExceptGivenElement restoreHiddenElements setLetterbox softFullscreenResizeWebGLRenderTarget doRequestFullscreen fillPointerlockChangeEventData registerPointerlockChangeEventCallback registerPointerlockErrorEventCallback requestPointerLock fillVisibilityChangeEventData registerVisibilityChangeEventCallback registerTouchEventCallback fillGamepadEventData registerGamepadEventCallback registerBeforeUnloadEventCallback fillBatteryEventData battery registerBatteryEventCallback setCanvasElementSize getCanvasElementSize jsStackTrace stackTrace checkWasiClock wasiRightsToMuslOFlags wasiOFlagsToMuslOFlags createDyncallWrapper safeSetTimeout setImmediateWrapped clearImmediateWrapped polyfillSetImmediate getPromise makePromise idsToPromises makePromiseCallback Browser_asyncPrepareDataCounter setMainLoop getSocketFromFD getSocketAddress FS_unlink FS_mkdirTree _setNetworkCallback".split(" ").forEach(function (a) {
					"undefined" === typeof globalThis ||
						Object.getOwnPropertyDescriptor(globalThis, a) || Object.defineProperty(globalThis, a, { configurable: !0, get() { var b = `\`${a}\` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line`, c = a; c.startsWith("_") || (c = "$" + a); b += ` (e.g. -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='${c}')`; Va(a) && (b += ". Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you"); N(b); } }); Xa(a);
				}); "run addOnPreRun addOnInit addOnPreMain addOnExit addOnPostRun addRunDependency removeRunDependency FS_createFolder FS_createPath FS_createLazyFile FS_createLink FS_createDevice FS_readFile out err callMain abort wasmMemory wasmExports stackAlloc stackSave stackRestore getTempRet0 setTempRet0 writeStackCookie checkStackCookie convertI32PairToI53Checked ptrToString zeroMemory exitJS getHeapMax growMemory MONTH_DAYS_REGULAR MONTH_DAYS_LEAP MONTH_DAYS_REGULAR_CUMULATIVE MONTH_DAYS_LEAP_CUMULATIVE isLeapYear ydayFromDate arraySum addDays ERRNO_CODES ERRNO_MESSAGES DNS Protocols Sockets initRandomFill randomFill timers warnOnce UNWIND_CACHE readEmAsmArgsArray jstoi_s getExecutableName keepRuntimeAlive asyncLoad alignMemory mmapAlloc wasmTable noExitRuntime getCFunc uleb128Encode sigToWasmTypes generateFuncType convertJsFunctionToWasm freeTableIndexes functionsInTableMap getEmptyTableSlot updateTableMap getFunctionAddress PATH PATH_FS UTF8Decoder UTF8ArrayToString stringToUTF8Array intArrayFromString stringToAscii UTF16Decoder stringToUTF8OnStack writeArrayToMemory JSEvents specialHTMLTargets findCanvasEventTarget currentFullscreenStrategy restoreOldWindowedStyle ExitStatus getEnvStrings doReadv doWritev promiseMap Browser getPreloadedImageData__data wget SYSCALLS preloadPlugins FS_createPreloadedFile FS_modeStringToFlags FS_getMode FS_stdin_getChar_buffer FS_stdin_getChar FS_createDataFile MEMFS TTY PIPEFS SOCKFS".split(" ").forEach(Xa);
				var tc; Ia = function uc() { tc || vc(); tc || (Ia = uc); };
				function vc() {
					if (!(0 < Ha)) {
						pc(); var a = za(); u(0 == (a & 3)); 0 == a && (a += 4); G[a >> 2] = 34821223; G[a + 4 >> 2] = 2310721022; G[0] = 1668509029; if (e.preRun) for ("function" == typeof e.preRun && (e.preRun = [e.preRun]); e.preRun.length;)a = e.preRun.shift(), Da.unshift(a); for (; 0 < Da.length;)Da.shift()(e); if (!(0 < Ha)) {
							if (!tc && (tc = !0, e.calledRun = !0, !sa)) {
								u(!Ga); Ga = !0; ya(); e.noFSInit || R.R.Y || R.R(); for (R.ua = !1; 0 < Ea.length;)Ea.shift()(e); aa(e); u(!e._main, 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]'); for (ya(); 0 <
									Fa.length;)Fa.shift()(e);
							} ya();
						}
					}
				} function lc() { var a = qa, b = z, c = !1; qa = z = () => { c = !0; }; try { Pb(0), ["stdout", "stderr"].forEach(function (d) { (d = Mb("/dev/" + d)) && mb[d.object.rdev]?.output?.length && (c = !0); }); } catch (d) { } qa = a; z = b; c && N("stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc."); } vc();


				return moduleArg.ready
			}
		);
	})();

	class LuaApi {
		static async initialize(customWasmFileLocation, environmentVariables) {
			const module = await initWasmModule({
				locateFile: (path, scriptDirectory) => {
					return customWasmFileLocation || scriptDirectory + path;
				},
				preRun: (initializedModule) => {
					if (typeof environmentVariables === 'object') {
						Object.entries(environmentVariables).forEach(([k, v]) => (initializedModule.ENV[k] = v));
					}
				},
			});
			return new LuaApi(module);
		}
		constructor(module) {
			this.pointerRefs = new Map();
			this.referenceTracker = new WeakMap();
			this.referenceMap = new Map();
			this.availableReferences = [];
			this.module = module;
			this.luaL_error = this.cwrap('luaL_error', null, ['number', 'string']);
			this.luaL_argerror = this.cwrap('luaL_argerror', 'number', ['number', 'number', 'string']);
			this.luaL_typerror = this.cwrap('luaL_typerror', 'number', ['number', 'number', 'string']);
			this.luaL_addlstring = this.cwrap('luaL_addlstring', null, ['number', 'string', 'number']);
			this.luaL_addstring = this.cwrap('luaL_addstring', null, ['number', 'string']);
			this.luaL_addvalue = this.cwrap('luaL_addvalue', null, ['number']);
			this.luaL_checkany = this.cwrap('luaL_checkany', null, ['number', 'number']);
			this.luaL_checkinteger = this.cwrap('luaL_checkinteger', 'number', ['number', 'number']);
			this.luaL_checklstring = this.cwrap('luaL_checklstring', 'string', ['number', 'number', 'number']);
			this.luaL_checknumber = this.cwrap('luaL_checknumber', 'number', ['number', 'number']);
			this.luaL_checkstack = this.cwrap('luaL_checkstack', null, ['number', 'number', 'string']);
			this.luaL_checktype = this.cwrap('luaL_checktype', null, ['number', 'number', 'number']);
			this.luaL_checkudata = this.cwrap('luaL_checkudata', 'number', ['number', 'number', 'string']);
			this.luaL_loadbuffer = this.cwrap('luaL_loadbuffer', 'number', ['number', 'string|number', 'number', 'string|number']);
			this.luaL_loadfile = this.cwrap('luaL_loadfile', 'number', ['number', 'string']);
			this.luaL_loadfilex = this.luaL_loadfile;
			this.luaL_loadstring = this.cwrap('luaL_loadstring', 'number', ['number', 'string']);
			this.luaL_buffinit = this.cwrap('luaL_buffinit', null, ['number', 'number']);
			this.luaL_callmeta = this.cwrap('luaL_callmeta', 'number', ['number', 'number', 'string']);
			this.luaL_getmetafield = this.cwrap('luaL_getmetafield', 'number', ['number', 'number', 'string']);
			this.luaL_gsub = this.cwrap('luaL_gsub', 'string', ['number', 'string', 'string', 'string']);
			this.luaL_newmetatable = this.cwrap('luaL_newmetatable', 'number', ['number', 'string']);
			this.luaL_newstate = this.cwrap('luaL_newstate', 'number', []);
			this.luaL_openlibs = this.cwrap('luaL_openlibs', null, ['number']);
			this.luaL_optinteger = this.cwrap('luaL_optinteger', 'number', ['number', 'number', 'number']);
			this.luaL_optlstring = this.cwrap('luaL_optlstring', 'string', ['number', 'number', 'string', 'number']);
			this.luaL_optnumber = this.cwrap('luaL_optnumber', 'number', ['number', 'number', 'number']);
			this.luaL_prepbuffer = this.cwrap('luaL_prepbuffer', 'string', ['number', 'number']);
			this.luaL_pushresult = this.cwrap('luaL_pushresult', null, ['number']);
			this.luaL_pushresultsize = this.cwrap('luaL_pushresultsize', null, ['number', 'number']);
			this.luaL_register = this.cwrap('luaL_register', null, ['number', 'string', 'array']);
			this.luaL_ref = this.cwrap('luaL_ref', 'number', ['number', 'number']);
			this.luaL_unref = this.cwrap('luaL_unref', null, ['number', 'number', 'number']);
			this.luaL_where = this.cwrap('luaL_where', null, ['number', 'number']);
			this.lua_atpanic = this.cwrap('lua_atpanic', 'number', ['number', 'number']);
			this.lua_call = this.cwrap('lua_call', null, ['number', 'number', 'number']);
			this.lua_checkstack = this.cwrap('lua_checkstack', 'number', ['number', 'number']);
			this.lua_close = this.cwrap('lua_close', null, ['number']);
			this.lua_concat = this.cwrap('lua_concat', null, ['number', 'number']);
			this.lua_createtable = this.cwrap('lua_createtable', null, ['number', 'number', 'number']);
			this.lua_dump = this.cwrap('lua_dump', 'number', ['number', 'number', 'number', 'number']);
			this.lua_equal = this.cwrap('lua_equal', 'number', ['number', 'number', 'number']);
			this.lua_error = this.cwrap('lua_error', 'number', ['number']);
			this.lua_gc = this.cwrap('lua_gc', 'number', ['number', 'number', 'number']);
			this.lua_getallocf = this.cwrap('lua_getallocf', 'number', ['number', 'number']);
			this.lua_getfenv = this.cwrap('lua_getfenv', null, ['number', 'number']);
			this.lua_getfield = this.cwrap('lua_getfield', null, ['number', 'number', 'string']);
			this.lua_gethook = this.cwrap('lua_gethook', 'number', ['number']);
			this.lua_gethookcount = this.cwrap('lua_gethookcount', 'number', ['number']);
			this.lua_gethookmask = this.cwrap('lua_gethookmask', 'number', ['number']);
			this.lua_getinfo = this.cwrap('lua_getinfo', 'number', ['number', 'string', 'number']);
			this.lua_getlocal = this.cwrap('lua_getlocal', 'string', ['number', 'number', 'number']);
			this.lua_getmetatable = this.cwrap('lua_getmetatable', 'number', ['number', 'number']);
			this.lua_getstack = this.cwrap('lua_getstack', 'number', ['number', 'number', 'number']);
			this.lua_gettable = this.cwrap('lua_gettable', 'number', ['number', 'number']);
			this.lua_gettop = this.cwrap('lua_gettop', 'number', ['number']);
			this.lua_getupvalue = this.cwrap('lua_getupvalue', 'string', ['number', 'number', 'number']);
			this.lua_insert = this.cwrap('lua_insert', null, ['number', 'number']);
			this.lua_lessthan = this.cwrap('lua_lessthan', 'number', ['number', 'number', 'number']);
			this.lua_load = this.cwrap('lua_load', 'number', ['number', 'number', 'number', 'string', 'string']);
			this.lua_newstate = this.cwrap('lua_newstate', 'number', ['number', 'number']);
			this.lua_newthread = this.cwrap('lua_newthread', 'number', ['number']);
			this.lua_newuserdata = this.cwrap('lua_newuserdata', 'number', ['number', 'number']);
			this.lua_next = this.cwrap('lua_next', 'number', ['number', 'number']);
			this.lua_objlen = this.cwrap('lua_objlen', 'number', ['number', 'number']);
			this.lua_pcall = this.cwrap('lua_pcall', 'number', ['number', 'number', 'number', 'number']);
			this.lua_iscfunction = this.cwrap('lua_iscfunction', 'number', ['number', 'number']);
			this.lua_isnumber = this.cwrap('lua_isnumber', 'number', ['number', 'number']);
			this.lua_isstring = this.cwrap('lua_isstring', 'number', ['number', 'number']);
			this.lua_isuserdata = this.cwrap('lua_isuserdata', 'number', ['number', 'number']);
			this.lua_pushboolean = this.cwrap('lua_pushboolean', null, ['number', 'number']);
			this.lua_pushcclosure = this.cwrap('lua_pushcclosure', null, ['number', 'number', 'number']);
			this.lua_pushcfunction = (...args) => this.lua_pushcclosure(...args, 0);
			this.lua_pushfstring = this.cwrap('lua_pushfstring', 'string', ['number', 'string', 'array']);
			this.lua_pushinteger = this.cwrap('lua_pushinteger', null, ['number', 'number']);
			this.lua_pushlightuserdata = this.cwrap('lua_pushlightuserdata', null, ['number', 'number']);
			this.lua_pushnil = this.cwrap('lua_pushnil', null, ['number']);
			this.lua_pushnumber = this.cwrap('lua_pushnumber', null, ['number', 'number']);
			this.lua_pushlstring = this.cwrap('lua_pushlstring', null, ['number', 'string|number', 'number']);
			this.lua_pushstring = this.cwrap('lua_pushstring', null, ['number', 'string|number']);
			this.lua_pushthread = this.cwrap('lua_pushthread', 'number', ['number']);
			this.lua_pushvalue = this.cwrap('lua_pushvalue', null, ['number', 'number']);
			this.lua_pushvfstring = this.cwrap('lua_pushvfstring', 'string', ['number', 'string', 'number']);
			this.lua_toboolean = this.cwrap('lua_toboolean', 'number', ['number', 'number']);
			this.lua_tocfunction = this.cwrap('lua_tocfunction', 'number', ['number', 'number']);
			this.lua_tointeger = this.cwrap('lua_tointeger', 'number', ['number', 'number']);
			this.lua_tolstring = this.cwrap('lua_tolstring', 'string', ['number', 'number', 'number']);
			this.lua_tostring = (...args) => this.lua_tolstring(...args, null);
			this.lua_tonumber = this.cwrap('lua_tonumber', 'number', ['number', 'number']);
			this.lua_topointer = this.cwrap('lua_topointer', 'number', ['number', 'number']);
			this.lua_tothread = this.cwrap('lua_tothread', 'number', ['number', 'number']);
			this.lua_touserdata = this.cwrap('lua_touserdata', 'number', ['number', 'number']);
			this.lua_rawequal = this.cwrap('lua_rawequal', 'number', ['number', 'number', 'number']);
			this.lua_rawget = this.cwrap('lua_rawget', 'number', ['number', 'number']);
			this.lua_rawgeti = this.cwrap('lua_rawgeti', 'number', ['number', 'number', 'number']);
			this.lua_rawset = this.cwrap('lua_rawset', null, ['number', 'number']);
			this.lua_rawseti = this.cwrap('lua_rawseti', null, ['number', 'number', 'number']);
			this.lua_remove = this.cwrap('lua_remove', null, ['number', 'number']);
			this.lua_replace = this.cwrap('lua_replace', null, ['number', 'number']);
			this.lua_resume = this.cwrap('lua_resume', 'number', ['number', 'number']);
			this.lua_setallocf = this.cwrap('lua_setallocf', null, ['number', 'number', 'number']);
			this.lua_setfenv = this.cwrap('lua_setfenv', 'number', ['number', 'number']);
			this.lua_setfield = this.cwrap('lua_setfield', null, ['number', 'number', 'string']);
			this.lua_sethook = this.cwrap('lua_sethook', null, ['number', 'number', 'number', 'number']);
			this.lua_setlocal = this.cwrap('lua_setlocal', 'string', ['number', 'number', 'number']);
			this.lua_setmetatable = this.cwrap('lua_setmetatable', 'number', ['number', 'number']);
			this.lua_settable = this.cwrap('lua_settable', null, ['number', 'number']);
			this.lua_settop = this.cwrap('lua_settop', null, ['number', 'number']);
			this.lua_setupvalue = this.cwrap('lua_setupvalue', 'string', ['number', 'number', 'number']);
			this.lua_status = this.cwrap('lua_status', 'number', ['number']);
			this.lua_type = this.cwrap('lua_type', 'number', ['number', 'number']);
			this.lua_typename = this.cwrap('lua_typename', 'string', ['number', 'number']);
			this.lua_xmove = this.cwrap('lua_xmove', null, ['number', 'number', 'number']);
			this.lua_yield = this.cwrap('lua_yield', 'number', ['number', 'number']);
			this.luaopen_base = this.cwrap('luaopen_base', 'number', ['number']);
			this.luaopen_table = this.cwrap('luaopen_table', 'number', ['number']);
			this.luaopen_io = this.cwrap('luaopen_io', 'number', ['number']);
			this.luaopen_os = this.cwrap('luaopen_os', 'number', ['number']);
			this.luaopen_string = this.cwrap('luaopen_string', 'number', ['number']);
			this.luaopen_math = this.cwrap('luaopen_math', 'number', ['number']);
			this.luaopen_debug = this.cwrap('luaopen_debug', 'number', ['number']);
			this.luaopen_package = this.cwrap('luaopen_package', 'number', ['number']);
		}
		lua_absindex(L, idx) {
			return idx > 0 || idx <= LUA_REGISTRYINDEX ? idx : this.lua_gettop(L) + idx + 1;
		}
		lua_setglobal(L, name) {
			this.lua_setfield(L, LUA_GLOBALSINDEX, name);
		}
		lua_getglobal(L, name) {
			this.lua_getfield(L, LUA_GLOBALSINDEX, name);
			return this.lua_type(L, -1);
		}
		lua_pop(L, n) {
			this.lua_settop(L, -n - 1);
		}
		luaL_getmetatable(L, tname) {
			this.lua_getfield(L, LUA_REGISTRYINDEX, tname);
			return this.lua_type(L, -1);
		}
		luaL_typename(L, idx) {
			return this.lua_typename(L, this.lua_type(L, idx));
		}
		luaL_tolstring(L, idx) {
			idx = this.lua_absindex(L, idx);
			let result;
			if (this.luaL_callmeta(L, idx, '__tostring')) {
				if (!this.lua_isstring(L, -1)) {
					this.luaL_error(L, "'__tostring' must return a string");
				}
				result = this.luaL_tolstring(L, -1);
			}
			else {
				const type = this.lua_type(L, idx);
				if (type === exports.LuaType.Number) {
					result = `${this.lua_tonumber(L, idx)}`;
				}
				else if (type === exports.LuaType.String) {
					result = this.lua_tostring(L, idx);
				}
				else if (type === exports.LuaType.Boolean) {
					result = this.lua_toboolean(L, idx) ? 'true' : 'false';
				}
				else if (type === exports.LuaType.Nil) {
					result = 'nil';
				}
				else {
					const tt = this.luaL_getmetafield(L, idx, '__name');
					const kind = tt === exports.LuaType.String ? this.lua_tostring(L, -1) : this.luaL_typename(L, idx);
					if (tt !== exports.LuaType.Nil) {
						this.lua_remove(L, -2);
					}
					result = `${kind}: 0x${this.lua_topointer(L, idx).toString(16)}`;
				}
			}
			this.lua_pushstring(L, result);
			return result;
		}
		lua_upvalueindex(index) {
			return LUA_GLOBALSINDEX - index;
		}
		ref(data) {
			const existing = this.referenceTracker.get(data);
			if (existing) {
				existing.refCount++;
				return existing.index;
			}
			const availableIndex = this.availableReferences.pop();
			const index = availableIndex === undefined ? this.referenceMap.size + 1 : availableIndex;
			this.referenceMap.set(index, data);
			this.referenceTracker.set(data, {
				refCount: 1,
				index,
			});
			this.lastRefIndex = index;
			return index;
		}
		unref(index) {
			const ref = this.referenceMap.get(index);
			if (ref === undefined) {
				return;
			}
			const metadata = this.referenceTracker.get(ref);
			if (metadata === undefined) {
				this.referenceTracker.delete(ref);
				this.availableReferences.push(index);
				return;
			}
			metadata.refCount--;
			if (metadata.refCount <= 0) {
				this.referenceTracker.delete(ref);
				this.referenceMap.delete(index);
				this.availableReferences.push(index);
			}
		}
		getRef(index) {
			return this.referenceMap.get(index);
		}
		getLastRefIndex() {
			return this.lastRefIndex;
		}
		printRefs() {
			for (const [key, value] of this.referenceMap.entries()) {
				console.log(key, value);
			}
		}
		cwrap(name, returnType, argTypes) {
			const commonType = ['number', 'string', 'array', 'boolean'];
			const isCommonCase = argTypes.every((argType) => commonType.includes(argType));
			if (isCommonCase) {
				return (...args) => {
					return this.module.ccall(name, returnType, argTypes, args);
				};
			}
			return (...args) => {
				const pointersToBeFreed = [];
				const resolvedArgTypes = [];
				const resolvedArgs = [];
				argTypes.forEach((argType, i) => {
					var _a;
					if (commonType.includes(argType)) {
						resolvedArgTypes.push(argType);
						resolvedArgs.push(args[i]);
					}
					else if (argType === 'string|number') {
						if (typeof args[i] === 'number') {
							resolvedArgTypes.push('number');
							resolvedArgs.push(args[i]);
						}
						else {
							if (((_a = args[i]) === null || _a === void 0 ? void 0 : _a.length) > 1024) {
								const bufferPointer = this.module.stringToNewUTF8(args[i]);
								resolvedArgTypes.push('number');
								resolvedArgs.push(bufferPointer);
								pointersToBeFreed.push(bufferPointer);
							}
							else {
								resolvedArgTypes.push('string');
								resolvedArgs.push(args[i]);
							}
						}
					}
				});
				try {
					return this.module.ccall(name, returnType, resolvedArgTypes, resolvedArgs);
				}
				finally {
					for (const pointer of pointersToBeFreed) {
						this.module._free(pointer);
					}
				}
			};
		}
	}

	class FuncManager {
		constructor(luaApi) {
			this.pointerRedirect = new Map();
			this.indexMap = new Map();
			this.gc = 0;
			this.call = 0;
			this.func_call = 0;
			this.index = 0;
			this.new_index = 0;
			this.to_string = 0;
			this.luaApi = luaApi;
		}
		registerGcFunction(thread) {
			if (this.gc) {
				return this.gc;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const userdata = callThread.luaApi.lua_touserdata(L, 1);
				const pointer = callThread.luaApi.lua_topointer(L, 1);
				const functionPointer = this.pointerRedirect.get(pointer);
				this.removeIndexRedirect(functionPointer);
				const ref = callThread.luaApi.module.getValue(userdata, '*');
				thread.luaApi.unref(ref);
				callThread.luaApi.lua_getmetatable(L, 1);
				if (callThread.luaApi.lua_type(L, -1) === exports.LuaType.Nil) {
					callThread.luaApi.lua_pop(L, 1);
					return exports.LuaReturn.Ok;
				}
				callThread.luaApi.lua_getfield(L, -1, '__func_pointers');
				if (callThread.luaApi.lua_type(L, -1) !== exports.LuaType.Table) {
					callThread.luaApi.lua_pop(L, 2);
					return exports.LuaReturn.Ok;
				}
				callThread.luaApi.lua_pushnil(L);
				while (callThread.luaApi.lua_next(L, -2) !== 0) {
					const funcPointer = callThread.luaApi.lua_tonumber(L, -1);
					callThread.luaApi.module.removeFunction(funcPointer);
					callThread.luaApi.lua_pop(L, 1);
				}
				return exports.LuaReturn.Ok;
			}, 'ii');
			this.gc = pointer;
			return pointer;
		}
		registerCallFunction(thread) {
			if (this.call) {
				return this.call;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const top = callThread.getTop();
				const target = callThread.getValue(1);
				const args = [];
				for (let i = 2; i <= top; i++) {
					args.push(callThread.getValue(i));
				}
				try {
					const result = target(...args);
					callThread.pushValue(result);
					return 1;
				}
				catch (e) {
					if (typeof (e === null || e === void 0 ? void 0 : e.message) === 'string') {
						callThread.pushValue(e === null || e === void 0 ? void 0 : e.message);
					}
					else {
						callThread.pushValue('Error: An exception occurred during the process of calling a JavaScript function.');
					}
					callThread.luaApi.lua_error(L);
				}
				return 0;
			}, 'ii');
			this.call = pointer;
			return pointer;
		}
		registerFuncCallFunction(thread) {
			if (this.func_call) {
				return this.func_call;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const top = callThread.getTop();
				const args = [];
				for (let i = 1; i <= top; i++) {
					args.push(callThread.getValue(i));
				}
				const userdata = callThread.luaApi.lua_touserdata(L, callThread.luaApi.lua_upvalueindex(1));
				const ref = callThread.luaApi.module.getValue(userdata, '*');
				const func = thread.luaApi.getRef(ref);
				try {
					const result = func.apply(thread, args);
					callThread.pushValue(result);
					return 1;
				}
				catch (e) {
					if (typeof (e === null || e === void 0 ? void 0 : e.message) === 'string') {
						callThread.pushValue(e === null || e === void 0 ? void 0 : e.message);
					}
					else {
						callThread.pushValue('Error: An exception occurred during the process of calling a JavaScript function.');
					}
					callThread.luaApi.lua_error(L);
				}
				return 0;
			}, 'ii');
			this.func_call = pointer;
			return pointer;
		}
		registerIndexFunction(thread) {
			if (this.index) {
				return this.index;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const key = callThread.getValue(2);
				const pointer = callThread.luaApi.lua_topointer(L, 1);
				const target = this.indexMap.get(pointer);
				if (target) {
					const value = target[key];
					callThread.pushValue(typeof value === 'function' ? value.bind(target) : value);
					return 1;
				}
				const userdata = callThread.luaApi.lua_touserdata(L, 1);
				const ref = callThread.luaApi.module.getValue(userdata, '*');
				const tar = thread.luaApi.getRef(ref);
				const value = tar === null || tar === void 0 ? void 0 : tar[key];
				callThread.pushValue(typeof value === 'function' ? value.bind(tar) : value);
				return 1;
			}, 'ii');
			this.index = pointer;
			return pointer;
		}
		registerNewIndexFunction(thread) {
			if (this.new_index) {
				return this.new_index;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const key = callThread.getValue(2);
				const value = callThread.getValue(3);
				const pointer = callThread.luaApi.lua_topointer(L, 1);
				const target = this.indexMap.get(pointer);
				if (target) {
					target[key] = value;
					return exports.LuaReturn.Ok;
				}
				const userdata = callThread.luaApi.lua_touserdata(L, 1);
				const ref = callThread.luaApi.module.getValue(userdata, '*');
				thread.luaApi.getRef(ref)[key] = value;
				return exports.LuaReturn.Ok;
			}, 'ii');
			this.new_index = pointer;
			return pointer;
		}
		registerToStringFunction(thread) {
			if (this.to_string) {
				return this.to_string;
			}
			const pointer = thread.luaApi.module.addFunction((L) => {
				const callThread = thread.stateToThread(L);
				const pointer = callThread.luaApi.lua_topointer(L, 1);
				const typename = callThread.luaApi.luaL_typename(L, 1);
				const target = this.indexMap.get(pointer);
				if (target) {
					callThread.pushValue(`${typename}: 0x${pointer.toString(16)} -> ${target.toString()}`);
					return 1;
				}
				const userdata = callThread.luaApi.lua_touserdata(L, 1);
				const ref = callThread.luaApi.module.getValue(userdata, '*');
				const value = thread.luaApi.getRef(ref).toString();
				callThread.pushValue(`${typename}: 0x${pointer.toString(16)} -> ${value}`);
				return 1;
			}, 'ii');
			this.to_string = pointer;
			return pointer;
		}
		addIndexRedirect(pointer, target) {
			if (!pointer) {
				return;
			}
			this.indexMap.set(pointer, target);
		}
		removeIndexRedirect(pointer) {
			if (!pointer) {
				return;
			}
			this.indexMap.delete(pointer);
		}
	}

	var version = "1.18.10";

	const getContextProxy = (global) => {
		return new Proxy(global, {
			get: (target, key) => {
				if (key === Symbol.iterator) {
					return {
						next: () => {
							return 1;
						},
					};
				}
				if (typeof key === 'symbol') {
					return undefined;
				}
				return target.get(key);
			},
			set: (target, key, value) => {
				target.set(key, value);
				return true;
			},
			has: (target, key) => {
				return target.get(key) !== undefined;
			},
		});
	};

	class Lua {
		static async create(options = {}) {
			var _a;
			options = {
				openStandardLibs: true,
				traceAllocations: false,
				...options,
			};
			if (!options.customWasmUri) {
				const isBrowser = (typeof window === 'object' && typeof window.document !== 'undefined') ||
					(typeof self === 'object' && ((_a = self === null || self === void 0 ? void 0 : self.constructor) === null || _a === void 0 ? void 0 : _a.name) === 'DedicatedWorkerGlobalScope');
				if (isBrowser) {
					options.customWasmUri = `https://unpkg.com/wasmoon-lua5.1@${version}/dist/liblua5.1.wasm`;
				}
			}
			const luaApi = await LuaApi.initialize(options.customWasmUri, options.environmentVariables);
			return new Lua(luaApi, options);
		}
		constructor(luaApi, options) {
			if (!options) {
				throw new Error('Lua.create(options) must be used to create a Lua instance');
			}
			this.luaApi = luaApi;
			this.global = new LuaGlobal(this.luaApi, options.traceAllocations);
			this.funcManager = new FuncManager(this.luaApi);
			this.initTypeBindings();
			this.ctx = getContextProxy(this.global);
			if (options.openStandardLibs) {
				this.luaApi.luaL_openlibs(this.global.address);
			}
		}
		mountFile(path, content) {
			const fileSep = path.lastIndexOf('/');
			const file = path.substring(fileSep + 1);
			const body = path.substring(0, path.length - file.length - 1);
			if (body.length > 0) {
				const parts = body.split('/').reverse();
				let parent = '';
				while (parts.length) {
					const part = parts.pop();
					if (!part) {
						continue;
					}
					const current = `${parent}/${part}`;
					try {
						this.luaApi.module.FS.mkdir(current);
					}
					catch (err) {
					}
					parent = current;
				}
			}
			this.luaApi.module.FS.writeFile(path, content);
		}
		unmountFile(path) {
			this.luaApi.module.FS.unlink(path);
		}
		doString(script) {
			const result = this.callByteCode((thread) => thread.loadString(script));
			return result;
		}
		doFile(filename) {
			return this.callByteCode((thread) => thread.loadFile(filename));
		}
		doStringSync(script) {
			this.global.loadString(script);
			const result = this.global.runSync();
			return result[0];
		}
		doFileSync(filename) {
			this.global.loadFile(filename);
			const result = this.global.runSync();
			return result[0];
		}
		initTypeBindings() {
			JsType.create('js-function', (value) => typeof value === 'function' && !value.toString().startsWith('class'))
				.gc(this.funcManager.registerGcFunction(this.global))
				.push(({ thread, target }) => {
					const ref = thread.luaApi.ref(target);
					const luaPointer = thread.luaApi.lua_newuserdata(thread.address, PointerSize);
					thread.luaApi.module.setValue(luaPointer, ref, '*');
					thread.luaApi.luaL_getmetatable(thread.address, 'js-function');
					thread.luaApi.lua_setmetatable(thread.address, -2);
					thread.luaApi.lua_pushcclosure(thread.address, this.funcManager.registerFuncCallFunction(this.global), 1);
					const pointer = thread.luaApi.lua_topointer(thread.address, -1);
					this.funcManager.pointerRedirect.set(luaPointer, pointer);
					this.funcManager.addIndexRedirect(pointer, target);
					thread.luaApi.lua_createtable(thread.address, 0, 0);
					thread.luaApi.lua_pushcfunction(thread.address, this.funcManager.registerToStringFunction(this.global));
					thread.luaApi.lua_setfield(thread.address, -2, '__tostring');
					thread.luaApi.lua_pushcfunction(thread.address, this.funcManager.registerIndexFunction(this.global));
					thread.luaApi.lua_setfield(thread.address, -2, '__index');
					thread.luaApi.lua_pushcfunction(thread.address, this.funcManager.registerNewIndexFunction(this.global));
					thread.luaApi.lua_setfield(thread.address, -2, '__newindex');
					thread.luaApi.lua_setmetatable(thread.address, -2);
				})
				.bind(this.global);
			JsType.create('js-userdata', () => true)
				.gc(this.funcManager.registerGcFunction(this.global))
				.call(this.funcManager.registerCallFunction(this.global))
				.index(this.funcManager.registerIndexFunction(this.global))
				.newindex(this.funcManager.registerNewIndexFunction(this.global))
				.tostring(this.funcManager.registerToStringFunction(this.global))
				.priority(-1)
				.bind(this.global);
		}
		async callByteCode(loader) {
			const thread = this.global.newThread();
			const threadIndex = this.global.getTop();
			try {
				loader(thread);
				const result = await thread.run(0);
				if (result.length > 0) {
					this.luaApi.lua_xmove(thread.address, this.global.address, result.length);
					const ret = this.global.getValue(this.global.getTop() - result.length + 1);
					return ret;
				}
				return undefined;
			}
			finally {
				this.global.remove(threadIndex);
			}
		}
	}

	export default {
		JsType: JsType,
		LUA_ENVIRONINDEX: LUA_ENVIRONINDEX,
		LUA_GLOBALSINDEX: LUA_GLOBALSINDEX,
		LUA_IDSIZE: LUA_IDSIZE,
		LUA_MULTRET: LUA_MULTRET,
		LUA_REGISTRYINDEX: LUA_REGISTRYINDEX,
		Lua: Lua,
		LuaApi: LuaApi,
		LuaGlobal: LuaGlobal,
		LuaMultiReturn: MultiReturn,
		LuaThread: LuaThread,
		LuaTimeoutError: LuaTimeoutError,
		PointerSize: PointerSize,
		mapTransform: mapTransform,
		LuaReturn: exports.LuaReturn,
		LuaType: exports.LuaType,
		LuaEventCodes: exports.LuaEventCodes,
		LuaEventMasks: exports.LuaEventMasks,
		LuaLibraries: exports.LuaLibraries,
		DictType: exports.DictType
	};
