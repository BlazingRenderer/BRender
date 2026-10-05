## br_actor
The basic unit of scene construction. The br_actor object is designed to facilitate hierarchical relationships between elements of a scene, particularly in terms of position and orientation.
See The Actor, page 12, in the structured description for further details.

## br_allocator
This structure represents the definition of a memory allocation system or memory handler. All BRender's memory allocation is provided by just three functions, which can be specified by the programmer. This is essential in cases where the standard C library functions, which BRender's handler uses by default, are not available. Sometimes, more sophisticated behaviour is desired, or diagnostic features are needed.

## br_bounds
A data structure describing an axis-aligned bounding box for a model or hierarchy of actors.

## br_camera
BRender's camera data structure. See Camera Actors.

## br_diaghandler
This structure represents the definition of a diagnostic handler. All BRender's diagnostics are handled by just two functions, which can be specified by the programmer. This is essential in cases where stdout and stderr (as used by the standard C library functions) are not available (or not suitable).

## br_euler
Euler angles can be used to represent the orientation of an object. Three separate rotations are applied in turn, in a specific order. Euler angles can be either static or relative. With relative Euler angles, the rotations are performed around axes relative to the rotations already performed. With static Euler angles, the rotations are performed around static axes. (Footnote: Pronounced 'oiler' as in boiler.)

## br_face
The face data structure, describing a single triangular face.

## br_filesystem
BRender routes all filing system calls through an instance of this structure. The syntax of each call-back function corresponds exactly with the standard C library calls. This allows the user to tailor BRender's file system characteristics to suit any platform.
See BrFilesystemSet() for details of how to specify a particular filing system handler.

## br_font
The font data structure, describing a BRender font. Up to 223 bit mapped characters are supported. The font does not necessarily need to accord with ASCII character codes, however the non-printing ASCII codes (0-31 & 127) of the 256 codes possible are reserved for such a purpose.
Three ASCII fonts (covering codes 32-126) are predefined by BRender:
- BrFontFixed3x5 — 3 pixels wide by 5 high, fixed pitch font
- BrFontProp4x6 — 4 pixels wide by 6 high, pixel proportional font
- BrFontProp7x9 — 7 pixels wide by 9 high, pixel proportional font
