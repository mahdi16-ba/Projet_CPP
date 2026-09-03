#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Connection c;
    bool test = c.createconnect();

    MainWindow w;

    if (test) {
        w.show();
    } else {
        // Le driver Oracle n'est pas chargé: on ne bloque plus l'application
        // pour éviter d'afficher la fenêtre d'erreur "Driver not loaded".
        qWarning() << "Connexion Oracle indisponible au démarrage: le driver n'est pas chargé.";
        w.show();
    }

    return a.exec();
}