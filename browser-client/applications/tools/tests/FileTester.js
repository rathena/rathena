/**
 * applications/tools/tests/FileTester.js
 *
 * File Tester Helper
 * Lets the user pick GRF/files, then iterates every matching file
 * through a caller-supplied callback so loaders can be smoke-tested in bulk.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import Configs from 'Core/Configs.js';
import Thread from 'Core/Thread.js';
import Memory from 'Core/MemoryManager.js';
import Events from 'Core/Events.js';

// Always run in development mode for easier debugging
Configs.set('development', true);
Configs.set('saveFiles', false);

/**
 * File Tester
 *
 * @param {string}   ext      – file extension to search for (e.g. 'spr')
 * @param {Function} callback – called with the raw ArrayBuffer for each file
 */
class FileTester {
	constructor(ext, callback) {
		this.ext = ext;
		this.callback = callback;
	}


	start() {
		Thread.hook('THREAD_READY', () => {
			// Mini file picker inline — sem dependência de UIComponent/UIManager  
			this._showFilePicker();
			Events.process(Date.now() + 100);
		});

		Thread.init();
	}

	/** Lightweight file picker that replaces Intro for tools */
	// applications/tools/tests/FileTester.js  

	_showFilePicker() {
		// Inject Google Fonts (same as index.html)  
		if (!document.querySelector('link[href*="Cinzel"]')) {
			const preconnect1 = document.createElement('link');
			preconnect1.rel = 'preconnect';
			preconnect1.href = 'https://fonts.googleapis.com';
			document.head.appendChild(preconnect1);

			const preconnect2 = document.createElement('link');
			preconnect2.rel = 'preconnect';
			preconnect2.href = 'https://fonts.gstatic.com';
			preconnect2.crossOrigin = '';
			document.head.appendChild(preconnect2);

			const fontLink = document.createElement('link');
			fontLink.rel = 'stylesheet';
			fontLink.href = 'https://fonts.googleapis.com/css2?family=Cinzel:wght@400;600;700&family=Inter:wght@300;400;500;600&display=swap';
			document.head.appendChild(fontLink);
		}

		// Base styles matching index.html design tokens  
		document.body.style.cssText = `  
        margin:0; padding:0;  
        font-family:'Inter',system-ui,sans-serif;  
        background:#060810;  
        color:#e2e8f0;  
        min-height:100vh;  
        overflow-x:hidden;  
    `;

		document.body.innerHTML = `  
        <style>  
            *, *::before, *::after { box-sizing:border-box; margin:0; padding:0; }  
  
            #starfield { position:fixed; inset:0; pointer-events:none; z-index:0; overflow:hidden; }  
            .star {  
                position:absolute; border-radius:50%; background:#fff;  
                animation: twinkle var(--dur,3s) ease-in-out infinite alternate;  
                opacity: var(--base-opacity,0.4);  
            }  
            @keyframes twinkle {  
                from { opacity:var(--base-opacity,0.4); transform:scale(1); }  
                to   { opacity:var(--peak-opacity,0.9); transform:scale(1.5); }  
            }  
  
            .ambient { position:fixed; border-radius:50%; filter:blur(120px); pointer-events:none; z-index:0; }  
            .ambient-1 {  
                width:600px;height:500px;  
                background:radial-gradient(circle,rgba(232,184,75,0.08) 0%,transparent 70%);  
                top:-200px;left:-100px;  
                animation:drift1 18s ease-in-out infinite alternate;  
            }  
            .ambient-2 {  
                width:500px;height:500px;  
                background:radial-gradient(circle,rgba(74,158,255,0.07) 0%,transparent 70%);  
                bottom:-100px;right:-100px;  
                animation:drift2 22s ease-in-out infinite alternate;  
            }  
            @keyframes drift1 { to { transform:translate(60px,40px); } }  
            @keyframes drift2 { to { transform:translate(-40px,-60px); } }  
  
            .picker-wrapper {  
                position:relative; z-index:1;  
                max-width:520px; margin:0 auto; padding:80px 24px;  
                text-align:center;  
            }  
  
            .emblem {  
                display:inline-flex; align-items:center; justify-content:center;  
                width:80px; height:80px; margin-bottom:24px; position:relative;  
            }  
            .emblem-ring {  
                position:absolute; inset:0; border-radius:50%;  
                border:2px solid #e8b84b;  
                box-shadow:0 0 20px rgba(232,184,75,0.4), inset 0 0 20px rgba(232,184,75,0.1);  
                animation:spin-slow 20s linear infinite;  
            }  
            .emblem-ring::before {  
                content:''; position:absolute; inset:6px; border-radius:50%;  
                border:1px solid rgba(232,184,75,0.3);  
            }  
            @keyframes spin-slow { to { transform:rotate(360deg); } }  
            .emblem-icon { font-size:32px; z-index:1; filter:drop-shadow(0 0 12px rgba(232,184,75,0.8)); }  
  
            .picker-title {  
                font-family:'Cinzel',serif;  
                font-size:clamp(22px,4vw,32px); font-weight:700;  
                background:linear-gradient(135deg,#f5d47c 0%,#e8b84b 50%,#9a7030 100%);  
                -webkit-background-clip:text; -webkit-text-fill-color:transparent;  
                background-clip:text; letter-spacing:0.04em; line-height:1.1;  
                margin-bottom:8px;  
            }  
            .picker-subtitle {  
                font-family:'Cinzel',serif;  
                font-size:clamp(11px,1.8vw,14px);  
                color:#64748b; letter-spacing:0.2em; text-transform:uppercase;  
                margin-bottom:32px;  
            }  
  
            .ornament {  
                display:flex; align-items:center; justify-content:center;  
                gap:12px; margin:0 auto 36px; max-width:280px;  
            }  
            .ornament-line { flex:1; height:1px; background:linear-gradient(to right,transparent,#9a7030); }  
            .ornament-line:last-child { background:linear-gradient(to left,transparent,#9a7030); }  
            .ornament-diamond {  
                width:8px;height:8px; background:#e8b84b;  
                transform:rotate(45deg); box-shadow:0 0 8px #e8b84b; flex-shrink:0;  
            }  
            .ornament-dot { width:4px;height:4px; background:#9a7030; border-radius:50%; flex-shrink:0; }  
  
            .drop-zone {  
                border:2px dashed rgba(232,184,75,0.25);  
                border-radius:12px; padding:40px 24px;  
                background:rgba(21,29,46,0.6);  
                backdrop-filter:blur(10px);  
                transition:border-color 0.3s, box-shadow 0.3s, background 0.3s;  
                cursor:pointer; margin-bottom:24px;  
            }  
            .drop-zone:hover, .drop-zone.dragover {  
                border-color:rgba(232,184,75,0.6);  
                box-shadow:0 0 30px rgba(232,184,75,0.12);  
                background:rgba(21,29,46,0.8);  
            }  
            .drop-zone-icon { font-size:36px; margin-bottom:12px; }  
            .drop-zone-text { color:#64748b; font-size:14px; line-height:1.6; }  
            .drop-zone-text strong { color:#e2e8f0; }  
  
            .file-input { display:none; }  
  
            .file-list {  
                text-align:left; margin:16px 0;  
                max-height:120px; overflow-y:auto;  
                font-size:12px; color:#64748b;  
                padding:0 8px;  
            }  
            .file-list div {  
                padding:4px 0;  
                border-bottom:1px solid rgba(232,184,75,0.08);  
                white-space:nowrap; overflow:hidden; text-overflow:ellipsis;  
            }  
            .file-list div span { color:#e8b84b; margin-right:6px; }  
  
            .btn-start {  
                display:inline-flex; align-items:center; gap:8px;  
                padding:14px 40px;  
                font-family:'Cinzel',serif; font-size:14px; font-weight:600;  
                letter-spacing:0.1em; text-transform:uppercase;  
                color:#060810; background:linear-gradient(135deg,#f5d47c,#e8b84b,#9a7030);  
                border:none; border-radius:8px; cursor:pointer;  
                transition:transform 0.2s, box-shadow 0.3s;  
                box-shadow:0 4px 20px rgba(232,184,75,0.3);  
            }  
            .btn-start:hover {  
                transform:translateY(-2px);  
                box-shadow:0 6px 30px rgba(232,184,75,0.45);  
            }  
            .btn-start:disabled {  
                opacity:0.4; cursor:not-allowed;  
                transform:none; box-shadow:none;  
            }  
  
            .back-link {  
                display:inline-block; margin-top:28px;  
                font-size:12px; color:#64748b; text-decoration:none;  
                letter-spacing:0.05em; transition:color 0.3s;  
            }  
            .back-link:hover { color:#e8b84b; }  
  
            @keyframes fadeUp {  
                from { opacity:0; transform:translateY(24px); }  
                to   { opacity:1; transform:translateY(0); }  
            }  
            .picker-wrapper { animation:fadeUp 0.7s ease both; }  
  
            ::-webkit-scrollbar { width:6px; }  
            ::-webkit-scrollbar-track { background:transparent; }  
            ::-webkit-scrollbar-thumb { background:rgba(232,184,75,0.15); border-radius:3px; }  
            ::-webkit-scrollbar-thumb:hover { background:rgba(232,184,75,0.3); }  
        </style>  
  
        <div id="starfield"></div>  
        <div class="ambient ambient-1"></div>  
        <div class="ambient ambient-2"></div>  
  
        <div class="picker-wrapper">  
            <div class="emblem">  
                <div class="emblem-ring"></div>  
                <span class="emblem-icon">🧪</span>  
            </div>  
  
            <h1 class="picker-title">${this.ext.toUpperCase()} Tester</h1>  
            <p class="picker-subtitle">Asset File Smoke Test</p>  
  
            <div class="ornament">  
                <div class="ornament-line"></div>  
                <div class="ornament-dot"></div>  
                <div class="ornament-diamond"></div>  
                <div class="ornament-dot"></div>  
                <div class="ornament-line"></div>  
            </div>  
  
            <div class="drop-zone" id="dropZone">  
                <div class="drop-zone-icon">📂</div>  
                <div class="drop-zone-text">  
                    <strong>Drop GRF / data files here</strong><br/>  
                    or click to browse  
                </div>  
                <input type="file" class="file-input" id="toolFiles" multiple />  
            </div>  
  
            <div class="file-list" id="fileList"></div>  
  
            <button class="btn-start" id="toolStart" disabled>⚔️ Start Test</button>  
  
            <br/>  
            <a class="back-link" href="../index.html">← Back to Tools</a>  
        </div>  
    `;

		// Starfield  
		const starfield = document.getElementById('starfield');
		for (let i = 0; i < 120; i++) {
			const s = document.createElement('div');
			s.className = 'star';
			const size = Math.random() * 2.2 + 0.4;
			s.style.cssText = `  
            width:${size}px;height:${size}px;  
            left:${Math.random() * 100}%;top:${Math.random() * 100}%;  
            --dur:${(Math.random() * 4 + 2).toFixed(1)}s;  
            --base-opacity:${(Math.random() * 0.4 + 0.15).toFixed(2)};  
            --peak-opacity:${(Math.random() * 0.5 + 0.5).toFixed(2)};  
            animation-delay:${(Math.random() * 6).toFixed(1)}s;  
        `;
			starfield.appendChild(s);
		}

		// File picker logic  
		const dropZone = document.getElementById('dropZone');
		const fileInput = document.getElementById('toolFiles');
		const fileList = document.getElementById('fileList');
		const startBtn = document.getElementById('toolStart');
		let selectedFiles = null;

		dropZone.addEventListener('click', () => fileInput.click());
		dropZone.addEventListener('dragover', e => { e.preventDefault(); dropZone.classList.add('dragover'); });
		dropZone.addEventListener('dragleave', () => dropZone.classList.remove('dragover'));
		dropZone.addEventListener('drop', e => {
			e.preventDefault();
			dropZone.classList.remove('dragover');
			handleFiles(e.dataTransfer.files);
		});
		fileInput.addEventListener('change', () => handleFiles(fileInput.files));

		const handleFiles = (files) => {
			if (!files.length) return;
			selectedFiles = files;
			fileList.innerHTML = '';
			for (let i = 0; i < Math.min(files.length, 10); i++) {
				const d = document.createElement('div');
				d.innerHTML = `<span>◆</span> ${files[i].name}`;
				fileList.appendChild(d);
			}
			if (files.length > 10) {
				const d = document.createElement('div');
				d.textContent = `… and ${files.length - 10} more`;
				fileList.appendChild(d);
			}
			startBtn.disabled = false;
		};

		startBtn.addEventListener('click', () => {
			if (!selectedFiles) return;
			Client.onFilesLoaded = () => {
				const pattern = new RegExp(`data\\\\[^\\0]+\\.${this.ext}`, 'gi');
				Client.search(pattern, list => this._run(list));
			};
			Client.init(Array.from(selectedFiles));
		});
	}
	/**
	 * Iterate every matched file path, call the loader callback, report
	 * progress and errors live in the DOM.
	 *
	 * @param {string[]} list
	 */
	_run(list) {
		const { ext, callback } = this;
		const count = list.length;

		// Keep starfield & ambient from the picker, rebuild content area  
		document.body.innerHTML = `  
        <style>  
            *, *::before, *::after { box-sizing:border-box; margin:0; padding:0; }  
            body {  
                font-family:'Inter',system-ui,sans-serif;  
                background:#060810; color:#e2e8f0;  
                min-height:100vh; overflow-x:hidden;  
            }  
  
            #starfield { position:fixed; inset:0; pointer-events:none; z-index:0; overflow:hidden; }  
            .star {  
                position:absolute; border-radius:50%; background:#fff;  
                animation:twinkle var(--dur,3s) ease-in-out infinite alternate;  
                opacity:var(--base-opacity,0.4);  
            }  
            @keyframes twinkle {  
                from { opacity:var(--base-opacity,0.4); transform:scale(1); }  
                to   { opacity:var(--peak-opacity,0.9); transform:scale(1.5); }  
            }  
  
            .ambient { position:fixed; border-radius:50%; filter:blur(120px); pointer-events:none; z-index:0; }  
            .ambient-1 {  
                width:600px;height:500px;  
                background:radial-gradient(circle,rgba(232,184,75,0.08) 0%,transparent 70%);  
                top:-200px;left:-100px;  
                animation:drift1 18s ease-in-out infinite alternate;  
            }  
            .ambient-2 {  
                width:500px;height:500px;  
                background:radial-gradient(circle,rgba(74,158,255,0.07) 0%,transparent 70%);  
                bottom:-100px;right:-100px;  
                animation:drift2 22s ease-in-out infinite alternate;  
            }  
            @keyframes drift1 { to { transform:translate(60px,40px); } }  
            @keyframes drift2 { to { transform:translate(-40px,-60px); } }  
  
            .test-wrapper {  
                position:relative; z-index:1;  
                max-width:800px; margin:0 auto; padding:48px 24px 80px;  
            }  
  
            .test-header { text-align:center; margin-bottom:36px; }  
  
            .test-title {  
                font-family:'Cinzel',serif;  
                font-size:clamp(22px,4vw,34px); font-weight:700;  
                background:linear-gradient(135deg,#f5d47c 0%,#e8b84b 50%,#9a7030 100%);  
                -webkit-background-clip:text; -webkit-text-fill-color:transparent;  
                background-clip:text; letter-spacing:0.04em; margin-bottom:6px;  
            }  
            .test-desc {  
                font-size:13px; color:#64748b; letter-spacing:0.05em;  
            }  
            .test-desc code {  
                color:#e8b84b; background:rgba(232,184,75,0.1);  
                padding:1px 6px; border-radius:4px; font-size:12px;  
            }  
  
            .ornament {  
                display:flex; align-items:center; justify-content:center;  
                gap:12px; margin:20px auto 0; max-width:240px;  
            }  
            .ornament-line { flex:1; height:1px; background:linear-gradient(to right,transparent,#9a7030); }  
            .ornament-line:last-child { background:linear-gradient(to left,transparent,#9a7030); }  
            .ornament-diamond {  
                width:6px;height:6px; background:#e8b84b;  
                transform:rotate(45deg); box-shadow:0 0 6px #e8b84b; flex-shrink:0;  
            }  
  
            /* ── Progress panel ─────────────────────────── */  
            .progress-panel {  
                background:rgba(21,29,46,0.7);  
                border:1px solid rgba(232,184,75,0.15);  
                border-radius:12px; padding:24px;  
                backdrop-filter:blur(10px);  
                margin-bottom:24px;  
            }  
            .progress-label {  
                font-family:'Cinzel',serif;  
                font-size:11px; font-weight:600; color:#e8b84b;  
                letter-spacing:0.2em; text-transform:uppercase;  
                margin-bottom:12px;  
            }  
  
            .progress-bar-wrap {  
                width:100%; height:8px;  
                background:rgba(255,255,255,0.06);  
                border-radius:4px; overflow:hidden;  
                margin-bottom:14px;  
            }  
            .progress-bar {  
                height:100%; width:0%;  
                background:linear-gradient(90deg,#9a7030,#e8b84b,#f5d47c);  
                border-radius:4px;  
                transition:width 0.3s ease;  
            }  
  
            .progress-stats {  
                display:flex; justify-content:space-between; align-items:center;  
                flex-wrap:wrap; gap:8px;  
            }  
            .stat {  
                font-size:12px; color:#64748b;  
            }  
            .stat strong { color:#e2e8f0; }  
            .stat .gold { color:#e8b84b; }  
  
            .progress-file {  
                font-size:11px; color:#64748b;  
                margin-top:10px;  
                white-space:nowrap; overflow:hidden; text-overflow:ellipsis;  
                max-width:100%;  
            }  
            .progress-file span { color:#4a9eff; }  
  
            /* ── Errors panel ──────────────────────────── */  
            .errors-panel {  
                background:rgba(21,29,46,0.7);  
                border:1px solid rgba(239,68,68,0.15);  
                border-radius:12px; padding:24px;  
                backdrop-filter:blur(10px);  
            }  
            .errors-header {  
                display:flex; align-items:center; gap:12px;  
                margin-bottom:16px;  
            }  
            .errors-label {  
                font-family:'Cinzel',serif;  
                font-size:11px; font-weight:600; color:#ef4444;  
                letter-spacing:0.2em; text-transform:uppercase;  
            }  
            .errors-line { flex:1; height:1px; background:rgba(239,68,68,0.15); }  
            .error-count {  
                font-size:11px; color:#ef4444;  
                background:rgba(239,68,68,0.1);  
                border:1px solid rgba(239,68,68,0.2);  
                padding:2px 10px; border-radius:12px;  
            }  
  
            .error-list { max-height:400px; overflow-y:auto; }  
  
            .error-card {  
                background:rgba(239,68,68,0.04);  
                border:1px solid rgba(239,68,68,0.1);  
                border-radius:8px; padding:16px;  
                margin-bottom:10px;  
            }  
            .error-card h4 {  
                font-size:12px; color:#f87171;  
                word-break:break-all; margin-bottom:8px;  
                font-family:'Inter',monospace;  
            }  
            .error-card p {  
                font-size:12px; color:#e2e8f0;  
                margin-bottom:6px;  
            }  
            .error-card pre {  
                font-size:10px; color:#64748b;  
                background:rgba(0,0,0,0.3);  
                padding:10px; border-radius:6px;  
                overflow-x:auto; white-space:pre-wrap;  
                word-break:break-all;  
                max-height:120px;  
            }  
  
            .no-errors {  
                text-align:center; padding:20px;  
                font-size:13px; color:#64748b;  
            }  
            .no-errors span { font-size:20px; display:block; margin-bottom:6px; }  
  
            /* ── Done banner ───────────────────────────── */  
            .done-banner {  
                display:none; text-align:center;  
                background:rgba(34,197,94,0.08);  
                border:1px solid rgba(34,197,94,0.2);  
                border-radius:12px; padding:24px;  
                margin-bottom:24px;  
            }  
            .done-banner.visible { display:block; }  
            .done-banner .done-icon { font-size:32px; margin-bottom:8px; }  
            .done-banner h3 {  
                font-family:'Cinzel',serif;  
                font-size:16px; color:#22c55e;  
                margin-bottom:4px;  
            }  
            .done-banner p { font-size:13px; color:#64748b; }  
            .done-banner p strong { color:#e2e8f0; }  
  
            .back-link {  
                display:inline-block; margin-top:24px;  
                font-size:12px; color:#64748b; text-decoration:none;  
                letter-spacing:0.05em; transition:color 0.3s;  
                text-align:center; width:100%;  
            }  
            .back-link:hover { color:#e8b84b; }  
  
            @keyframes fadeUp {  
                from { opacity:0; transform:translateY(24px); }  
                to   { opacity:1; transform:translateY(0); }  
            }  
            .test-wrapper { animation:fadeUp 0.6s ease both; }  
  
            ::-webkit-scrollbar { width:6px; }  
            ::-webkit-scrollbar-track { background:transparent; }  
            ::-webkit-scrollbar-thumb { background:rgba(232,184,75,0.15); border-radius:3px; }  
            ::-webkit-scrollbar-thumb:hover { background:rgba(232,184,75,0.3); }  
        </style>  
  
        <div id="starfield"></div>  
        <div class="ambient ambient-1"></div>  
        <div class="ambient ambient-2"></div>  
  
        <div class="test-wrapper">  
            <div class="test-header">  
                <h1 class="test-title">${ext.toUpperCase()} Tester</h1>  
                <p class="test-desc">Loading each <code>.${ext}</code> file to detect parse errors…</p>  
                <div class="ornament">  
                    <div class="ornament-line"></div>  
                    <div class="ornament-diamond"></div>  
                    <div class="ornament-line"></div>  
                </div>  
            </div>  
  
            <div id="doneBanner" class="done-banner">  
                <div class="done-icon">✅</div>  
                <h3>Test Complete</h3>  
                <p id="doneText"></p>  
            </div>  
  
            <div class="progress-panel">  
                <div class="progress-label">⚔️ &nbsp; Progress</div>  
                <div class="progress-bar-wrap">  
                    <div class="progress-bar" id="progressBar"></div>  
                </div>  
                <div class="progress-stats">  
                    <span class="stat"><strong id="statCurrent">0</strong> / <strong id="statTotal">${count}</strong> files</span>  
                    <span class="stat">ETA: <span class="gold" id="statEta">—</span></span>  
                    <span class="stat">Errors: <span class="gold" id="statErrors">0</span></span>  
                </div>  
                <div class="progress-file" id="progressFile">Initializing…</div>  
            </div>  
  
            <div class="errors-panel">  
                <div class="errors-header">  
                    <span class="errors-label">🔴 &nbsp; Errors</span>  
                    <div class="errors-line"></div>  
                    <span class="error-count" id="errorBadge">0</span>  
                </div>  
                <div class="error-list" id="errorList">  
                    <div class="no-errors" id="noErrors">  
                        <span>✨</span>  
                        No errors yet — looking good!  
                    </div>  
                </div>  
            </div>  
  
            <a class="back-link" href="../index.html">← Back to Tools</a>  
        </div>  
    `;

		// Starfield  
		const starfield = document.getElementById('starfield');
		for (let i = 0; i < 120; i++) {
			const s = document.createElement('div');
			s.className = 'star';
			const size = Math.random() * 2.2 + 0.4;
			s.style.cssText = `  
            width:${size}px;height:${size}px;  
            left:${Math.random() * 100}%;top:${Math.random() * 100}%;  
            --dur:${(Math.random() * 4 + 2).toFixed(1)}s;  
            --base-opacity:${(Math.random() * 0.4 + 0.15).toFixed(2)};  
            --peak-opacity:${(Math.random() * 0.5 + 0.5).toFixed(2)};  
            animation-delay:${(Math.random() * 6).toFixed(1)}s;  
        `;
			starfield.appendChild(s);
		}

		// DOM refs  
		const progressBar = document.getElementById('progressBar');
		const statCurrent = document.getElementById('statCurrent');
		const statEta = document.getElementById('statEta');
		const statErrors = document.getElementById('statErrors');
		const progressFile = document.getElementById('progressFile');
		const errorList = document.getElementById('errorList');
		const errorBadge = document.getElementById('errorBadge');
		const noErrors = document.getElementById('noErrors');
		const doneBanner = document.getElementById('doneBanner');
		const doneText = document.getElementById('doneText');

		let errors = 0;
		let index = 0;
		const start = Date.now();

		const ctx = {
			count, callback, start,
			progressBar, statCurrent, statEta, statErrors,
			progressFile, errorList, errorBadge, noErrors,
			doneBanner, doneText, ext,
			errors: () => errors,
			incErrors: () => { errors++; statErrors.textContent = errors; errorBadge.textContent = errors; },
			getIndex: () => index,
			incIndex: () => index++,
		};

		const concurrency = Math.min(5, count);
		for (let i = 0; i < concurrency; i++) {
			this._loadNext(list, index++, ctx);
		}
	}

	_loadNext(list, i, ctx) {
		const { count, callback, start,
			progressBar, statCurrent, statEta,
			progressFile, errorList, noErrors,
			doneBanner, doneText, ext } = ctx;

		Client.getFile(list[i], data => {
			const currentIndex = ctx.getIndex();
			const elapsed = Date.now() - start;
			const pct = ((currentIndex + 1) / count * 100).toFixed(1);
			const eta = currentIndex > 0
				? Math.floor(((elapsed / currentIndex) * count - elapsed) * 0.001 + 1)
				: 0;

			// Update progress UI  
			progressBar.style.width = pct + '%';
			statCurrent.textContent = currentIndex + 1;
			statEta.textContent = eta + 's';
			progressFile.innerHTML = `<span>▸</span> ${list[i]}`;

			try {
				callback(data);
			} catch (e) {
				noErrors.style.display = 'none';
				ctx.incErrors();

				const card = document.createElement('div');
				card.className = 'error-card';
				card.innerHTML = `  
                <h4>📄 ${list[i]}</h4>  
                <p>${e.message}</p>  
                <pre>${e.stack}</pre>  
            `;
				errorList.appendChild(card);
			}

			Memory.remove(null, list[i]);
			ctx.incIndex();

			const next = ctx.getIndex();
			if (next < count) {
				this._loadNext(list, next, ctx);
			}

			// All concurrent slots finished  
			if (next >= count + Math.min(5, count) - 1) {
				const secs = Math.floor((Date.now() - start) / 1000);
				progressBar.style.width = '100%';
				doneText.innerHTML = `<strong>${count}</strong> .${ext} files loaded — <strong>${ctx.errors()}</strong> error(s) in <strong>${secs}s</strong>`;
				doneBanner.classList.add('visible');
			}
		});
	}
}

export default FileTester;