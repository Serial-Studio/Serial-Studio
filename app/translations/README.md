# Translations

Two scripts live in this directory:

- `translation_manager.py` — wraps Qt's `lupdate`/`lrelease` and creates new `.ts` files.
- `llm_translate.py` — fills missing translations across all `.ts` files using an LLM (Anthropic Claude by default; OpenAI optional).

The `ts/` folder holds source `.ts` files; `qm/` holds the compiled `.qm` files the app loads at runtime.

## Prerequisites

- **Qt Linguist tools.** `lupdate` and `lrelease` must be on your `PATH`:

  ```bash
  lupdate --version
  lrelease --version
  ```

- **Python 3.x.**

- For `llm_translate.py`, install one of:

  ```bash
  pip3 install anthropic lxml      # Anthropic (default)
  pip3 install openai lxml         # OpenAI
  ```

  And export the corresponding API key:

  ```bash
  export ANTHROPIC_API_KEY=sk-ant-...
  # or
  export OPENAI_API_KEY=sk-...
  ```

## `translation_manager.py`

The script exposes three operations: create a new `.ts` file, run `lupdate`, run `lrelease`.

### Create a new translation file

```bash
python3 translation_manager.py --new-ts es_MX
```

Creates `ts/es_MX.ts` with the source language set to `en_US`.

### Update existing `.ts` files

```bash
python3 translation_manager.py --lupdate
```

Scans `.cpp`, `.h`, and `.qml` sources under `app/` and `lib/`, updates the `.ts` files in `ts/`, and prunes obsolete entries.

### Compile `.ts` into `.qm`

```bash
python3 translation_manager.py --lrelease
```

Writes binary `.qm` files into `qm/`.

### Both at once

```bash
python3 translation_manager.py --lupdate --lrelease
```

## `llm_translate.py`

Fills in missing translations across every `.ts` file in `ts/` (skipping `en_US.ts`, which is treated as the source). It runs `lupdate` first, then translates, then runs `lrelease`.

The script defaults to **Anthropic Claude** (`claude-sonnet-4-5`). Pass `--provider openai` to switch to `gpt-4.1`.

### Translate everything

```bash
python3 llm_translate.py
```

### One language only

```bash
python3 llm_translate.py --lang fr_FR
```

### Switch provider or model

```bash
python3 llm_translate.py --provider openai
python3 llm_translate.py --provider anthropic --model claude-sonnet-4-5
```

### Reset and re-translate

```bash
python3 llm_translate.py --reset                  # all languages
python3 llm_translate.py --reset --lang fr_FR     # one language
```

`--reset` clears every translation in non-`en_US` `.ts` files. Combine with a normal run to re-translate from scratch.

### Re-apply deterministic capitalization (no LLM)

```bash
python3 llm_translate.py --verify-only
```

Walks existing translations and re-applies the title-case / acronym / language-specific casing rules without calling the LLM.

### Lint `en_US.ts` source strings (no LLM)

```bash
python3 llm_translate.py --lint-sources
```

Scans the English source strings for style violations (Apple-HIG voice: imperative actions, neutral labels, no filler politeness, technical acronyms uppercase, etc.).

### Quality features

- Domain glossary in the prompt — disambiguates `Will Topic` (MQTT field), `Holding Register` (Modbus), `Frame` (CAN/serial vs UI), `Dataset` (data series).
- Translation memory — top-K most similar already-validated translations from the same `.ts` file are injected as few-shot examples per batch.
- Confidence scoring — the LLM rates each translation 1–5; entries below `--min-score` (default 4) keep `type='unfinished'` so Qt Linguist surfaces them for human review.
- Source-aware acronym enforcement — only acronyms present in the EN source get force-uppercased in the translation.
- Language-aware title case — Spanish `Cuadrícula de Datos` (lowercase `de`), French `Grille de Données`, German capitalizes nouns.

## Folder structure

```
app/translations/
├── ts/                       # Source .ts files
│   ├── en_US.ts
│   ├── es_MX.ts
│   └── ...
├── qm/                       # Compiled .qm files
├── translations.qrc
├── translation_manager.py
└── llm_translate.py
```

## Notes

- The source language is `en_US`. `translation_manager.py --new-ts` sets it automatically.
- `llm_translate.py` never sends `en_US.ts` to the LLM — it only translates the other locales.
- The `lib/` folder needs to sit next to `app/` for `lupdate` to find third-party sources.
