// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

// Thanks to Thomas Nussbaumer with FAT for this routine!
// (modified for use with X3D)

/******************************************************************************
*
* project name:  FAT-Engine
* file name:     fastsqrt.c
* initial date:  16/04/2002
* author:        thomas.nussbaumer@gmx.net
* description:   fast square root algorithm using a 256 bytes lookup table
*
* (algorithm posted at comp.graphics.algorithms)
*
* $Id: fastsqrt.c,v 1.1 2002/04/18 16:09:53 tnussb Exp $
*
******************************************************************************/

/*
// Integer Square Root function
// Contributors include Arne Steinarson for the basic approximation idea, Dann
// Corbit and Mathew Hendry for the first cut at the algorithm, Lawrence Kirby
// for the rearrangement, improvments and range optimization and Paul Hsieh
// for the round-then-adjust idea.
*/
unsigned int x_sqrt(unsigned int x) {
    static const unsigned char sqq_table[] = {
        0, 16, 22, 27, 32, 35, 39, 42, 45, 48, 50, 53, 55, 57,
        59, 61, 64, 65, 67, 69, 71, 73, 75, 76, 78, 80, 81, 83,
        84, 86, 87, 89, 90, 91, 93, 94, 96, 97, 98, 99, 101, 102,
        103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 117, 118,
        119, 120, 121, 122, 123, 124, 125, 126, 128, 128, 129, 130, 131, 132,
        133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145,
        146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155, 156, 157,
        158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
        169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178,
        179, 180, 181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188,
        189, 189, 190, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197, 197,
        198, 199, 199, 200, 201, 201, 202, 203, 203, 204, 204, 205, 206, 206,
        207, 208, 208, 209, 209, 210, 211, 211, 212, 212, 213, 214, 214, 215,
        215, 216, 217, 217, 218, 218, 219, 219, 220, 221, 221, 222, 222, 223,
        224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230, 230, 231,
        231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
        239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246,
        246, 247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253,
        253, 254, 254, 255
    };

    unsigned int xn;

    if(x >= 0x10000) {
        if(x >= 0x1000000) {
            if(x >= 0x10000000) {
                if(x >= 0x40000000) {
                    if(x >= 65535UL * 65535UL) return 65535;
                    xn = sqq_table[x >> 24] << 8;
                } else {
                    xn = sqq_table[x >> 22] << 7;
                }
            } else if(x >= 0x4000000) xn = sqq_table[x >> 20] << 6;
            else                     xn = sqq_table[x >> 18] << 5;
        } else {
            if(x >= 0x100000) {
                if(x >= 0x400000) xn = sqq_table[x >> 16] << 4;
                else               xn = sqq_table[x >> 14] << 3;
            } else {
                if(x >= 0x40000) xn = sqq_table[x >> 12] << 2;
                else              xn = sqq_table[x >> 10] << 1;
            }
            goto nr1;
        }
    } else if(x >= 0x100) {
        if(x >= 0x1000) {
            if(x >= 0x4000) xn = (sqq_table[x >> 8] >> 0) + 1;
            else             xn = (sqq_table[x >> 6] >> 1) + 1;
        } else if(x >= 0x400) xn = (sqq_table[x >> 4] >> 2) + 1;
        else                 xn = (sqq_table[x >> 2] >> 3) + 1;
        goto adj;
    } else {
        return sqq_table[x] >> 4;
    }

    xn = (xn + 1 + x / xn) >> 1;
nr1:
    xn = (xn + 1 + x / xn) >> 1;
adj:

    // we don't care about being one on the wrong side - therefore the
    // next line is commented out.

    // if (xn * xn > x) xn--;

    return xn;
}


//#############################################################################
// Revision History
//#############################################################################
//
// $Log: fastsqrt.c,v $
// Revision 1.1  2002/04/18 16:09:53  tnussb
// initial version
//
//