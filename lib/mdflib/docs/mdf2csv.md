---
layout: default
title: MDF to CSV
---

# MDF to CSV

The MDF to CSV (mdf2csv.exe) is a command line application that convert an MDF file to one or more CSV files. It is
a replacement of the CSS mdf2csv application as this application handle all types of MDF file not just CSS log files.

Simplest use is to either drag and drop MDF file(s) (Windows only) or supply the MDF file path as a command line 

Available command line options.
- **-h [ --help ]**. Display all available command line options and then exit.
- **-V [ --version ]**. Display the tool version and then exit
- **-v [ --verbosity ]** arg. Sets the log level. 
- **--non-interactive**. Does not print any progress output.
- **-q [ --quiet ]**. Does not print any progress output.
- **--silent**. Does not print any progress output.
- **--force**. Overwrite existing CSV files.
- **-d [ --delete-converted ]**. Delete original files after they are converted. Ignored by this application.
- **-X [ --no-append-root ]**. Do not append "_out" when converting folders. Ignored by this application.
- **-b [ --buffer ] arg**. Buffer size in bytes. 0 disables and -1 takes entire file.
- **-p [ --password-file ] arg**. Path to password file. Ignored by this application.
- **-O [ --output-directory ] arg**. Output directory path.
- **-i [ --input-files ] arg**. List of input file(s)/folder(s).

