#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================================== */
/* ======================================== Define ======================================== */
/* ======================================================================================== */

/* Log Level */
enum
{
    // Silence, 0000 0000
    LOG_LEVEL_S = 0,

    // Fatal, 0000 0001
    LOG_LEVEL_F = 1 << 0,

    // Error, 0000 0010
    LOG_LEVEL_E = 1 << 1,

    // Warning, 0000 0100
    LOG_LEVEL_W = 1 << 2,

    // Notice, 0000 1000
    LOG_LEVEL_N = 1 << 3,

    // Info, 0001 0000
    LOG_LEVEL_I = 1 << 4,

    // Debug, 0010 0000
    LOG_LEVEL_D = 1 << 5,

    // Trace, 0100 0000
    LOG_LEVEL_T = 1 << 6,

    // Kernel, 1000 0000
    LOG_LEVEL_K = 1 << 7,

    // All, 1111 1111
    LOG_LEVEL_A = 0xFF
};

/* Log Level Combine */
enum
{
    // Fatal, Error, Warning, Notice, Info
    LOG_LEVEL_PRODUCTION = LOG_LEVEL_F | LOG_LEVEL_E | LOG_LEVEL_W | LOG_LEVEL_N | LOG_LEVEL_I,

    // Fatal, Error, Warning, Notice, Info, Debug, Trace
    LOG_LEVEL_DEVELOPMENT = LOG_LEVEL_PRODUCTION | LOG_LEVEL_D | LOG_LEVEL_T,

    // Fatal, Error, Warning, Notice, Info, Debug, Trace, Kernel
    LOG_LEVEL_FULL = LOG_LEVEL_A
};

enum
{
    CTL_STOP_PROGRAM = 0,
    CTL_CHANGE_LEVEL,
    CTL_SWITCH_PAGE,
};

/* ======================================================================================== */
/* ======================================== Socket ======================================== */
/* ======================================================================================== */

struct Litelog_Socket_Wrap
{
    // socket device.
    int device;
    // this socket's ip and port.
    struct sockaddr_in self_address;
};

/**
 * @brief Create device descriptor for socket wrap.
 * 
 * @param p_socket 
 * @return success or fail.
 * @retval 0, success.
 * @retval -1, socket open device fail.
 */
int Litelog_Socket_Open_Device(struct Litelog_Socket_Wrap* p_socket);

/**
 * @brief Bind ip and port for one socket.
 * 
 * @param p_socket socket wrap.
 * @param p_self_ip ip of itself.
 * @param p_self_port port of itself.
 * @return int 
 */
int Litelog_Socket_Bind_Target(struct Litelog_Socket_Wrap* p_socket, const char* p_self_ip, uint16_t* p_self_port);

int Litelog_Socket_Init(struct Litelog_Socket_Wrap* p_socket, const char* p_self_ip, uint16_t p_self_port);

void Litelog_Socket_Create_Target(struct sockaddr_in* p_target, const char* p_target_ip, uint16_t p_target_port);

int Litelog_Socket_Send(int device, uint8_t* buffer, size_t n, struct sockaddr* target);

int Litelog_Socket_Recv(int device, uint8_t* buffer, size_t n, struct sockaddr* target, int timeout_ms);

void Litelog_Socket_Exit(struct Litelog_Socket_Wrap* p_socket);

/* ======================================================================================== */
/* ======================================== Client ======================================== */
/* ======================================================================================== */

extern struct Litelog_Socket_Wrap local;
// local of monitor
extern struct sockaddr_in monitor;
// local of controller
extern struct sockaddr_in controller;

#define PROGRAM_NAME_DISPLAY_WIDTH 15
#define PROGRAM_NAME_BUFFER_SIZE (PROGRAM_NAME_DISPLAY_WIDTH + 5)
static char program_name[PROGRAM_NAME_BUFFER_SIZE] = {0};

void Litelog_Init(const char* p_program_name);

void Litelog_Exit();

int Litelog_Send(uint8_t* buffer, size_t n, struct sockaddr_in target);

int Litelog_Log(uint8_t level, const char* str, size_t n);

int Litelog_Log_Manual(uint8_t level, const char* file, int line, const char* func, const char* format, ...);

int Litelog_Shutdown();

int Litelog_Change_Level(uint8_t level);

int Litelog_Switch_Page();

/* ===================================================================================== */
/* ======================================== API ======================================== */
/* ===================================================================================== */

void Litelog_Log_Fatal(const char* format, ...);

void Litelog_Log_Error(const char* format, ...);

void Litelog_Log_Warning(const char* format, ...);

void Litelog_Log_Notice(const char* format, ...);

void Litelog_Log_Info(const char* format, ...);

void Litelog_Log_Debug(const char* format, ...);

void Litelog_Log_Trace(const char* format, ...);

struct LitelogLevel
{
    /**
     * @brief Fatal error, the program cannot continue running.
     */
    void (*fatal)(const char* format, ...);
    /**
     * @brief Error, but the program can continue to run.
     */
    void (*error)(const char* format, ...);
    /**
     * @brief Warning, possible problems
     */
    void (*warning)(const char* format, ...);
    /**
     * @brief Important but not false information.
     */
    void (*notice)(const char* format, ...);
    /**
     * @brief Normal information.
     */
    void (*info)(const char* format, ...);
    /**
     * @brief Debug Information
     */
    void (*debug)(const char* format, ...);
    /**
     * @brief The most detailed tracking information。
     */
    void (*trace)(const char* format, ...);
    /**
     * @brief Manually call the log writing interface.
     * 
     * @param level used to specific level, @see LOG_LEVEL_*.
     * @param file __FILE__
     * @param line __LINE__
     * @param func __func__
     */
    int (*manual)(uint8_t level, const char* file, int line, const char* func, const char* format, ...);
};

struct LitelogControl
{
    /**
     * @brief Terminate the litelog process.
     */
    int (*shutdown)();
    /**
     * @brief Change log level.
     * @param level used to specific level, @see LOG_LEVEL_*.
     */
    int (*change_level)(uint8_t level);
    /**
     * @brief Split the log and create a new log file; aka new-page.
     */
    int (*switch_page)();
};

struct Litelog
{
    /**
     * @brief Init the local litelog client's param.
     * @param p_program_name name of current process.
     */
    void (*init)(const char* p_program_name);
    /**
     * @brief Release the resources allocated by the current .h file, mainly socket.
     */
    void (*exit)();
    struct LitelogControl ctl;
    struct LitelogLevel log;
};

extern struct Litelog litelog;

#ifdef __cplusplus
}
#endif