#include "Default.h"
#include <iostream>
#include <cstdlib>
#include "SimpleIni.h"
#include "SimConnect.h"


Default* Default::m_Instance = 0;

Default::Default(const char* baseIniFileName)
{
    char msg[512];
	m_iniFileName = std::string(baseIniFileName) + ".ini";
 
    // Load ini files: first from AppData folder
    CSimpleIniA ini;

    SI_Error rc = ini.LoadFile(m_iniFileName.c_str());

    if (rc < 0) {
        m_logLevel = DEFAULT_LOG_LEVEL;
        m_lvarUpdateFrequency = DEFAULT_LVAR_UPDATE_FREQ;
        m_startEventNo = DEFAULT_START_EVENT_NO;
        return;
    }

	const char* pv;
	pv = ini.GetValue("General", "LogLevel");
    if (strcmp(pv, "Debug") == 0)
        m_logLevel = 4;
    else if (strcmp(pv, "Info") == 0)
        m_logLevel = 2;
    else if (strcmp(pv, "Buffer") == 0)
        m_logLevel = 3;
    else if (strcmp(pv, "Disable") == 0)
        m_logLevel = 1;
    else if (strcmp(pv, "Enable") == 0)
        m_logLevel = 6;
	else if (strcmp(pv, "Trace") == 0)
		m_logLevel = 5;
    else {
        m_logLevel = DEFAULT_LOG_LEVEL;
	}


    pv = ini.GetValue("General", "StartEventNo");
    if (pv) {
        m_startEventNo = strtol(pv + 2, nullptr, 16);
        if (m_startEventNo < 0x11000 || m_startEventNo > 0x1FFFF) {
            m_startEventNo = DEFAULT_START_EVENT_NO;
        }
    }
    else m_startEventNo = DEFAULT_START_EVENT_NO;

    pv = ini.GetValue("General", "LvarUpdateFrequency");
    if (pv) {
        m_lvarUpdateFrequency = atoi(pv);
        if (m_lvarUpdateFrequency < 0 || m_lvarUpdateFrequency > 100)
            m_lvarUpdateFrequency = 0;
    }
    else m_lvarUpdateFrequency = DEFAULT_LVAR_UPDATE_FREQ;

    pv = ini.GetValue("General", "UseSimConnection");
    if (pv) {
        m_simConnectConnection = atoi(pv);
     }
    else m_simConnectConnection = SIMCONNECT_OPEN_CONFIGINDEX_LOCAL;

}

Default::~Default() {}

Default* Default::GetInstance(const char* text) throw ()
{
    if (m_Instance == 0)
    {
        m_Instance = new Default(text);
    }
    return m_Instance;
}


int Default::GetLogLevel() { return m_logLevel;}
int Default::GetStartEventNo() { return m_startEventNo; };
int Default::GetLvarUpdateFrequency() { return m_lvarUpdateFrequency; };
int Default::GetSimConnectConnection() { return m_simConnectConnection; };
