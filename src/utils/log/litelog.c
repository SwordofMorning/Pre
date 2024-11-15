#include "litelog.h"

/* ======================================================================================== */
/* ======================================== Socket ======================================== */
/* ======================================================================================== */

int Litelog_Socket_Open_Device(struct Litelog_Socket_Wrap* p_socket)
{
    p_socket->device = socket(AF_INET, SOCK_DGRAM, 0);

    return p_socket->device == -1 ? -1 : 0;
}

int Litelog_Socket_Bind_Target(struct Litelog_Socket_Wrap* p_socket, const char* p_self_ip, uint16_t* p_self_port)
{
    int retval = 0;

    p_socket->self_address.sin_family = AF_INET;
    p_socket->self_address.sin_addr.s_addr = inet_addr(p_self_ip);
    p_socket->self_address.sin_port = htons(*p_self_port);

    socklen_t slen = sizeof(p_socket->self_address);

    while (bind(p_socket->device, (struct sockaddr*)&(p_socket->self_address), slen) == -1)
    {
        if (errno != EADDRINUSE)
        {
            perror("bind fail");
            close(p_socket->device);
            retval = -1;
            goto out_return;
        }
        (*p_self_port)++;
        if (*p_self_port > 65535)
        {
            *p_self_port = 60000;
        }
        p_socket->self_address.sin_port = htons(*p_self_port);
    }

out_return:
    return retval;
}

int Litelog_Socket_Init(struct Litelog_Socket_Wrap* p_socket, const char* p_self_ip, uint16_t p_self_port)
{
    int retval = 0;

    if (Litelog_Socket_Open_Device(p_socket) != 0)
    {
        retval = -1;
        goto out_return;
    }

    if (Litelog_Socket_Bind_Target(p_socket, p_self_ip, &p_self_port) != 0)
    {
        retval = -2;
        goto out_return;
    }

out_return:
    return retval;
}

void Litelog_Socket_Create_Target(struct sockaddr_in* p_target, const char* p_target_ip, uint16_t p_target_port)
{
    (*p_target).sin_family = AF_INET;
    (*p_target).sin_addr.s_addr = inet_addr(p_target_ip);
    (*p_target).sin_port = htons(p_target_port);
}

int Litelog_Socket_Send(int device, uint8_t* buffer, size_t n, struct sockaddr* target)
{
    socklen_t length = sizeof(*target);
    return sendto(device, buffer, n, MSG_CONFIRM, target, length);
}

int Litelog_Socket_Recv(int device, uint8_t* buffer, size_t n, struct sockaddr* target, int timeout_ms)
{
    fd_set rfds;
    struct timeval tv;
    struct timeval* ptv = NULL;
    int selectRet;

    if (timeout_ms > 0)
    {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        ptv = &tv;
    }

    FD_ZERO(&rfds);
    FD_SET(device, &rfds);

    selectRet = select(device + 1, &rfds, NULL, NULL, ptv);

    // Select ret error
    if (selectRet == -1)
    {
        return -1;
    }
    // Timeout
    else if (selectRet == 0)
    {
        errno = EAGAIN;
        return -2;
    }
    // Return data
    else
    {
        socklen_t length = sizeof(*target);
        return recvfrom(device, (char*)buffer, n, MSG_WAITALL, target, &length);
    }
}

void Litelog_Socket_Exit(struct Litelog_Socket_Wrap* p_socket)
{
    if (p_socket->device != -1)
    {
        close(p_socket->device);
        p_socket->device = -1;
    }
}

/* ======================================================================================== */
/* ======================================== Client ======================================== */
/* ======================================================================================== */

struct Litelog_Socket_Wrap local;
// local of monitor
struct sockaddr_in monitor;
// local of controller
struct sockaddr_in controller;

void Litelog_Init(const char* p_program_name)
{
    const char* local_ip = "127.0.0.1";
    uint16_t local_port = 0;

    const char* monitor_ip = "127.0.0.1";
    uint16_t monitor_port = 20000;

    const char controller_ip[] = "127.0.0.1";
    uint16_t controller_port = 20001;

    Litelog_Socket_Init(&local, local_ip, local_port);
    Litelog_Socket_Create_Target(&monitor, monitor_ip, monitor_port);
    Litelog_Socket_Create_Target(&controller, controller_ip, controller_port);

    // Program Name
    size_t name_len = strlen(p_program_name);
    if (name_len <= PROGRAM_NAME_DISPLAY_WIDTH)
    {
        snprintf(program_name, PROGRAM_NAME_BUFFER_SIZE, "[%s]", p_program_name);
    }
    else
    {
        // The program name is too long and needs to be truncated
        char truncated_name[PROGRAM_NAME_DISPLAY_WIDTH + 1];
        strncpy(truncated_name, p_program_name, PROGRAM_NAME_DISPLAY_WIDTH - 3);
        truncated_name[PROGRAM_NAME_DISPLAY_WIDTH - 3] = '.';
        truncated_name[PROGRAM_NAME_DISPLAY_WIDTH - 2] = '.';
        truncated_name[PROGRAM_NAME_DISPLAY_WIDTH - 1] = '.';
        truncated_name[PROGRAM_NAME_DISPLAY_WIDTH] = '\0';
        snprintf(program_name, PROGRAM_NAME_BUFFER_SIZE, "[%s]", truncated_name);
    }
}

void Litelog_Exit()
{
    Litelog_Socket_Exit(&local);
}

int Litelog_Send(uint8_t* buffer, size_t n, struct sockaddr_in target)
{
    return Litelog_Socket_Send(local.device, buffer, n, (struct sockaddr*)&target);
}

int Litelog_Log(uint8_t level, const char* str, size_t n)
{
    int ret = 0;

    // Valid Check
    uint8_t valid_levels = ~LOG_LEVEL_K & 0xFF;
    if ((level & ~valid_levels) != 0 || (level & (level - 1)) != 0)
    {
        ret = -1;
        return ret;
    }

    // Calculate length
    size_t program_name_len = strlen(program_name);

    // Malloc Buffer: level(1) + program_name + content
    uint8_t* buffer = (uint8_t*)malloc(1 + program_name_len + n);
    if (buffer == NULL)
    {
        ret = -2;
        return ret;
    }

    // Fill data
    buffer[0] = level;
    memcpy(buffer + 1, program_name, program_name_len);
    memcpy(buffer + 1 + program_name_len, str, n);

    ret = Litelog_Send(buffer, 1 + program_name_len + n, monitor);

    free(buffer);

out_return:
    return ret;
}

int Litelog_Log_Manual(uint8_t level, const char* file, int line, const char* func, const char* format, ...)
{
    int ret = 0;

    // Valid Check
    uint8_t valid_levels = ~LOG_LEVEL_K & 0xFF;
    if ((level & ~valid_levels) != 0 || (level & (level - 1)) != 0)
    {
        ret = -1;
        return ret;
    }

    // Create Log Message
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);

    // Get File Name
    const char* file_name = strrchr(file, '/') ? strrchr(file, '/') + 1 : file;

    // Format Log Message
    char formatted_log[512];
    snprintf(formatted_log, sizeof(formatted_log), "%s:%d %s: %s", file_name, line, func, log_buffer);

    // Calculate length
    size_t program_name_len = strlen(program_name);
    size_t log_length = strlen(formatted_log);

    // Malloc Buffer
    uint8_t* buffer = (uint8_t*)malloc(1 + program_name_len + log_length);
    if (buffer == NULL)
    {
        ret = -2;
        return ret;
    }

    // Fill data
    buffer[0] = level;
    memcpy(buffer + 1, program_name, program_name_len);
    memcpy(buffer + 1 + program_name_len, formatted_log, log_length);

    ret = Litelog_Send(buffer, 1 + program_name_len + log_length, monitor);

    free(buffer);

out_return:
    return ret;
}

int Litelog_Shutdown()
{
    uint8_t command[1] = {CTL_STOP_PROGRAM};
    return Litelog_Send(command, 1, controller);
}

int Litelog_Change_Level(uint8_t level)
{
    uint8_t command[2] = {CTL_CHANGE_LEVEL, level};
    return Litelog_Send(command, 2, controller);
}

int Litelog_Switch_Page()
{
    uint8_t command[1] = {CTL_SWITCH_PAGE};
    return Litelog_Send(command, 1, controller);
}

/* ===================================================================================== */
/* ======================================== API ======================================== */
/* ===================================================================================== */

void Litelog_Log_Fatal(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_F, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Error(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_E, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Warning(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_W, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Notice(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_N, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Info(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_I, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Debug(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_D, log_buffer, strlen(log_buffer));
}

void Litelog_Log_Trace(const char* format, ...)
{
    char log_buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buffer, sizeof(log_buffer), format, args);
    va_end(args);
    Litelog_Log(LOG_LEVEL_T, log_buffer, strlen(log_buffer));
}

// clang-format off
struct Litelog litelog =
{
    .init = Litelog_Init,
    .exit = Litelog_Exit,
    .ctl = {
        .shutdown = Litelog_Shutdown,
        .change_level = Litelog_Change_Level,
        .switch_page = Litelog_Switch_Page,
    },
    .log = {
        .fatal = Litelog_Log_Fatal,
        .error = Litelog_Log_Error,
        .warning = Litelog_Log_Warning,
        .notice = Litelog_Log_Notice,
        .info = Litelog_Log_Info,
        .debug = Litelog_Log_Debug,
        .trace = Litelog_Log_Trace,
        .manual  = Litelog_Log_Manual
    }
};
// clang-format on