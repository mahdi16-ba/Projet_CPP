@echo off
setlocal

REM =========================================================
REM Script de lancement pour Qt + Oracle Instant Client
REM =========================================================

REM Ajuste ces chemins selon ton installation locale
set ORA_CLIENT=C:\oracle\instantclient_19_11
set QT_BIN=C:\Qt\6.7.3\mingw_64\bin
set APP_DIR=C:\Users\USER\OneDrive\Bureau\Projet_CPP\build\Desktop_Qt_6_7_3_MinGW_64_bit_Debug

REM 1) Ajouter Oracle Instant Client au PATH
set PATH=%ORA_CLIENT%;%PATH%

REM 2) Ajouter le dossier Qt bin si besoin
set PATH=%QT_BIN%;%PATH%

REM 3) Copier les DLL Oracle dans le dossier de sortie du binaire
if exist "%ORA_CLIENT%\*.dll" (
    copy /Y "%ORA_CLIENT%\*.dll" "%APP_DIR%\" >nul
)

REM 4) Créer le dossier plugins/sqldrivers s'il n'existe pas
if not exist "%APP_DIR%\plugins\sqldrivers" (
    mkdir "%APP_DIR%\plugins\sqldrivers"
)

REM 5) Copier le plugin QOCI dans plugins/sqldrivers
if exist "C:\Qt\6.7.3\mingw_64\plugins\sqldrivers\qsqloci.dll" (
    copy /Y "C:\Qt\6.7.3\mingw_64\plugins\sqldrivers\qsqloci.dll" "%APP_DIR%\plugins\sqldrivers\" >nul
)

REM 6) Lancer l'application
cd /d "%APP_DIR%"
start "Projet_CPP" "Projet_CPP.exe"

endlocal
