# ESP32-S3 Bare-Metal Graphics Engine: Font Dictionary (`font_module.h`)

## Overview
This header file acts as the Read-Only Memory (ROM) for the custom graphics engine. It contains `ascii_8x16_font`, a 2D array that mathematically defines the visual representation of 95 standard ASCII characters (from Space ` ` to Tilde `~`). 

## Memory Architecture & Efficiency
By declaring this array as `const`, the C compiler is instructed to place this massive dictionary into the ESP32-S3's read-only flash memory (`.rodata` section) rather than consuming precious internal SRAM. 

The array dimensions are `[95][16]`:
* **95 Rows:** Represents the total number of printable ASCII characters supported.
* **16 Columns (Bytes):** Represents the 16 horizontal rows of pixels that make up a single, highly readable character.

## How It Works: The Bitmap Matrix
This font does not use complex vector graphics. It uses pure binary bitmapping within an 8x16 pixel grid. 
* **The Index Offset:** In standard ASCII, the first printable character (Space) is decimal `32`. To save memory, this array starts the Space character at index `0`. Therefore, to look up any character, the graphics engine simply subtracts 32 from its ASCII value: `ascii_8x16_font[ *(user_text) - 32 ]`.
* **The Binary Translation:** Each hexadecimal value in the array translates to an 8-bit binary number. A `1` represents a drawn pixel, and a `0` represents an empty space.