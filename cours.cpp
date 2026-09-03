#include "cours.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QVariant>

Cours::Cours() : id(0), intitule(""), description(""), dureeHeures(0), niveau(""), idSalle(0)
{
}

Cours::Cours(int id, const QString &intitule, const QString &description,
             int dureeHeures, const QString &niveau, int idSalle)
    : id(id), intitule(intitule), description(description),
      dureeHeures(dureeHeures), niveau(niveau), idSalle(idSalle)
{
}

int Cours::getId() const { return id; }
void Cours::setId(int value) { id = value; }

QString Cours::getIntitule() const { return intitule; }
void Cours::setIntitule(const QString &value) { intitule = value; }

QString Cours::getDescription() const { return description; }
void Cours::setDescription(const QString &value) { description = value; }

int Cours::getDureeHeures() const { return dureeHeures; }
void Cours::setDureeHeures(int value) { dureeHeures = value; }

QString Cours::getNiveau() const { return niveau; }
void Cours::setNiveau(const QString &value) { niveau = value; }

int Cours::getIdSalle() const { return idSalle; }
void Cours::setIdSalle(int value) { idSalle = value; }

// ---------------------------------------------------------------------
// CRUD
// ---------------------------------------------------------------------

bool Cours::ajouter()
{
    QSqlQuery idQuery;
    idQuery.exec("SELECT NVL(MAX(id_cours), 100) + 1 FROM cours");
    int nouvelId = 1;
    if (idQuery.next())
        nouvelId = idQuery.value(0).toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO cours (id_cours, intitule, description, duree_heures, niveau, id_salle) "
                   "VALUES (:id, :intitule, :description, :duree, :niveau, :idSalle)");
    query.bindValue(":id", nouvelId);
    query.bindValue(":intitule", intitule);
    query.bindValue(":description", description.isEmpty() ? QVariant(QVariant::String) : description);
    query.bindValue(":duree", dureeHeures);
    query.bindValue(":niveau", niveau.isEmpty() ? QVariant(QVariant::String) : niveau);
    query.bindValue(":idSalle", idSalle > 0 ? QVariant(idSalle) : QVariant(QVariant::Int));

    if (!query.exec()) {
        qDebug() << "Erreur ajout cours:" << query.lastError().text();
        return false;
    }

    id = nouvelId;
    return true;
}

QSqlQueryModel *Cours::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT c.id_cours, c.intitule, c.description, c.duree_heures, "
                     "c.niveau, NVL(s.nom, '(non assignée)') "
                     "FROM cours c LEFT JOIN salle s ON c.id_salle = s.id_salle "
                     "ORDER BY c.id_cours");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur affichage cours:" << model->lastError().text();
    }

    return model;
}

bool Cours::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM cours WHERE id_cours = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur suppression cours:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Cours::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE cours SET intitule = :intitule, description = :description, "
                   "duree_heures = :duree, niveau = :niveau, id_salle = :idSalle "
                   "WHERE id_cours = :id");
    query.bindValue(":intitule", intitule);
    query.bindValue(":description", description.isEmpty() ? QVariant(QVariant::String) : description);
    query.bindValue(":duree", dureeHeures);
    query.bindValue(":niveau", niveau.isEmpty() ? QVariant(QVariant::String) : niveau);
    query.bindValue(":idSalle", idSalle > 0 ? QVariant(idSalle) : QVariant(QVariant::Int));
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur modification cours:" << query.lastError().text();
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------
// Métier : recherche + tri multicritères
// ---------------------------------------------------------------------

QSqlQueryModel *Cours::rechercherTrier(const QString &intitule, const QString &niveau,
                                        int dureeMin, int dureeMax,
                                        const QString &colonneTri, bool ascendant)
{
    static const QMap<QString, QString> colonnes = {
        {"Intitulé", "c.intitule"},
        {"Durée", "c.duree_heures"},
        {"Niveau", "c.niveau"},
        {"Salle", "s.nom"}
    };

    QString tri = colonnes.value(colonneTri, "c.id_cours");

    QString requete = "SELECT c.id_cours, c.intitule, c.description, c.duree_heures, "
                       "c.niveau, NVL(s.nom, '(non assignée)') "
                       "FROM cours c LEFT JOIN salle s ON c.id_salle = s.id_salle "
                       "WHERE LOWER(c.intitule) LIKE :intitule "
                       "AND (:niveau = '' OR c.niveau = :niveau) "
                       "AND NVL(c.duree_heures, 0) BETWEEN :dureeMin AND :dureeMax "
                       "ORDER BY " + tri + (ascendant ? " ASC" : " DESC");

    QSqlQuery query;
    query.prepare(requete);
    query.bindValue(":intitule", "%" + intitule.trimmed().toLower() + "%");
    query.bindValue(":niveau", niveau);
    query.bindValue(":dureeMin", dureeMin);
    query.bindValue(":dureeMax", dureeMax);

    QSqlQueryModel *model = new QSqlQueryModel();
    if (!query.exec()) {
        qDebug() << "Erreur recherche cours:" << query.lastError().text();
    }
    model->setQuery(std::move(query));

    return model;
}

QStringList Cours::listeNiveaux()
{
    QStringList niveaux;
    QSqlQuery query("SELECT DISTINCT niveau FROM cours WHERE niveau IS NOT NULL ORDER BY niveau");
    while (query.next())
        niveaux << query.value(0).toString();
    return niveaux;
}

// ---------------------------------------------------------------------
// Métier : statistiques (nombre de cours par niveau)
// ---------------------------------------------------------------------

QMap<QString, int> Cours::statParNiveau()
{
    QMap<QString, int> stats;
    QSqlQuery query("SELECT NVL(niveau, 'Non défini'), COUNT(*) FROM cours GROUP BY niveau ORDER BY 1");
    while (query.next())
        stats.insert(query.value(0).toString(), query.value(1).toInt());
    return stats;
}

QMap<QString, int> Cours::statAssignationSalle()
{
    QMap<QString, int> stats;
    stats.insert("Salle assignée", 0);
    stats.insert("Non assignée", 0);

    QSqlQuery query("SELECT CASE WHEN id_salle IS NULL THEN 0 ELSE 1 END, COUNT(*) "
                     "FROM cours GROUP BY CASE WHEN id_salle IS NULL THEN 0 ELSE 1 END");
    while (query.next()) {
        QString cle = query.value(0).toInt() == 1 ? "Salle assignée" : "Non assignée";
        stats[cle] = query.value(1).toInt();
    }
    return stats;
}

// ---------------------------------------------------------------------
// Métier au choix : dupliquer un cours
// ---------------------------------------------------------------------

bool Cours::dupliquer(int idSource)
{
    QSqlQuery lecture;
    lecture.prepare("SELECT intitule, description, duree_heures, niveau, id_salle "
                     "FROM cours WHERE id_cours = :id");
    lecture.bindValue(":id", idSource);
    if (!lecture.exec() || !lecture.next()) {
        qDebug() << "Erreur lecture cours à dupliquer:" << lecture.lastError().text();
        return false;
    }

    Cours copie;
    copie.setIntitule(lecture.value(0).toString() + " (copie)");
    copie.setDescription(lecture.value(1).toString());
    copie.setDureeHeures(lecture.value(2).toInt());
    copie.setNiveau(lecture.value(3).toString());
    copie.setIdSalle(lecture.value(4).isNull() ? 0 : lecture.value(4).toInt());

    return copie.ajouter();
}

// ---------------------------------------------------------------------
// Métier au choix : assigner automatiquement une salle disponible
// ---------------------------------------------------------------------

bool Cours::assignerSalleDisponible(int idCours, int capaciteRequise, QString *nomSalleAffectee)
{
    QSqlDatabase::database().transaction();

    QSqlQuery recherche;
    recherche.prepare(
        "SELECT * FROM ("
        "  SELECT id_salle, nom FROM salle "
        "  WHERE disponibilite = 1 AND capacite >= :capacite "
        "  ORDER BY capacite ASC"
        ") WHERE ROWNUM = 1");
    recherche.bindValue(":capacite", capaciteRequise > 0 ? capaciteRequise : 0);

    if (!recherche.exec() || !recherche.next()) {
        qDebug() << "Aucune salle disponible pour ce besoin:" << recherche.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    int idSalleTrouvee = recherche.value(0).toInt();
    QString nomSalle = recherche.value(1).toString();

    QSqlQuery majCours;
    majCours.prepare("UPDATE cours SET id_salle = :idSalle WHERE id_cours = :idCours");
    majCours.bindValue(":idSalle", idSalleTrouvee);
    majCours.bindValue(":idCours", idCours);

    QSqlQuery majSalle;
    majSalle.prepare("UPDATE salle SET disponibilite = 0 WHERE id_salle = :idSalle");
    majSalle.bindValue(":idSalle", idSalleTrouvee);

    if (!majCours.exec() || !majSalle.exec()) {
        qDebug() << "Erreur assignation salle:" << majCours.lastError().text() << majSalle.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }

    QSqlDatabase::database().commit();

    if (nomSalleAffectee)
        *nomSalleAffectee = nomSalle;

    return true;
}
