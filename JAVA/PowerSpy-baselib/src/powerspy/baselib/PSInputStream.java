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

import com.fazecast.jSerialComm.SerialPort;
import java.io.IOException;
import java.io.InputStream;

/**
 *
 * @author redxef
 */
public class PSInputStream implements IODefs {

    private final SerialPort source;
    byte[] buffer;
    boolean reading;
    char position;

    public PSInputStream(SerialPort port) {
        source = port;
        buffer = new byte[BUFFER_SIZE];
        reading = false;
        position = 0;
    }

    public int available() throws IOException {
        return source.bytesAvailable();
    }

    public byte read() throws IOException {
        byte[] b = new byte[1];
        source.readBytes(b, 1);
        return b[0];
    }

    public boolean packageStarted() {
        return buffer[0] == START_OF_TEXT;
    }

    public boolean packageFinished() {
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

    public boolean readPackage() throws IOException {
        byte read_;
        while (true) {
            read_ = read();
            if (reading) {
                buffer[position++] = read_;
                if (read_ == END_OF_TEXT) {
                    reading = false;
                    return true;
                }
            } else {
                if (read_ == START_OF_TEXT) {
                    buffer[position++] = read_;
                }
            }
        }
    }

    public byte getType() throws PackageException {
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

    private void seekFront() throws PackageException {
        if (!packageFinished()) {
            throw new PackageException("The package is not complete.");
        }
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == START_OF_TEXT) {
                position = (char) (i + 2);
            }
        }
    }

    private void clear() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = 0;
        }
        position = 0;
        reading = false;
    }

    private byte getChar() {
        return buffer[position++];
    }

    private char availableChar() {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == END_OF_TEXT) {
                return (char) (i - position - 1);
            }
        }
        return (char) -1;
    }

    public boolean isInt8() throws PackageException {
        return getType() == INT8;
    }

    public boolean isInt16() throws PackageException {
        return getType() == INT16;
    }

    public boolean isInt24() throws PackageException {
        return getType() == INT24;
    }

    public boolean isInt32() throws PackageException {
        return getType() == INT32;
    }

    public boolean isFloat() throws PackageException {
        return getType() == FLOAT;
    }

    public boolean isString() throws PackageException {
        return getType() == STRING;
    }

    public int readInt8() throws PackageException {
        int res = 0;
        if (!isInt8()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        res |= getChar();
        clear();

        return res;
    }

    public int readInt16() throws PackageException {
        int res = 0;
        if (!isInt8()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        res |= (getChar() & 0xff) << 8;
        res |= getChar() & 0xff;
        clear();

        return res;
    }

    public int readInt24() throws PackageException {
        int res = 0;
        if (!isInt24()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        res |= (getChar() & 0xff) << 16;
        res |= (getChar() & 0xff) << 8;
        res |= getChar() & 0xff;
        clear();

        return res;
    }

    public int readInt32() throws PackageException {
        int res = 0;
        if (!isInt32()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        res |= (getChar() & 0xff) << 24;
        res |= (getChar() & 0xff) << 16;
        res |= (getChar() & 0xff) << 8;
        res |= getChar() & 0xff;
        clear();

        return res;
    }

    public float readFloat() throws PackageException {
        int res = 0;
        if (!isFloat()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        res |= (getChar() & 0xff) << 16;
        res |= (getChar() & 0xff) << 8;
        res |= getChar() & 0xff;
        clear();

        return Float.intBitsToFloat(res);
    }

    public String readString() throws PackageException {
        StringBuilder sb = new StringBuilder();
        if (!isFloat()) {
            throw new PackageException("Wrong package type.");
        }

        seekFront();
        while (availableChar() > 0) {
            sb.append(getChar());
        }
        clear();

        return sb.toString();
    }

    public Object readObj() throws PackageException {
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
