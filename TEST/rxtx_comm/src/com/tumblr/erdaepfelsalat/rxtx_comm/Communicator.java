package com.tumblr.erdaepfelsalat.rxtx_comm;

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
import com.tumblr.erdaepfelsalat.util.terminal.Command;
import com.tumblr.erdaepfelsalat.util.terminal.Response;
import com.tumblr.erdaepfelsalat.util.terminal.Terminal;
import gnu.io.*;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Communicator {

    private static Communicator c;
    private static Terminal t;

    private static int connection_state;

    private static FileThread file_thread;

    private static final int CONNECTION_STATE_NONE = 0;
    private static final int CONNECTION_STATE_INITED = 10;
    private static final int CONNECTION_STATE_SEARCHED = 20;
    private static final int CONNECTION_STATE_CONNECTED = 30;
    private static final int CONNECTION_STATE_IO_DONE = 40;
    private static final int CONNECTION_STATE_CLOSED = 50;

    public static void main(String[] args) throws IOException, InterruptedException {
        if (args.length == 1) {
            com.tumblr.erdaepfelsalat.System.addToJavaLibraryPathWithUpdate(args[0]);
        }
        System.loadLibrary("rxtxSerial");

        connection_state = CONNECTION_STATE_NONE;

        t = new Terminal(System.in, System.out);
        initTerm();
        t.println("rxtx communicator by Manuel Federanko");
        t.start();
    }

    private static void initTerm() {
        t.init();
        t.registerCommand("i", new Command("i") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state == CONNECTION_STATE_NONE
                        || connection_state == CONNECTION_STATE_CLOSED) {
                    if (args.length == 0) {
                        c = new Communicator(9600);
                        connection_state = CONNECTION_STATE_INITED;
                    } else if (args.length == 1) {
                        try {
                            c = new Communicator(Integer.parseInt(args[0]));
                            connection_state = CONNECTION_STATE_INITED;
                        } catch (NumberFormatException ex) {
                            return new Response("number expected");
                        }
                    } else {
                        return new Response("only 0 or 1 argument");
                    }
                } else {
                    return new Response("connection already set up,"
                            + " close the current one before opening a new one");
                }
                return new Response();
            }
        });

        t.registerCommand("s", new Command("s") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_INITED) {
                    return new Response("you first need to init a new connection");
                }
                List<String> ports = c.searchForPorts();
                ports.stream().forEach((String port) -> {
                    println(t, port);
                });
                connection_state = CONNECTION_STATE_SEARCHED;
                return new Response();
            }
        });

        t.registerCommand("c", new Command("c") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_SEARCHED) {
                    return new Response("you first need to perform a search for devices");
                }
                if (args.length != 1) {
                    return new Response("expected 1 argument");
                }
                connection_state = CONNECTION_STATE_CONNECTED;
                try {
                    return new Response(c.connect(
                            c.getPortKey(
                                    Integer.parseInt(args[0]))));
                } catch (NumberFormatException ex) {
                    return new Response(c.connect(args[0]));
                }
            }
        });

        t.registerCommand("is", new Command("is") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED) {
                    return new Response("you first need to open a connection");
                }
                connection_state = CONNECTION_STATE_IO_DONE;
                return new Response(c.initIOStream());
            }
        });

        t.registerCommand("dc", new Command("dc") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }
                connection_state = CONNECTION_STATE_CLOSED;
                return new Response(c.disconnect());
            }
        });

        t.registerCommand("ws", new Command("ws") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }

                for (String s : args) {
                    try {
                        for (byte b : s.getBytes()) {
                            c.doWrite(b);
                        }
                        c.doWrite(' ');
                        c.flush();
                    } catch (IOException ex) {
                        return new Response("couldn't write to ports");
                    }
                }
                return new Response();
            }
        });

        t.registerCommand("wd", new Command("wd") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }

                try {
                    for (String s : args) {
                        if (s.startsWith("0x")) {
                            c.doWrite(Integer.parseInt(s.substring(2), 16));
                        } else if (s.startsWith("0b")) {
                            c.doWrite(Integer.parseInt(s.substring(2), 2));
                        } else {
                            c.doWrite(Integer.parseInt(s, 10));
                        }
                    }
                } catch (IOException ex) {
                    Logger.getLogger(Communicator.class.getName()).log(Level.SEVERE, null, ex);
                }

                return new Response();
            }
        });

        t.registerCommand("rc", new Command("rc") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }
                try {
                    while (c.available() > 0) {
                        println(t, (char) c.doRead());
                    }
                } catch (IOException ex) {
                    return new Response("couldn't read from ports");
                }
                return new Response();
            }
        });

        t.registerCommand("rb", new Command("rb") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }

                try {
                    while (c.available() > 0) {
                        if (args.length == 0) {
                            println(t, Integer.toString(c.doRead()));
                        } else {
                            if (args[0].charAt(0) == 'x') {
                                println(t, Integer.toHexString(c.doRead()));
                            } else if (args[0].charAt(0) == 'b') {
                                println(t, Integer.toBinaryString(c.doRead()));
                            }
                        }
                    }
                } catch (IOException ex) {
                    return new Response("couldn't read from ports");
                }
                return new Response();
            }
        });

        t.registerCommand("fs", new Command("fs") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (connection_state != CONNECTION_STATE_CONNECTED
                        && connection_state != CONNECTION_STATE_IO_DONE) {
                    return new Response("no open connection");
                }

                File f = new File(System.getProperty("user.home") + "/Desktop/stream_output.txt");
                if (!f.exists()) {
                    try {
                        f.createNewFile();
                    } catch (IOException ex) {
                        return new Response("couldnt create file");
                    }
                }

                try {
                    file_thread = new FileThread(t, c, System.getProperty("user.home") + "/Desktop/rxtx_output.txt");
                } catch (IOException ex) {
                    return new Response("failed to init file stream");
                }

                file_thread.start();
                return new Response();
            }
        });

        t.registerCommand("sf", new Command("sf") {

            @Override
            public Response doCommand(Terminal t, String[] args) {
                if (file_thread != null) {
                    try {
                        file_thread.terminate();
                    } catch (InterruptedException | IOException ex) {
                        return new Response("failed to close file stream");
                    }
                    file_thread = null;
                }
                return new Response();
            }
        });
    }

    //baud_rate of connection
    private final int baud_rate;

    //all serial ports
    private Enumeration ports;
    private final HashMap<String, CommPortIdentifier> port_map;
    private final HashMap<Integer, String> nr_port_name_map;

    //the port
    private CommPortIdentifier selected_port_identifier;
    private SerialPort serial_port;

    //io of the port
    private InputStream input;
    private OutputStream output;

    //timeout of port
    private static final int TIMEOUT = 2000;

    public Communicator(int baud_rate) {
        this.baud_rate = baud_rate;
        ports = null;
        port_map = new HashMap();
        nr_port_name_map = new HashMap();
        selected_port_identifier = null;
        serial_port = null;
        input = null;
        output = null;
    }

    //search for all the serial ports
    //pre: none
    //post: adds all the found ports to a combo box on the GUI
    public List<String> searchForPorts() {
        int index;
        CommPortIdentifier curr_port;
        LinkedList<String> ports__ = new LinkedList<>();
        ports = CommPortIdentifier.getPortIdentifiers();

        index = 0;
        while (ports.hasMoreElements()) {
            curr_port = (CommPortIdentifier) ports.nextElement();

            if (curr_port.getPortType() == CommPortIdentifier.PORT_SERIAL) {
                nr_port_name_map.put(index, curr_port.getName());
                port_map.put(curr_port.getName(), curr_port);
                ports__.add("[" + index + "]: " + curr_port.getName());
                index++;
            }
        }
        return ports__;
    }

    public String getPortKey(int index) {
        return nr_port_name_map.get(index);
    }

    //connect to the selected port in the combo box
    //pre: ports are already found by using the searchForPorts method
    //post: the connected comm port is stored in commPort, otherwise,
    //an exception is generated
    public String connect(String port_key) {
        CommPort comm_port;
        selected_port_identifier = (CommPortIdentifier) port_map.get(port_key);

        try {
            comm_port = selected_port_identifier.open("TigerControlPanel", TIMEOUT);
            serial_port = (SerialPort) comm_port;

            serial_port.setSerialPortParams(
                    baud_rate,
                    SerialPort.DATABITS_8,
                    SerialPort.STOPBITS_1,
                    SerialPort.PARITY_NONE);

            return "connected successfully";
        } catch (PortInUseException e) {
            return port_key + " is in use.";
        } catch (UnsupportedCommOperationException e) {
            return "failed to open " + port_key;
        }
    }

    //open the input and output streams
    //pre: an open port
    //post: initialized intput and output streams for use to communicate data
    public String initIOStream() {
        if (serial_port == null) {
            return "no serial port initiated";
        }
        try {
            input = serial_port.getInputStream();
            output = serial_port.getOutputStream();

            return "created io streams";
        } catch (NullPointerException ex) {
            return "some element hasn't been properly initialized";
        } catch (IOException e) {
            return "failed to open ios";
        }
    }

    //disconnect the serial port
    //pre: an open serial port
    //post: clsoed serial port
    public String disconnect() {
        try {
            if (input != null) {
                input.close();
            }
            if (output != null) {
                output.close();
            }
            if (serial_port != null) {
                serial_port.close();
            }

            return "disconnected";
        } catch (Exception e) {
            e.printStackTrace(System.err);
            return "failed to close serial port";
        }
    }

    public void doWrite(int i) throws IOException {
        output.write(i);
    }

    public void flush() throws IOException {
        output.flush();
    }

    public int doRead() throws IOException {
        return input.read();
    }

    public int available() throws IOException {
        return input.available();
    }
}

class FileThread extends Thread {

    private boolean keep_alive;
    private final Terminal t;
    private final Communicator c;
    private final FileOutputStream fos;

    public FileThread(Terminal t, Communicator c, String file) throws IOException {
        File f = new File(file);
        this.t = t;
        this.c = c;

        if (!f.exists()) {
            f.createNewFile();
        }
        fos = new FileOutputStream(f);
    }

    @Override
    public void run() {
        keep_alive = true;
        while (keep_alive) {
            try {
                if (c.available() > 0) {
                    fos.write(c.doRead());
                }
            } catch (IOException ex) {
                ex.printStackTrace(System.err);
            }
        }
    }

    public void terminate() throws InterruptedException, IOException {
        keep_alive = false;
        Thread.sleep(100);
        fos.flush();
        fos.close();
    }
}
