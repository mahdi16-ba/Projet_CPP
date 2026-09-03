#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QStringList>
#include <QList>

class QWidget;

// Génération de documents PDF personnalisés (métier commun aux modules
// Cours et Salle) : construit un document HTML mis en page (titre, date,
// tableau de données, pied de page) puis l'imprime dans un fichier PDF.
// Ce n'est pas un simple "imprime écran" : le contenu est entièrement
// recomposé à partir des données passées en paramètre.
class Document
{
public:
    static bool genererListePdf(QWidget *parent,
                                 const QString &titre,
                                 const QStringList &entetes,
                                 const QList<QStringList> &lignes,
                                 const QString &nomFichierParDefaut);
};

#endif // DOCUMENT_H
