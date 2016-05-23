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
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

/**
 *
 * @author redxef
 */
public class PowerSpy {

        public static void main(String[] args) throws InterruptedException
        {
                //Set the default look and feel to the system independent one
                try {
                        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
                } catch (ClassNotFoundException | InstantiationException |
                        IllegalAccessException | UnsupportedLookAndFeelException ex) {
                        ex.printStackTrace(System.err);
                }

                //create new Frame and link it with a new Controller
                Frame f = new Frame();
                Controller c = new Controller(f);
                f.installController(c);

                //add the serial ports
                for (SerialPort p : SerialPort.getCommPorts())
                        if (!p.getDescriptivePortName().contains("Dial-In")) //remove the clutter
                                if (!p.getDescriptivePortName().contains("Bluetooth"))
                                        f.ports.addItem(p);

                //display the frame
                java.awt.EventQueue.invokeLater(() -> {
                        f.setVisible(true);
                });
        }
}
