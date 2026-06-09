#include "AudioLog.h"

// 定义全局日志指针
AudioLog* g_pAudioLog = NULL;

AudioLog::AudioLog(const char* filename) {
    if (filename != NULL) {
        // 追加模式。若需每次覆盖则改为 "w"
        m_pFile = fopen(filename, "a");
    } else {
        m_pFile = NULL;
    }
}

AudioLog::~AudioLog() {
    Close();
}

void AudioLog::Write(const char* format, ...) {
    if (m_pFile != NULL) {
        va_list args;
        va_start(args, format);
        vfprintf(m_pFile, format, args);
        va_end(args);
        
        // 保证不丢失崩溃前的数据
        fflush(m_pFile); 
    }
}

void AudioLog::Close() {
    if (m_pFile != NULL) {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}
