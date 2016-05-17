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
import java.io.*;

/**
 *
 * @author redxef
 */
public class Test {

        static SerialPort p;

        static void log(PrintStream ps, char t, Object o)
        {
                ps.append("[");
                ps.append(Long.toString(System.currentTimeMillis()));
                ps.append("] ");
                ps.print(Character.toString(t));
                ps.print(", ");
                ps.println(o);
                ps.flush();
        }

        public static void main(String[] args) throws IOException, PackageException, InterruptedException
        {
                char type;
                Object o;
                SerialPort[] ports = SerialPort.getCommPorts();
                byte[] buff = new byte[]{'g'};
                p = null;
                PrintStream ps = new PrintStream(new File("/Users/redxef/test.txt"));

                System.out.println("searching");
                for (SerialPort p_ : ports) {
                        if (p_.getDescriptivePortName().contains("PowerSpy")) {
                                p = p_;
                                break;
                        }
                }

                if (p == null) {
                        System.out.println("no ps");
                        return;
                }
                System.out.println("connecting to " + p.getDescriptivePortName());
                p.openPort();
                if (p.isOpen()) {
                        System.out.println("connected");
                } else {
                        System.out.println("failed");
                        System.exit(1);
                }
                PSInputStream is = new PSInputStream(p.getInputStream());

                Thread.sleep(2_000);

                System.out.println("reading...");
                p.writeBytes(buff, 1);

                while (true) {
                        try {
                                is.readPackage();
                        } catch (ArrayIndexOutOfBoundsException ex) {
                                ex.printStackTrace(System.err);
                                is.clear();
                        }
                        if (is.packageFinished()) {
                                type = (char) is.getType();
                                o = is.readObj();

                                if (o != null) {
                                        log(ps, type, o);
                                        log(System.out, type, o);
                                }
                                is.clear();
                        }
                        Thread.sleep(0, 500);
                }
        }
}
