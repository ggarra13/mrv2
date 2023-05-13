.. _port_forwarding:

##################################
Creating a server for the internet
##################################


Setting up a Linux internet server for remote SSH forwarding
------------------------------------------------------------

Creating a server of mrv2 to be accessible on the internet involves creating a ssh remote port forwarding on a server sitting on the internet (internet server form now on).  We assume this internet server is a Linux machine.  This will allow one mrv2 running as a server and one mrv2 running as a client to connect to each other.

First, you need to install openssh in case you don't have it on your internet server machine.  On Ubuntu or Debian::

    sudo apt-get install openssh-server

on Red Hat or Rocky Linux::

    sudo yum install openssh-server

Then you need to edit the SSH server configuration file "/etc/ssh/sshd_config" with a text editor such as nano or vi.  Look for the setting called 'AllowedTcpForwarding' and set it to yes::

    AllowTcpForwarding yes.

You should also need to modify the /etc/ssh/ssh_config file to add or ensure that it is set to::

    GatewayPorts yes
   
Restart the SSH server to apply the changes::

    sudo systemctl restart sshd
   
On Linux or macOS
-----------------

Now, on the machine where mrv2 will run as a server, you need to modify the /etc/ssh/ssh_config file to add or ensure that it is set to::

   GatewayPorts yes


Now, on the server machine, create an SSH tunnel that forwards traffic from port 443 on the server to port 55150 on the local machine::


   ssh -R 443:localhost:55150 public-ip-of-internet-server


On Windows
----------

To do remote port forwarding on Windows, you can use the SSH client called "PuTTY." PuTTY is a popular SSH client for Windows that provides a graphical interface for configuring and establishing SSH connections.

Here's how you can perform remote port forwarding using PuTTY:

1. Download and install PuTTY from the official website: https://www.chiark.greenend.org.uk/~sgtatham/putty/

2. Launch PuTTY and enter the hostname or IP address of the SSH server in the "Host Name (or IP address)" field.

3. Under the "Connection" category on the left-hand side, expand the "SSH" option and click on "Tunnels."

4. In the "Add new forwarded port" section, enter the source port and destination in the following format:
   
    <Source Port> - Enter the local port on your Windows machine that you want to forward.  For our example, 443.
    
    <Destination> - Enter the destination address and port in the format destination_address:port. For example, localhost:55150 to forward to localhost on port 55150.

5. Select the "Remote" option and click the "Add" button.

6. Verify that the added port forwarding configuration appears in the "Forwarded ports" list.

7. Navigate back to the "Session" category and enter a name for the session in the "Saved Sessions" field. Click the "Save" button to save the session for future use.

8. Click the "Open" button to establish the SSH connection with port forwarding.

Once the SSH connection is established with the remote port forwarding configuration, any connections made to the specified source port on your Windows machine will be forwarded to the specified destination address and port on the SSH server.

Make sure the SSH server allows remote port forwarding and has the necessary configuration for GatewayPorts if you want to access the forwarded port from other machines.
