#include <QtGlobal>
#include "assemblercontroller.h"

AssemblerController::AssemblerController(QObject *parent)
   : QObject(parent),
     m_timer(NULL),
     m_speed(NORMAL),
     m_state(NO_SOURCE)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AssemblerController::timerUpdate);
}

void AssemblerController::setSourceCode(const QString &asmSource)
{
    m_assembler.setSourceCode(asmSource);
    setState(asmSource.trimmed().isEmpty() ? NO_SOURCE : RESET);
    m_assembler.parse();
}

bool AssemblerController::lineHasError(int line) const
{
    if (m_assembler.errors().isEmpty())
        return false;
    for (const Assembler::Error& error : m_assembler.errors()) {
        if (error.line == line)
            return true;
    }
    return false;
}

void AssemblerController::timerUpdate()
{
    translateNextLine();
    if (m_timer->interval() != timerInterval())
        m_timer->setInterval(timerInterval());
}

int AssemblerController::timerInterval()
{
    static const int NORMAL_TIMER_INTERVAL = 1000;
    static const double INTERVAL_MULTIPLIER[] = { 2.0, 1.5, 1.0, 0.5, 0.25 };
    return qRound(NORMAL_TIMER_INTERVAL * INTERVAL_MULTIPLIER[m_speed]);
}

void AssemblerController::translateNextLine()
{
    m_assembler.translateNextLine();
    if (m_assembler.hasMoreLines())
        emit currentLineChanged(m_assembler.binaryCode().length() - 1);
    else
        setState(FINISHED);
}

void AssemblerController::setState(AssemblerController::State newState)
{
    if (m_state == newState) return;
    m_state = newState;

    if (m_state == RUNNING)
        m_timer->start(0);
    else if (m_timer->isActive())
        m_timer->stop();

    emit stateChanged(m_state);
}

void AssemblerController::run()
{
    if (m_state == FINISHED)
        reset();
    setState(RUNNING);
}

void AssemblerController::pause()
{
    setState(PAUSED);
}

void AssemblerController::step()
{
    if (m_state == RUNNING || m_state == RESET)
        pause();
    else if (m_state == FINISHED)
        reset();
    translateNextLine();
}

void AssemblerController::reset()
{
    m_assembler.clearTranslationData();
    setState(RESET);
}

void AssemblerController::translateAll()
{
    m_assembler.translateAll();
    setState(FINISHED);
}
