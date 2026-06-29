#include "SoundUtil.h"

#include <QApplication>

namespace {
void playBeep() {
    QApplication::beep();
}
}

void SoundUtil::playAttack() { playBeep(); }
void SoundUtil::playHit() { playBeep(); }
void SoundUtil::playHeal() { playBeep(); }
void SoundUtil::playVictory() { playBeep(); }
