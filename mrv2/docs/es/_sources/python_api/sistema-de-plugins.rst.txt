###################
Sistema de Plug-ins
###################

Plug-ins
--------

mrv2 suporta plug-ins de python para agregar entradas a los menus o
incluso crear nuevas entradas.
Esto te permite agregar comandos y clases a mrv2, yendo más allá de lo que la
consola de Python te permite.

Para usar los plug-ins, debes definir la variable de entorno::

     MRV2_PYTHON_PLUGINS

con una lista de directorios separados por dos puntos (Linux or macOS) o
semi-comas (Windows) indicando dónde residen los plug-ins.

Allí, archivos comunes de python (.py) deben tener esta estructura::

    import mrv2
    from mrv2 import plugin, timeline
  
    class HolaPlugin(plugin.Plugin):
        def __init__(self):
	    super().__init__()
	    pass
	    
        def hola(self):
            print("Hola desde un plugin de python")
       
	"""
	Esta función crea un nuevo menu llamado Menu con dos entradas.
	Una llamada Hola con una línea divisoria que llama al método self.hola
	y una llamada Reproducir que ejecuta la reproducción del clip actual.
	"""
        def menus(self):
            menu = { "Menu/Hola" : (self.hola, "__divider__") },
                     "Menu/Reproducir" : timeline.playForwards }
            return menu

Se puede definir más de un plug-in por achivo de Python.
Para un ejemplo más completo, refiérase a python/plug-ins/mrv2_hello.py en la
distribución de mrv2.
