# Translation manager

`translation_manager.py` is a Python script for managing Qt translation files (`.ts` and `.qm`). It can:

1. Create a new translation source (`.ts`) file for a given language.
2. Update existing `.ts` files by running `lupdate` and pruning obsolete strings.
3. Compile `.ts` files into binary `.qm` files using `lrelease`.

## Prerequisites

Before running this script, make sure you have:

- **Qt Linguist.** The `lupdate` and `lrelease` commands come with the Qt toolchain. They need to be on your `PATH`.

You can check the install with:

```bash
lupdate --version
lrelease --version
```

You also need Python 3.x.

## Usage

The script exposes three main operations:

- Create a new `.ts` file for a language.
- Update existing `.ts` files with `lupdate`.
- Compile `.ts` files into `.qm` files with `lrelease`.

### 1. Creating a new translation file

Pass `--new-ts` with the language code (for example `es_MX` for Mexican Spanish):

```bash
python3 translation_manager.py --new-ts es_MX
```

This creates `es_MX.ts` in `app/translations/ts`, with the source language set to `en_US`.

### 2. Updating existing `.ts` files

To refresh `.ts` files and drop obsolete entries, use `--lupdate`:

```bash
python3 translation_manager.py --lupdate
```

This scans `.cpp`, `.h`, and `.qml` sources in the `app` and `lib` directories and updates the `.ts` files in `app/translations/ts`.

### 3. Compiling `.ts` into `.qm`

To build the binary `.qm` files that the app loads at runtime, use `--lrelease`:

```bash
python3 translation_manager.py --lrelease
```

The `.qm` files land in `app/translations/qm`.

### 4. Running both `lupdate` and `lrelease`

You can chain the two:

```bash
python3 translation_manager.py --lupdate --lrelease
```

### 5. Help

Running the script with no arguments prints help:

```bash
python3 translation_manager.py
```

Output:

```
usage: translation_manager.py [-h] [--new-ts LANGUAGE] [--lupdate] [--lrelease]

Manage translations with lupdate and lrelease.

optional arguments:
  -h, --help           show this help message and exit
  --new-ts LANGUAGE    Create a new .ts file for the given language code (e.g., "es" for Spanish).
  --lupdate            Run lupdate to update all existing .ts files.
  --lrelease           Run lrelease to compile .ts files into .qm files.
```

## Folder structure

The script expects a layout like this:

```
app/
├── translations/
│   ├── ts/                      # Source .ts files
│   │   ├── en_US.ts
│   │   ├── es_MX.ts
│   │   └── ru_RU.ts
│   ├── qm/                      # Compiled .qm files
│   └── translation_manager.py   # This script
```

## Example commands

1. Create a new French translation file (`fr_FR.ts`):

```bash
python3 translation_manager.py --new-ts fr_FR
```

2. Update all existing `.ts` files:

```bash
python3 translation_manager.py --lupdate
```

3. Compile `.ts` files into `.qm`:

```bash
python3 translation_manager.py --lrelease
```

4. Update and compile in one go:

```bash
python3 translation_manager.py --lupdate --lrelease
```

## Notes

- The source language is assumed to be `en_US`, and it's set automatically when you create a new `.ts` file.
- The `lib` folder (which holds extra source files) needs to sit next to `app`.
