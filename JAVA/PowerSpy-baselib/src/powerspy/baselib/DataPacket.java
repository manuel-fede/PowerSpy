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

import java.nio.ByteBuffer;
import java.util.Arrays;
import static powerspy.baselib.IODefs.*;

/**
 *
 * @author redxef
 */
public class DataPacket {

        private static final byte INT8_BYTES = 1;
        private static final byte INT16_BYTES = 2;
        private static final byte INT24_BYTES = 3;
        private static final byte INT32_BYTES = 4;
        private static final byte FLOAT_BYTES = 3;

        private char type;
        private int length;
        private final byte[] data;
        private boolean string_fin;

        private int position;

        /**
         * Creates a new, empty DataPacket with BUFFER_SIZE data buffer.
         */
        public DataPacket()
        {
                type = NONE;
                length = 0;
                data = new byte[BUFFER_SIZE];
                position = 0;
                string_fin = false;
        }

        /**
         * Sets the type of this DataPacket. Every value is valid, but only the
         * ones defined in IODefs are of real use.
         *
         * @param c the type
         */
        public void setType(char c)
        {
                type = c;
        }

        /**
         * Adds a Byte of data to the end of the data[] and increments the write
         * counter.
         *
         * @param b the data to write
         */
        public void addByte(byte b)
        {
                if (!isFinished()) {
                        data[position++] = b;
                        length++;
                }
        }

        /**
         * Sets the finished String flag.
         */
        public void setFinishedString()
        {
                string_fin = true;
        }

        /**
         * Returns the type of this DataPacket.
         *
         * @return the type
         */
        public char getType()
        {
                return type;
        }

        /**
         * Returns the length of this data, this is NOT the length of the
         * byte[].
         *
         * @return the number of bytes in this DataPacket
         */
        public int getLength()
        {
                return length;
        }

        /**
         * Seeks the fornt of this DataPacket.
         */
        public void seekFront()
        {
                position = 0;
        }

        /**
         * Reads the next Byte of data from the array and returns it.
         *
         * @return the read data
         *
         * @throws PackageException when the end of the buffer is accessed
         */
        public byte readNext() throws PackageException
        {
                if (position == data.length) {
                        throw new PackageException("reached end of package");
                }

                return data[position++];
        }

        /**
         * Returns the next item in the buffer array. This is the same as
         * {@code readNext()}.
         *
         * @return the read data
         *
         * @throws PackageException when the end of the buffer is accessed
         */
        public byte getChar() throws PackageException
        {
                return readNext();
        }

        /**
         * Returns the number of available bytes.
         *
         * @return the number of bytes
         */
        public int availableChar()
        {
                return length - position;
        }

        /**
         * Returns true if this DataPacket has no byte stored in it, otherwise
         * false.
         *
         * @return if this DataPacket is empty
         */
        public boolean isEmpty()
        {
                for (int i = 0; i < data.length; i++) {
                        if (i != 0)
                                return false;
                }
                return true;
        }

        private boolean isFinishedNr()
        {
                switch (type) {
                        case INT8:
                        case UINT8:
                                return length == INT8_BYTES;
                        case INT16:
                        case UINT16:
                                return length == INT16_BYTES;
                        case INT24:
                        case UINT24:
                                return length == INT24_BYTES;
                        case INT32:
                        case UINT32:
                                return length == INT32_BYTES;
                        case FLOAT:
                                return length == FLOAT_BYTES;
                        default:
                                return false;
                }
        }

        private boolean isFinishedString()
        {
                return string_fin;
        }

        /**
         * Checks if this DataPacket is finished, effectively preventing any
         * further modification.
         *
         * @return true if the Packet is finished, otherwise false
         */
        public boolean isFinished()
        {
                if (type >= INT8 && type <= UINT32 || type == FLOAT) {
                        return isFinishedNr();
                } else if (type == STRING) {
                        return isFinishedString();
                } else {
                        return false;
                }
        }

        /**
         * Checks if this DataPacket is of type INT8.
         *
         * @return true if the type matches
         */
        public boolean isInt8()
        {
                return getType() == INT8;
        }

        /**
         * Checks if this DataPacket is of type INT16.
         *
         * @return true if the type matches
         */
        public boolean isInt16()
        {
                return getType() == INT16;
        }

        /**
         * Checks if this DataPacket is of type INT24.
         *
         * @return true if the type matches
         */
        public boolean isInt24()
        {
                return getType() == INT24;
        }

        /**
         * Checks if this DataPacket is of type INT32.
         *
         * @return true if the type matches
         */
        public boolean isInt32()
        {
                return getType() == INT32;
        }

        /**
         * Checks if this DataPacket is of type UINT8.
         *
         * @return true if the type matches
         */
        public boolean isUInt8()
        {
                return getType() == UINT8;
        }

        /**
         * Checks if this DataPacket is of type UINT16.
         *
         * @return true if the type matches
         */
        public boolean isUInt16()
        {
                return getType() == UINT16;
        }

        /**
         * Checks if this DataPacket is of type UINT24.
         *
         * @return true if the type matches
         */
        public boolean isUInt24()
        {
                return getType() == UINT24;
        }

        /**
         * Checks if this DataPacket is of type UINT32.
         *
         * @return true if the type matches
         */
        public boolean isUInt32()
        {
                return getType() == UINT32;
        }

        /**
         * Checks if this DataPacket is of type Float.
         *
         * @return true if the type matches
         */
        public boolean isFloat()
        {
                return getType() == FLOAT;
        }

        /**
         * Checks if this DataPacket is of type String.
         *
         * @return true if the type matches
         */
        public boolean isString()
        {
                return getType() == STRING;
        }

        /**
         * Reads an unsigned 8 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readUInt8() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isUInt8()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();

                b[0] = 0;
                b[1] = 0;
                b[2] = 0;
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                return res;
        }

        /**
         * Reads an unsigned 16 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readUInt16() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isUInt16()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();
                b[0] = 0;
                b[1] = 0;
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                return res;
        }

        /**
         * Reads an unsigned 24 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readUInt24() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isUInt24()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();
                b[0] = 0;
                b[1] = getChar();
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                return res;
        }

        /**
         * Reads an unsigned 32 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readUInt32() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isUInt32()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();
                b[0] = getChar();
                b[1] = getChar();
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                return res;
        }

        /**
         * Reads an 8 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readInt8() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt8()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();

                b[0] = 0;
                b[1] = 0;
                b[2] = 0;
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                if ((res & 0x80) > 0) {
                        res = ~res + 1; //invert
                        res &= 0xff;
                        res = ~res + 1;
                }
                return res;
        }

        /**
         * Reads an 16 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readInt16() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt16()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();

                b[0] = 0;
                b[1] = 0;
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                if ((res & 0x8000) > 0) {
                        res = ~res + 1; //invert
                        res &= 0xffff;
                        res = ~res + 1;
                }
                return res;
        }

        /**
         * Reads an 24 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readInt24() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt24()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();

                b[0] = 0;
                b[1] = getChar();
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                if ((res & 0x800000) > 0) {
                        res = ~res + 1; //invert
                        res &= 0xffffff;
                        res = ~res + 1;
                }
                return res;
        }

        /**
         * Reads an 32 bit Integer from the data[].
         *
         * @return the Integer
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public int readInt32() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt32()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();

                b[0] = getChar();
                b[1] = getChar();
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                if ((res & 0x80000000) > 0) {
                        res = ~res + 1; //invert
                        res &= 0xffffffff;
                        res = ~res + 1;
                }
                return res;
        }

        /**
         * Reads an 24 bit Float from the data[].
         *
         * @return the Float
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public float readFloat() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isFloat()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();
                b[0] = 0;
                b[1] = getChar();
                b[2] = getChar();
                b[3] = getChar();

                res = ByteBuffer.wrap(b).getInt();

                return Float.intBitsToFloat(res << 8);
        }

        /**
         * Reads an String from the data[].
         *
         * @return the String
         *
         * @throws PackageException if the type of the packet doesn't match with
         *                          the type to read.
         */
        public String readString() throws PackageException
        {
                StringBuilder sb = new StringBuilder();
                if (!isString()) {
                        throw new PackageException("Wrong package type.");
                }

                seekFront();
                while (availableChar() > 0) {
                        sb.append((char) getChar());
                }

                return sb.toString();
        }

        /**
         * Reads any type of data from the data[]. This is a convenience method
         * intended for debugging.
         *
         * @return the result as Object or null if the Package is not finished
         *
         * @throws PackageException this Exception is never thrown
         */
        public Object readObj() throws PackageException
        {
                switch (getType()) {
                        case INT8:
                                return readInt8();
                        case INT16:
                                return readInt16();
                        case INT24:
                                return readInt24();
                        case INT32:
                                return readInt32();
                        case UINT8:
                                return readUInt8();
                        case UINT16:
                                return readUInt16();
                        case UINT24:
                                return readUInt24();
                        case UINT32:
                                return readUInt32();
                        case FLOAT:
                                return readFloat();
                        case STRING:
                                return readString();
                        default:
                                return null;
                }
        }

        @Override
        public String toString()
        {
                StringBuilder sb = new StringBuilder();

                sb.append("type: ");
                sb.append(type);
                sb.append("; data: ");
                sb.append(Arrays.toString(data));
                sb.append(";");

                return sb.toString();
        }
}
