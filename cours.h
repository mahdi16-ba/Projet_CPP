#ifndef COURS_H
#define COURS_H

#include <QString>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>

// Entité Cours : mappée sur la table COURS
// (ID_COURS, INTITULE, DESCRIPTION, DUREE_HEURES, NIVEAU, ID_SALLE -> FK SALLE)
class Cours
{
private:
    int id;
    QString intitule;
    QString description;
    int dureeHeures;
    QString niveau;
    int idSalle; // 0 = aucune salle assignée (colonne nullable en base)

public:
    Cours();
    Cours(int id, const QString &intitule, const QString &description,
          int dureeHeures, const QString &niveau, int idSalle);

    int getId() const;
    void setId(int value);

    QString getIntitule() const;
    void setIntitule(const QString &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    int getDureeHeures() const;
    void setDureeHeures(int value);

    QString getNiveau() const;
    void setNiveau(const QString &value);

    int getIdSalle() const;
    void setIdSalle(int value);

    // --- CRUD (fonctionnalités de base) ---
    bool ajouter();
    QSqlQueryModel *afficher();
    bool supprimer(int id);
    bool modifier();

    // --- Métier 1 : recherche + tri multicritères (>= 3 critères) ---
    // Critères : intitulé (contient), niveau (exact, vide = tous),
    // durée horaire (plage min/max). Tri : colonne + ordre.
    QSqlQueryModel *rechercherTrier(const QString &intitule,
                                     const QString &niveau,
                                     int dureeMin, int dureeMax,
                                     const QString &colonneTri,
                                     bool ascendant);

    // Liste distincte des niveaux existants (pour peupler le filtre)
    QStringList listeNiveaux();

    // --- Métier 2 : statistiques (répartition du nombre de cours par niveau) ---
    QMap<QString, int> statParNiveau();

    // Statistique complémentaire : cours avec/sans salle assignée
    QMap<QString, int> statAssignationSalle();

    // --- Métier 4 (au choix) : dupliquer un cours existant ---
    bool dupliquer(int idSource);

    // --- Métier 5 (au choix) : assigner automatiquement une salle disponible ---
    // Choisit la salle disponible dont la capacité est la plus proche du besoin
    // (>= capaciteRequise si possible), l'assigne au cours et la marque occupée.
    bool assignerSalleDisponible(int idCours, int capaciteRequise, QString *nomSalleAffectee = nullptr);
};

#endif // COURS_H
