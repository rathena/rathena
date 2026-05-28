// Display formatters — comma-separated numbers, durations, etc.
// Pure functions, safe to call anywhere. Import from `@lib/format`.

/**
 * Format a number with thousands separators using `,`. Faster + smaller
 * than `Number.prototype.toLocaleString()` and works even if the V8
 * build was compiled without Intl support.
 *
 * @example
 * formatNumber(1234567)  // "1,234,567"
 */
export function formatNumber(n: number): string {
    const s = String(Math.trunc(n));
    const sign = s.startsWith("-") ? "-" : "";
    const digits = sign ? s.slice(1) : s;
    let out = "";
    for (let i = digits.length; i > 0; i -= 3) {
        const start = Math.max(0, i - 3);
        out = (start === 0 ? "" : ",") + digits.slice(start, i) + out;
    }
    return sign + out;
}

/**
 * Format zeny with a trailing `z` suffix.
 *
 * @example formatZeny(15000)  // "15,000z"
 */
export function formatZeny(amount: number): string {
    return `${formatNumber(amount)}z`;
}

/**
 * Format a duration in seconds as `Xh Ym Zs` (parts omitted when 0).
 *
 * @example
 * formatDuration(3725)  // "1h 2m 5s"
 * formatDuration(45)    // "45s"
 */
export function formatDuration(seconds: number): string {
    if (seconds <= 0) return "0s";
    const h = Math.trunc(seconds / 3600);
    const m = Math.trunc((seconds % 3600) / 60);
    const s = seconds % 60;
    const parts: string[] = [];
    if (h > 0) parts.push(`${h}h`);
    if (m > 0) parts.push(`${m}m`);
    if (s > 0 || parts.length === 0) parts.push(`${s}s`);
    return parts.join(" ");
}

/**
 * Pad a number to N digits, left-filling with `0`.
 *
 * @example
 * padNum(7, 3)  // "007"
 */
export function padNum(n: number, width: number): string {
    const s = String(Math.trunc(n));
    return s.length >= width ? s : "0".repeat(width - s.length) + s;
}
