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

import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;

/**
 *
 * @author redxef
 */
class ProgressCircleUI extends javax.swing.plaf.basic.BasicProgressBarUI {

        private static final int ALPHA = 80;
        private static final int FULLROT = 360;
        private static final int HALFROT = FULLROT / 2;
        private static final int RANGE = FULLROT - ALPHA;
        private static final int GAMMA = (HALFROT - ALPHA) / 2;
        private static final int STARTANGLE = HALFROT + GAMMA;
        private static final double ALPHA_RAD = ALPHA * Math.PI / 180;
        private static final double FULLROT_RAD = 2 * Math.PI;
        private static final double HALFROT_RAD = Math.PI;
        private static final double RANGE_RAD = HALFROT * Math.PI / 180;
        private static final double GAMMA_RAD = GAMMA * Math.PI / 180;
        private static final double STARTANGLE_RAD = STARTANGLE * Math.PI / 180;

        private final int boldness;
        private final Color bg;
        private final Color fg;

        /**
         * Returns a new ProgressCircleUI Object in the PowerSpy design with the
         * correct colours.
         *
         * @return the ProgrssCircleUI
         */
        public static ProgressCircleUI getPSDesign()
        {
                return new ProgressCircleUI(20, Color.LIGHT_GRAY, Defs.PS_ORANGE);
        }

        /**
         * Contructs a new ProgrssCircleUI with the boldness of the arc, a
         * foreground colour and a background colour.
         *
         * @param boldness the boldness of the arc
         * @param bg the background colour
         * @param fg the foreground colour
         */
        public ProgressCircleUI(int boldness, Color bg, Color fg)
        {
                this.boldness = boldness;
                this.bg = bg;
                this.fg = fg;
        }

        @Override
        public Dimension getPreferredSize(JComponent c)
        {
                Dimension d = super.getPreferredSize(c);
                int v = Math.max(d.width, d.height);
                d.setSize(v, v);
                return d;
        }

        private static void fillRoundArc(Graphics g, int x, int y, int width, int height, int thickness)
        {
                g.fillArc(x, y, width, height, -GAMMA, RANGE);

                int x_ = (int) (-Math.cos(GAMMA_RAD - 0.13) * ((width - thickness)/2 + 1));
                int y_ = (int) (Math.sin(GAMMA_RAD - 0.13) * ((height - thickness)/2 + 1));
                g.setColor(Color.RED);
                g.fillArc(x_, y_, thickness, thickness, 0, 360);
                
                x_ = (int) (Math.cos(GAMMA_RAD + 0.13) * ((width - thickness)/2 + 5));
                g.fillArc(x_, y_, thickness, thickness, 0, 360);
        }

        @Override
        public void paint(Graphics g, JComponent c)
        {
                Graphics2D g2d;

                int width = progressBar.getWidth()
                        - progressBar.getInsets().right
                        - progressBar.getInsets().left;
                int heigh = progressBar.getHeight()
                        - progressBar.getInsets().top
                        - progressBar.getInsets().bottom;

                heigh += (int) (heigh / 2 * (1 - Math.cos(ALPHA_RAD / 2)));

                int ratio = (progressBar.getValue() * RANGE) / 100;

                if (width <= 0 || heigh <= 0)
                        return;

                g2d = (Graphics2D) g.create();

                Shape o0 = new Arc2D.Double(0, 0, width, heigh, STARTANGLE, -ratio, Arc2D.PIE);
                Shape o1 = new Arc2D.Double(0, 0, width, heigh, STARTANGLE, -RANGE, Arc2D.PIE);
                Shape i = new Ellipse2D.Double(boldness, boldness, width - 2 * boldness, heigh - 2 * boldness);
                Area a0 = new Area(o0);
                Area a1 = new Area(o1);
                a0.subtract(new Area(i));
                a1.subtract(new Area(i));

                g2d.setColor(Color.LIGHT_GRAY);
                g2d.fill(a1);
                g2d.setColor(getSelectionForeground());
                g2d.fill(a0);

                if (progressBar.isStringPainted()) {
                        paintString(g, progressBar.getInsets().left,
                                progressBar.getInsets().top,
                                width, heigh, 0, progressBar.getInsets());
                }
        }
        
        @Override
        protected Color getSelectionForeground()
        {
                return Defs.PS_ORANGE;
        }

        @Override
        protected Color getSelectionBackground()
        {
                return Defs.PS_ORANGE;
        }
}
