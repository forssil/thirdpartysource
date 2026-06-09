#ifndef AUDIO_LOG_H
#define AUDIO_LOG_H

#include <stdio.h>
#include <stdarg.h>

class AudioLog {
public:
    // 构造函数，传入日志文件名
    AudioLog(const char* filename);
    ~AudioLog();

    // 格式化输出日志
    void Write(const char* format, ...);
    
    // 关闭日志文件
    void Close();

private:
    FILE* m_pFile;
};

// 声明全局日志实例指针，供宏内部使用
extern AudioLog* g_pAudioLog;

// ==========================================
// 宏封装
// ==========================================

// 1. 初始化日志（程序启动或需要开始记录时调用）
#define AUDIO_LOG_INIT(filename) \
    do { \
        if (g_pAudioLog == NULL) { \
            g_pAudioLog = new AudioLog(filename); \
        } \
    } while(0)

// 2. 销毁日志（程序退出时调用）
#define AUDIO_LOG_DESTROY() \
    do { \
        if (g_pAudioLog != NULL) { \
            delete g_pAudioLog; \
            g_pAudioLog = NULL; \
        } \
    } while(0)

// 3. 基础写入宏：与直接调用 Write() 效果完全一致
#define AUDIO_LOG_WRITE(format, ...) \
    do { \
        if (g_pAudioLog != NULL) { \
            g_pAudioLog->Write(format, ##__VA_ARGS__); \
        } \
    } while(0)

// 4. 增强写入宏（推荐）：自动附带当前【文件名】和【行号】
#define AUDIO_LOG_INFO(format, ...) \
    do { \
        if (g_pAudioLog != NULL) { \
            g_pAudioLog->Write("[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while(0)

#endif // AUDIO_LOG_H
