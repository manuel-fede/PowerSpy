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
package powerspy.client;

import com.fazecast.jSerialComm.SerialPort;
import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.GraphicsEnvironment;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Arrays;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import powerspy.baselib.ArrayInputStream;
import static powerspy.baselib.IODefs.*;

/**
 *
 * @author redxef
 */
public class PowerSpy {

        private static final ArrayInputStream DUMMY_IS;
        private static final Thread DUMMY_THREAD;
        private static final Random RANDOM = new Random();

        static {
                DUMMY_IS = new ArrayInputStream();

                DUMMY_THREAD = new Thread() {

                        private void insertKey(char key)
                        {
                                byte[] b = new byte[3];
                                b[0] = START_OF_TEXT;
                                b[1] = UINT8;
                                b[2] = (byte) (key & 0xff);

                                DUMMY_IS.insert(b, 0, b.length);
                        }

                        private void insertValue(int i)
                        {
                                byte[] b = new byte[5];
                                b[0] = START_OF_TEXT;
                                b[1] = INT24;
                                b[2] = (byte) (i >> 16 & 0xff);
                                b[3] = (byte) (i >> 8 & 0xff);
                                b[4] = (byte) (i & 0xff);

                                DUMMY_IS.insert(b, 0, b.length);
                        }

                        @Override
                        public void run()
                        {
                                while (true) {
                                        insertKey(K_CURRENT);
                                        insertValue(RANDOM.nextInt(5000));
                                        insertKey(K_APPARENTEPOWER);
                                        insertValue(RANDOM.nextInt(1150000));
                                        insertKey(K_REALPOWER);
                                        insertValue(RANDOM.nextInt(1150000));
                                        insertKey(K_REACTIVEPOWER);
                                        insertValue(RANDOM.nextInt(1150000));

                                        try {
                                                Thread.sleep(1000);
                                        } catch (InterruptedException ex) {
                                        }
                                }
                        }
                };
        }

        private static void loadFont(String path) throws URISyntaxException, FontFormatException, IOException
        {
                Font f = Font.createFont(Font.TRUETYPE_FONT, PowerSpy.class.getResourceAsStream(path));
                GraphicsEnvironment.getLocalGraphicsEnvironment().registerFont(f);
        }

        public static void main(String[] args) throws InterruptedException
        {
                //Set the default look and feel to the system independent one
                try {
                        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                } catch (ClassNotFoundException | InstantiationException |
                        IllegalAccessException | UnsupportedLookAndFeelException ex) {
                        ex.printStackTrace(System.err);
                }

                try {
                        System.out.println("loading fonts...");
                        loadFont("/powerspy/client/fonts/Agency_FB.ttf");
                } catch (URISyntaxException | FontFormatException | IOException ex) {
                        ex.printStackTrace(System.err);
                }

                //create new Frame and link it with a new Controller
                Frame f = new Frame(null);
                Controller c = new Controller(f);
                f.installController(c);

                //add the serial ports
                Arrays.asList(SerialPort.getCommPorts()).forEach((SerialPort p) -> {
                        System.out.print("found port: ");
                        System.out.print(p.getSystemPortName());
                        System.out.print(": ");
                        System.out.println(p.getDescriptivePortName());
                        if (!p.getDescriptivePortName().contains("Dial-In")) //no low power ports
                        {
                                if (!p.getDescriptivePortName().contains("Bluetooth-Incoming-Port")) {
                                        System.out.println("adding port...");
                                        f.ports.addItem(p);
                                }
                        }
                });

                //display the frame
                java.awt.EventQueue.invokeLater(() -> {
                        f.setVisible(true);
                });
        }
}
