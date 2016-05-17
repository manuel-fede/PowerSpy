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

import java.io.IOException;
import static powerspy.baselib.IODefs.*;

/**

 @author redxef
 */
public class Test2 {

        public static void main(String[] args) throws IOException, PackageException
        {
                ArrayInputStream is = new ArrayInputStream();
                PSInputStream pis = new PSInputStream(is);
                byte[] b = new byte[4];
                b[0] = START_OF_TEXT;
                b[1] = INT8;
                b[2] = 13;
                b[3] = END_OF_TEXT;

                is.insert(b, 0, b.length);

                if (pis.readPackage()) {
                        System.out.println(pis.readObj());
                        pis.clear();
                }
        }
}
