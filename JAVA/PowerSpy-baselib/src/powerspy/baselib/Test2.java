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
import java.util.logging.Level;
import java.util.logging.Logger;

/**

 @author redxef
 */
public class Test2 {

        public static void main(String[] args) throws IOException
        {
                ArrayInputStream is = new ArrayInputStream();
                byte[] data = "[1]hellohellohellohello[2]".getBytes();
                
                
                is.insert(data, 0, data.length);
                while (is.available() > 0) {
                        System.out.println((char)is.read());
                }
                is.close();
        }
}
