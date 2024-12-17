#include "motor.h"

Motor::Motor() : fd_(-1), running_(false)
{
    // 初始化默认回调函数
    receive_callback_ = [](const uint8_t* data, size_t len) {
        std::cout << "Received " << len << " bytes: ";
        for (size_t i = 0; i < len; ++i) {
            printf("%02X ", data[i]);
        }
        std::cout << std::endl;
    };
}

Motor::~Motor()
{
    Stop();
}

bool Motor::Start() {
    if (running_) {
        std::cout << "UART already running" << std::endl;
        return false;
    }

    // 初始化串口
    fd_ = UART_Init(const_cast<char*>(DEVICE_NAME), BAUDRATE);
    if (fd_ < 0) {
        std::cout << "Failed to initialize UART" << std::endl;
        return false;
    }

    // 启动监听线程
    running_ = true;
    listen_thread_ = std::thread(&Motor::ListenThread, this);
    
    return true;
}

void Motor::Stop() {
    if (running_) {
        running_ = false;
        if (listen_thread_.joinable()) {
            listen_thread_.join();
        }
    }

    if (fd_ >= 0) {
        UART_Exit(fd_);
        fd_ = -1;
    }
}

void Motor::operator()() {
    ListenThread();
}

void Motor::ListenThread() {
    uint8_t buffer[BUFFER_SIZE];
    
    while (running_) {
        // 读取数据
        ssize_t bytes_read = read(fd_, buffer, BUFFER_SIZE);
        
        if (bytes_read > 0) {
            // 如果有回调函数，则调用回调函数处理数据
            if (receive_callback_) {
                receive_callback_(buffer, bytes_read);
            }
        } else if (bytes_read < 0) {
            // 读取错误
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Error reading from UART: " << strerror(errno) << std::endl;
                break;
            }
        }

        // 可以添加短暂延时以减少CPU占用
        usleep(1000);  // 1ms delay
    }
}

bool Motor::Send(const std::vector<uint8_t>& data)
{
    if (!running_ || fd_ < 0) {
        std::cerr << "Motor not running or invalid fd" << std::endl;
        return false;
    }

    if (data.empty()) {
        std::cerr << "Empty data" << std::endl;
        return false;
    }

    // 使用互斥锁保护写操作
    std::lock_guard<std::mutex> lock(write_mutex_);

    size_t total_written = 0;
    while (total_written < data.size()) {
        ssize_t written = write(fd_, data.data() + total_written, 
                              data.size() - total_written);
        
        if (written < 0) {
            if (errno == EINTR) {
                // 如果是被信号中断，继续写入
                continue;
            }
            std::cerr << "Failed to write to serial port: " 
                      << strerror(errno) << std::endl;
            return false;
        }
        
        total_written += written;
    }

    // 等待数据发送完成
    tcdrain(fd_);
    
    return true;
}