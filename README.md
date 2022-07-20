# ubackup
## Introduction to ubackup

​Ubackup is an efficient and fast backup and restore software.  that can serve a variety of scenarios (system restore, data restore). if you are working in system restore mode and you have backed up your system before, the restore point will be displayed in the grub menu when the system restart, select it as the boot item, restore the system.

## Purpose

Ubackup is a system operation and maintenance management software for user operating system copy backup or system backup, which can simplify operation and maintenance management and improve system.

## Install

There are various ways of installing ubackup.

### Building from source

To build ubackup from source code, you need:

* boost  [version 1.73 or later](https://www.boost.org/).
* libtool  [version 2.4.6 or later](https://www.gnu.org/software/libtool/).
* gcc-c++ [version 7.3.0 or later](https://gcc.gnu.org/)

You can build a ubackup locally with the following commands:

    $ aclocal
    $ libtoolize --force --automake --copy
    $ autoheader
    $ automake --add-missing --copy
    $ autoconf
    $ . ./configure
    $ make
    $ make install

## Instructions

​	The functions are triggered by the webui interface.

## Copyright

​	Copyright (C) 2022 Uniontech Software Technology Co., Ltd.

## License

​	Ubackup under the terms of the GNU General Public License version 2.
