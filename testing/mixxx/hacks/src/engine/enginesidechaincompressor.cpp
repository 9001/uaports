#include <QtDebug>

#include "engine/enginesidechaincompressor.h"

EngineSideChainCompressor::EngineSideChainCompressor(const char* group)
        : m_compressRatio(0.0),
          m_threshold(1.0),
          m_strength(0.0),
          m_attackTime(0),
          m_sustainTime(0),
          m_decayTime(0),
          m_sustainAge(0),
          m_attackPerFrame(0.0),
          m_decayPerFrame(0.0) {
    Q_UNUSED(group);
}

void EngineSideChainCompressor::calculateRates() {
    // Don't allow completely zero rates, or else if parameters change
    // we might get stuck on a compression value.
    if (m_attackTime == 0) {
        // Attack really shouldn't be instant, but we allow it.
        m_attackPerFrame = m_strength;
    } else {
        m_attackPerFrame = m_strength / m_attackTime;
    }
    if (m_decayTime == 0) {
        m_decayPerFrame = m_strength;
    } else {
        m_decayPerFrame = m_strength / m_decayTime;
    }
    if (m_attackPerFrame <= 0) {
        m_attackPerFrame = 0.005;
    }
    if (m_decayPerFrame <= 0) {
        m_decayPerFrame = 0.005;
    }
    qDebug() << "Compressor attack per frame: " << m_attackPerFrame
             << "decay per frame: " << m_decayPerFrame;
}

void EngineSideChainCompressor::clearKeys() {
    // noop
}

void EngineSideChainCompressor::processKey(const CSAMPLE* pIn, const int iBufferSize) {
    if (m_sustainAge > 0)
		m_sustainAge--;
		
    for (int i = 0; i + 1 < iBufferSize; i += 2) {
        CSAMPLE val = (pIn[i] + pIn[i + 1]) / 2;
        if (val > m_threshold) {
            m_sustainedSample = val;
            m_sustainAge = m_sustainTime;
            return;
        }
    }
}

double EngineSideChainCompressor::calculateCompressedGain(int frames) {
    if (m_sustainAge > 0) {
		if (m_sustainAge >= frames)
			m_sustainAge -= frames;
		else
			m_sustainAge = 0;
		
        if (m_compressRatio < m_strength) {
            m_compressRatio += m_attackPerFrame * frames;
            if (m_compressRatio > m_strength) {
                // If we overshot, clamp.
                m_compressRatio = m_strength;
            }
        } else if (m_compressRatio > m_strength) {
            // If the strength param was changed, we might be compressing too much.
            m_compressRatio -= m_decayPerFrame * frames;
        }
    } else {
        if (m_compressRatio > 0) {
            m_compressRatio -= m_decayPerFrame * frames;
            if (m_compressRatio < 0) {
                // If we overshot, clamp.
                m_compressRatio = 0;
            }
        } else if (m_compressRatio < 0) {
            // Complain loudly.
            qWarning() << "Programming error, below-zero compression detected.";
            m_compressRatio += m_attackPerFrame * frames;
        }
    }
    return (1. - m_compressRatio);
}
