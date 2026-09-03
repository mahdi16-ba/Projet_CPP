#include "connection.h"
#include <QDebug>
#include <QCoreApplication>
#include <QSysInfo>
#include <QLibraryInfo>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>

Connection::Connection() {}

bool Connection::createconnect() {

    // --- Diagnostic ---
    QString appPath   = QCoreApplication::applicationFilePath();
    QString arch      = QSysInfo::currentCpuArchitecture();
    QString archHuman = arch.contains("64") ? "64-bit" : "32-bit";
    qDebug() << "Application path:"      << appPath;
    qDebug() << "Architecture:"          << arch << "(" << archHuman << ")";
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();
    qDebug() << "Qt plugins path:"       << QLibraryInfo::path(QLibraryInfo::PluginsPath);

    // Fichier de log
    QString logPath = QCoreApplication::applicationDirPath() + QDir::separator() + "db_diag.log";
    auto appendLog = [&](const QString &line) {
        QFile f(logPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&f);
            out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << line << "\n";
            f.close();
        }
    };
    appendLog(QString("Arch: %1  Drivers: %2").arg(arch).arg(QSqlDatabase::drivers().join(",")));

    // ---------------------------------------------------------------
    // Stratégie 1 : QODBC via le DSN "OracleXE_PROJET_CPP"
    // (driver Oracle ODBC sqora32.dll 64-bit déjà installé avec Oracle 11.2 XE)
    // ---------------------------------------------------------------
    if (QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "Trying QODBC with DSN OracleXE_PROJET_CPP...";
        appendLog("Trying QODBC with DSN OracleXE_PROJET_CPP");

        // Connexion unique enregistrée comme connexion PAR DEFAUT de Qt
        // (QSqlDatabase::defaultConnection) : tous les QSqlQuery des classes
        // métier (Cours, Salle, ...) l'utilisent automatiquement sans avoir
        // à préciser de nom de connexion.
        if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName("OracleXE_PROJET_CPP");
        db.setUserName("PROJET_CPP");
        db.setPassword("pass123");

        if (db.open()) {
            qDebug() << "QODBC: connexion reussie via DSN OracleXE_PROJET_CPP";
            appendLog("QODBC: connexion reussie");
            return true;
        } else {
            QString err = QString("QODBC DSN failed: %1").arg(db.lastError().text());
            qDebug() << err;
            appendLog(err);

            // Tentative QODBC sans DSN (connexion directe par chaîne de connexion)
            QStringList connStrings = {
                "DRIVER={Oracle in XE};DBQ=XE;UID=PROJET_CPP;PWD=pass123;",
                "DRIVER={Oracle in XE};SERVER=localhost:1521/XE;UID=PROJET_CPP;PWD=pass123;"
            };
            for (const QString &cs : connStrings) {
                db.setDatabaseName(cs);
                qDebug() << "Trying QODBC direct:" << cs;
                appendLog(QString("Trying QODBC direct: %1").arg(cs));
                if (db.open()) {
                    qDebug() << "QODBC direct: connexion reussie";
                    appendLog("QODBC direct: connexion reussie");
                    return true;
                } else {
                    appendLog(QString("QODBC direct failed: %1").arg(db.lastError().text()));
                    qDebug() << "QODBC direct failed:" << db.lastError().text();
                }
            }
        }
    } else {
        qDebug() << "QODBC non disponible";
        appendLog("QODBC non disponible");
    }

    // ---------------------------------------------------------------
    // Stratégie 2 : QOCI natif (si disponible et compilé pour Qt 6.7.3)
    // ---------------------------------------------------------------
    if (QSqlDatabase::isDriverAvailable("QOCI")) {
        qDebug() << "Trying QOCI native...";
        appendLog("Trying QOCI native");

        if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
            QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

        db = QSqlDatabase::addDatabase("QOCI");
        db.setHostName("localhost");
        db.setPort(1521);
        db.setUserName("PROJET_CPP");
        db.setPassword("pass123");

        // Oracle 11.2 XE utilise le SID "XE"
        for (const QString &name : QStringList{"XE", "//localhost:1521/XE", "//127.0.0.1:1521/XE"}) {
            db.setDatabaseName(name);
            qDebug() << "QOCI trying:" << name;
            appendLog(QString("QOCI trying: %1").arg(name));
            if (db.open()) {
                qDebug() << "QOCI: connexion reussie avec" << name;
                appendLog(QString("QOCI: connexion reussie avec %1").arg(name));
                return true;
            } else {
                appendLog(QString("QOCI failed [%1]: %2").arg(name).arg(db.lastError().text()));
                qDebug() << "QOCI failed [" << name << "]:" << db.lastError().text();
            }
        }
    } else {
        qDebug() << "QOCI non disponible - qsqloci.dll non charge";
        appendLog("QOCI non disponible - qsqloci.dll manquant ou dependances absentes");
    }

    appendLog("Toutes les tentatives de connexion ont echoue.");
    return false;
}

void Connection::closeConnection() {
    if (db.isOpen())
        db.close();
}