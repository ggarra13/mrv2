As of v1.6.7 of vmrv2, there's basic support for Autodesk Flow.  You are supposed to improve upon it based on your facility needs.

By default, vmrv2 and mrv2 come with the script uninstalled and without the shotgrid api.

$INSTALL here refers to the install root location of mrv2 or vmrv2.

You should install it with:

    $INSTALL/bin/python -m pip install shotgun_api3

Copy the $INSTALL/python/plug-ins/demos/flow-bridge.py $INSTALL/python/plug-ins

In that file, change the FLOW constants:
       FLOW_URL		- Your studio project
       FLOW_SCRIPT_NAME	- Your studio's script created for mrv2 or vmrv2
       			  (Yes!  You need to create a script in Flow before you
			  	 can use it)
       FLOW_API_KEY	- Your studio's Flow key.
