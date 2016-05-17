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

import java.io.*;
import java.nio.ByteBuffer;

/**

 @author redxef
 */
public class PSInputStream extends InputStream implements IODefs {

        private final InputStream is;
        private final byte[] buffer;
        private boolean reading;
        private char position;

        public PSInputStream(InputStream is)
        {
                this.is = is;
                buffer = new byte[BUFFER_SIZE];
                reading = false;
                position = 0;
        }

        @Override
        public int available() throws IOException
        {
                //return source.bytesAvailable();
                return is.available();
        }

        @Override
        public int read() throws IOException
        {
                /*
                 byte[] b = new byte[1]; int read = source.readBytes(b, 1); if
                 (read > 0) { return b[0] & 0xff; } else { return -1; }
                 */
                return is.read();
        }

        @Override
        public synchronized void reset() throws IOException
        {
                is.reset();
        }

        @Override
        public synchronized void mark(int readlimit)
        {
                is.mark(readlimit);
        }

        private byte read_() throws IOException
        {
                return (byte) (read() & 0xff);
        }

        public boolean packageStarted()
        {
                return buffer[0] == START_OF_TEXT;
        }

        public boolean packageFinished()
        {
                if (packageStarted()) {
                        for (int i = 1; i < BUFFER_SIZE; i++) {
                                if (buffer[i] == END_OF_TEXT) {
                                        return true;
                                }
                        }
                        return false;
                } else {
                        return false;
                }
        }

        public boolean readPackage() throws IOException
        {
                byte read_;
                while (available() > 0) {
                        read_ = read_();
                        if (reading) {
                                buffer[position++] = read_;
                                if (read_ == END_OF_TEXT) {
                                        reading = false;
                                        return true;
                                }
                        } else {
                                if (read_ == START_OF_TEXT) {
                                        buffer[position++] = read_;
                                        reading = true;
                                }
                        }
                }
                return false;
        }

        public byte getType() throws PackageException
        {
                if (!packageFinished()) {
                        throw new PackageException("The package is not complete.");
                }
                for (int i = 0; i < BUFFER_SIZE; i++) {
                        if (buffer[i] == START_OF_TEXT) {
                                return buffer[i + 1];
                        }
                }
                return NONE;
        }

        private void seekFront() throws PackageException
        {
                if (!packageFinished()) {
                        throw new PackageException("The package is not complete.");
                }

                for (int i = 0; i < BUFFER_SIZE; i++) {
                        if (buffer[i] == START_OF_TEXT) {
                                position = (char) (i + 2);
                                break;
                        }
                }
        }

        public void clear()
        {
                for (int i = 0; i < BUFFER_SIZE; i++) {
                        buffer[i] = 0;
                }
                position = 0;
                reading = false;
        }

        private byte getChar()
        {
                return buffer[position++];
        }

        private char availableChar()
        {
                for (int i = 0; i < BUFFER_SIZE; i++) {
                        if (buffer[i] == END_OF_TEXT) {
                                return (char) (i - position);
                        }
                }
                return (char) -1;
        }

        public boolean isInt8() throws PackageException
        {
                return getType() == INT8;
        }

        public boolean isInt16() throws PackageException
        {
                return getType() == INT16;
        }

        public boolean isInt24() throws PackageException
        {
                return getType() == INT24;
        }

        public boolean isInt32() throws PackageException
        {
                return getType() == INT32;
        }
        
        public boolean isUInt8() throws PackageException
        {
                return getType() == UINT8;
        }

        public boolean isUInt16() throws PackageException
        {
                return getType() == UINT16;
        }

        public boolean isUInt24() throws PackageException
        {
                return getType() == UINT24;
        }

        public boolean isUInt32() throws PackageException
        {
                return getType() == UINT32;
        }

        public boolean isFloat() throws PackageException
        {
                return getType() == FLOAT;
        }

        public boolean isString() throws PackageException
        {
                return getType() == STRING;
        }

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
        
        public int readInt16() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt8()) {
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
        
        public int readInt24() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt8()) {
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
        
        public int readInt32() throws PackageException
        {
                int res;
                byte[] b = new byte[4];
                if (!isInt8()) {
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
                        case FLOAT:
                                return readFloat();
                        case STRING:
                                return readString();
                        default:
                                return null;
                }
        }
}
