#ifndef HACKASSEMBLEREDITOR_H
#define HACKASSEMBLEREDITOR_H

#include <QFileInfo>
#include <QListWidget>
#include <QMainWindow>

#include "aboutdialog.h"
#include "helpers/assemblercontroller.h"
#include "helpers/hacksyntaxhighlighter.h"

namespace Ui {
class MainWindow;
}

class HackAssemblerEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit HackAssemblerEditor(QWidget *parent = 0);
    ~HackAssemblerEditor();

protected:
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_action_NewAsmSource_triggered();
    void on_action_OpenAsmSource_triggered();
    void on_action_SaveAsmSource_triggered();
    void on_action_SaveAsmSourceAs_triggered();
    void on_action_OpenCmpBinary_triggered();
    void on_action_SaveTranslatedBinary_triggered();

    void on_action_Exit_triggered();
    void on_action_About_triggered();

    void on_action_RunPauseTranslation_triggered(bool checked);
    void on_action_RunPauseTranslation_toggled(bool checked);

    void on_action_StepTranslation_triggered();
    void on_action_ResetTranslation_triggered();
    void on_action_TranslateAll_triggered();

    void on_speedSlider_valueChanged(int value);
    void on_errorButton_toggled(bool checked);
    void on_errorList_currentRowChanged(int currentRow);
    void on_errorList_itemActivated(QListWidgetItem *item);

    void on_sourceTextEdit_textChanged();

    void asmControllerStateChanged(AssemblerController::State newState);
    void asmControllerCurrentLineChanged(int line);

    void translatedCodeModelChanged(const QModelIndex &parent, int first, int last);
    void translatedCodeModelReset();

    void on_translatedCode_itemActivated(QListWidgetItem *item);

    void on_translatedCode_currentRowChanged(int currentRow);
    void on_referenceCode_currentRowChanged(int currentRow);

    void translatedCodeScrollMoved(int value);
    void referenceCodeScrollMoved(int value);

    void on_copyTranslatedButton_clicked();
    void on_copyReferenceButton_clicked();

    void cursorPositionChanged();

private:
    QFileInfo openSourceFile(const QString &filename);
    QFileInfo openReferenceBinaryFile(const QString &filename);

    void addLineToListWidget(QListWidget *listWidget, QString line);
    void copyListWidgetContentsToClipboard(const QListWidget *listWidget);

    void updateBinDiff();
    void updateBinDiffLine(int line);
    void clearReferenceCodeBinDiff();

    bool handleSourceSaving();
    bool saveSourceAs();
    bool saveSource(const QString& filename);

    void goToSourceLine(int sourceLine);

    static const int DEFAULT_SPEED;

    Ui::MainWindow *ui;
    AboutDialog *m_about;

    AssemblerController* m_asmController;
    HackSyntaxHighlighter *m_hackSyntaxHighlighter;
};

#endif // HACKASSEMBLEREDITOR_H
