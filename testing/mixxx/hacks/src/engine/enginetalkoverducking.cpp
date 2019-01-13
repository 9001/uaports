#include "control/controlproxy.h"
#include "engine/enginetalkoverducking.h"

void EngineTalkoverDucking::hack(double mode) {
	if (mode == EngineTalkoverDucking::MANUAL) {
		m_DuckThreshold = 0.00001;
		m_AttackFraction = 0.5;
	} else {
		m_AttackFraction = 0.1;
		m_DuckThreshold = 0.1;
	}
}

EngineTalkoverDucking::EngineTalkoverDucking(
        UserSettingsPointer pConfig, const char* group)
    : EngineSideChainCompressor(group),
      m_pConfig(pConfig),
      m_group(group) {
	hack(42);
    m_pMasterSampleRate = new ControlProxy(m_group, "samplerate", this);
    m_pMasterSampleRate->connectValueChanged(SLOT(slotSampleRateChanged(double)),
                                             Qt::DirectConnection);

    m_pDuckStrength = new ControlPotmeter(ConfigKey(m_group, "duckStrength"), 0.0, 1.0);
    m_pDuckStrength->set(
            m_pConfig->getValue<double>(ConfigKey(m_group, "duckStrength"), 90) / 100);
    connect(m_pDuckStrength, SIGNAL(valueChanged(double)),
            this, SLOT(slotDuckStrengthChanged(double)),
            Qt::DirectConnection);

    // We only allow the strength to be configurable for now.  The next most likely
    // candidate for customization is the threshold, which may be too low for
    // noisy club situations.
    setParameters(
            m_DuckThreshold,
            m_pDuckStrength->get(),
            m_pMasterSampleRate->get() / 2 * m_AttackFraction,
            m_pMasterSampleRate->get() / 2 * 0.2,
            m_pMasterSampleRate->get() / 2);

    double duckMode = m_pConfig->getValue<double>(
            ConfigKey(m_group, "duckMode"), AUTO);
            
    m_pTalkoverDucking = new ControlPushButton(ConfigKey(m_group, "talkoverDucking"));
    m_pTalkoverDucking->setButtonMode(ControlPushButton::TOGGLE);
    m_pTalkoverDucking->setStates(3);
    m_pTalkoverDucking->set(duckMode);
    connect(m_pTalkoverDucking, SIGNAL(valueChanged(double)),
            this, SLOT(slotDuckModeChanged(double)),
            Qt::DirectConnection);
    
    hack(duckMode);
}

EngineTalkoverDucking::~EngineTalkoverDucking() {
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(m_pDuckStrength->get() * 100));
    m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(m_pTalkoverDucking->get()));

    delete m_pDuckStrength;
    delete m_pTalkoverDucking;
}

void EngineTalkoverDucking::slotSampleRateChanged(double samplerate) {
    setParameters(
            m_DuckThreshold, m_pDuckStrength->get(),
            samplerate / 2 * m_AttackFraction,
            samplerate / 2 * 0.2,
            samplerate / 2);
}

void EngineTalkoverDucking::slotDuckStrengthChanged(double strength) {
    setParameters(
            m_DuckThreshold, strength,
            m_pMasterSampleRate->get() / 2 * m_AttackFraction,
            m_pMasterSampleRate->get() / 2 * 0.2,
            m_pMasterSampleRate->get() / 2);
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(strength * 100));
}

void EngineTalkoverDucking::slotDuckModeChanged(double mode) {
	hack(mode);
	setParameters(
			m_DuckThreshold, m_pDuckStrength->get(),
			m_pMasterSampleRate->get() / 2 * m_AttackFraction,
			m_pMasterSampleRate->get() / 2 * 0.2,
			m_pMasterSampleRate->get() / 2);
	m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(mode));
}

CSAMPLE EngineTalkoverDucking::getGain(int numFrames) {
    // Apply microphone ducking.
    switch (getMode()) {
      case EngineTalkoverDucking::OFF:
        return 1.0;
      case EngineTalkoverDucking::AUTO:
        return calculateCompressedGain(numFrames);
      case EngineTalkoverDucking::MANUAL:
        //return m_pDuckStrength->get();
        return calculateCompressedGain(numFrames);
    }
    qWarning() << "Invalid ducking mode, returning 1.0";
    return 1.0;
}
