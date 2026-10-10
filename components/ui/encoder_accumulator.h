#pragma once

constexpr int kEncoderStep = 50;

inline int encoder_accumulate(int current_delta, bool cw, bool ccw)
{
    (void) current_delta;
    int delta = 0;
    if (cw) delta += kEncoderStep;
    if (ccw) delta -= kEncoderStep;
    return delta;
}
