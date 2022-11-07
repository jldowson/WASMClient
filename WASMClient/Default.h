#pragma once
#include <string>

#define DEFAULT_LOG_LEVEL				2
#define DEFAULT_LVAR_UPDATE_FREQ		0
#define DEFAULT_START_EVENT_NO			0x1FFF0

class Default
{
	public:
		static Default* GetInstance(const char* text) throw ();
		int GetLogLevel();
		int GetLvarUpdateFrequency();
		int GetSimConnectConnection();

	protected:
		Default(const char* text);
		~Default();

	private:
		static Default* m_Instance;
		std::string		m_iniFileName;
		int				m_logLevel;
		int				m_lvarUpdateFrequency;
		int				m_simConnectConnection;
};
