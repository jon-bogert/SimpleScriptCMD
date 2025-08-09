# Simple Script CMD

## Description

"Simple Script CMD" is a set of Windows terminal tools for writing film scripts. This is newer than the GUI version of [SimpleScript](https://github.com/jon-bogert/SimpleScript) and is the intentded version. The following tools are provided:

- `ss-view`: Allows for viewing the script with professional formatting. Double click on text to open it's corisponding file in it's default application (Usually Notepad).
- `ss-format`: Formats all project files to follow a stricter and consistant formatting convention.
- `ss-export`: Exports the project as a DOCX file (Optimized for [OnlyOffice](https://www.onlyoffice.com/), there are page formatting issues when opening files in Microsoft Word).

It is recommended to use Notepad to edit your text files on Windows 11 as it includes spell check tools.

## Requirements

- "Courier Prime" Font is required to be installed on your system. Download from [Google Fonts](https://fonts.google.com/specimen/Courier+Prime).
- [SFML](https://www.sfml-dev.org/) is required for `ss-view`. MSVC static libraries included in this repository.
- [minidocx](https://github.com/totravel/minidocx/tree/master) ("master" branch) is required for `ss-export`. MSVC static library included in this repository.

## Project Layout

### Root Folder
The root of your project folder can be located anywhere. It is recommended that you do not place any extra files within this directory. All Simple Script tools should be run from a terminal session in this directory.

The root folder can contain file called `_char.txt`. This contains information on important characters within your scene as well as color highlights for their names within `ss-view`. It is not required to list all characters in this file, but it can be helpful to be able to see your main characters quickly if they have unique colors. Use the following format within this file:
```
[NAME]{ R8, G8, B8, A8 }
Description contents
```
Note that names should be all-caps and color values are 8-bit RGBA (0-255 per channel).

### Sequence Folders
Within the root folder, you can make sequcene folders. Think of these like chapters in your script. `ss-view` will show one sequence at a time. The order of these sequences will be shown alphabetically, it is recommended you use the following format: `##_Name`. For example: `00_Prologue`.

### Script Files
Within sequence folders is where you create your script files. these are `.txt` files that will be read in alphabetical order and are named after the slug line. Files will be renamed by `ss-format` so you do not need to accurately name them yourself as long as they are properly paced in alphabetical order. The standard format is `###_SLUG_LINE_CONTENTS.txt`. If you need to place a file between `001_` and `002_` we recommend `001a_` and they will be renamed with a new, whole number by `ss-format`.

## Writing in Script files.

The following types of blocks are availabl and have the following syntax withing the script file:

|Block Type   |Syntax                          |
|-------------|--------------------------------|
|Slug Line    |`#` at beginning of line        |
|Action       |`*` at beginning of line        |
|Character    |Contents surrounded by `[` & `]`|
|Parenthetical|Contents surrounded by `(` & `)`|
|Dialogue     |Content without extra symbols   |
|Comment      |`//` at beginning of line       |

### Requriements
Each script file must start with a "Slug Line" (`#`) followed by an "Action" (`*`). You will get errors if you try to run the tools without these. Each script file should only contain one "Slug Line". Create a new file when you need a new "Slug Line".

"Character" blocks are not case-sensitive, but will be reformatted to all-caps by `ss-format`.

"Parentheticals" and "Dialogue" will be assigned to the last listed "Character". You do not need to relist the character after an "Action" if the character doesn't change. A "Character" must be listed before any "Parenthetical" or "Dialogue" can be used.

"Comments" are not officially part of your script and will not be exported by `ss-export`.

### Example
```
# INT. SIMON'S HOUSE - NIGHT

* Simon enters his house though th front door. Phil is sitting in the main room waiting for him.

[PHIL]
(casually)
Suprise.

* Simon jumps and lets out a small scream

(playfully)
Hah! Got you good.

[SIMON]
Geez Phil.
(beat)
How'd you even get in here?
```

## Repository Information
Within each tool's folder, run `generate-vs2022.bat` to generate a Visual Studio 2022 solution.
