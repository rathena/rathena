/**
 * vite/csshotreload.plugin.js
 *
 * Hot reload CSS changes in UI components without full page reload
 *
 * @author AoShinHo
 */

export default function uiCssHmrPlugin() {
	return {
		name: 'ui-css-hmr',
		apply: 'serve',
		transform(code, id) {
			if (!id.includes('/src/UI/Components/') || !id.endsWith('.js')) {
				return null;
			}

			const cssImportRegex = /import\s+(\w+)\s+from\s+['"](\.[^'"]+\.css\?raw)['"]/g;
			const match = cssImportRegex.exec(code);
			if (!match) return null;

			const cssPath = match[2];

			const compRegex = /(?:new\s+GUIComponent)\(\s*['"](\w+)['"]/;
			const compMatch = compRegex.exec(code);
			if (!compMatch) return null;

			const componentName = compMatch[1];

			const hmrBlock = `
				if (import.meta.hot) {
					import.meta.hot.accept('${cssPath}', (newModule) => {
						if (newModule && newModule.default) {
							const comp = UIManager.components['${componentName}'];
							if (comp && comp._shadow) {
								const style = comp._shadow.querySelector('style[data-component]');
								if (style) style.textContent = newModule.default;
							}
						}
					});
				}`;
			return { code: code + hmrBlock, map: null };
		}
	};
}
