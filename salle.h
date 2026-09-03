#ifndef SALLE_H
#define SALLE_H

#include <QString>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>

// Entité Salle : mappée sur la table SALLE
// (ID_SALLE, NOM, CAPACITE, TYPE_SALLE, DISPONIBILITE 0/1)
class Salle
{
private:
    int id;
    QString nom;
    int capacite;
    QString typeSalle;
    bool disponibilite;

public:
    Salle();
    Salle(int id, const QString &nom, int capacite, const QString &typeSalle, bool disponibilite);

    int getId() const;
    void setId(int value);

    QString getNom() const;
    void setNom(const QString &value);

    int getCapacite() const;
    void setCapacite(int value);

    QString getTypeSalle() const;
    void setTypeSalle(const QString &value);

    bool getDisponibilite() const;
    void setDisponibilite(bool value);

    // --- CRUD (fonctionnalités de base) ---
    bool ajouter();
    QSqlQueryModel *afficher();
    bool supprimer(int id);
    bool modifier();

    // --- Métier 1 : recherche + tri multicritères (>= 3 critères) ---
    // Critères : nom (contient), type de salle (exact, vide = tous),
    // capacité (plage min/max), disponibilité (-1 = toutes, 0/1).
    QSqlQueryModel *rechercherTrier(const QString &nom, const QString &typeSalle,
                                     int capaciteMin, int capaciteMax, int disponibilite,
                                     const QString &colonneTri, bool ascendant);

    QStringList listeTypes();

    // Libellés "id -> Nom (capacité, statut)" pour peupler une liste déroulante
    // (utilisé par le module Cours pour choisir la salle assignée).
    QMap<int, QString> listeLibelles();

    // --- Métier 2 : statistiques (répartition du nombre de salles par type) ---
    QMap<QString, int> statParType();

    // Statistique complémentaire : disponibles vs indisponibles
    QMap<QString, int> statDisponibilite();

    // --- Métier 4 (au choix) : basculer la disponibilité d'une salle ---
    bool basculerDisponibilite(int idSalle);

    // --- Métier 5 (au choix) : suggérer les salles adaptées à une capacité voulue ---
    QSqlQueryModel *suggererPourCapacite(int capaciteMin);
};

#endif // SALLE_H
