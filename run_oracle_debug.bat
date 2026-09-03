@echo off
setlocal

REM -------------------------
REM Ajuster selon ton installation Oracle Instant Client
REM -------------------------
set ORA_CLIENT=C:\oracle\instantclient_19_11

REM Ajouter le dossier Oracle Instant Client au PATH
set PATH=%ORA_CLIENT%;%PATH%

REM Si tu as Qt installé ailleurs, ajoute aussi le dossier Qt bin au PATH si besoin
REM Exemple :
REM set PATH=C:\Qt\6.7.3\mingw_64\bin;%PATH%

REM Dossier du binaire de ton projet
set APP_DIR=C:\Users\USER\OneDrive\Bureau\Projet_CPP\build\Desktop_Qt_6_7_3_MinGW_64_bit_Debug

cd /d "%APP_DIR%"

REM Lancer l'exécutable du projet
start "Projet_CPP" "Projet_CPP.exe"

endlocal
