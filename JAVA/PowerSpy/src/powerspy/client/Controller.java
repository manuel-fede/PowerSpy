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

import java.awt.event.ActionEvent;
import java.io.IOException;
import javax.swing.Timer;
import powerspy.baselib.*;
import static powerspy.baselib.IODefs.*;

/**

 @author redxef
 */
public class Controller extends Thread {

        private final Frame f;
        private final Timer update_interval;
        private PSInputStream is;
        private PSOutputStream os;

        private boolean keep_alive;

        private int current;
        private int real_power;
        private int apparent_power;
        private int reactive_power;

        public Controller(Frame f)
        {
                keep_alive = false;
                this.f = f;
                is = null;
                os = null;
                update_interval = new Timer(500, (ActionEvent e) -> {
                        doUpdate();
                });
        }

        public synchronized void setPSInputStream(PSInputStream is)
        {
                this.is = is;
        }

        public synchronized PSInputStream getPSInputStream()
        {
                return is;
        }

        public synchronized void setPSOutputStream(PSOutputStream os)
        {
                this.os = os;
        }

        public synchronized PSOutputStream getPSOutputStream()
        {
                return os;
        }

        private void doUpdate()
        {
                f.setCurrent((float) current / 1000);
                f.setRealPower((float) real_power / 1000);
                f.setApparentPower((float) apparent_power / 1000);
                f.setReactivePower((float) reactive_power / 1000);
        }

        @Override
        public void start()
        {
                keep_alive = true;
                update_interval.start();
                super.start();
        }

        private synchronized char doRun(char read_mode) throws IOException, PackageException
        {
                if (is != null && is.readPackage()) {
                        if (read_mode == NONE && is.isInt8()) {
                                read_mode = (char) is.readInt8();
                                is.clear();
                        } else if (read_mode != NONE && is.isInt24()) {
                                switch (read_mode) {
                                        case K_CURRENT:
                                                current = is.readInt24();
                                                break;
                                        case K_REALPOWER:
                                                real_power = is.readInt24();
                                                break;
                                        case K_APPARENTEPOWER:
                                                apparent_power = is.readInt24();
                                                break;
                                        case K_REACTIVEPOWER:
                                                reactive_power = is.readInt24();
                                                break;
                                        default:
                                                System.out.print(read_mode);
                                                System.out.print(": ");
                                                System.out.println(is.readObj());
                                                break;
                                }
                                read_mode = NONE;
                        } else {
                                System.out.print(read_mode);
                                System.out.print(": ");
                                System.out.println(is.readObj());
                        }
                        is.clear();
                }
                System.out.flush();
                return read_mode;
        }

        @Override
        public void run()
        {
                char read_mode = NONE;
                while (keep_alive) {
                        try {
                                read_mode = doRun(read_mode);
                        } catch (IOException | PackageException ex) {
                                ex.printStackTrace(System.err);
                        }
                }
        }

        public void terminate()
        {
                keep_alive = false;
                update_interval.stop();
        }

}
