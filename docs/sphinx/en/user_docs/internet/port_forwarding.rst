.. _port_forwarding:

##################################
Creating a server for the internet
##################################

This document is provisory.  Please consult your system administrator for more information on how to do remote por forwarding safely.
Another alternative not discussed heree is setting up a VPN.

On Linux
--------

To make mrv2 accessible on the internet involves creating a ssh remote port forwarding to a router you have full access to (or to a relay server on the internet).

First, you need to install openssh in case you don't have it.  On Ubuntu or Debian::

    sudo apt-get install openssh-server


on Red Hat or Rocky Linux::


    sudo yum install openssh-server


On the machine that is going to be your server machine, you need to edit the SSH server configuration file "/etc/ssh/sshd_config" with a text editor such as nano or vi.

Add the following line at the end of the file (or change it if is already set)::


    GatewayPorts yes


Restart the SSH server to apply the changes::


    sudo systemctl restart sshd

On your router, make sure to open port 55150.  This varies from router to router, but it usually involves going to the page of the router with your browser at, usually, 192.168.0.1.  Then entering the router login and password to get access to it.

Now, on the machine running mrv2 as a server, create an SSH tunnel that forwards traffic from port 55150 on the server to port 55150 on the router::

    ssh -R 55150:localhost:55150 user@public-ip-of-router



On Windows
----------

To set up port forwarding on Windows, you can follow these steps:

1. Open the Windows Firewall with Advanced Security. You can search for it in the Windows Start menu or Control Panel.

2. Click on "Inbound Rules" in the left pane, and then click on "New Rule" in the right pane.

3. Select "Port" and click "Next".

4. Choose the protocol you want to use (TCP) and enter the port number you want to forward (55150).

5. Select "Allow the connection" and click "Next".

6. Choose when the rule should apply, and click "Next".

7. Give the rule a name and click "Finish".

8. Finally, configure your router to forward incoming traffic on the port you just opened to the IP address of your Windows computer.

.. note::

   These steps are general and may vary depending on the version of Windows you are using and the specific router you have. It's always a good idea to consult the documentation for your router and Windows operating system for more detailed instructions.
