
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

class Summation implements Runnable // Note runnable.
{
  private int upper;
  private Sum sumValue; // Sum class defined above. Just a get/set int.

  public Summation(int upper, Sum sumValue) {
    if (upper < 0)
      throw new IllegalArgumentException();

    this.upper = upper;
    this.sumValue = sumValue;
  }

  public void run() { // Do a cumulative sum from 0->upper. Save sum in sumValue.
    int sum = 0;

    for (int i = 0; i <= upper; i++)
      sum += i;

    sumValue.set(sum);
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
