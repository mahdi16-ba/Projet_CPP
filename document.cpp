#include "document.h"

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QPageLayout>
#include <QMarginsF>
#include <QDateTime>

bool Document::genererListePdf(QWidget *parent, const QString &titre,
                                const QStringList &entetes, const QList<QStringList> &lignes,
                                const QString &nomFichierParDefaut)
{
    QString cheminFichier = QFileDialog::getSaveFileName(
        parent, "Enregistrer le document PDF", nomFichierParDefaut, "Fichiers PDF (*.pdf)");

    if (cheminFichier.isEmpty())
        return false;

    if (!cheminFichier.endsWith(".pdf", Qt::CaseInsensitive))
        cheminFichier += ".pdf";

    QString html;
    html += "<html><head><style>"
            "body { font-family: 'Segoe UI', Arial, sans-serif; color: #1f2937; }"
            "h1 { color: #1d4ed8; margin-bottom: 0; }"
            "p.soustitre { color: #6b7280; margin-top: 4px; }"
            "table { border-collapse: collapse; width: 100%; margin-top: 16px; }"
            "th { background-color: #1d4ed8; color: white; padding: 8px; text-align: left; }"
            "td { padding: 6px 8px; border-bottom: 1px solid #d1d5db; }"
            "tr:nth-child(even) td { background-color: #f3f4f6; }"
            "p.pied { color: #6b7280; margin-top: 18px; font-size: 10px; }"
            "</style></head><body>";

    html += "<h1>Centre de Formation — " + titre.toHtmlEscaped() + "</h1>";
    html += "<p class=\"soustitre\">Document généré le " +
            QDateTime::currentDateTime().toString("dd/MM/yyyy à HH:mm") + "</p>";

    html += "<table><thead><tr>";
    for (const QString &entete : entetes)
        html += "<th>" + entete.toHtmlEscaped() + "</th>";
    html += "</tr></thead><tbody>";

    for (const QStringList &ligne : lignes) {
        html += "<tr>";
        for (const QString &valeur : ligne)
            html += "<td>" + valeur.toHtmlEscaped() + "</td>";
        html += "</tr>";
    }
    html += "</tbody></table>";

    html += QString("<p class=\"pied\">%1 ligne(s) — Projet C++ / Qt / Oracle — Gestion de centre de formation</p>")
                .arg(lignes.size());
    html += "</body></html>";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(cheminFichier);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument document;
    document.setHtml(html);
    document.setPageSize(printer.pageRect(QPrinter::DevicePixel).size());
    document.print(&printer);

    QMessageBox::information(parent, "Document généré",
                              "Le PDF a été enregistré avec succès :\n" + cheminFichier);
    return true;
}
