
/**
 * Time-of-day server listening to port 6013.
 *
 * Figure 3.21
 *
 * @author Silberschatz, Gagne, and Galvin. 
 * Operating System Concepts  - Ninth Edition
 * Copyright John Wiley & Sons - 2013.
 */

/*


# Programming Problem 4.25.

## Start: From the Book

> Modify the socket-based date server (Figure 3.21) in Chapter 3 so that the
> server services each client request in a separate thread.

## End


# Plan:

*/

import java.net.*;
import java.io.*;

public class DateServer
{
  public static void main(String[] args)  {
    try {
      ServerSocket sock = new ServerSocket(6013);

      // now listen for connections
      while (true) {
        Socket client = sock.accept();
        // we have a connection
        
        PrintWriter pout = new PrintWriter(client.getOutputStream(), true);
        // write the Date to the socket
        pout.println(new java.util.Date().toString());

        // close the socket and resume listening for more connections
        client.close();
      }
    }
    catch (IOException ioe) {
        System.err.println(ioe);
    }
  }
}
