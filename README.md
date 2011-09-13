# Verteilte Datenstruktur mit _Subjects_
Implementierung einer verteilten Datenstruktur mit der _Subjects_ Simulationsumgebung.

## Installation
Ihr braucht:

1. [msysgit](http://code.google.com/p/msysgit/downloads/detail?name=Git-1.7.6-preview20110708.exe&can=2&q= "msysgit") zur Installation von Git unter Windows.
Bei der Installation von msysgit das Installieren der Shell-Extension deaktivieren (dafür nehmen wir TortoiseGit)

2. [TortoiseGit] (http://code.google.com/p/tortoisegit/ "TortoiseGit") zur Integrierung von Git im Kontextmenü von Windows (kennt ihr ja von TortoiseSVN)

3. Meldet euch bei mir, dass ich euch zum Projekt hinzufüge

4. Fertig!

## Kompilieren
Das Kompilieren benötigt den GCC in einer möglichst neuen Version (getestet mit 4.6.1).
Unter Windows sollte eine neue Version von MingW oder Cygwin funktionieren.

Das Kommando um mittels des Makefiles zu kompilieren lautet:
```make config=release``` oder ```make config=debug```

In Eclipse CDT gibt es die Möglichkeit ein "Makefile Projekt" zu erstellen. Damit wird das Makefile eingebunden und statt der internen Konfiguration zum Kompilieren verwendet.