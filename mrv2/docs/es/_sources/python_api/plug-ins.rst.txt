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

    class Plugin:
        def hola(self):
            print("¡Hola desde un python plug-in!")
       
        def menus(self):
            menu = { ("Menu/Hola", self.hola) }
            return menu

Para un ejemplo más completo, refiérase a python/plug-ins/mrv2_hello.py en la
distribución de mrv2.
