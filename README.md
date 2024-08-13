# WolfEdit (beta)

WolfEdit is a free and open source text editor with a Vim-like interface built on the Qt C++ framework. WolfEdit is designed to be used by programmers and people that frequently edit text files. WolfEdit is focused on simplicity and performance so that programmers can customize their editor by modifying the source directly and have a snappy user experience.

WolfEdit is a work in progress and there are frequent changes and bugs.

## Install
Download the Debian Linux Build from the GitHub releases page

https://github.com/argosopentech/WolfEdit/releases

## Building

##### Install Qt
```
sudo apt install qt5-default

```

```
git clone https://github.com/argosopentech/WolfEdit.git
cd WolfEdit
./scripts/build.sh
./build/WolfEdit

```

## Format
```
./scripts/format.sh
```

## Dependencies
### Qt
https://www.qt.io/

### FakeVim
https://github.com/hluk/FakeVim

FakeVim is included in the source of this repository:
https://github.com/argosopentech/WolfEdit/tree/main/third_party/FakeVim

Vim emulation in QTextEdit, QPlainTextEdit and similar Qt widgets


