/*
 * Copyright (C) 2015 redxef.
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
package com.tumblr.erdaepfelsalat.sc;

import com.fazecast.jSerialComm.SerialPort;
import com.tumblr.erdaepfelsalat.util.terminal.*;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

/**
 *
 * @author redxef
 */
public class Comm {

        public static void main(String[] args)
        {
                /*
                 SerialPort[] com_ports = SerialPort.getCommPorts();
                 SerialPort com_port;
                 byte[] buffer;

                 for (SerialPort com_port_ : com_ports) {
                 System.out.println(com_port_.getDescriptivePortName());
                 }

                 com_port = com_ports[2];
                 com_port.openPort();
                 try {
                 while (true) {
                 while (com_port.bytesAvailable() == 0) {
                 Thread.sleep(20);
                 }

                 buffer = new byte[com_port.bytesAvailable()];
                 int numRead = com_port.readBytes(buffer, buffer.length);
                 System.out.println("read " + numRead + " bytes.");
                 System.out.println(new String(buffer));
                 }
                 } catch (Exception e) {
                 e.printStackTrace(System.err);
                 }
                 com_port.closePort();*/
                Comm c = new Comm();
                c.start();
        }

        private final Terminal t;
        private SerialPort[] ports;
        private int selected_port;
        private ReadThread read_thread;

        private static final Charset UTF8 = StandardCharsets.UTF_8;

        public Comm()
        {
                t = new Terminal(System.in, System.out);
                t.init();
                initTerminal();
                ports = null;
                selected_port = -1;

                read_thread = null;
        }

        public void start()
        {
                t.start();
        }

        private void initTerminal()
        {
                t.registerCommand("search", new Command("search") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                ports = SerialPort.getCommPorts();

                                //print the ports with number
                                for (int i = 0; i < ports.length; i++) {
                                        print(t, "[" + i + "] ");
                                        appendln(t, ports[i].getDescriptivePortName());
                                }

                                return new Response();
                        }
                });

                t.registerCommand("select", new Command("select") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                if (args.length != 1) {
                                        return new Response("expected port number");
                                }
                                try {
                                        selected_port = Integer.parseInt(args[0]);
                                } catch (NumberFormatException ex) {
                                        return new Response("expected number");
                                }

                                if (selected_port < 0 || selected_port >= ports.length) {
                                        selected_port = -1;
                                        return new Response("index out of range");
                                }

                                return new Response("selected port: " + selected_port);
                        }
                });

                t.registerCommand("deselect", new Command("deselect") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                if (selected_port != -1) {
                                        selected_port = -1;
                                        return new Response("deselected");
                                }
                                return new Response("nothing selected");
                        }
                });

                t.registerCommand("open", new Command("open") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                if (selected_port > -1) {
                                        if (ports[selected_port].openPort()) {
                                                return new Response("opened port: " + selected_port);
                                        } else {
                                                return new Response("failed to open port: " + selected_port);
                                        }
                                }

                                return new Response("no port selected");
                        }
                });

                t.registerCommand("close", new Command("close") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                if (selected_port > -1) {
                                        if (ports[selected_port].closePort()) {
                                                return new Response("closed port: " + selected_port);
                                        } else {
                                                return new Response("failed to close port: " + selected_port);
                                        }
                                }

                                return new Response("no port selected");
                        }
                });

                t.registerCommand("read", new Command("read") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                if (args.length != 1) {
                                        return new Response("expected start or stop");
                                }

                                if (args[0].equals("start")) {
                                        if (read_thread != null) {
                                                return new Response("another thread is still open");
                                        }

                                        read_thread = new ReadThread(ports[selected_port]);

                                        read_thread.start();
                                        return new Response("read thread created");
                                } else if (args[0].equals("stop")) {
                                        if (read_thread == null) {
                                                return new Response("no read thread open");
                                        }
                                        read_thread.terminate();
                                        read_thread = null;
                                        return new Response("closed read thread");
                                }

                                return new Response();
                        }
                });

                t.registerCommand("send", new Command("send") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                StringBuilder sb = new StringBuilder();
                                if (selected_port > -1) {
                                        for (String arg : args) {
                                                sb.append(arg);
                                                sb.append(" ");
                                        }
                                        sb.deleteCharAt(sb.length() - 1);
                                        ports[selected_port].writeBytes(sb.toString().getBytes(UTF8), sb.length());
                                        return new Response("sent data: " + sb.toString());
                                }

                                return new Response("no port selected");
                        }
                });

                t.registerCommand("hex", new Command("hex") {

                        @Override
                        public Response doCommand(Terminal t, String[] args)
                        {
                                byte[] hex = new byte[args.length];
                                int i = 0;
                                if (selected_port > -1) {
                                        for (String arg : args) {
                                                hex[i++] = (byte) Integer.parseUnsignedInt(arg, 16);
                                        }
                                        ports[selected_port].writeBytes(hex, hex.length);
                                        return new Response("sent data: " + Arrays.toString(hex));
                                }

                                return new Response("no port selected");
                        }
                });
        }
}

class ReadThread extends Thread {

        boolean keep_alive = false;
        private final SerialPort port;

        public ReadThread(SerialPort port)
        {
                this.port = port;
        }

        @Override
        public void start()
        {
                keep_alive = true;
                super.start();
        }

        @Override
        public void run()
        {
                while (keep_alive) {
                        byte[] buffer = new byte[1024];

                        int read = port.readBytes(buffer, buffer.length);
                        if (read > 0) {
                                System.out.print("read bytes: ");
                                System.out.println(read);
                                System.out.print("received: ");
                                for (int i = 0; i < read; i++) {
                                        System.out.print(Integer.toString(buffer[i]));
                                        System.out.print(", ");
                                }
                                System.out.println();
                        }

                        try {
                                Thread.sleep(10);
                        } catch (InterruptedException ex) {
                        }
                }
        }

        public void terminate()
        {
                keep_alive = false;
        }
}
