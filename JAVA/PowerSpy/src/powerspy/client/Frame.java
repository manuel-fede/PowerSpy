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
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import javax.swing.BoxLayout;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;
import javax.swing.Timer;
import javax.swing.event.ListSelectionEvent;
import powerspy.baselib.PSInputStream;
import static powerspy.client.Defs.MAX_AMPS;
import static powerspy.client.Defs.MAX_POWER;
import static powerspy.client.Defs.MIN_AMPS;
import static powerspy.client.Defs.MIN_POWER;

/**

 @author redxef
 */
public class Frame extends JFrame implements Defs {

        Dimension BASIC_SIZE = new Dimension(300, 400);

        Controller c;
        JComboBox<SerialPort> ports;
        JLabel info;
        JProgressBar pb;
        JLabel min;
        JLabel max;
        Timer progress_update;
        int target;
        JTable t;

        public Frame()
        {
                super();
                setDefaultCloseOperation(EXIT_ON_CLOSE);
                getContentPane()
                        .setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));

                initCombobox();
                initJLabel();
                initProgressBar();
                initProgressTimer();
                initDesc();
                initJTable();

                pack();
                setSize(BASIC_SIZE);
                setPreferredSize(BASIC_SIZE);
                setMinimumSize(getSize());
                setMaximumSize(getSize());
                setLocationRelativeTo(null);
        }

        public void installController(Controller c)
        {
                this.c = c;
        }

        private void initCombobox()
        {
                ports = new JComboBox<>();
                ports.setRenderer(new PortListCellRenderer());
                ports.addActionListener((ActionEvent e) -> {
                        connect(e);
                });

                add(ports);
        }

        private void initJLabel()
        {
                info = new JLabel();

                add(info);
        }

        private void initProgressBar()
        {
                JPanel p = new JPanel();
                p.setLayout(new BorderLayout());
                pb = new JProgressBar() {

                        @Override
                        public void setValue(int n)
                        {
                                super.setValue(n);

                                if (t != null)
                                        if (t.getSelectedRow() == -1)
                                                setString("-");
                                        else
                                                setString(t.getValueAt(
                                                        t.getSelectedRow(), 1)
                                                        .toString());
                        }
                };
                pb.setUI(ProgressCircleUI.getPSDesign());
                pb.setForeground(PS_ORANGE);

                pb.setValue(0);
                pb.setStringPainted(true);

                p.add(pb);
                add(p);
        }

        private void initProgressTimer()
        {
                //some fancy animation
                progress_update = new Timer(5, (ActionEvent e) -> {
                        int diff = pb.getValue() - target;
                        diff = (diff > 0) ? diff : -diff;
                        diff = (int) (1 / Math.log(diff / 15.0));
                        if (diff <= 0)
                                diff = 1;

                        if (pb.getValue() > target)
                                pb.setValue(pb.getValue() - diff);
                        else if (pb.getValue() < target)
                                pb.setValue(pb.getValue() + diff);
                        else
                                progress_update.stop();
                });
                progress_update.start();
        }

        private void initDesc()
        {
                JPanel p = new JPanel(new GridLayout(1, 2));
                min = new JLabel(" ", SwingConstants.CENTER);
                max = new JLabel(" ", SwingConstants.CENTER);

                p.add(min);
                p.add(max);
                add(p);
        }

        private void initJTable()
        {
                t = new JTable(4, 2) {

                        @Override
                        public boolean isCellEditable(int row, int column)
                        {
                                return false;
                        }
                };
                t.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
                t.getSelectionModel().addListSelectionListener((ListSelectionEvent e) -> {
                        updateVals();
                });
                t.setValueAt("Current", 0, 0);
                t.setValueAt("Real Power", 1, 0);
                t.setValueAt("Active Power", 2, 0);
                t.setValueAt("Reactive Power", 3, 0);
                add(t);
        }

        private void connect(ActionEvent e)
        {
                new Thread() {

                        @Override
                        public void run()
                        {
                                SerialPort sp = (SerialPort) ports.getSelectedItem();
                                info.setText("connecting...");
                                sp.openPort();

                                if (sp.isOpen()) {
                                        info.setText("connected");
                                } else {
                                        info.setText("failed to connect");
                                        return;
                                }

                                sp.writeBytes(new byte[]{0}, 1);
                                if (c != null)
                                        c.setPSInputStream(new PSInputStream(sp));
                        }
                }.start();
        }

        private void setMin(String s)
        {
                min.setText("<html><div style='text-align: center;'>" + s + "</html>");
        }

        private void setMax(String s)
        {
                max.setText("<html><div style='text-align: center;'>" + s + "</html>");
        }

        private void updateVals()
        {
                Object val_;
                double val;
                if (t.getSelectedRow() == -1) {
                        target = 70;
                } else if (t.getSelectedRow() == 0) {//current
                        val_ = t.getValueAt(0, 1);
                        if (val_ == null)
                                return;
                        val = (double) val_;
                        target = (int) ((val * 100) / (MAX_AMPS - MIN_AMPS));
                        setMin(Integer.toString(MIN_AMPS));
                        setMax(Integer.toString(MAX_AMPS));
                } else {
                        val_ = t.getValueAt(t.getSelectedRow(), 1);
                        if (val_ == null)
                                return;
                        val = (double) val_;
                        target = (int) ((val * 100) / (MAX_POWER - MIN_POWER));
                        setMin(Integer.toString(MIN_POWER));
                        setMax(Integer.toString(MAX_POWER));
                }
                progress_update.start();
        }

        public void setCurrent(double d)
        {
                t.setValueAt(d, 0, 1);
                updateVals();
        }

        public void setRealPower(double d)
        {
                t.setValueAt(d, 1, 1);
                updateVals();
        }

        public void setActivePower(double d)
        {
                t.setValueAt(d, 2, 1);
                updateVals();
        }

        public void setReactivePower(double d)
        {
                t.setValueAt(d, 3, 1);
                updateVals();
        }
}
