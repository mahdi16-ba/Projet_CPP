#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QSqlQueryModel>
#include <QtCharts/QChartView>

#include "cours.h"
#include "salle.h"

class QLineEdit;
class QSpinBox;
class QComboBox;
class QTableView;
class QLabel;
class QCheckBox;
class QPlainTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // --- Onglet Cours ---
    void onCoursAjouter();
    void onCoursModifier();
    void onCoursSupprimer();
    void onCoursDupliquer();
    void onCoursAssignerSalle();
    void onCoursGenererPdf();
    void onCoursTableClicked(const QModelIndex &index);
    void onCoursFiltrerChange();
    void onCoursEffacerFiltres();

    // --- Onglet Salle ---
    void onSalleAjouter();
    void onSalleModifier();
    void onSalleSupprimer();
    void onSalleBasculerDispo();
    void onSalleGenererPdf();
    void onSalleTableClicked(const QModelIndex &index);
    void onSalleFiltrerChange();
    void onSalleEffacerFiltres();

private:
    // --- Onglet Cours : widgets ---
    int coursIdSelectionne = -1;
    QLineEdit      *coursIntituleEdit;
    QPlainTextEdit *coursDescriptionEdit;
    QSpinBox       *coursDureeSpin;
    QComboBox      *coursNiveauCombo;
    QComboBox      *coursSalleCombo;

    QLineEdit  *coursFiltreIntitule;
    QComboBox  *coursFiltreNiveau;
    QSpinBox   *coursFiltreDureeMin;
    QSpinBox   *coursFiltreDureeMax;
    QComboBox  *coursTriCombo;
    QCheckBox  *coursTriDescCheck;

    QTableView *coursTable;
    QLabel     *coursTotalLabel;
    QLabel     *coursStatutLabel;
    QChartView *coursChartViewNiveau;
    QChartView *coursChartViewAssignation;

    QSqlQueryModel *coursModel = nullptr;

    // --- Onglet Salle : widgets ---
    int salleIdSelectionne = -1;
    QLineEdit *salleNomEdit;
    QSpinBox  *salleCapaciteSpin;
    QComboBox *salleTypeCombo;
    QCheckBox *salleDisponibleCheck;

    QLineEdit *salleFiltreNom;
    QComboBox *salleFiltreType;
    QSpinBox  *salleFiltreCapMin;
    QSpinBox  *salleFiltreCapMax;
    QComboBox *salleFiltreDispo;
    QComboBox *salleTriCombo;
    QCheckBox *salleTriDescCheck;

    QTableView *salleTable;
    QLabel     *salleTotalLabel;
    QLabel     *salleStatutLabel;
    QChartView *salleChartViewType;
    QChartView *salleChartViewDispo;

    QSqlQueryModel *salleModel = nullptr;

    // --- Construction de l'interface ---
    QWidget *creerOngletCours();
    QWidget *creerOngletSalle();
    void appliquerStyleGlobal();

    // --- Rafraîchissement ---
    void rafraichirCours();
    void rafraichirSalle();
    void rafraichirStatsCours();
    void rafraichirStatsSalle();
    void rafraichirComboSalles();
    void viderFormulaireCours();
    void viderFormulaireSalle();

    // --- Contrôles de saisie ---
    // Valide le formulaire concerné ; remplit `erreurs` avec un message par
    // champ invalide et surligne ces champs en rouge. Retourne false s'il y
    // a au moins une erreur.
    bool validerFormulaireCours(QStringList &erreurs);
    bool validerFormulaireSalle(QStringList &erreurs);
    void marquerChamp(QWidget *champ, bool enErreur);
};

#endif // MAINWINDOW_H
