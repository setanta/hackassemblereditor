#ifndef ASSEMBLERCONTROLLER_H
#define ASSEMBLERCONTROLLER_H

#include <QObject>
#include <QTextEdit>
#include <QTimer>

#include "hackassembler/assembler.h"

class AssemblerController : public QObject
{
    Q_OBJECT
public:
    enum State {
        NO_SOURCE,
        RESET,
        PAUSED,
        RUNNING,
        FINISHED
        // ERROR ?
    };

    enum Speed {
        SLOWER = 0, // x0.25
        SLOW,       // x0.5
        NORMAL,     // x1
        FAST,       // x1.5
        FASTER      // x2
    };

    explicit AssemblerController(QObject *parent = 0);

    void setSourceCode(const QString& asmSource);
    const QStringList& binaryCode() const { return m_assembler.binaryCode(); }

    const Assembler::ErrorList& errors() const { return m_assembler.errors(); }
    bool lineHasError(int line) const;

    int sourceLineForBinaryLine(int line) { return m_assembler.sourceLineForBinaryLine(line); }
    int binaryLineForSourceLine(int line) { return m_assembler.binaryLineForSourceLine(line); }

    State state() { return m_state; }
    void setState(State newState);

    void run();
    void pause();
    void step();
    void reset();
    void translateAll();

    void setSpeed(Speed speed) { m_speed = speed; }

signals:
    void stateChanged(AssemblerController::State newState);
    void currentLineChanged(int line);

private slots:
    void timerUpdate();

private:
    int timerInterval();
    void translateNextLine();

    Assembler m_assembler;
    QTimer *m_timer;
    Speed m_speed;
    State m_state;
};

#endif // ASSEMBLERCONTROLLER_H
