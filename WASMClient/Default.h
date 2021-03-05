#pragma once
#include <string>
#include "Logger.h"

#define DEFAULT_LOG_LEVEL			LOG_LEVEL_INFO
#define DEFAULT_LVAR_UPDATE_FREQ	0
#define DEFAULT_START_EVENT_NO		0x1FFF0

using namespace CPlusPlusLogging;

class Default
{
	public:
		static Default* GetInstance(const char* text) throw ();
		LogLevel GetLogLevel();
		int GetLvarUpdateFrequency();
		int GetStartEventNo();

	protected:
		Default(const char* text);
		~Default();

	private:
		static Default* m_Instance;
		std::string		m_iniFileName;
		LogLevel		m_logLevel;
		int				m_startEventNo;
		int				m_lvarUpdateFrequency;
};
