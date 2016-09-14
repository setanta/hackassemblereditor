#include "code.h"

/**
 * dest   d1 d2 d3
 * ---------------
 * <NULL> 0  0  0
 * M      0  0  1
 * D      0  1  0
 * MD     0  1  1
 * A      1  0  0
 * AM     1  0  1
 * AD     1  1  0
 * AMD    1  1  1
 */
QString Code::dest(QString mnemonic)
{
    mnemonic = mnemonic.toUpper();
    return QString::number(mnemonic.count('A')) +
           QString::number(mnemonic.count('D')) +
           QString::number(mnemonic.count('M'));
}

/**
 * jump   j1 j2 j3
 * ---------------
 * <NULL> 0  0  0
 * JGT    0  0  1
 * JEQ    0  1  0
 * JGE    0  1  1
 * JLT    1  0  0
 * JNE    1  0  1
 * JLE    1  1  0
 * JMP    1  1  1
 */
QString Code::jump(QString mnemonic)
{
    mnemonic = mnemonic.toUpper();
    if (mnemonic == "JGT")
        return "001";
    else if (mnemonic == "JEQ")
        return "010";
    else if (mnemonic == "JGE")
        return "011";
    else if (mnemonic == "JLT")
        return "100";
    else if (mnemonic == "JNE")
        return "101";
    else if (mnemonic == "JLE")
        return "110";
    else if (mnemonic == "JMP")
        return "111";
    return "000";
}

/**
 * C-instruction: dest=comp;jump
 * Either the dest or jump fields may be empty.
 * If dest is empty, the "=" is omitted;
 * If jump is empty, the ";" is omitted.
 *               +--------comp-------+ +-dest--+ +-jump-+
 * Binary: 1 1 1 a  c1 c2 c3 c4  c5 c6 d1 d2  d3 j1 j2 j3
 *
 *       comp                       comp
 * (when a=0)   c1 c2 c3 c4 c5 c6   (when a=1)
 * -------------------------------------------
 *          0   1  0  1  0  1  0
 *          1   1  1  1  1  1  1
 *         -1   1  1  1  0  1  0
 *          D   0  0  1  1  0  0
 *          A   1  1  0  0  0  0    M
 *         !D   0  0  1  1  0  1
 *         !A   1  1  0  0  0  1    !M
 *         -D   0  0  1  1  1  1
 *         -A   1  1  0  0  1  1    -M
 *        D+1   0  1  1  1  1  1
 *        A+1   1  1  0  1  1  1    M+1
 *        D-1   0  0  1  1  1  0
 *        A-1   1  1  0  0  1  0    M-1
 *        D+A   0  0  0  0  1  0    D+M
 *        D-A   0  1  0  0  1  1    D-M
 *        A-D   0  0  0  1  1  1    M-D
 *        D&A   0  0  0  0  0  0    D&M
 *        D|A   0  1  0  1  0  1    D|M
 */
QString Code::comp(QString mnemonic)
{
    mnemonic = mnemonic.toUpper();
    QString code(7, '0');

    if (mnemonic == "0")
        code = "0101010";
    else if (mnemonic == "1")
        code = "0111111";
    else if (mnemonic == "-1")
        code = "0111010";
    else if (mnemonic == "D")
        code = "0001100";
    else if (mnemonic == "A" || mnemonic == "M")
        code = "0110000";
    else if (mnemonic == "!D")
        code = "0001101";
    else if (mnemonic == "!A" || mnemonic == "!M")
        code = "0110001";
    else if (mnemonic == "-D")
        code = "0001111";
    else if (mnemonic == "-A" || mnemonic == "-M")
        code = "0110011";
    else if (mnemonic == "D+1")
        code = "0011111";
    else if (mnemonic == "A+1" || mnemonic == "M+1")
        code = "0110111";
    else if (mnemonic == "D-1")
        code = "0001110";
    else if (mnemonic == "A-1" || mnemonic == "M-1")
        code = "0110010";
    else if (mnemonic == "D+A" || mnemonic == "D+M")
        code = "0000010";
    else if (mnemonic == "D-A" || mnemonic == "D-M")
        code = "0010011";
    else if (mnemonic == "A-D" || mnemonic == "M-D")
        code = "0000111";
    else if (mnemonic == "D&A" || mnemonic == "D&M")
        code = "0000000";
    else if (mnemonic == "D|A" || mnemonic == "D|M")
        code = "0010101";

    if (mnemonic.contains("M"))
        code[0] = '1';

    return code;
}
