# Translation Manager

`translation_manager.py` is a Python script designed to manage Qt translation files (`.ts` and `.qm`). It can be used to:
1. Create new translation source (`.ts`) files for a given language.
2. Update existing `.ts` files by running `lupdate` and removing obsolete strings.
3. Compile `.ts` files into binary `.qm` files using `lrelease`.

## Prerequisites

Before using this script, ensure the following tools are installed on your system:
- **Qt Linguist**: The `lupdate` and `lrelease` commands are part of the Qt toolchain. Make sure they are available in your system's path.

You can verify the installation by running:

```bash
lupdate --version
lrelease --version
```

Additionally, ensure that you have Python 3.x installed.

## Usage

The script provides three main functionalities:
- **Create a new `.ts` file for a language**
- **Update existing `.ts` files using `lupdate`**
- **Compile `.ts` files into `.qm` files using `lrelease`**

### 1. Creating a New Translation File

To create a new `.ts` file for a language, use the `--new-ts` option followed by the language code (e.g., `es_MX` for Mexican Spanish):

```bash
python3 translation_manager.py --new-ts es_MX
```

This will generate a new `es_MX.ts` file in the `app/translations/ts` folder with the source language set to `en_US`.

### 2. Updating Existing `.ts` Files

To update existing `.ts` files and remove obsolete entries, use the `--lupdate` option:

```bash
python3 translation_manager.py --lupdate
```

This will scan the source files (both `.cpp`, `.h`, and `.qml`) in the `app` and `lib` directories and update the translations in the `.ts` files located in the `app/translations/ts` folder.

### 3. Compiling `.ts` Files into `.qm` Files

To compile the `.ts` files into binary `.qm` files for use in the application, use the `--lrelease` option:

```bash
python3 translation_manager.py --lrelease
```

The `.qm` files will be generated and placed in the `app/translations/qm` folder.

### 4. Running Both `lupdate` and `lrelease`

You can also combine both updating and compiling into a single command:

```bash
python3 translation_manager.py --lupdate --lrelease
```

### 5. Help and Usage Instructions

If no arguments are provided, the script will display the help message:

```bash
python3 translation_manager.py
```

This will output the following information:

```
usage: translation_manager.py [-h] [--new-ts LANGUAGE] [--lupdate] [--lrelease]

Manage translations with lupdate and lrelease.

optional arguments:
  -h, --help           show this help message and exit
  --new-ts LANGUAGE    Create a new .ts file for the given language code (e.g., "es" for Spanish).
  --lupdate            Run lupdate to update all existing .ts files.
  --lrelease           Run lrelease to compile .ts files into .qm files.
```

## Folder Structure

Here’s an example of the folder structure where the script operates:

```
app/
├── translations/
│   ├── ts/          # Folder containing the .ts files (source translations)
│   │   ├── en_US.ts
│   │   ├── es_MX.ts
│   │   └── ru_RU.ts
│   ├── qm/          # Folder where the .qm files (compiled translations) are stored
│   └── translation_manager.py   # This script
```

## Example Commands

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

4. Perform both update and compile:

```bash
python3 translation_manager.py --lupdate --lrelease
```

## Notes

- The script assumes that your source language is `en_US`, and this is automatically set when creating new `.ts` files.
- Make sure that the `lib` folder (which contains additional source files) exists at the same level as the `app` folder.
