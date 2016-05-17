/*
 * Copyright (C) 2016 redxef.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
package powerspy.baselib;

/**

 @author redxef
 */
public interface IODefs {

        public static final char NONE = '0';
        public static final char STRING = '1';
        public static final char INT8 = '2';
        public static final char INT16 = '3';
        public static final char INT24 = '4';
        public static final char INT32 = '5';
        public static final char UINT8 = '6';
        public static final char UINT16 = '7';
        public static final char UINT24 = '8';
        public static final char UINT32 = '9';
        public static final char FLOAT = 'F';

        public static final char K_CURRENT = 'c';
        public static final char K_VOLTAGE = 'v';
        public static final char K_ANGLE = 'a';
        public static final char K_APPARENTEPOWER = 'A';
        public static final char K_REALPOWER = 'r';
        public static final char K_REACTIVEPOWER = 'R';

        public static final char START_OF_TEXT = 2;
        public static final char END_OF_TEXT = 3;
        public static final char BUFFER_SIZE = 1024;
}
