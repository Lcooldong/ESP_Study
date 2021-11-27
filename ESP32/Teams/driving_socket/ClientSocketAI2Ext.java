// -*- mode: java; c-basic-offset: 2; -*-
// Copyright 2009-2011 Google, All Rights reserved
// Copyright 2011-2012 MIT, All rights reserved
// Released under the Apache License, Version 2.0
// http://www.apache.org/licenses/LICENSE-2.0

package com.gmail.at.moicjarod;

import com.google.appinventor.components.runtime.*;
import com.google.appinventor.components.runtime.util.RuntimeErrorAlert;
import com.google.appinventor.components.annotations.DesignerComponent;
import com.google.appinventor.components.annotations.DesignerProperty;
import com.google.appinventor.components.annotations.PropertyCategory;
import com.google.appinventor.components.annotations.SimpleEvent;
import com.google.appinventor.components.annotations.SimpleFunction;
import com.google.appinventor.components.annotations.SimpleObject;
import com.google.appinventor.components.annotations.SimpleProperty;
import com.google.appinventor.components.annotations.UsesLibraries;
import com.google.appinventor.components.annotations.UsesPermissions;
import com.google.appinventor.components.common.ComponentCategory;
import com.google.appinventor.components.common.PropertyTypeConstants;
import com.google.appinventor.components.runtime.util.AsynchUtil;
import com.google.appinventor.components.runtime.util.ErrorMessages;
import com.google.appinventor.components.runtime.util.YailList;
import com.google.appinventor.components.runtime.util.SdkLevel;

import com.google.appinventor.components.runtime.errors.YailRuntimeError;

import android.app.Activity;
import android.text.TextUtils;
import android.util.Log;
import android.os.StrictMode;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.InetSocketAddress;
import java.net.SocketException;

/**
 * Simple Client Socket
 * @author moicjarod@gmail.com (Jean-Rodolphe Letertre)
 * with the help of the work of lizlooney @ google.com (Liz Looney) and josmasflores @ gmail.com (Jose Dominguez)
 * the help of Alexey Brylevskiy for debugging
 * and the help of Hossein Amerkashi from AppyBuilder for compatibility with AppyBuilder
 */
@DesignerComponent(version = 4,
  description = "Non-visible component that provides client socket connectivity.",
  category = ComponentCategory.EXTENSION,
  nonVisible = true,
  iconName = "http://jr.letertre.free.fr/Projets/AIClientSocket/clientsocket.png")
  @SimpleObject(external = true)
  @UsesPermissions(permissionNames = "android.permission.INTERNET")


public class ClientSocketAI2Ext extends AndroidNonvisibleComponent implements Component
{
  private static final String LOG_TAG = "ClientSocketAI2Ext";

  private final Activity activity;

  // the socket object
  private Socket clientSocket = null;
  // the address to connect to
  private String serverAddress = "";
  // the port to connect to
  private String serverPort = "";
  // boolean that indicates the state of the connection, true = connected, false = not connected
  private boolean connectionState = false;
  // boolean that indicates the mode used, false = string sent as is, true = String is considered as hexadecimal data and will be converted before sending
  // same behavior is used when receiving data
  private boolean hexaStringMode = false;
  // boolean to enable debug messages "YailRuntimeError" (true by default to ensure same behavior as before)
  private boolean debugMessages = true;

  InputStream inputStream = null;

  /**
   * Creates a new Client Socket component.
   *
   * @param container the Form that this component is contained in.
   */
  public ClientSocketAI2Ext(ComponentContainer container)
  {
    super(container.$form());
    activity = container.$context();
    // compatibility with AppyBuilder (thx Hossein Amerkashi <kkashi01 [at] gmail [dot] com>)
    StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
    StrictMode.setThreadPolicy(policy);
  }

  /**
   * Method that returns the server's address.
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "The address of the server the client will connect to.")
  public String ServerAddress()
  {
    return serverAddress;
  }

  /**
   * Method to specify the server's address
   */
  @DesignerProperty(editorType = PropertyTypeConstants.PROPERTY_TYPE_STRING)
  @SimpleProperty
  public void ServerAddress(String address)
  {
    serverAddress = address;
  }

  /**
   * Method that returns the server's port.
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "The port of the server the client will connect to.")
  public String ServerPort()
  {
    return serverPort;
  }

  /**
   * Method to specify the server's port
   */
  @DesignerProperty(editorType = PropertyTypeConstants.PROPERTY_TYPE_STRING)
  @SimpleProperty
  public void ServerPort(String port)
  {
    serverPort = port;
  }

  /**
   * Method that returns the connection state
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "The state of the connection - true = connected, false = disconnected")
  public boolean ConnectionState()
  {
    return connectionState;
  }

  /**
   * Method that returns a string containing "\n\r" sequence
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "returns a string containing \"\n\r\" sequence")
  public String SeqNewLineAndRet()
  {
    String seq = "";
    seq = seq + '\n' + '\r';
    return seq;
  }

  /**
   * Method that returns a string containing "\r\n" sequence
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "returns a string containing \"\r\n\" sequence")
  public String SeqRetAndNewLine()
  {
    String seq = "";
    seq = seq + '\r' + '\n';
    return seq;
  }

  /**
   * Method that returns a string containing "\r" sequence
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "returns a string containing \"\r\" sequence")
  public String SeqRet()
  {
    String seq = "";
    seq = seq + '\r';
    return seq;
  }

  /**
   * Method that returns a string containing "\n" sequence
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "returns a string containing \"\n\" sequence")
  public String SeqNewLine()
  {
    String seq = "";
    seq = seq + '\n';
    return seq;
  }

  /**
   * Method that returns the mode (string or hexastring)
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "The mode of sending and receiving data.")
  public boolean HexaStringMode()
  {
    return hexaStringMode;
  }

  /**
   * Method to specify the mode (string or hexastring)
   */
  @DesignerProperty(editorType = PropertyTypeConstants.PROPERTY_TYPE_BOOLEAN)
  @SimpleProperty
  public void HexaStringMode(boolean mode)
  {
    hexaStringMode = mode;
  }

  /**
   * Method that returns the display of debug messages
   */
  @SimpleProperty(category = PropertyCategory.BEHAVIOR, description = "The display of debug messages.")
  public boolean DebugMessages()
  {
    return debugMessages;
  }

  /**
   * Method to specify the display of debug messages
   */
  @DesignerProperty(editorType = PropertyTypeConstants.PROPERTY_TYPE_BOOLEAN, defaultValue="True")
  @SimpleProperty
  public void DebugMessages(boolean displayDebugMessages)
  {
    debugMessages = displayDebugMessages;
  }

  /**
   * Creates the socket, connect to the server and launches the thread to receive data from server
   */
  @SimpleFunction(description = "Tries to connect to the server and launches the thread for receiving data (blocking until connected or failed)")
  public void Connect()
  {
    if (connectionState == true)
    {
      if (debugMessages == true)
        throw new YailRuntimeError("Connect error, socket connected yet, please disconnect before reconnect !", "Error");
    }
    try
    {
      // connecting the socket
      clientSocket = new Socket();
      clientSocket.connect(new InetSocketAddress(serverAddress, Integer.parseInt(serverPort)), 5000);
      connectionState = true;
      // begin the receive loop in a new thread
      AsynchUtil.runAsynchronously(new Runnable()
      {
        @Override
        public void run()
        {
          ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream(1024);
          byte[] buffer = new byte[1024];
          int bytesRead = 0;

          try
          {
            // get the input stream and save the data
            inputStream = clientSocket.getInputStream();
            while (true)
            {
              // test if there is a server problem then close socket properly (thx Axeley :-))
              try
              {
                bytesRead = inputStream.read(buffer);
                if(bytesRead == -1)
                  break;
              }
              catch(SocketException e)
              {
                //throw e;
              }
              catch(IOException e)
              {
                if(e.getMessage().indexOf("ETIMEDOUT") >= 0)
                  break;
                //throw e;
              }

              byteArrayOutputStream.write(buffer, 0, bytesRead);
              final String dataReceived;
              // hexaStringMode is false, so we don't transform the string received
              if (hexaStringMode == false)
              {
                dataReceived = byteArrayOutputStream.toString("UTF-8");
              }
              // hexaStringMode is true, so we make a string with each character as an hexa symbol representing the received message
              else
              {
                int i;
                char hexaSymbol1, hexaSymbol2;
                String tempData = "";
                byte[] byteArray = byteArrayOutputStream.toByteArray();
                for (i = 0; i < byteArrayOutputStream.size(); i++)
                {
                  if (((byteArray[i] & 0xF0) >> 4) < 0xA)
                  // 0 to 9 symbol
                  hexaSymbol1 = (char)(((byteArray[i] & 0xF0) >> 4) + 0x30);
                  else
                    // A to F symbol
                    hexaSymbol1 = (char)(((byteArray[i] & 0xF0) >> 4) + 0x37);
                  if ((byteArray[i] & 0x0F) < 0xA)
                  hexaSymbol2 = (char)((byteArray[i] & 0x0F) + 0x30);
                  else
                    hexaSymbol2 = (char)((byteArray[i] & 0x0F) + 0x37);
                  tempData = tempData + hexaSymbol1 + hexaSymbol2;
                }

                dataReceived = tempData;
              }
              // reset of the byteArrayOutputStream to flush the content
              byteArrayOutputStream.reset();
              // then we send the data to the user using an event
              // events must be sent by the main thread (UI)
              activity.runOnUiThread(new Runnable()
              {
                @Override
                public void run()
                {
                  DataReceived(dataReceived);
                }
              } );
            }
            // When we go there, either we have
            // - server shutdown
            // - disconnection asked (inputstream closed => -1 returned)
            // - connection problem
            // so, if it is not disconnected yet, we disconnect the socket and inform the user of it.
            if (connectionState == true)
            {
              Disconnect();
              // events must be sent by the main thread (UI)
              activity.runOnUiThread(new Runnable()
              {
                @Override
                public void run()
                {
                  RemoteConnectionClosed();
                }
              } );
            }
          }
          catch (SocketException e)
          {
            Log.e(LOG_TAG, "ERROR_READ", e);
            if (debugMessages == true)
              throw new YailRuntimeError("Connect error (read)" + e.getMessage(), "Error");
          }
          catch (IOException e)
          {
            Log.e(LOG_TAG, "ERROR_READ", e);
            if (debugMessages == true)
              throw new YailRuntimeError("Connect error (read)", "Error");
          }
          catch (Exception e)
          {
            connectionState = false;
            Log.e(LOG_TAG, "ERROR_READ", e);
            if (debugMessages == true)
              throw new YailRuntimeError("Connect error (read)" + e.getMessage(), "Error");
          }
        }
      } );
    }
    catch (SocketException e)
    {
      Log.e(LOG_TAG, "ERROR_CONNECT", e);
      if (debugMessages == true)
        throw new YailRuntimeError("Connect error" + e.getMessage(), "Error");
    }
    catch (Exception e)
    {
      connectionState = false;
      Log.e(LOG_TAG, "ERROR_CONNECT", e);
      if (debugMessages == true)
        throw new YailRuntimeError("Connect error (Socket Creation, please check Ip or hostname -> )" + e.getMessage(), "Error");
    }
  }

  /**
   * Send data through the socket to the server
   */
  @SimpleFunction(description = "Send data to the server")
  public void SendData(final String data)
  {
    if (connectionState == false)
    {
      if (debugMessages == true)
        throw new YailRuntimeError("Send error, socket not connected.", "Error");
    }
    final byte [] dataToSend;
    byte [] dataCopy = data.getBytes();
    if (hexaStringMode == false)
    {
      //dataSend = new byte [data.length()];
      // if hexaStringMode is false, we send data as is
      dataToSend = dataCopy;
    }
    else
    {
      // if hexaStringMode is true, we begin to verify we can transcode the symbols
      // verify if the data we want to send contains only hexa symbols
      int i;
      for (i = 0; i < data.length(); i++)
      {
        if (((dataCopy[i] < 0x30) || (dataCopy[i] > 0x39)) && ((dataCopy[i] < 0x41) || (dataCopy[i] > 0x46)) && ((dataCopy[i] < 0x61) || (dataCopy[i] > 0x66)))
          if (debugMessages == true)
            throw new YailRuntimeError("Send data : hexaStringMode is selected and non hexa symbol found in send String.", "Error");
      }
      // verify that the number of symbols is even
      if ((data.length() %2) == 1)
      {
        if (debugMessages == true)
          throw new YailRuntimeError("Send data : hexaStringMode is selected and send String length is odd. Even number of characters needed.", "Error");
      }
      // if all tests pass, we transcode the data :
      dataToSend=new byte[data.length()/2+1];
      for (i = 0; i < data.length(); i=i+2)
      {
        byte [] temp1 = new byte [2];
        temp1 [0] = dataCopy[i];
        temp1 [1] = dataCopy[i+1];
        String temp2 = new String (temp1);
        dataToSend[i/2]=(byte)Integer.parseInt(temp2, 16);
      }
      // end of c-type string character
      dataToSend[i/2] = (byte)0x00;
    }

    // we then send asynchonously the data
    AsynchUtil.runAsynchronously(new Runnable()
    {
      @Override
      public void run()
      {
        try
        {
          OutputStream out;
          out = clientSocket.getOutputStream();
          out.write(dataToSend);
        }
        catch (SocketException e)
        {
          Log.e(LOG_TAG, "ERROR_SEND", e);
          if (debugMessages == true)
            throw new YailRuntimeError("Send data" + e.getMessage(), "Error");
        }
        catch (Exception e)
        {
          Log.e(LOG_TAG, "ERROR_UNABLE_TO_SEND_DATA", e);
          if (debugMessages == true)
            throw new YailRuntimeError("Send Data", "Error");
        }
      }
    } );
  }

  /**
   * Close the socket
   */
  @SimpleFunction(description = "Disconnect to the server")
  public void Disconnect()
  {
    if (connectionState == true)
    {
      connectionState = false;
      try
      {
        // shutdown the input socket,
        //clientSocket.shutdownInput();
        //clientSocket.shutdownOutput();
        clientSocket.close();
      }
      catch (SocketException e)
      {
        // modifications by axeley too :-)
        if(e.getMessage().indexOf("ENOTCONN") == -1)
        {
          Log.e(LOG_TAG, "ERROR_CONNECT", e);
          if (debugMessages == true)
            throw new YailRuntimeError("Disconnect" + e.getMessage(), "Error");
         }
         // if not connected, then just ignore the exception
      }
      catch (IOException e)
      {
        Log.e(LOG_TAG, "ERROR_CONNECT", e);
        if (debugMessages == true)
          throw new YailRuntimeError("Disconnect" + e.getMessage(), "Error");
      }
      catch (Exception e)
      {
        Log.e(LOG_TAG, "ERROR_CONNECT", e);
        if (debugMessages == true)
          throw new YailRuntimeError("Disconnect" + e.getMessage(), "Error");
      }
      finally
      {
        clientSocket=null;
      }

    }
    else
      if (debugMessages == true)
        throw new YailRuntimeError("Socket not connected, can't disconnect.", "Error");
  }

  /**
   * Event indicating that a message has been received
   *
   * @param data the data sent by the server
   */
  @SimpleEvent
  public void DataReceived(String data)
  {
    // invoke the application's "DataReceived" event handler.
    EventDispatcher.dispatchEvent(this, "DataReceived", data);
  }

  /**
   * Event indicating that the remote socket closed the connection
   *
   */
  @SimpleEvent
  public void RemoteConnectionClosed()
  {
    // invoke the application's "RemoteConnectionClosed" event handler.
    EventDispatcher.dispatchEvent(this, "RemoteConnectionClosed");
  }
}
