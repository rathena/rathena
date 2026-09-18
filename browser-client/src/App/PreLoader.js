/**
 * App/PreLoader.js
 *
 * Manages the initial loading screen shown before the game engine is ready.
 * Standalone module with no project imports to avoid circular dependencies.
 */

const PRELOADER_CSS = `  
	#ro-preloader {  
		position: fixed;  
		top: 0; left: 0;  
		width: 100%; height: 100%;  
		z-index: 99999;  
		background: rgba(6, 8, 16, 0.97);  
		display: flex;  
		align-items: center;  
		justify-content: center;  
		flex-direction: column;  
		transition: opacity 0.3s ease;  
	}  
	#ro-preloader.fade-out { opacity: 0; }  
	#ro-preloader .pre-spinner {  
		width: 48px; height: 48px;  
		margin: 0 auto 16px;  
		border: 4px solid rgba(232, 184, 75, 0.2);  
		border-top-color: #e8b84b;  
		border-radius: 50%;  
		animation: ro-pre-spin 0.8s linear infinite;  
	}  
	#ro-preloader .pre-text {  
		font-family: serif;  
		font-size-adjust: none; /* keep serif's native x-height, outside body's Arial normalization */  
		font-size: 16px;  
		letter-spacing: 3px;  
		text-transform: uppercase;  
		color: #e8b84b;  
	}  
	#ro-preloader .pre-text span {  
		display: inline-block;  
		animation: ro-pre-wave 1.2s ease-in-out infinite;  
		animation-delay: calc(var(--i) * 0.08s);  
	}  
	@keyframes ro-pre-spin { to { transform: rotate(360deg); } }  
	@keyframes ro-pre-wave {  
		0%, 100% { transform: translateY(0); }  
		50% { transform: translateY(-6px); }  
	}  
`;

const PRELOADER_INNER_HTML =
	'<div class="pre-spinner"></div>' +
	'<p class="pre-text">' +
	'<span style="--i:0">L</span>' +
	'<span style="--i:1">o</span>' +
	'<span style="--i:2">a</span>' +
	'<span style="--i:3">d</span>' +
	'<span style="--i:4">i</span>' +
	'<span style="--i:5">n</span>' +
	'<span style="--i:6">g</span>' +
	'<span style="--i:7">.</span>' +
	'<span style="--i:8">.</span>' +
	'<span style="--i:9">.</span>' +
	'</p>';

export const roInitSpinner = {
	divElem: null,
	styleElem: null,

	/**
	 * Grab the pre-existing #ro-preloader from the HTML.
	 * If it doesn't exist (e.g. inside an iframe / api.html),
	 * dynamically create one with the same styling.
	 */
	add: function () {
		let el = document.getElementById('ro-preloader');
		if (!el) {
			// Inject CSS
			const style = document.createElement('style');
			style.id = 'ro-preloader-style';
			style.textContent = PRELOADER_CSS;
			// Dummy style tag so roBrowser doesn't clobber ours
			document.head.appendChild(document.createElement('style'));
			roInitSpinner.styleElem = document.head.appendChild(style);

			// Create element
			el = document.createElement('div');
			el.id = 'ro-preloader';
			el.innerHTML = PRELOADER_INNER_HTML;
			document.body.appendChild(el);
		}
		roInitSpinner.divElem = el;
	},

	/**
	 * Fade out and remove the preloader.
	 * Safe to call multiple times.
	 */
	remove: function () {
		const el = roInitSpinner.divElem;
		if (!el) return;
		el.classList.add('fade-out');
		const cleanup = () => {
			el.remove();
			roInitSpinner.divElem = null;
			if (roInitSpinner.styleElem) {
				roInitSpinner.styleElem.remove();
				roInitSpinner.styleElem = null;
			}
		};
		el.addEventListener('transitionend', cleanup);
		// Fallback if transitionend doesn't fire
		setTimeout(cleanup, 400);
	}
};

export default roInitSpinner;
