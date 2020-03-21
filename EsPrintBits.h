/*Copyright (c) 2020 Boris Chirkov

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// File created 12.03.2020

#pragma once

void print_byte_as_bits(char val)
{
    for (int i = 7; 0 <= i; i--)
    {
	printf("%c", (val & (1 << i)) ? '1' : '0');
    }
}

void print_bits(unsigned char* bytes, size_t num_bytes)
{
    for (size_t i = 0; i < num_bytes; i++)
    {
	print_byte_as_bits(bytes[i]);
    }
    printf(" ");
}

#define SHOW(T,V) {T x = V; print_bits((unsigned char*) &x, sizeof(x));}
