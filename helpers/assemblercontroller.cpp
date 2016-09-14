#include <QDebug>
#include <QtGlobal>
#include "assemblercontroller.h"

AssemblerController::AssemblerController(QObject *parent)
   : QObject(parent),
     m_timer(NULL),
     m_speed(NORMAL),
     m_state(NO_SOURCE),
     m_currentLine(-1)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AssemblerController::timerUpdate);
}

void AssemblerController::setSourceCode(const QString &asmSource)
{
    m_assembler.setSourceCode(asmSource);
    setState(asmSource.trimmed().isEmpty() ? NO_SOURCE : RESET);
    m_assembler.parse();
    m_assembler.translate();
}

void AssemblerController::timerUpdate()
{
    if (m_timer->interval() != timerInterval())
        m_timer->setInterval(timerInterval());
    translateNextLine();
}

int AssemblerController::timerInterval()
{
    static const int NORMAL_TIMER_INTERVAL = 1000;
    static const double INTERVAL_MULTIPLIER[] = { 2.0, 1.5, 1.0, 0.5, 0.25 };
    return qRound(NORMAL_TIMER_INTERVAL * INTERVAL_MULTIPLIER[m_speed]);
}

void AssemblerController::translateNextLine()
{
    if (m_currentLine == m_assembler.binaryCode().length() - 1) {
        setState(FINISHED);
    } else {
        m_currentLine++;
        emit currentLineChanged(m_currentLine);
    }
}

void AssemblerController::setState(AssemblerController::State newState)
{
    if (m_state == newState) return;
    m_state = newState;

    if (m_state == NO_SOURCE || m_state == FINISHED || m_state == RESET)
        m_currentLine = -1;

    if (m_state == RUNNING)
        m_timer->start(0);
    else if (m_timer->isActive())
        m_timer->stop();

    emit stateChanged(m_state);
}

void AssemblerController::run()
{
    qDebug() << "AssemblerController::run()" << m_state;
//    m_assembler.translate();
    if (m_state == FINISHED)
        reset();
    setState(RUNNING);
}

void AssemblerController::pause()
{
    qDebug() << "AssemblerController::pause()";
    setState(PAUSED);
}

void AssemblerController::step()
{
    qDebug() << "AssemblerController::step()" << m_state;
    if (m_state == RUNNING || m_state == RESET)
        pause();
    else if (m_state == FINISHED)
        reset();
    translateNextLine();
}

void AssemblerController::reset()
{
    qDebug() << "AssemblerController::reset()";
    setState(RESET);
}

void AssemblerController::translateAll()
{
//    m_assembler.translate();
    setState(RESET);
    setState(FINISHED);
}
