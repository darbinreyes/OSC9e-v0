
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
1. Use Driver.java example to create a class that extends Runnable.
This class will print the data to the socket then close the connection.

*/

import java.net.*;
import java.io.*;

class MyDateSocketPrinter implements Runnable // Note runnable.
{
  private Socket client;

  public MyDateSocketPrinter(Socket client) { // Constructor.
    if (client == NULL)
      throw new IllegalArgumentException();

    this.client = client;
  }

  public void run() { // Get date. Print to socket. Close socket. Exit.
    PrintWriter pout = new PrintWriter(client.getOutputStream(), true);
    // write the Date to the socket
    pout.println(new java.util.Date().toString());

    // close the socket and resume listening for more connections
    this.client.close();
  }
}

public class DateServer
{
  public static void main(String[] args)  {
    try {
      ServerSocket sock = new ServerSocket(6013);

      // now listen for connections
      while (true) {
        Socket client = sock.accept();
        // we have a connection
        

      }
    }
    catch (IOException ioe) {
        System.err.println(ioe);
    }
  }
}
