README.txt 28 November 2014

IMPORTANT! READ ME FIRST!
=========================

The programs in this package are NOT a KSP mod. They are stand alone utilities.
Do not unzip the package into your KSP game folder. Read all of the README.txt
file FIRST!


Contents of this package
========================

file: README.txt (you're reading it)
file: gpl3.txt (Gnu Public License Version 3)

directory: linux32 (pre-compiled executables for 32 bit Linux)
directory: linux64 (pre-compiled executables for 64 bit Linux)
directory: lodepng (the "lodepng" utilities, (c) 2014 Lode Vandevenne)
directory: source (the C source code for the 4 utilities)
directory: windows (pre-compiled executables for Windows)


What do these utilities do?
===========================

The KSP (Kerbal Space Program) game uses graphic images as textures for rendering
the 3D parts on the screen. Many of these textures are in a propriatary format
with an "MBM" extension.

Many KSP users would like to edit these textures to their own liking (for example,
add decals to rocket bodies, change the color of some parts, etc...)

Unfortunately, the MBM image format cannot be easily opened using common graphic
editors such as Photoshop, PaintShop Pro or Gimp.

These utilities convert the MBM file format to either PNG (Portable Network Graphics)
or TGA (TrueVision TAGRA) format. Both of these formats are widely supported by
graphic editors.

The other two utilities convert a PNG or a TGA image file back into the propriatary
MBM format for KSP to use.

Additionally, game modders may find it easier to create their part textures in the
PNG format using a standard graphics editor, then convert it to MBM for use by KSP.


How to use this stuff
=====================

For Windows users, unzip the package into a temporary directory, then drag the four
files found in the "windows" directory into the "Windows\System32" directory.

Alternately, you can place them in a directory that contains KSP textures that you
wish to convert.

These four files are all that you need. If you're not interested in the C source code,
you can safely delete everything else (that is, delete everything except the four EXE
files in the "windows" directory).

Now, to convert one or more texture files, simply select the file(s), then drag them
into the appropriate utility.

For example, if you wish to make PNG versions of "model000.mbm" and "model001.mbm",
simply select both files and drag them into "mbm2png.exe".

The utility will generate two new files called "model000.png" and "model001.png" in
the same directory. The original MBM files will not be altered or deleted.

Now imagine you made some edits to these two PNG files and you want to convert them
back into MBM files.

First (this is optional), create a folder called "backup" (or whatever you like) and
drag your original MBM files into "backup" (you are saving the originals just in case).

Next, select the two new files "model000.png" and "model001.png" and drag them into
"png2mbm.exe". The utility will convert the PNG files into MBM files.

If you did NOT move the originals out of the way, the utility will OVERWRITE the original
MBM files with the new versions (converted from PNG). Therefore, it's a smart idea to save
the originals just in case you want to revert to the original file or if you want to start
fresh and create/edit new PNG files.

If you wish to work with the Targa (TGA) format instead, simply use the two utilities
"tga2mbm.exe" and "mbm2tga.exe". Notice that the filenames explain what each utility does:

"mbm2tga.exe" -> converts MBM format to TGA format
"mbm2png.exe" -> converts MBM format to PNG format
"tga2mbm.exe" -> converts TGA format to MBM format
"png2mbm.exe" -> converts PNG format to MBM format


Information for Linux users
===========================

Basically the same as above. Either use the utilities in "drag-n-drop" mode as described
above, or else use the command line (a bash shell). You can pipe multiple files into the
converter utility and it will process one after the other. For example:

ls *.mbm | mbm2png

Will read every filename with the ".mbm" extension and send it through the pipe into
mbm2png. Then, the mbm2png utility will open each file and create the PNG version of
the MBM file in the same directory.


Which version to use in my Linux?
=================================

If you have 64 bit Linux (any distro), use the utilities in the "linux64" directory. If
you are using 32 bit Linux, use the utilities in the "linux32" directory. If you're not
sure, try a 64 bit utility. If you get an error message, try the 32 bit version. If
neither of them work, contact me and let me know what version and distro of Linux you
are using. This should never happen, but who knows?  :)


Lastly......
============

Any problems or questions? PM me in the KSP Forum: Use this URL:

http://forum.kerbalspaceprogram.com/private.php?do=newpm&u=83088


-- end of README.txt --
