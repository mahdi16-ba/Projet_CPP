#include "mainwindow.h"
#include "document.h"

#include <QTabWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QStatusBar>
#include <QStyle>
#include <QMessageBox>
#include <QSqlError>
#include <QPainter>
#include <QColor>
#include <QBrush>

#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

namespace {

// Libellé de champ de formulaire ; les champs obligatoires portent un
// astérisque rouge, cohérent avec la légende "* champ obligatoire" du
// formulaire.
QLabel *creerLabelChamp(const QString &texte, bool obligatoire, QWidget *parent = nullptr)
{
    QString html = obligatoire ? texte + " <span style='color:#f87171;'>*</span>" : texte;
    QLabel *label = new QLabel(html, parent);
    label->setObjectName("champLabel");
    return label;
}

QLabel *creerLegendeObligatoire(QWidget *parent)
{
    QLabel *label = new QLabel("* champ obligatoire", parent);
    label->setObjectName("aideChamp");
    return label;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Gestion de Centre de Formation — Cours & Salles");
    resize(1300, 800);

    appliquerStyleGlobal();

    QTabWidget *onglets = new QTabWidget(this);
    onglets->addTab(creerOngletCours(), "Gestion des Cours");
    onglets->addTab(creerOngletSalle(), "Gestion des Salles");
    setCentralWidget(onglets);

    connect(onglets, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0)
            rafraichirComboSalles();
    });

    rafraichirSalle();
    rafraichirComboSalles(); // le combo "Salle assignée" du formulaire Cours
                              // n'est sinon peuplé qu'au changement d'onglet
    rafraichirCours();

    statusBar()->showMessage("Prêt");
}

MainWindow::~MainWindow()
{
}

void MainWindow::appliquerStyleGlobal()
{
    setStyleSheet(R"(
        QMainWindow, QTabWidget::pane {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #0f172a, stop:1 #111827);
        }
        QWidget { color: #e5e7eb; }
        QTabBar::tab {
            background: #1e293b;
            color: #cbd5e1;
            padding: 10px 18px;
            font-weight: 700;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        QTabBar::tab:selected { background: #2563eb; color: white; }
        QGroupBox {
            background: rgba(15, 23, 42, 180);
            border: 1px solid #334155;
            border-radius: 12px;
            margin-top: 10px;
            padding: 12px;
            font-weight: 700;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 16px;
            padding: 0 8px;
            color: #93c5fd;
        }
        QLabel { color: #dbeafe; font-size: 12px; font-weight: 400; }
        QLineEdit, QSpinBox, QComboBox, QPlainTextEdit {
            background: #0b1220;
            border: 1px solid #475569;
            border-radius: 8px;
            padding: 6px 8px;
            color: #f8fafc;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus {
            border: 1px solid #60a5fa;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #3b82f6, stop:1 #2563eb);
            border: none;
            border-radius: 8px;
            color: white;
            font-weight: 700;
            padding: 8px 14px;
            min-height: 28px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #60a5fa, stop:1 #3b82f6);
        }
        QPushButton#btnDanger {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #ef4444, stop:1 #dc2626);
        }
        QPushButton#btnDanger:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #f87171, stop:1 #ef4444);
        }
        QTableView {
            background: #f8fafc;
            border: 1px solid #334155;
            border-radius: 10px;
            gridline-color: #cbd5e1;
            selection-background-color: #2563eb;
            selection-color: white;
            alternate-background-color: #e2e8f0;
            color: #0f172a;
            font-size: 13px;
        }
        QTableView::item {
            padding: 6px 8px;
            border-bottom: 1px solid #cbd5e1;
        }
        QTableView::item:selected {
            background: #2563eb;
            color: white;
        }
        QHeaderView::section {
            background: #1e293b;
            color: #ffffff;
            padding: 10px 8px;
            border: none;
            border-right: 1px solid #0f172a;
            border-bottom: 2px solid #3b82f6;
            font-weight: 700;
            font-size: 12.5px;
        }
        QCheckBox { color: #dbeafe; }
        QStatusBar { background: #0f172a; color: #cbd5e1; }
        QLabel#champLabel {
            color: #93c5fd;
            font-weight: 600;
        }
        QLabel#aideChamp {
            color: #64748b;
            font-size: 11px;
            font-style: italic;
            font-weight: 400;
        }
        QLineEdit[erreur="true"], QComboBox[erreur="true"],
        QPlainTextEdit[erreur="true"], QSpinBox[erreur="true"] {
            border: 1.5px solid #ef4444;
            background: #241318;
        }
    )");
}

void MainWindow::marquerChamp(QWidget *champ, bool enErreur)
{
    champ->setProperty("erreur", enErreur);
    champ->style()->unpolish(champ);
    champ->style()->polish(champ);
}

// =======================================================================
// Onglet Cours
// =======================================================================

QWidget *MainWindow::creerOngletCours()
{
    QWidget *onglet = new QWidget(this);
    QHBoxLayout *layoutPrincipal = new QHBoxLayout(onglet);

    // ---- Colonne gauche : formulaire + recherche + tableau ----
    QWidget *colonneGauche = new QWidget(onglet);
    QVBoxLayout *layoutGauche = new QVBoxLayout(colonneGauche);

    QGroupBox *groupeForm = new QGroupBox("Fiche cours", colonneGauche);
    QGridLayout *grilleForm = new QGridLayout(groupeForm);

    coursIntituleEdit = new QLineEdit(groupeForm);
    coursIntituleEdit->setPlaceholderText("Ex. Programmation C++");
    coursIntituleEdit->setMaxLength(100); // VARCHAR2(100) en base
    connect(coursIntituleEdit, &QLineEdit::textChanged, this, [this](const QString &texte) {
        if (!texte.trimmed().isEmpty())
            marquerChamp(coursIntituleEdit, false);
    });
    coursDescriptionEdit = new QPlainTextEdit(groupeForm);
    coursDescriptionEdit->setPlaceholderText("Description (255 caractères max.)");
    coursDescriptionEdit->setMaximumHeight(55);
    connect(coursDescriptionEdit, &QPlainTextEdit::textChanged, this, [this]() {
        QString texte = coursDescriptionEdit->toPlainText();
        if (texte.size() > 255) { // VARCHAR2(255) en base
            texte.truncate(255);
            coursDescriptionEdit->blockSignals(true);
            coursDescriptionEdit->setPlainText(texte);
            coursDescriptionEdit->blockSignals(false);
            QTextCursor curseur = coursDescriptionEdit->textCursor();
            curseur.movePosition(QTextCursor::End);
            coursDescriptionEdit->setTextCursor(curseur);
        }
    });
    coursDureeSpin = new QSpinBox(groupeForm);
    coursDureeSpin->setRange(1, 500); // contrainte CHECK duree_heures > 0
    coursDureeSpin->setSuffix(" h");
    coursNiveauCombo = new QComboBox(groupeForm);
    coursNiveauCombo->setEditable(true);
    coursNiveauCombo->setMaxCount(50);
    coursNiveauCombo->lineEdit()->setMaxLength(20); // VARCHAR2(20) en base
    coursNiveauCombo->addItems({"1ère Année", "2ème Année", "3ème Année"});
    coursNiveauCombo->setCurrentIndex(-1);
    coursSalleCombo = new QComboBox(groupeForm);

    grilleForm->addWidget(creerLabelChamp("Intitulé :", true, groupeForm), 0, 0);
    grilleForm->addWidget(coursIntituleEdit, 0, 1);
    grilleForm->addWidget(creerLabelChamp("Description :", false, groupeForm), 1, 0);
    grilleForm->addWidget(coursDescriptionEdit, 1, 1);
    grilleForm->addWidget(creerLabelChamp("Durée horaire :", true, groupeForm), 2, 0);
    grilleForm->addWidget(coursDureeSpin, 2, 1);
    grilleForm->addWidget(creerLabelChamp("Niveau :", false, groupeForm), 3, 0);
    grilleForm->addWidget(coursNiveauCombo, 3, 1);
    grilleForm->addWidget(creerLabelChamp("Salle assignée :", false, groupeForm), 4, 0);
    grilleForm->addWidget(coursSalleCombo, 4, 1);
    grilleForm->addWidget(creerLegendeObligatoire(groupeForm), 5, 0, 1, 2);

    QHBoxLayout *boutonsForm = new QHBoxLayout();
    QPushButton *btnAjouter = new QPushButton("Ajouter", groupeForm);
    QPushButton *btnModifier = new QPushButton("Modifier", groupeForm);
    QPushButton *btnSupprimer = new QPushButton("Supprimer", groupeForm);
    btnSupprimer->setObjectName("btnDanger");
    QPushButton *btnDupliquer = new QPushButton("Dupliquer", groupeForm);
    QPushButton *btnAssigner = new QPushButton("Assigner salle auto", groupeForm);
    boutonsForm->addWidget(btnAjouter);
    boutonsForm->addWidget(btnModifier);
    boutonsForm->addWidget(btnSupprimer);
    boutonsForm->addWidget(btnDupliquer);
    boutonsForm->addWidget(btnAssigner);
    grilleForm->addLayout(boutonsForm, 6, 0, 1, 2);

    connect(btnAjouter, &QPushButton::clicked, this, &MainWindow::onCoursAjouter);
    connect(btnModifier, &QPushButton::clicked, this, &MainWindow::onCoursModifier);
    connect(btnSupprimer, &QPushButton::clicked, this, &MainWindow::onCoursSupprimer);
    connect(btnDupliquer, &QPushButton::clicked, this, &MainWindow::onCoursDupliquer);
    connect(btnAssigner, &QPushButton::clicked, this, &MainWindow::onCoursAssignerSalle);

    layoutGauche->addWidget(groupeForm);

    // Recherche / tri multicritères
    QGroupBox *groupeRecherche = new QGroupBox("Recherche et tri multicritères", colonneGauche);
    QGridLayout *grilleRecherche = new QGridLayout(groupeRecherche);

    coursFiltreIntitule = new QLineEdit(groupeRecherche);
    coursFiltreIntitule->setPlaceholderText("Contient...");
    coursFiltreNiveau = new QComboBox(groupeRecherche);
    coursFiltreNiveau->addItem("Tous les niveaux", "");
    coursFiltreDureeMin = new QSpinBox(groupeRecherche);
    coursFiltreDureeMin->setRange(0, 500);
    coursFiltreDureeMax = new QSpinBox(groupeRecherche);
    coursFiltreDureeMax->setRange(0, 500);
    coursFiltreDureeMax->setValue(500);
    coursTriCombo = new QComboBox(groupeRecherche);
    coursTriCombo->addItems({"Intitulé", "Durée", "Niveau", "Salle"});
    coursTriDescCheck = new QCheckBox("Ordre décroissant", groupeRecherche);

    grilleRecherche->addWidget(creerLabelChamp("Intitulé :", false, groupeRecherche), 0, 0);
    grilleRecherche->addWidget(coursFiltreIntitule, 0, 1);
    grilleRecherche->addWidget(creerLabelChamp("Niveau :", false, groupeRecherche), 0, 2);
    grilleRecherche->addWidget(coursFiltreNiveau, 0, 3);
    grilleRecherche->addWidget(creerLabelChamp("Durée entre :", false, groupeRecherche), 1, 0);
    grilleRecherche->addWidget(coursFiltreDureeMin, 1, 1);
    grilleRecherche->addWidget(new QLabel("et :"), 1, 2);
    grilleRecherche->addWidget(coursFiltreDureeMax, 1, 3);
    grilleRecherche->addWidget(creerLabelChamp("Trier par :", false, groupeRecherche), 2, 0);
    grilleRecherche->addWidget(coursTriCombo, 2, 1);
    grilleRecherche->addWidget(coursTriDescCheck, 2, 2, 1, 2);

    QPushButton *btnEffacerFiltres = new QPushButton("Réinitialiser les filtres", groupeRecherche);
    grilleRecherche->addWidget(btnEffacerFiltres, 3, 0, 1, 4);

    connect(coursFiltreIntitule, &QLineEdit::textChanged, this, &MainWindow::onCoursFiltrerChange);
    connect(coursFiltreNiveau, &QComboBox::currentTextChanged, this, &MainWindow::onCoursFiltrerChange);
    connect(coursFiltreDureeMin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onCoursFiltrerChange);
    connect(coursFiltreDureeMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onCoursFiltrerChange);
    connect(coursTriCombo, &QComboBox::currentTextChanged, this, &MainWindow::onCoursFiltrerChange);
    connect(coursTriDescCheck, &QCheckBox::toggled, this, &MainWindow::onCoursFiltrerChange);
    connect(btnEffacerFiltres, &QPushButton::clicked, this, &MainWindow::onCoursEffacerFiltres);

    layoutGauche->addWidget(groupeRecherche);

    coursTable = new QTableView(colonneGauche);
    coursTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    coursTable->setSelectionMode(QAbstractItemView::SingleSelection);
    coursTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    coursTable->setAlternatingRowColors(true);
    coursTable->verticalHeader()->setVisible(false);
    coursTable->verticalHeader()->setDefaultSectionSize(34);
    coursTable->horizontalHeader()->setStretchLastSection(true);
    connect(coursTable, &QTableView::clicked, this, &MainWindow::onCoursTableClicked);
    layoutGauche->addWidget(coursTable, 1);

    QHBoxLayout *ligneBas = new QHBoxLayout();
    coursTotalLabel = new QLabel("Total : 0 cours", colonneGauche);
    coursStatutLabel = new QLabel("Statut : prêt", colonneGauche);
    QPushButton *btnPdf = new QPushButton("Générer PDF", colonneGauche);
    connect(btnPdf, &QPushButton::clicked, this, &MainWindow::onCoursGenererPdf);
    ligneBas->addWidget(coursTotalLabel);
    ligneBas->addWidget(coursStatutLabel);
    ligneBas->addStretch();
    ligneBas->addWidget(btnPdf);
    layoutGauche->addLayout(ligneBas);

    layoutPrincipal->addWidget(colonneGauche, 3);

    // ---- Colonne droite : statistiques mises à jour en direct ----
    QGroupBox *groupeStats = new QGroupBox("Statistiques", onglet);
    QVBoxLayout *layoutStats = new QVBoxLayout(groupeStats);

    coursChartViewNiveau = new QChartView(groupeStats);
    coursChartViewNiveau->setRenderHint(QPainter::Antialiasing);
    coursChartViewAssignation = new QChartView(groupeStats);
    coursChartViewAssignation->setRenderHint(QPainter::Antialiasing);

    layoutStats->addWidget(coursChartViewNiveau, 1);
    layoutStats->addWidget(coursChartViewAssignation, 1);

    layoutPrincipal->addWidget(groupeStats, 2);

    return onglet;
}

void MainWindow::rafraichirComboSalles()
{
    int idPrecedent = coursSalleCombo->currentData().toInt();

    coursSalleCombo->blockSignals(true);
    coursSalleCombo->clear();
    coursSalleCombo->addItem("(Aucune salle)", 0);

    Salle salle;
    const QMap<int, QString> libelles = salle.listeLibelles();
    for (auto it = libelles.constBegin(); it != libelles.constEnd(); ++it)
        coursSalleCombo->addItem(it.value(), it.key());

    int index = coursSalleCombo->findData(idPrecedent);
    coursSalleCombo->setCurrentIndex(index >= 0 ? index : 0);
    coursSalleCombo->blockSignals(false);
}

void MainWindow::rafraichirCours()
{
    Cours c;

    QString niveauFiltre = coursFiltreNiveau->currentData().toString();
    if (coursModel)
        coursModel->deleteLater();

    coursModel = c.rechercherTrier(coursFiltreIntitule->text(), niveauFiltre,
                                    coursFiltreDureeMin->value(), coursFiltreDureeMax->value(),
                                    coursTriCombo->currentText(), !coursTriDescCheck->isChecked());
    coursModel->setParent(this);

    if (coursModel->lastError().isValid()) {
        QMessageBox::critical(this, "Erreur SQL", coursModel->lastError().text());
    }

    coursModel->setHeaderData(0, Qt::Horizontal, "ID");
    coursModel->setHeaderData(1, Qt::Horizontal, "Intitulé");
    coursModel->setHeaderData(2, Qt::Horizontal, "Description");
    coursModel->setHeaderData(3, Qt::Horizontal, "Durée (h)");
    coursModel->setHeaderData(4, Qt::Horizontal, "Niveau");
    coursModel->setHeaderData(5, Qt::Horizontal, "Salle");

    coursTable->setModel(coursModel);
    coursTable->setColumnHidden(0, true);
    coursTable->setColumnWidth(1, 200);
    coursTable->setColumnWidth(2, 220);

    coursTotalLabel->setText(QString("Total : %1 cours").arg(coursModel->rowCount()));
    coursStatutLabel->setText(coursModel->rowCount() == 0 ? "Statut : aucun résultat"
                                                            : "Statut : données à jour");

    // Actualisation du combo "Niveau" des filtres (garde la sélection courante)
    QString niveauActuel = coursFiltreNiveau->currentData().toString();
    coursFiltreNiveau->blockSignals(true);
    coursFiltreNiveau->clear();
    coursFiltreNiveau->addItem("Tous les niveaux", "");
    for (const QString &n : c.listeNiveaux())
        coursFiltreNiveau->addItem(n, n);
    int idx = coursFiltreNiveau->findData(niveauActuel);
    coursFiltreNiveau->setCurrentIndex(idx >= 0 ? idx : 0);
    coursFiltreNiveau->blockSignals(false);

    rafraichirStatsCours();
}

void MainWindow::rafraichirStatsCours()
{
    Cours c;

    // Graphique 1 : nombre de cours par niveau
    {
        QMap<QString, int> stats = c.statParNiveau();
        QBarSet *set = new QBarSet("Cours");
        QStringList categories;
        for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
            *set << it.value();
            categories << it.key();
        }
        QBarSeries *series = new QBarSeries();
        series->append(set);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Cours par niveau");
        chart->legend()->hide();
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setTitleBrush(QBrush(QColor("#93c5fd")));

        QBarCategoryAxis *axeX = new QBarCategoryAxis();
        axeX->append(categories);
        axeX->setLabelsColor(QColor("#e5e7eb"));
        chart->addAxis(axeX, Qt::AlignBottom);
        series->attachAxis(axeX);

        QValueAxis *axeY = new QValueAxis();
        axeY->setLabelFormat("%d");
        axeY->setLabelsColor(QColor("#e5e7eb"));
        axeY->applyNiceNumbers();
        chart->addAxis(axeY, Qt::AlignLeft);
        series->attachAxis(axeY);

        QChart *ancien = coursChartViewNiveau->chart();
        coursChartViewNiveau->setChart(chart);
        delete ancien;
    }

    // Graphique 2 : cours avec/sans salle assignée
    {
        QMap<QString, int> stats = c.statAssignationSalle();
        QPieSeries *series = new QPieSeries();
        series->append("Salle assignée", stats.value("Salle assignée"));
        series->append("Non assignée", stats.value("Non assignée"));
        if (series->slices().size() > 0) {
            series->slices().at(0)->setBrush(QColor("#22c55e"));
            series->slices().at(0)->setLabelVisible(true);
        }
        if (series->slices().size() > 1) {
            series->slices().at(1)->setBrush(QColor("#f59e0b"));
            series->slices().at(1)->setLabelVisible(true);
        }

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Affectation des salles");
        chart->legend()->setLabelColor(QColor("#e5e7eb"));
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setTitleBrush(QBrush(QColor("#93c5fd")));

        QChart *ancien = coursChartViewAssignation->chart();
        coursChartViewAssignation->setChart(chart);
        delete ancien;
    }
}

void MainWindow::viderFormulaireCours()
{
    coursIdSelectionne = -1;
    coursIntituleEdit->clear();
    coursDescriptionEdit->clear();
    coursDureeSpin->setValue(1);
    coursNiveauCombo->setCurrentIndex(-1);
    coursSalleCombo->setCurrentIndex(0);
    marquerChamp(coursIntituleEdit, false);
    coursIntituleEdit->setFocus();
}

bool MainWindow::validerFormulaireCours(QStringList &erreurs)
{
    marquerChamp(coursIntituleEdit, false);

    QString intitule = coursIntituleEdit->text().trimmed();
    if (intitule.isEmpty()) {
        erreurs << "L'intitulé du cours est obligatoire.";
        marquerChamp(coursIntituleEdit, true);
    }

    // La durée (QSpinBox, borne 1-500h) et le niveau/description (longueurs
    // limitées par setMaxLength) sont déjà contrôlés au niveau du widget ;
    // ils ne peuvent pas produire de valeur invalide côté C++.

    return erreurs.isEmpty();
}

void MainWindow::onCoursAjouter()
{
    QStringList erreurs;
    if (!validerFormulaireCours(erreurs)) {
        QMessageBox::warning(this, "Saisie incomplète", erreurs.join("\n"));
        return;
    }

    Cours cours;
    cours.setIntitule(coursIntituleEdit->text().trimmed());
    cours.setDescription(coursDescriptionEdit->toPlainText().trimmed());
    cours.setDureeHeures(coursDureeSpin->value());
    cours.setNiveau(coursNiveauCombo->currentText().trimmed());
    cours.setIdSalle(coursSalleCombo->currentData().toInt());

    if (cours.ajouter()) {
        coursStatutLabel->setText("Statut : cours ajouté");
        viderFormulaireCours();
        rafraichirCours();
        statusBar()->showMessage("Cours ajouté avec succès", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout du cours.");
    }
}

void MainWindow::onCoursModifier()
{
    if (coursIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un cours dans le tableau.");
        return;
    }

    QStringList erreurs;
    if (!validerFormulaireCours(erreurs)) {
        QMessageBox::warning(this, "Saisie incomplète", erreurs.join("\n"));
        return;
    }

    Cours cours;
    cours.setId(coursIdSelectionne);
    cours.setIntitule(coursIntituleEdit->text().trimmed());
    cours.setDescription(coursDescriptionEdit->toPlainText().trimmed());
    cours.setDureeHeures(coursDureeSpin->value());
    cours.setNiveau(coursNiveauCombo->currentText().trimmed());
    cours.setIdSalle(coursSalleCombo->currentData().toInt());

    if (cours.modifier()) {
        coursStatutLabel->setText("Statut : cours modifié");
        viderFormulaireCours();
        rafraichirCours();
        statusBar()->showMessage("Cours modifié avec succès", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification du cours.");
    }
}

void MainWindow::onCoursSupprimer()
{
    if (coursIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un cours à supprimer.");
        return;
    }

    if (QMessageBox::question(this, "Confirmation", "Supprimer ce cours ?") != QMessageBox::Yes)
        return;

    Cours cours;
    if (cours.supprimer(coursIdSelectionne)) {
        coursStatutLabel->setText("Statut : cours supprimé");
        viderFormulaireCours();
        rafraichirCours();
        statusBar()->showMessage("Cours supprimé", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression du cours.");
    }
}

void MainWindow::onCoursDupliquer()
{
    if (coursIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un cours à dupliquer.");
        return;
    }

    Cours cours;
    if (cours.dupliquer(coursIdSelectionne)) {
        rafraichirCours();
        statusBar()->showMessage("Cours dupliqué avec succès", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la duplication du cours.");
    }
}

void MainWindow::onCoursAssignerSalle()
{
    if (coursIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un cours dans le tableau.");
        return;
    }

    Cours cours;
    QString nomSalle;
    if (cours.assignerSalleDisponible(coursIdSelectionne, 0, &nomSalle)) {
        QMessageBox::information(this, "Salle assignée",
                                  QString("La salle « %1 » a été assignée automatiquement.").arg(nomSalle));
        rafraichirCours();
        rafraichirComboSalles();
    } else {
        QMessageBox::warning(this, "Aucune salle disponible",
                              "Aucune salle disponible n'a été trouvée pour ce cours.");
    }
}

void MainWindow::onCoursGenererPdf()
{
    if (!coursModel || coursModel->rowCount() == 0) {
        QMessageBox::information(this, "Aucune donnée", "Aucun cours à exporter.");
        return;
    }

    QStringList entetes = {"Intitulé", "Description", "Durée (h)", "Niveau", "Salle"};
    QList<QStringList> lignes;
    for (int row = 0; row < coursModel->rowCount(); ++row) {
        QStringList ligne;
        for (int col = 1; col <= 5; ++col)
            ligne << coursModel->data(coursModel->index(row, col)).toString();
        lignes << ligne;
    }

    Document::genererListePdf(this, "Liste des Cours", entetes, lignes, "liste_cours.pdf");
}

void MainWindow::onCoursTableClicked(const QModelIndex &index)
{
    if (!index.isValid() || !coursModel)
        return;

    int row = index.row();
    coursIdSelectionne = coursModel->data(coursModel->index(row, 0)).toInt();
    coursIntituleEdit->setText(coursModel->data(coursModel->index(row, 1)).toString());
    coursDescriptionEdit->setPlainText(coursModel->data(coursModel->index(row, 2)).toString());
    coursDureeSpin->setValue(coursModel->data(coursModel->index(row, 3)).toInt());
    coursNiveauCombo->setCurrentText(coursModel->data(coursModel->index(row, 4)).toString());

    QString nomSalle = coursModel->data(coursModel->index(row, 5)).toString();
    int idxSalle = coursSalleCombo->findText(nomSalle, Qt::MatchStartsWith);
    coursSalleCombo->setCurrentIndex(idxSalle >= 0 ? idxSalle : 0);

    statusBar()->showMessage("Cours sélectionné : " + coursIntituleEdit->text(), 3000);
}

void MainWindow::onCoursFiltrerChange()
{
    rafraichirCours();
}

void MainWindow::onCoursEffacerFiltres()
{
    coursFiltreIntitule->clear();
    coursFiltreNiveau->setCurrentIndex(0);
    coursFiltreDureeMin->setValue(0);
    coursFiltreDureeMax->setValue(500);
    coursTriCombo->setCurrentIndex(0);
    coursTriDescCheck->setChecked(false);
    rafraichirCours();
}

// =======================================================================
// Onglet Salle
// =======================================================================

QWidget *MainWindow::creerOngletSalle()
{
    QWidget *onglet = new QWidget(this);
    QHBoxLayout *layoutPrincipal = new QHBoxLayout(onglet);

    QWidget *colonneGauche = new QWidget(onglet);
    QVBoxLayout *layoutGauche = new QVBoxLayout(colonneGauche);

    QGroupBox *groupeForm = new QGroupBox("Fiche salle", colonneGauche);
    QGridLayout *grilleForm = new QGridLayout(groupeForm);

    salleNomEdit = new QLineEdit(groupeForm);
    salleNomEdit->setPlaceholderText("Ex. Salle Info 1");
    salleNomEdit->setMaxLength(50); // VARCHAR2(50) en base
    connect(salleNomEdit, &QLineEdit::textChanged, this, [this](const QString &texte) {
        if (!texte.trimmed().isEmpty())
            marquerChamp(salleNomEdit, false);
    });
    salleCapaciteSpin = new QSpinBox(groupeForm);
    salleCapaciteSpin->setRange(1, 1000); // contrainte CHECK capacite > 0
    salleCapaciteSpin->setSuffix(" places");
    salleTypeCombo = new QComboBox(groupeForm);
    salleTypeCombo->setEditable(true);
    salleTypeCombo->setMaxCount(50);
    salleTypeCombo->lineEdit()->setMaxLength(30); // VARCHAR2(30) en base
    salleTypeCombo->addItems({"Informatique", "Amphithéâtre", "Laboratoire", "Salle de cours"});
    salleTypeCombo->setCurrentIndex(-1);
    connect(salleTypeCombo, &QComboBox::currentTextChanged, this, [this](const QString &texte) {
        if (!texte.trimmed().isEmpty())
            marquerChamp(salleTypeCombo, false);
    });
    salleDisponibleCheck = new QCheckBox("Disponible", groupeForm);
    salleDisponibleCheck->setChecked(true);

    grilleForm->addWidget(creerLabelChamp("Nom :", true, groupeForm), 0, 0);
    grilleForm->addWidget(salleNomEdit, 0, 1);
    grilleForm->addWidget(creerLabelChamp("Capacité :", true, groupeForm), 1, 0);
    grilleForm->addWidget(salleCapaciteSpin, 1, 1);
    grilleForm->addWidget(creerLabelChamp("Type :", true, groupeForm), 2, 0);
    grilleForm->addWidget(salleTypeCombo, 2, 1);
    grilleForm->addWidget(salleDisponibleCheck, 3, 1);
    grilleForm->addWidget(creerLegendeObligatoire(groupeForm), 4, 0, 1, 2);

    QHBoxLayout *boutonsForm = new QHBoxLayout();
    QPushButton *btnAjouter = new QPushButton("Ajouter", groupeForm);
    QPushButton *btnModifier = new QPushButton("Modifier", groupeForm);
    QPushButton *btnSupprimer = new QPushButton("Supprimer", groupeForm);
    btnSupprimer->setObjectName("btnDanger");
    QPushButton *btnBasculer = new QPushButton("Basculer disponibilité", groupeForm);
    boutonsForm->addWidget(btnAjouter);
    boutonsForm->addWidget(btnModifier);
    boutonsForm->addWidget(btnSupprimer);
    boutonsForm->addWidget(btnBasculer);
    grilleForm->addLayout(boutonsForm, 5, 0, 1, 2);

    connect(btnAjouter, &QPushButton::clicked, this, &MainWindow::onSalleAjouter);
    connect(btnModifier, &QPushButton::clicked, this, &MainWindow::onSalleModifier);
    connect(btnSupprimer, &QPushButton::clicked, this, &MainWindow::onSalleSupprimer);
    connect(btnBasculer, &QPushButton::clicked, this, &MainWindow::onSalleBasculerDispo);

    layoutGauche->addWidget(groupeForm);

    QGroupBox *groupeRecherche = new QGroupBox("Recherche et tri multicritères", colonneGauche);
    QGridLayout *grilleRecherche = new QGridLayout(groupeRecherche);

    salleFiltreNom = new QLineEdit(groupeRecherche);
    salleFiltreNom->setPlaceholderText("Contient...");
    salleFiltreType = new QComboBox(groupeRecherche);
    salleFiltreType->addItem("Tous les types", "");
    salleFiltreCapMin = new QSpinBox(groupeRecherche);
    salleFiltreCapMin->setRange(0, 1000);
    salleFiltreCapMax = new QSpinBox(groupeRecherche);
    salleFiltreCapMax->setRange(0, 1000);
    salleFiltreCapMax->setValue(1000);
    salleFiltreDispo = new QComboBox(groupeRecherche);
    salleFiltreDispo->addItem("Toutes", -1);
    salleFiltreDispo->addItem("Disponible", 1);
    salleFiltreDispo->addItem("Occupée", 0);
    salleTriCombo = new QComboBox(groupeRecherche);
    salleTriCombo->addItems({"Nom", "Capacité", "Type", "Disponibilité"});
    salleTriDescCheck = new QCheckBox("Ordre décroissant", groupeRecherche);

    grilleRecherche->addWidget(creerLabelChamp("Nom :", false, groupeRecherche), 0, 0);
    grilleRecherche->addWidget(salleFiltreNom, 0, 1);
    grilleRecherche->addWidget(creerLabelChamp("Type :", false, groupeRecherche), 0, 2);
    grilleRecherche->addWidget(salleFiltreType, 0, 3);
    grilleRecherche->addWidget(creerLabelChamp("Capacité entre :", false, groupeRecherche), 1, 0);
    grilleRecherche->addWidget(salleFiltreCapMin, 1, 1);
    grilleRecherche->addWidget(new QLabel("et :"), 1, 2);
    grilleRecherche->addWidget(salleFiltreCapMax, 1, 3);
    grilleRecherche->addWidget(creerLabelChamp("Disponibilité :", false, groupeRecherche), 2, 0);
    grilleRecherche->addWidget(salleFiltreDispo, 2, 1);
    grilleRecherche->addWidget(creerLabelChamp("Trier par :", false, groupeRecherche), 2, 2);
    grilleRecherche->addWidget(salleTriCombo, 2, 3);
    grilleRecherche->addWidget(salleTriDescCheck, 3, 0, 1, 2);

    QPushButton *btnEffacerFiltres = new QPushButton("Réinitialiser les filtres", groupeRecherche);
    grilleRecherche->addWidget(btnEffacerFiltres, 3, 2, 1, 2);

    connect(salleFiltreNom, &QLineEdit::textChanged, this, &MainWindow::onSalleFiltrerChange);
    connect(salleFiltreType, &QComboBox::currentTextChanged, this, &MainWindow::onSalleFiltrerChange);
    connect(salleFiltreCapMin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSalleFiltrerChange);
    connect(salleFiltreCapMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSalleFiltrerChange);
    connect(salleFiltreDispo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSalleFiltrerChange);
    connect(salleTriCombo, &QComboBox::currentTextChanged, this, &MainWindow::onSalleFiltrerChange);
    connect(salleTriDescCheck, &QCheckBox::toggled, this, &MainWindow::onSalleFiltrerChange);
    connect(btnEffacerFiltres, &QPushButton::clicked, this, &MainWindow::onSalleEffacerFiltres);

    layoutGauche->addWidget(groupeRecherche);

    QGroupBox *groupeSuggestion = new QGroupBox("Suggestion de salle adaptée", colonneGauche);
    QHBoxLayout *layoutSuggestion = new QHBoxLayout(groupeSuggestion);
    QSpinBox *spinCapaciteVoulue = new QSpinBox(groupeSuggestion);
    spinCapaciteVoulue->setRange(1, 1000);
    spinCapaciteVoulue->setObjectName("spinCapaciteVoulue");
    QPushButton *btnSuggerer = new QPushButton("Suggérer les salles adaptées", groupeSuggestion);
    layoutSuggestion->addWidget(new QLabel("Capacité recherchée :"));
    layoutSuggestion->addWidget(spinCapaciteVoulue);
    layoutSuggestion->addWidget(btnSuggerer);
    layoutSuggestion->addStretch();
    connect(btnSuggerer, &QPushButton::clicked, this, [this, spinCapaciteVoulue]() {
        Salle salle;
        QSqlQueryModel *suggestions = salle.suggererPourCapacite(spinCapaciteVoulue->value());
        if (suggestions->rowCount() == 0) {
            QMessageBox::information(this, "Aucune suggestion",
                                      "Aucune salle disponible pour cette capacité.");
        } else {
            suggestions->setHeaderData(1, Qt::Horizontal, "Nom");
            suggestions->setHeaderData(2, Qt::Horizontal, "Capacité");
            suggestions->setHeaderData(3, Qt::Horizontal, "Type");
            suggestions->setHeaderData(4, Qt::Horizontal, "Statut");
            salleTable->setModel(suggestions);
            salleTable->setColumnHidden(0, true);
            salleStatutLabel->setText(QString("Statut : %1 salle(s) suggérée(s)").arg(suggestions->rowCount()));
        }
    });
    layoutGauche->addWidget(groupeSuggestion);

    salleTable = new QTableView(colonneGauche);
    salleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    salleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    salleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    salleTable->setAlternatingRowColors(true);
    salleTable->verticalHeader()->setVisible(false);
    salleTable->verticalHeader()->setDefaultSectionSize(34);
    salleTable->horizontalHeader()->setStretchLastSection(true);
    connect(salleTable, &QTableView::clicked, this, &MainWindow::onSalleTableClicked);
    layoutGauche->addWidget(salleTable, 1);

    QHBoxLayout *ligneBas = new QHBoxLayout();
    salleTotalLabel = new QLabel("Total : 0 salle", colonneGauche);
    salleStatutLabel = new QLabel("Statut : prêt", colonneGauche);
    QPushButton *btnPdf = new QPushButton("Générer PDF", colonneGauche);
    connect(btnPdf, &QPushButton::clicked, this, &MainWindow::onSalleGenererPdf);
    ligneBas->addWidget(salleTotalLabel);
    ligneBas->addWidget(salleStatutLabel);
    ligneBas->addStretch();
    ligneBas->addWidget(btnPdf);
    layoutGauche->addLayout(ligneBas);

    layoutPrincipal->addWidget(colonneGauche, 3);

    QGroupBox *groupeStats = new QGroupBox("Statistiques", onglet);
    QVBoxLayout *layoutStats = new QVBoxLayout(groupeStats);

    salleChartViewType = new QChartView(groupeStats);
    salleChartViewType->setRenderHint(QPainter::Antialiasing);
    salleChartViewDispo = new QChartView(groupeStats);
    salleChartViewDispo->setRenderHint(QPainter::Antialiasing);

    layoutStats->addWidget(salleChartViewType, 1);
    layoutStats->addWidget(salleChartViewDispo, 1);

    layoutPrincipal->addWidget(groupeStats, 2);

    return onglet;
}

void MainWindow::rafraichirSalle()
{
    Salle s;

    int dispoFiltre = salleFiltreDispo->currentData().toInt();
    if (salleModel)
        salleModel->deleteLater();

    salleModel = s.rechercherTrier(salleFiltreNom->text(), salleFiltreType->currentData().toString(),
                                    salleFiltreCapMin->value(), salleFiltreCapMax->value(), dispoFiltre,
                                    salleTriCombo->currentText(), !salleTriDescCheck->isChecked());
    salleModel->setParent(this);

    if (salleModel->lastError().isValid()) {
        QMessageBox::critical(this, "Erreur SQL", salleModel->lastError().text());
    }

    salleModel->setHeaderData(0, Qt::Horizontal, "ID");
    salleModel->setHeaderData(1, Qt::Horizontal, "Nom");
    salleModel->setHeaderData(2, Qt::Horizontal, "Capacité");
    salleModel->setHeaderData(3, Qt::Horizontal, "Type");
    salleModel->setHeaderData(4, Qt::Horizontal, "Statut");

    salleTable->setModel(salleModel);
    salleTable->setColumnHidden(0, true);

    salleTotalLabel->setText(QString("Total : %1 salle(s)").arg(salleModel->rowCount()));
    salleStatutLabel->setText(salleModel->rowCount() == 0 ? "Statut : aucun résultat"
                                                            : "Statut : données à jour");

    QString typeActuel = salleFiltreType->currentData().toString();
    salleFiltreType->blockSignals(true);
    salleFiltreType->clear();
    salleFiltreType->addItem("Tous les types", "");
    for (const QString &t : s.listeTypes())
        salleFiltreType->addItem(t, t);
    int idx = salleFiltreType->findData(typeActuel);
    salleFiltreType->setCurrentIndex(idx >= 0 ? idx : 0);
    salleFiltreType->blockSignals(false);

    rafraichirStatsSalle();
}

void MainWindow::rafraichirStatsSalle()
{
    Salle s;

    // Graphique 1 : salles par type
    {
        QMap<QString, int> stats = s.statParType();
        QBarSet *set = new QBarSet("Salles");
        QStringList categories;
        for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
            *set << it.value();
            categories << it.key();
        }
        QBarSeries *series = new QBarSeries();
        series->append(set);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Salles par type");
        chart->legend()->hide();
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setTitleBrush(QBrush(QColor("#93c5fd")));

        QBarCategoryAxis *axeX = new QBarCategoryAxis();
        axeX->append(categories);
        axeX->setLabelsColor(QColor("#e5e7eb"));
        chart->addAxis(axeX, Qt::AlignBottom);
        series->attachAxis(axeX);

        QValueAxis *axeY = new QValueAxis();
        axeY->setLabelFormat("%d");
        axeY->setLabelsColor(QColor("#e5e7eb"));
        axeY->applyNiceNumbers();
        chart->addAxis(axeY, Qt::AlignLeft);
        series->attachAxis(axeY);

        QChart *ancien = salleChartViewType->chart();
        salleChartViewType->setChart(chart);
        delete ancien;
    }

    // Graphique 2 : disponibilité
    {
        QMap<QString, int> stats = s.statDisponibilite();
        QPieSeries *series = new QPieSeries();
        series->append("Disponible", stats.value("Disponible"));
        series->append("Occupée", stats.value("Occupée"));
        if (series->slices().size() > 0) {
            series->slices().at(0)->setBrush(QColor("#22c55e"));
            series->slices().at(0)->setLabelVisible(true);
        }
        if (series->slices().size() > 1) {
            series->slices().at(1)->setBrush(QColor("#ef4444"));
            series->slices().at(1)->setLabelVisible(true);
        }

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Disponibilité des salles");
        chart->legend()->setLabelColor(QColor("#e5e7eb"));
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setTitleBrush(QBrush(QColor("#93c5fd")));

        QChart *ancien = salleChartViewDispo->chart();
        salleChartViewDispo->setChart(chart);
        delete ancien;
    }
}

void MainWindow::viderFormulaireSalle()
{
    salleIdSelectionne = -1;
    salleNomEdit->clear();
    salleCapaciteSpin->setValue(1);
    salleTypeCombo->setCurrentIndex(-1);
    salleDisponibleCheck->setChecked(true);
    marquerChamp(salleNomEdit, false);
    marquerChamp(salleTypeCombo, false);
    salleNomEdit->setFocus();
}

bool MainWindow::validerFormulaireSalle(QStringList &erreurs)
{
    marquerChamp(salleNomEdit, false);
    marquerChamp(salleTypeCombo, false);

    if (salleNomEdit->text().trimmed().isEmpty()) {
        erreurs << "Le nom de la salle est obligatoire.";
        marquerChamp(salleNomEdit, true);
    }
    if (salleTypeCombo->currentText().trimmed().isEmpty()) {
        erreurs << "Le type de la salle est obligatoire.";
        marquerChamp(salleTypeCombo, true);
    }

    // La capacité (QSpinBox, borne 1-1000) est déjà contrôlée au niveau du
    // widget ; elle ne peut pas produire de valeur invalide côté C++.

    return erreurs.isEmpty();
}

void MainWindow::onSalleAjouter()
{
    QStringList erreurs;
    if (!validerFormulaireSalle(erreurs)) {
        QMessageBox::warning(this, "Saisie incomplète", erreurs.join("\n"));
        return;
    }

    Salle salle;
    salle.setNom(salleNomEdit->text().trimmed());
    salle.setCapacite(salleCapaciteSpin->value());
    salle.setTypeSalle(salleTypeCombo->currentText().trimmed());
    salle.setDisponibilite(salleDisponibleCheck->isChecked());

    if (salle.ajouter()) {
        salleStatutLabel->setText("Statut : salle ajoutée");
        viderFormulaireSalle();
        rafraichirSalle();
        rafraichirComboSalles();
        statusBar()->showMessage("Salle ajoutée avec succès", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout de la salle.");
    }
}

void MainWindow::onSalleModifier()
{
    if (salleIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une salle dans le tableau.");
        return;
    }

    QStringList erreurs;
    if (!validerFormulaireSalle(erreurs)) {
        QMessageBox::warning(this, "Saisie incomplète", erreurs.join("\n"));
        return;
    }

    Salle salle;
    salle.setId(salleIdSelectionne);
    salle.setNom(salleNomEdit->text().trimmed());
    salle.setCapacite(salleCapaciteSpin->value());
    salle.setTypeSalle(salleTypeCombo->currentText().trimmed());
    salle.setDisponibilite(salleDisponibleCheck->isChecked());

    if (salle.modifier()) {
        salleStatutLabel->setText("Statut : salle modifiée");
        viderFormulaireSalle();
        rafraichirSalle();
        rafraichirComboSalles();
        statusBar()->showMessage("Salle modifiée avec succès", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification de la salle.");
    }
}

void MainWindow::onSalleSupprimer()
{
    if (salleIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une salle à supprimer.");
        return;
    }

    if (QMessageBox::question(this, "Confirmation", "Supprimer cette salle ?") != QMessageBox::Yes)
        return;

    Salle salle;
    if (salle.supprimer(salleIdSelectionne)) {
        salleStatutLabel->setText("Statut : salle supprimée");
        viderFormulaireSalle();
        rafraichirSalle();
        rafraichirComboSalles();
        statusBar()->showMessage("Salle supprimée", 3000);
    } else {
        QMessageBox::critical(this, "Erreur",
                               "Échec de la suppression : des cours utilisent peut-être encore cette salle.");
    }
}

void MainWindow::onSalleBasculerDispo()
{
    if (salleIdSelectionne <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une salle.");
        return;
    }

    Salle salle;
    if (salle.basculerDisponibilite(salleIdSelectionne)) {
        rafraichirSalle();
        rafraichirComboSalles();
        statusBar()->showMessage("Disponibilité mise à jour", 3000);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la mise à jour de la disponibilité.");
    }
}

void MainWindow::onSalleGenererPdf()
{
    if (!salleModel || salleModel->rowCount() == 0) {
        QMessageBox::information(this, "Aucune donnée", "Aucune salle à exporter.");
        return;
    }

    QStringList entetes = {"Nom", "Capacité", "Type", "Statut"};
    QList<QStringList> lignes;
    for (int row = 0; row < salleModel->rowCount(); ++row) {
        QStringList ligne;
        for (int col = 1; col <= 4; ++col)
            ligne << salleModel->data(salleModel->index(row, col)).toString();
        lignes << ligne;
    }

    Document::genererListePdf(this, "Liste des Salles", entetes, lignes, "liste_salles.pdf");
}

void MainWindow::onSalleTableClicked(const QModelIndex &index)
{
    if (!index.isValid() || !salleModel)
        return;

    int row = index.row();
    salleIdSelectionne = salleModel->data(salleModel->index(row, 0)).toInt();
    salleNomEdit->setText(salleModel->data(salleModel->index(row, 1)).toString());
    salleCapaciteSpin->setValue(salleModel->data(salleModel->index(row, 2)).toInt());
    salleTypeCombo->setCurrentText(salleModel->data(salleModel->index(row, 3)).toString());
    salleDisponibleCheck->setChecked(salleModel->data(salleModel->index(row, 4)).toString() == "Disponible");

    statusBar()->showMessage("Salle sélectionnée : " + salleNomEdit->text(), 3000);
}

void MainWindow::onSalleFiltrerChange()
{
    rafraichirSalle();
}

void MainWindow::onSalleEffacerFiltres()
{
    salleFiltreNom->clear();
    salleFiltreType->setCurrentIndex(0);
    salleFiltreCapMin->setValue(0);
    salleFiltreCapMax->setValue(1000);
    salleFiltreDispo->setCurrentIndex(0);
    salleTriCombo->setCurrentIndex(0);
    salleTriDescCheck->setChecked(false);
    rafraichirSalle();
}
