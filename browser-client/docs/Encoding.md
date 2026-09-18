# Encoding & Text Handling (CRITICAL)

## Overview

roBrowserLegacy uses **multiple encodings depending on data source**.
There is **NO global encoding**.

All encoding logic is centralized in:

```javascript
Utils / CodepageManager.js;
```

---

## Core Rule

> **Never assume encoding. Always decode bytes with context.**

---

## Data Encoding Contract

### 1. Filesystem / GRF (filenames, lookup)

- Encoding: `'windows-1252'`
- Used for:
    - GRF file paths
    - filename lookup

```javascript
TextEncoding.setCharset('windows-1252');
```

**Reason:**
Browser string handling and path lookup are more stable with windows-1252.

---

### 2. User-facing text (DB, Lua, UI)

- Encoding: `userCharpage`
- Determined by:

```javascript
TextEncoding.detectEncodingByLangtype(langType);
```

Examples:

- Korean → `windows-949` → langType 0
- Russian → `windows-1251` → langType 5
- Default → `windows-1252` → langType 1 or 12

**Usage:**

```javascript
TextEncoding.decode(bytes, userCharpage);
```

---

### 3. Hybrid / Unknown data (network, binary blobs)

- Attempt UTF-8 first
- Fallback to user charset if invalid (each new instance of require() have the own userCharpage/default[windows-1252])

```javascript
TextEncoding.decode(bytes, 'utf-8');
```

Internally:

```javascript
UTF-8 → if contains '�' → fallback
```

---

## Pipeline Contract (VERY IMPORTANT)

```text
Client.loadFile
    ↓
FileManager → returns Uint8Array (RAW BYTES ONLY)
    ↓
Consumer (DB / Loader / Engine) decides encoding
    ↓
Decoded string (final)
```

### DO NOT:

- Decode inside `FileManager`
- Use `String.fromCharCode` for text
- Use `TextDecoder` directly
- Re-decode already decoded strings

---

## Correct Pattern for User-facing text in DB

```javascript
Client.loadFile(filename, function (buffer) {
	var text = TextEncoding.decode(buffer, userCharpage);
});
```

---

## Incorrect Patterns (Legacy Bugs)

```javascript
// WRONG: implicit decoding
str += String.fromCharCode(byte);

// WRONG: forcing UTF-8 everywhere
new TextDecoder('utf-8').decode(buffer);

// WRONG: decoding twice
TextEncoding.decodeString(alreadyDecodedString);
```

---

## File-specific Encoding Control

Some files must explicitly use user charset:

```javascript
loadTable(..., useCharPage = true);
```

Others should NOT:

- GRF metadata
- internal system files

---

## CSV / Base64 Handling

Newer clients may use Base64-encoded CSV.

Rules:

- If Base64 → decode → UTF-8
- Else → treat as plain text (tab-separated)

---

## Common Pitfalls

### 1. Broken Korean / Russian text

→ Wrong charset used (missing `userCharpage`)

### 2. File not found in GRF

→ Wrong encoding for filename (must be `windows-1252`)

### 3. Random `�` characters

→ UTF-8 decode on non-UTF8 data without fallback

### 4. Text works in one place but not another

→ Double decoding or mixed encoding contexts

---

## Design Principle

> Encoding is not a property of the data —
> it is a property of the **context where the data is interpreted**.

---
