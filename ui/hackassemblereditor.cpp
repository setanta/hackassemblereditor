#include <QClipboard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextStream>

#include "hackassemblereditor.h"
#include "ui_hackassemblereditor.h"

const int HackAssemblerEditor::DEFAULT_SPEED = 2;

HackAssemblerEditor::HackAssemblerEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_hackAsmHelp(NULL),
    m_about(NULL)
{
    ui->setupUi(this);

    m_hackSyntaxHighlighter = new HackSyntaxHighlighter(ui->sourceTextEdit->document());

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

    QSettings settings;

    QString lastSourceFile = settings.value("editor/asmSrcPath", QString()).toString();
    if (!lastSourceFile.isEmpty())
        openSourceFile(lastSourceFile);

    QString lastBinaryReferenceFile = settings.value("editor/refBinPath", QString()).toString();
    if (!lastBinaryReferenceFile.isEmpty())
        openReferenceBinaryFile(lastBinaryReferenceFile);

    ui->speedSlider->setValue(settings.value("assembler/speed", HackAssemblerEditor::DEFAULT_SPEED).toInt());
    restoreGeometry(settings.value("editor/geometry").toByteArray());

    ui->sourceTextEdit->setFocus();
}

HackAssemblerEditor::~HackAssemblerEditor()
{
    delete ui;
}

void HackAssemblerEditor::closeEvent(QCloseEvent *event)
{
    if (ui->sourceTextEdit->document()->isModified() && !handleSourceSaving()) {
        event->ignore();
        return;
    }

    QSettings settings;
    settings.setValue("editor/geometry", saveGeometry());
    settings.setValue("assembler/speed", ui->speedSlider->value());
    settings.sync();

    ui->translatedCode->model()->disconnect();
    event->accept();
}

void HackAssemblerEditor::on_action_NewAsmSource_triggered()
{
    if (ui->sourceTextEdit->document()->isModified() && !handleSourceSaving())
        return;

    ui->sourceTextEdit->clear();
    ui->sourceTextEdit->setDocumentTitle(QString());

    QGuiApplication::setApplicationDisplayName(QString());
    setWindowModified(false);

    m_asmController->setState(AssemblerController::NO_SOURCE);

    QSettings settings;
    settings.remove("editor/asmSrcPath");
    settings.sync();
}

void HackAssemblerEditor::on_action_OpenAsmSource_triggered()
{
    QSettings settings;
    QString asmSrcDir = settings.value("editor/asmSrcDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Hack Assembly source"),
                                                    asmSrcDir,
                                                    tr("Hack Assembly Files (*.asm);;All Files (*)"));
    QFileInfo fileInfo = openSourceFile(filename);
    if (fileInfo.exists()) {
        settings.setValue("editor/asmSrcPath", fileInfo.absoluteFilePath());
        settings.setValue("editor/asmSrcDir", fileInfo.absolutePath());
        settings.sync();
    }
}

void HackAssemblerEditor::on_action_SaveAsmSource_triggered()
{
    if (!ui->sourceTextEdit->document()->isModified())
        return;

    if (ui->sourceTextEdit->documentTitle().isEmpty())
        saveSourceAs();
    else
        saveSource(ui->sourceTextEdit->documentTitle());
}

void HackAssemblerEditor::on_action_SaveAsmSourceAs_triggered()
{
    saveSourceAs();
}

void HackAssemblerEditor::on_action_OpenCmpBinary_triggered()
{
    QSettings settings;
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
    QSettings settings;
    QString binOutDir = settings.value("editor/binOutDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Translated Hack Binary"),
                                                    binOutDir,
                                                    tr("Hack Binary Files (*.hack);;All Files (*)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream outStream(&file);
    for (int i = 0; i < ui->translatedCode->count(); i++)
        outStream << ui->translatedCode->item(i)->text().replace(" ", "") << "\n";

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

void HackAssemblerEditor::on_action_HackAsmHelp_triggered()
{
    if (!m_hackAsmHelp)
        m_hackAsmHelp = new HackAssemblyHelp(this);
    m_hackAsmHelp->show();
}

void HackAssemblerEditor::cursorPositionChanged()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->sourceTextEdit->textCursor();
    selection.cursor.clearSelection();

    Qt::GlobalColor bgColor(Qt::yellow);
    if (m_asmController->lineHasError(selection.cursor.blockNumber()))
        bgColor = Qt::red;
    QColor lineColor = QColor(bgColor).lighter(160);
    selection.format.setBackground(lineColor);

    extraSelections.append(selection);
    ui->sourceTextEdit->setExtraSelections(extraSelections);

    int translatedLineNumber = m_asmController->binaryLineForSourceLine(selection.cursor.blockNumber());
    if (translatedLineNumber != ui->translatedCode->currentRow())
        ui->translatedCode->setCurrentRow(translatedLineNumber);
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
    m_asmController->reset();
    m_asmController->translateAll();
}

void HackAssemblerEditor::on_speedSlider_valueChanged(int value)
{
    static const char * const Speed[] = { "x0.25", "x0.5", "x1", "x1.5", "x2" };
    ui->speedLabel->setText(Speed[value]);
    m_asmController->setSpeed(AssemblerController::Speed(value));
}

void HackAssemblerEditor::on_errorButton_toggled(bool checked)
{
    QSizePolicy::Policy vertical = checked ? QSizePolicy::Fixed : QSizePolicy::Ignored;
    ui->errorList->setSizePolicy(QSizePolicy::Expanding, vertical);
}

void HackAssemblerEditor::on_errorList_currentRowChanged(int currentRow)
{
    if (currentRow < 0 || m_asmController->errors().empty())
        return;
    goToSourceLine(m_asmController->errors().at(currentRow).line);
}

void HackAssemblerEditor::on_errorList_itemActivated(QListWidgetItem *item)
{
    goToSourceLine(m_asmController->errors().at(ui->errorList->row(item)).line);
    ui->sourceTextEdit->setFocus();
}

void HackAssemblerEditor::on_sourceTextEdit_textChanged()
{
    setWindowModified(ui->sourceTextEdit->document()->isModified());
    m_asmController->setSourceCode(ui->sourceTextEdit->toPlainText());

    const Assembler::ErrorList& errors = m_asmController->errors();
    ui->errorList->clear();
    ui->errorButton->setEnabled(!errors.empty());

    cursorPositionChanged();

    if (errors.empty()) {
        ui->errorButton->setChecked(false);
    } else {
        for (const Assembler::Error& error : m_asmController->errors()) {
            QString formattedLine = QString::number(error.line + 1).rightJustified(3, ' ');
            ui->errorList->addItem(QString("%1: %2").arg(formattedLine).arg(error.message));
        }
    }
}

void HackAssemblerEditor::asmControllerStateChanged(AssemblerController::State newState)
{
    switch (newState) {
    case AssemblerController::NO_SOURCE:
        ui->translatedCode->clear();
        ui->runPauseButton->setChecked(false);
        ui->runPauseButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        break;

    case AssemblerController::RESET:
        ui->translatedCode->clear();
        ui->runPauseButton->setChecked(false);
        ui->runPauseButton->setEnabled(true);
        ui->nextButton->setEnabled(true);
        ui->resetButton->setEnabled(false);
        break;

    case AssemblerController::FINISHED:
        ui->runPauseButton->setChecked(false);
        ui->runPauseButton->setEnabled(true);
        ui->nextButton->setEnabled(false);
        ui->resetButton->setEnabled(true);

        // FINISHED after RESET: translate all command.
        if (ui->translatedCode->count() == 0) {
            for (const QString &line : m_asmController->binaryCode())
                addLineToListWidget(ui->translatedCode, line);
            int selectedSourceLine = ui->sourceTextEdit->textCursor().blockNumber();
            ui->translatedCode->setCurrentRow(m_asmController->binaryLineForSourceLine(selectedSourceLine));
        }
        break;

    case AssemblerController::PAUSED:
        ui->resetButton->setEnabled(true);
        ui->runPauseButton->setChecked(false);
        break;

    case AssemblerController::RUNNING:
        ui->resetButton->setEnabled(true);
        ui->runPauseButton->setChecked(true);
        break;

    default:
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

void HackAssemblerEditor::translatedCodeModelChanged(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
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
    if (!fileInfo.exists())
        return fileInfo;

    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream sourceStream(&file);

    ui->sourceTextEdit->setPlainText(sourceStream.readAll());
    ui->sourceTextEdit->setDocumentTitle(fileInfo.absoluteFilePath());

    QGuiApplication::setApplicationDisplayName(fileInfo.fileName());
    setWindowModified(ui->sourceTextEdit->document()->isModified());

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

void HackAssemblerEditor::on_translatedCode_itemActivated(QListWidgetItem *item)
{
    Q_UNUSED(item);
    ui->sourceTextEdit->setFocus();
}

void HackAssemblerEditor::on_translatedCode_currentRowChanged(int currentRow)
{
    int newRow = currentRow < ui->referenceCode->count() ? currentRow : -1;
    if (newRow != ui->referenceCode->currentRow())
        ui->referenceCode->setCurrentRow(newRow);

    int sourceLine = m_asmController->sourceLineForBinaryLine(ui->translatedCode->currentRow());
    if (sourceLine > -1)
        goToSourceLine(sourceLine);
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

bool HackAssemblerEditor::handleSourceSaving()
{
    if (!ui->sourceTextEdit->document()->isModified())
        return false;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this,
                                 tr("Unsaved Modifications"),
                                 tr("Save source code changes?\n"
                                    "If you don't save, the changes will be permanently lost."),
                                 QMessageBox::Discard|QMessageBox::Cancel|QMessageBox::Save);
    switch (reply) {
    case QMessageBox::Save:
        if (ui->sourceTextEdit->documentTitle().isEmpty())
            return saveSourceAs();
        return saveSource(ui->sourceTextEdit->documentTitle());
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }

    return true;
}

bool HackAssemblerEditor::saveSourceAs()
{
    QSettings settings;
    QString asmSrcDir = settings.value("editor/asmSrcDir", QDir::homePath()).toString();

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Hack Assembly source as..."),
                                                    asmSrcDir,
                                                    tr("Hack Assembly Files (*.asm);;All Files (*)"));
    if (filename.isEmpty())
        return false;

    QFileInfo fileInfo(filename);
    if (!saveSource(fileInfo.absoluteFilePath()))
        return false;

    ui->sourceTextEdit->setDocumentTitle(fileInfo.absoluteFilePath());
    QGuiApplication::setApplicationDisplayName(fileInfo.fileName());
    setWindowModified(false);

    settings.setValue("editor/asmSrcPath", fileInfo.absoluteFilePath());
    settings.sync();
    return true;
}

bool HackAssemblerEditor::saveSource(const QString& filename)
{
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream outStream(&file);
    outStream << ui->sourceTextEdit->toPlainText();

    ui->sourceTextEdit->document()->setModified(false);
    setWindowModified(false);
    return true;
}

void HackAssemblerEditor::goToSourceLine(int sourceLine)
{
    QTextCursor cursor = ui->sourceTextEdit->textCursor();
    cursor.setPosition(sourceLine);
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, sourceLine);
    cursor.movePosition(QTextCursor::EndOfLine);
    ui->sourceTextEdit->setTextCursor(cursor);
}
