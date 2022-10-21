# Projekt IPK 1
Úkolem projektu bylo vytvořit server, který bude prostřednictvím protokolu HTTP poskytovat různé informace o serveru.

# Spuštění
Spolu se zdrojovým kódem je dodán makefile, pomocí kterého se příkazem _make_ vytvoří spustitelný soubor hinfosvc. Tento soubor následně spustíme s parametrem, který udává číslo portu, na kterém server poběží. 
_Příklad spuštění:_
>make
>./hinfosvc 12345

Po vypnutí serveru pomocí CTRL+C je možné použít _make clean_ pro vyčištění pracovního prostoru
## Funkce

- Získání doménového jména
- Získání informací o CPU
- Aktuální zátěž CPU

na server lze posílat dotazy pomocí webového prohlížeče nebo pomocí příkazu curl

#### _Příklady užití:_
-curl http://localhost:12345/hostname
-curl http://localhost:12345/cpu-name
-curl http://localhost:12345/load
