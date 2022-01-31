# PDF Stream Extractor CLI

## Description
A command-line tool which extracts and decodes every single FlateDecode stream
from within a PDF and saves it to a file. Useful for when the usual tools
aren't able to extract what you want.

This tool attempts to figure out what kind of data was in the FlateDecode
stream by matching file signatures. If it's able to determine file type, the
data will be saved with the appropriate file filext.

The relational database of file signatures to file filexts is manually
programmed into this application. One day, I may restructure that to be a
configuration file.

## Library requirements
*(make sure you have these built for and installed on your toolchain)*
* zlib
* regex

>*If you're using Arch and compiling for windows using mingw-w64, you may find
the following packages useful:*
>* *mingw-w64-zlib
<sup>[AUR](https://aur.archlinux.org/packages/mingw-w64-zlib/)</sup>*
>* *mingw-w64-libgnurx
<sup>[AUR](https://aur.archlinux.org/packages/mingw-w64-libgnurx/)</sup>*

## Additional Information
This appliction is set up to be built using autotools. You may use
`autoreconf --install` to produce `./configure`. It's a good idea to use
`./configure` in a `build/` subdirectory or outside of the build tree.

Feel free to propose changes to the functionality or documentation of this
application.
