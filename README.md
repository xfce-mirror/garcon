## What is it?

This is garcon, a [freedesktop.org compliant menu](https://specifications.freedesktop.org/menu-spec/latest/) implementation based
on GLib and GIO. It was started as a complete rewrite of the former
Xfce menu library called libxfce4menu, which, in contrast to garcon,
was lacking menu merging features essential for loading menus modified
with menu editors.


## Current state

Garcon covers almost every part of the menu specification except for
legacy menus and a few XML attributes. In contrast to
libxfce4menu, it can also load menus modified with menu editors such
as Alacarte as menu merging is now supported. The only crucial
feature still missing is monitoring menus and menu items for changes.
This is something that will be worked on for the next release.

The garcon API will most likely not be frozen until its 1.0.0 release!


## Installation

The file [`INSTALL`](INSTALL) contains generic installation instructions.


## Debugging Support

garcon currently supports three different levels of debugging support,
which can be setup using the configure flag `--enable-debug` (check the output
of `configure --help`):

| Argument  | Description |
| -------   | ----------- |
|  `yes`    | This is the default for Git snapshot builds. It adds all kinds of checks to the code, and is therefore likely to run slower. Use this for development of garcon and locating bugs in garcon. |
| `minimum` | This is the default for release builds. **This is the recommended behaviour.** |
| `no`      | Disables all sanity checks. Don't use this unless you know exactly what you do. |

## How to report bugs?

Bugs should be reported to the [Xfce bug tracking system](http://bugzilla.xfce.org)
against the product Garcon. You will need to create an
account for yourself.

Please read the [`HACKING`](HACKING) file for information on where to send changes or
bugfixes for this package.
