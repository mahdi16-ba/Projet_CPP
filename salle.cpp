#include "salle.h"

#include <QDebug>
#include <QVariant>

Salle::Salle() : id(0), nom(""), capacite(0), typeSalle(""), disponibilite(true)
{
}

Salle::Salle(int id, const QString &nom, int capacite, const QString &typeSalle, bool disponibilite)
    : id(id), nom(nom), capacite(capacite), typeSalle(typeSalle), disponibilite(disponibilite)
{
}

int Salle::getId() const { return id; }
void Salle::setId(int value) { id = value; }

QString Salle::getNom() const { return nom; }
void Salle::setNom(const QString &value) { nom = value; }

int Salle::getCapacite() const { return capacite; }
void Salle::setCapacite(int value) { capacite = value; }

QString Salle::getTypeSalle() const { return typeSalle; }
void Salle::setTypeSalle(const QString &value) { typeSalle = value; }

bool Salle::getDisponibilite() const { return disponibilite; }
void Salle::setDisponibilite(bool value) { disponibilite = value; }

// ---------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------

bool Salle::ajouter()
{
    QSqlQuery idQuery;
    idQuery.exec("SELECT NVL(MAX(id_salle), 0) + 1 FROM salle");
    int nouvelId = 1;
    if (idQuery.next())
        nouvelId = idQuery.value(0).toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO salle (id_salle, nom, capacite, type_salle, disponibilite) "
                   "VALUES (:id, :nom, :capacite, :type, :dispo)");
    query.bindValue(":id", nouvelId);
    query.bindValue(":nom", nom);
    query.bindValue(":capacite", capacite > 0 ? QVariant(capacite) : QVariant(QVariant::Int));
    query.bindValue(":type", typeSalle);
    query.bindValue(":dispo", disponibilite ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "Erreur ajout salle:" << query.lastError().text();
        return false;
    }

    id = nouvelId;
    return true;
}

QSqlQueryModel *Salle::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT id_salle, nom, capacite, type_salle, "
                     "CASE disponibilite WHEN 1 THEN 'Disponible' ELSE 'Occupée' END "
                     "FROM salle ORDER BY id_salle");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur affichage salle:" << model->lastError().text();
    }

    return model;
}

bool Salle::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM salle WHERE id_salle = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression salle:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Salle::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE salle SET nom = :nom, capacite = :capacite, "
                   "type_salle = :type, disponibilite = :dispo WHERE id_salle = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":capacite", capacite > 0 ? QVariant(capacite) : QVariant(QVariant::Int));
    query.bindValue(":type", typeSalle);
    query.bindValue(":dispo", disponibilite ? 1 : 0);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur modification salle:" << query.lastError().text();
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------
// Métier : recherche + tri multicritères
// ---------------------------------------------------------------------

QSqlQueryModel *Salle::rechercherTrier(const QString &nom, const QString &typeSalle,
                                        int capaciteMin, int capaciteMax, int disponibilite,
                                        const QString &colonneTri, bool ascendant)
{
    static const QMap<QString, QString> colonnes = {
        {"Nom", "nom"},
        {"Capacité", "capacite"},
        {"Type", "type_salle"},
        {"Disponibilité", "disponibilite"}
    };

    QString tri = colonnes.value(colonneTri, "id_salle");

    // NB : en Oracle, une chaîne vide liée en paramètre équivaut à NULL, donc
    // "NULL = ''" ne vaut jamais vrai. On ne construit la condition type que
    // lorsqu'un filtre est réellement demandé (le filtre disponibilité, lui,
    // compare des entiers : pas de piège NULL/'' pour -1).
    QString requete = "SELECT id_salle, nom, capacite, type_salle, "
                       "CASE disponibilite WHEN 1 THEN 'Disponible' ELSE 'Occupée' END "
                       "FROM salle "
                       "WHERE LOWER(nom) LIKE :nom "
                       "AND NVL(capacite, 0) BETWEEN :capMin AND :capMax "
                       "AND (:dispo = -1 OR disponibilite = :dispo) ";

    QString typeTrimme = typeSalle.trimmed();
    if (!typeTrimme.isEmpty())
        requete += "AND type_salle = :type ";

    requete += "ORDER BY " + tri + (ascendant ? " ASC" : " DESC");

    QSqlQuery query;
    query.prepare(requete);
    query.bindValue(":nom", "%" + nom.trimmed().toLower() + "%");
    query.bindValue(":capMin", capaciteMin);
    query.bindValue(":capMax", capaciteMax);
    query.bindValue(":dispo", disponibilite);
    if (!typeTrimme.isEmpty())
        query.bindValue(":type", typeTrimme);

    QSqlQueryModel *model = new QSqlQueryModel();
    if (!query.exec()) {
        qDebug() << "Erreur recherche salle:" << query.lastError().text();
    }
    model->setQuery(std::move(query));

    return model;
}

QStringList Salle::listeTypes()
{
    QStringList types;
    QSqlQuery query("SELECT DISTINCT type_salle FROM salle WHERE type_salle IS NOT NULL ORDER BY type_salle");
    while (query.next())
        types << query.value(0).toString();
    return types;
}

QMap<int, QString> Salle::listeLibelles()
{
    QMap<int, QString> libelles;
    QSqlQuery query("SELECT id_salle, nom, capacite, disponibilite FROM salle ORDER BY nom");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nom = query.value(1).toString();
        int capacite = query.value(2).toInt();
        bool dispo = query.value(3).toInt() == 1;
        libelles.insert(id, QString("%1 (%2 pl. — %3)")
                                 .arg(nom).arg(capacite).arg(dispo ? "Disponible" : "Occupée"));
    }
    return libelles;
}

// ---------------------------------------------------------------------
// Métier : statistiques
// ---------------------------------------------------------------------

QMap<QString, int> Salle::statParType()
{
    QMap<QString, int> stats;
    QSqlQuery query("SELECT NVL(type_salle, 'Non défini'), COUNT(*) FROM salle GROUP BY type_salle ORDER BY 1");
    while (query.next())
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    return stats;
}

QMap<QString, int> Salle::statDisponibilite()
{
    QMap<QString, int> stats;
    stats.insert("Disponible", 0);
    stats.insert("Occupée", 0);

    QSqlQuery query("SELECT disponibilite, COUNT(*) FROM salle GROUP BY disponibilite");
    while (query.next()) {
        QString cle = query.value(0).toInt() == 1 ? "Disponible" : "Occupée";
        stats[cle] = query.value(1).toInt();
    }
    return stats;
}

// ---------------------------------------------------------------------
// Métier au choix : basculer la disponibilité
// ---------------------------------------------------------------------

bool Salle::basculerDisponibilite(int idSalle)
{
    QSqlQuery query;
    query.prepare("UPDATE salle SET disponibilite = CASE disponibilite WHEN 1 THEN 0 ELSE 1 END "
                   "WHERE id_salle = :id");
    query.bindValue(":id", idSalle);

    if (!query.exec()) {
        qDebug() << "Erreur bascule disponibilité:" << query.lastError().text();
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------
// Métier au choix : suggérer des salles adaptées à une capacité
// ---------------------------------------------------------------------

QSqlQueryModel *Salle::suggererPourCapacite(int capaciteMin)
{
    QSqlQuery query;
    query.prepare("SELECT id_salle, nom, capacite, type_salle, "
                   "CASE disponibilite WHEN 1 THEN 'Disponible' ELSE 'Occupée' END "
                   "FROM salle WHERE disponibilite = 1 AND capacite >= :capacite "
                   "ORDER BY capacite ASC");
    query.bindValue(":capacite", capaciteMin > 0 ? capaciteMin : 0);

    QSqlQueryModel *model = new QSqlQueryModel();
    if (!query.exec()) {
        qDebug() << "Erreur suggestion salle:" << query.lastError().text();
    }
    model->setQuery(std::move(query));

    return model;
}
