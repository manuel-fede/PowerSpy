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

/**

 @author redxef
 */
public class Controller extends Thread {

        Frame f;
        PSInputStream is;
        PSOutputStream os;
        Timer update_interval;

        private boolean keep_alive;

        private float current;
        private float real_power;
        private float active_power;
        private float reactive_power;

        public Controller(Frame f)
        {
                keep_alive = false;
                this.f = f;
                is = null;
                os = null;
                update_interval = new Timer(1_000, (ActionEvent e) -> {
                        doUpdate();
                });
        }

        public void setPSInputStream(PSInputStream is)
        {
                this.is = is;
        }

        public void setPSOutputStream(PSOutputStream os)
        {
                this.os = os;
        }

        private void doUpdate()
        {
                f.setCurrent(current);
                f.setRealPower(real_power);
                f.setActivePower(active_power);
                f.setReactivePower(reactive_power);
        }

        @Override
        public void start()
        {
                keep_alive = true;
                update_interval.start();
                super.start();
        }

        @Override
        public void run()
        {
                char read_mode = IODefs.NONE;
                while (keep_alive) {
                        try {
                                if (is != null && is.readPackage()) {
                                        if (read_mode == IODefs.NONE && is.isInt8()) {
                                                read_mode = (char) is.readInt8();
                                                is.clear();
                                        } else if (read_mode != IODefs.NONE && is.isFloat()) {
                                                switch (read_mode) {
                                                        case IODefs.K_CURRENT:
                                                                current = is.readFloat();
                                                                break;
                                                        case IODefs.K_REALPOWER:
                                                                real_power = is.readFloat();
                                                                break;
                                                        case IODefs.K_ACTIVEPOWER:
                                                                active_power = is.readFloat();
                                                                break;
                                                        case IODefs.K_REACTIVEPOWER:
                                                                reactive_power = is.readFloat();
                                                                break;
                                                }
                                                read_mode = IODefs.NONE;
                                        }
                                }
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
