# Background

It started as an internal component of [qmarkdowntextedit](https://github.com/pbek/qmarkdowntextedit) to provide syntax highlighting(kind of like **highlight.js**). Ithttps://github.com/pbek/qmarkdowntextedit is currently being used in QOwnNotes and you can test it there or run the demo here.

It doesn't use any regex because I want it to be fast. It can currently load a 100,000 lines of source code in ~0.4 seconds on my 4th Gen Intel Core i5 4300U.

# Screenshot

![Cpp](screenshot/syntax.png)

# Usage

Add the `.pri` file to your project. Then initialize it like this:
```cpp
highlighter = new QSourceHighliter(plainTextEdit->document());
highlighter->setCurrentLanguage(QSourceHighlighter::CodeCpp);
```

# Themes

Currently there is only one theme 'Monokai' apart from the one that is created during highlighter initialization. More themes will be added soon. You can add more themes in QSourceHighlighterThemes.

## Supported Languages

Currently the following languages are supported (more being added):
- Bash script
- C
- C++
- C#
- CMake
- CSS
- Go
- Html
- INI
- Java
- Javascript
- JSON
- Make
- PHP
- Python
- QML
- Rust
- SQL
- Typescript
- V lang
- XML
- YAML
- Houdini Vex

## Adding more languages

If you want to add a language, collect the language data like keywords and types and add it to the `languagedata.h` file. For some languages it may not work, so create an issue and I will write a separate parser for that language.

## Dependencies

It has no dependency except Qt ofcourse. It should work with any Qt version > 5 but if it fails please create an issue.

## Building

Load the project into Qt Creator and click run.

## LICENSE

MIT License

