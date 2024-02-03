# WolfEdit (work in progress)

WolfEdit is a free and open source text editor built on the Qt C++ framework. WolfEdit is designed to be used by programmers and people that frequently edit text files. WolfEdit is focused on simplicity and performance so that programmers can customize their editor by modifying the source directly and have a snappy user experience.

WolfEdit is a work in progress and there are frequent changes and bugs.

## Building

```
git clone https://github.com/argosopentech/WolfEdit.git
cd WolfEdit
./scripts/build.sh
./build/WolfEdit

```

##### Install Qt
```
sudo apt install qt5-default

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

Vim emulation in QTextEdit, QPlainTextEdit and similar Qt widgets


