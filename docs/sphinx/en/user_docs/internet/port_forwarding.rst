.. _port_forwarding:

##################################
Creating a server for the internet
##################################

Allowing mrv2 to communicate on the internet means having a server with access to it and the use of remote ssh port forwarding to encode the transmission of the data.

In this tutorial, we will use:

https://ngrok.com/

to set up a free basic one-to-one connection on the internet.

If you need multiple users connecting to the mrv2 acting as a server, you will need to apply for a paid plan of ngrok or use another server provider.

Note that mrv2 does not transmit any movie or picture information when connected on the network.  Only commands are transmitted.  Each user at each end should download and load the files manually before connecting (or put them in a shared directory at each end and use path mapping).

How to setup ngrok
------------------

First, you should download ngrok from:

https://ngrok.com/download

and unzip on Windows or extract the tar file on Unix systems.

You should then place the ngrok executable in your path, like:

C:/Windows/System32 on Windows

/usr/local/bin      on macOS or Linux


Signing to the ngrok service
----------------------------

Fill the form at:

https://dashboard.ngrok.com/signup

Once you log in, you need to set up your Authtoken.

If you go to:

https://dashboard.ngrok.com/get-started/setup

It will list your auth token.  You need to add it ngrok, like::

   ngrok config add-authtoken <auth_token_id>

And with that you are done.


Starting a mrv2 server
----------------------

Fire up mrv2 and go to Panel->Network and set it up as a Server from the Client/Server Type selection.  For this example, we will leave it at its default port of 55150.

mrv2 will start listening for connections on port 55150.

From the terminal (on Linux or macOS) or from cmd.exe on Windows, run::

    ngrok tcp 55150

That will start the port forwarding.  You will see a line, like:

Forwarding                    tcp://0.tcp.sa.ngrok.io:12489 -> localhost:55150

You should provide the tcp://* address to your client.


Starting mrv2 as a client
-------------------------

Fire up mrv2 on the client (remote) machine and use Panel->Network. Leave it as a client.  Enter the tcp address (tcp://0.tcp.sa.ngrok.io:12489 in our example)  as the host name.  You can use CTRL+C to copy the address from an email, for example, and paste it in the Host input widget with CTRL+V.  Click on Connect.

With that mrv2 client will sync with with the server mrv2.  The server will send the list of files that it has loaded and the client will try to load them or match the basename of the file in case the files are already loaded in the mrv2 running as a client.

And with that both the server and the client will be synced.
