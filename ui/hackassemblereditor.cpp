#include <QDebug>
#include <QClipboard>
#include <QFileDialog>
#include <QScrollBar>
#include <QSettings>
#include <QTextStream>

#include "hackassemblereditor.h"
#include "ui_hackassemblereditor.h"

const int HackAssemblerEditor::DEFAULT_SPEED = 2;

HackAssemblerEditor::HackAssemblerEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_about(NULL),
    m_sourceModified(false)
{
    ui->setupUi(this);

    m_hackSyntaxHighlighter = new HackSyntaxHighlighter(ui->sourceTextEdit->document());

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &HackAssemblerEditor::handleQuit);
    connect(ui->sourceTextEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &HackAssemblerEditor::cursorPositionChanged);

    m_asmController = new AssemblerController(this);
    connect(m_asmController, &AssemblerController::stateChanged,
            this, &HackAssemblerEditor::asmControllerStateChanged);
    connect(m_asmController, &AssemblerController::currentLineChanged,
            this, &HackAssemblerEditor::asmControllerCurrentLineChanged);

    connect(ui->translatedCode->model(), &QAbstractItemModel::rowsInserted,
            this, &HackAssemblerEditor::translatedCodeModelChanged);
    connect(ui->translatedCode->model(), &QAbstractItemModel::modelReset,
            this, &HackAssemblerEditor::translatedCodeModelReset);

    connect(ui->translatedCode->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &HackAssemblerEditor::translatedCodeScrollMoved);
    connect(ui->referenceCode->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &HackAssemblerEditor::referenceCodeScrollMoved);

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    openSourceFile(settings.value("editor/asmSrcPath", QString()).toString());
    openReferenceBinaryFile(settings.value("editor/refBinPath", QString()).toString());
    ui->speedSlider->setValue(settings.value("assembler/speed", HackAssemblerEditor::DEFAULT_SPEED).toInt());
    restoreGeometry(settings.value("app/geometry").toByteArray());
}

HackAssemblerEditor::~HackAssemblerEditor()
{
    delete ui;
}

void HackAssemblerEditor::on_action_NewAsmSource_triggered()
{
    ui->sourceTextEdit->clear();
    ui->sourceTextEdit->setDocumentTitle(QString());
    ui->asmFileLabel->setText(QString());
    m_asmController->setState(AssemblerController::NO_SOURCE);
}

void HackAssemblerEditor::on_action_OpenAsmSource_triggered()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString srcDir = settings.value("editor/srcDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Hack Assembly source"),
                                                    srcDir,
                                                    tr("Hack Assembly Files (*.asm);;All Files (*)"));
    QFileInfo fileInfo = openSourceFile(filename);
    if (fileInfo.exists()) {
        settings.setValue("editor/srcDir", fileInfo.absolutePath());
        settings.sync();
    }
}

void HackAssemblerEditor::on_action_SaveAsmSource_triggered()
{
    qDebug() << "Save Hack Assembly source.";
    m_sourceModified = false;
}

void HackAssemblerEditor::on_action_OpenCmpBinary_triggered()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString binRefDir = settings.value("editor/binRefDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Hack Binary reference file"),
                                                    binRefDir,
                                                    tr("Hack Binary Files (*.hack);;All Files (*)"));
    QFileInfo fileInfo = openReferenceBinaryFile(filename);
    if (fileInfo.exists()) {
        updateBinDiff();
        settings.setValue("editor/binRefDir", fileInfo.absolutePath());
        settings.setValue("editor/refBinPath", fileInfo.absoluteFilePath());
        settings.sync();
    }
}

void HackAssemblerEditor::on_action_SaveTranslatedBinary_triggered()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString binOutDir = settings.value("editor/binOutDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Translated Hack Binary"),
                                                    binOutDir,
                                                    tr("Hack Binary Files (*.hack);;All Files (*)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream outStream(&file);
    for (int i = 0; i < ui->translatedCode->count(); i++) {
        outStream << ui->translatedCode->item(i)->text().replace(" ", "") << "\n";
    }
    file.close();

    QFileInfo fileInfo(filename);
    settings.setValue("editor/binOutDir", fileInfo.absolutePath());
    settings.sync();
}

void HackAssemblerEditor::on_action_Exit_triggered()
{
    close();
}

void HackAssemblerEditor::on_action_About_triggered()
{
    if (!m_about)
        m_about = new AboutDialog(this);
    m_about->show();
}

void HackAssemblerEditor::cursorPositionChanged()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QColor lineColor = QColor(Qt::yellow).lighter(160);

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->sourceTextEdit->textCursor();
    selection.cursor.clearSelection();

    extraSelections.append(selection);
    ui->sourceTextEdit->setExtraSelections(extraSelections);

    int translatedLineNumber = m_asmController->binaryLineForSourceLine(selection.cursor.blockNumber());
    if (translatedLineNumber != ui->translatedCode->currentRow())
        ui->translatedCode->setCurrentRow(translatedLineNumber);
}

void HackAssemblerEditor::handleQuit()
{
    // [TODO] Document is modified: save it?
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("editor/asmSrcPath", ui->sourceTextEdit->documentTitle());
    settings.setValue("assembler/speed", ui->speedSlider->value());
    settings.setValue("app/geometry", saveGeometry());
    settings.sync();
}

void HackAssemblerEditor::on_action_RunPauseTranslation_triggered(bool checked)
{
    if (checked)
        m_asmController->run();
    else
        m_asmController->pause();
}

void HackAssemblerEditor::on_action_RunPauseTranslation_toggled(bool checked)
{
    ui->runPauseButton->setChecked(checked);
}

void HackAssemblerEditor::on_action_StepTranslation_triggered()
{
    m_asmController->step();
}

void HackAssemblerEditor::on_action_ResetTranslation_triggered()
{
    m_asmController->reset();
}

void HackAssemblerEditor::on_action_TranslateAll_triggered()
{
    ui->translatedCode->clear();
    m_asmController->translateAll();
}

void HackAssemblerEditor::on_speedSlider_valueChanged(int value)
{
    static const char * const Speed[] = { "x0.25", "x0.5", "x1", "x1.5", "x2" };
    ui->speedLabel->setText(Speed[value]);
    m_asmController->setSpeed(AssemblerController::Speed(value));
}

void HackAssemblerEditor::on_sourceTextEdit_textChanged()
{
    m_sourceModified = true;
    m_asmController->setSourceCode(ui->sourceTextEdit->toPlainText());
}

void HackAssemblerEditor::asmControllerStateChanged(AssemblerController::State newState)
{
    bool hasSource = true;

    switch (newState) {
    case AssemblerController::NO_SOURCE:
        qDebug() << "State: NO_SOURCE";
        hasSource = false;
    case AssemblerController::RESET:
        qDebug() << "State: RESET";
        ui->translatedCode->clear();
    case AssemblerController::FINISHED:
        qDebug() << "State: FINISHED";
        ui->runPauseButton->setChecked(false);
        ui->runPauseButton->setEnabled(hasSource);
        ui->nextButton->setEnabled(hasSource && newState != AssemblerController::FINISHED);
        if (newState == AssemblerController::FINISHED && ui->translatedCode->count() == 0) {
            for (const QString &line : m_asmController->binaryCode())
                addLineToListWidget(ui->translatedCode, line);
            int selectedSourceLine = ui->sourceTextEdit->textCursor().blockNumber();
            ui->translatedCode->setCurrentRow(m_asmController->binaryLineForSourceLine(selectedSourceLine));
        }
        ui->resetButton->setEnabled(hasSource && ui->translatedCode->count() > 0);
        break;

    case AssemblerController::PAUSED:
        qDebug() << "State: PAUSED";
        ui->runPauseButton->setChecked(false);

    case AssemblerController::RUNNING:
        qDebug() << "State: RUNNING";
        ui->resetButton->setEnabled(true);
        break;
    }

    ui->action_RunPauseTranslation->setChecked(ui->runPauseButton->isChecked());
    ui->action_RunPauseTranslation->setEnabled(ui->runPauseButton->isEnabled());
    ui->action_StepTranslation->setEnabled(ui->nextButton->isEnabled());
    ui->action_ResetTranslation->setEnabled(ui->resetButton->isEnabled());
}

void HackAssemblerEditor::asmControllerCurrentLineChanged(int line)
{
    addLineToListWidget(ui->translatedCode, m_asmController->binaryCode().at(line));
    int selectedSourceLine = ui->sourceTextEdit->textCursor().blockNumber();
    if (selectedSourceLine == m_asmController->sourceLineForBinaryLine(line))
        ui->translatedCode->setCurrentRow(line);
}

void HackAssemblerEditor::translatedCodeModelChanged(const QModelIndex &, int, int last)
{
    ui->action_SaveTranslatedBinary->setEnabled(true);
    ui->saveHackBinaryButton->setEnabled(true);
    ui->copyTranslatedButton->setEnabled(true);
    updateBinDiffLine(last);
}

void HackAssemblerEditor::translatedCodeModelReset()
{
    ui->action_SaveTranslatedBinary->setEnabled(false);
    ui->saveHackBinaryButton->setEnabled(false);
    ui->copyTranslatedButton->setEnabled(false);
    clearReferenceCodeBinDiff();
}

QFileInfo HackAssemblerEditor::openSourceFile(const QString &filename)
{

    QFileInfo fileInfo(filename);

    if (fileInfo.exists()) {
        QFile file(filename);
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream sourceStream(&file);

        ui->sourceTextEdit->setPlainText(sourceStream.readAll());
        ui->sourceTextEdit->setDocumentTitle(fileInfo.absoluteFilePath());
        ui->asmFileLabel->setText(fileInfo.fileName());

        m_asmController->setSourceCode(ui->sourceTextEdit->toPlainText());
    }
    return fileInfo;
}

QFileInfo HackAssemblerEditor::openReferenceBinaryFile(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream sourceStream(&file);
    ui->referenceCode->clear();
    for (const QString &line : sourceStream.readAll().split("\n")) {
        if (line.trimmed().isEmpty())
            continue;
        addLineToListWidget(ui->referenceCode, line);
    }
    ui->copyReferenceButton->setEnabled(ui->referenceCode->count() > 0);
    return QFileInfo(filename);
}

void HackAssemblerEditor::addLineToListWidget(QListWidget *listWidget, QString line)
{
    listWidget->addItem(line.insert(12, ' ').insert(8, ' ').insert(4, ' '));
}

void HackAssemblerEditor::copyListWidgetContentsToClipboard(const QListWidget *listWidget)
{
    QStringList binaryCode;
    for (int i = 0; i < listWidget->count(); i++)
        binaryCode << listWidget->item(i)->text().replace(" ", "");
    QApplication::clipboard()->setText(binaryCode.join("\n"));
}

void HackAssemblerEditor::updateBinDiff()
{
    for (int line = 0; line < ui->translatedCode->count(); line++)
        updateBinDiffLine(line);
}

void HackAssemblerEditor::updateBinDiffLine(int line)
{
    QListWidgetItem* translatedLineItem = ui->translatedCode->item(line);
    Qt::GlobalColor lineColor(Qt::darkGreen);

    QListWidgetItem* referenceLineItem = NULL;
    if (line < ui->referenceCode->count())
         referenceLineItem = ui->referenceCode->item(line);

    if (!referenceLineItem)
        lineColor = Qt::darkGray;
    else if (translatedLineItem->text() != referenceLineItem->text())
        lineColor = Qt::red;

    translatedLineItem->setTextColor(lineColor);
    if (referenceLineItem)
        referenceLineItem->setTextColor(lineColor);
}

void HackAssemblerEditor::clearReferenceCodeBinDiff()
{
    for (int line = 0; line < ui->referenceCode->count(); line++) {
        QListWidgetItem* referenceLineItem = ui->referenceCode->item(line);
        referenceLineItem->setTextColor(Qt::black);
    }
}

void HackAssemblerEditor::on_translatedCode_itemActivated(QListWidgetItem *)
{
    ui->sourceTextEdit->setFocus();
}

void HackAssemblerEditor::on_translatedCode_currentRowChanged(int currentRow)
{
    int newRow = currentRow < ui->referenceCode->count() ? currentRow : -1;
    if (newRow != ui->referenceCode->currentRow())
        ui->referenceCode->setCurrentRow(newRow);

    int sourceLine = m_asmController->sourceLineForBinaryLine(ui->translatedCode->currentRow());
    if (sourceLine == -1)
        return;

    QTextCursor cursor = ui->sourceTextEdit->textCursor();
    cursor.setPosition(sourceLine);
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, sourceLine);
    ui->sourceTextEdit->setTextCursor(cursor);
}

void HackAssemblerEditor::on_referenceCode_currentRowChanged(int currentRow)
{
    if (currentRow > -1 && currentRow < ui->translatedCode->count())
        ui->translatedCode->setCurrentRow(currentRow );
}

void HackAssemblerEditor::translatedCodeScrollMoved(int value)
{
    if (value != ui->referenceCode->verticalScrollBar()->value())
        ui->referenceCode->verticalScrollBar()->setValue(value);
}

void HackAssemblerEditor::referenceCodeScrollMoved(int value)
{
    if (value != ui->translatedCode->verticalScrollBar()->value())
        ui->translatedCode->verticalScrollBar()->setValue(value);
}

void HackAssemblerEditor::on_copyTranslatedButton_clicked()
{
    copyListWidgetContentsToClipboard(ui->translatedCode);
}

void HackAssemblerEditor::on_copyReferenceButton_clicked()
{
    copyListWidgetContentsToClipboard(ui->referenceCode);
}