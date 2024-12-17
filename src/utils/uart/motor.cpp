#include "motor.h"

Motor::Motor()
    : m_fd(-1)
    , m_running(false)
{
    m_receive_callback = [](const uint8_t* data, size_t len) {
        std::cout << "Received " << len << " bytes: ";
        for (size_t i = 0; i < len; ++i)
        {
            printf("%02X ", data[i]);
        }
        std::cout << std::endl;
    };
}

Motor::~Motor()
{
    Stop();
}

bool Motor::Start()
{
    if (m_running)
    {
        std::cout << "UART already running" << std::endl;
        return false;
    }

    // 初始化串口
    m_fd = UART_Init(const_cast<char*>(DEVICE_NAME), BAUDRATE);
    if (m_fd < 0)
    {
        std::cout << "Failed to initialize UART" << std::endl;
        return false;
    }

    // 启动监听线程
    m_running = true;
    m_listen_thread = std::thread(&Motor::ListenThread, this);

    return true;
}

void Motor::Stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_listen_thread.joinable())
        {
            m_listen_thread.join();
        }
    }

    if (m_fd >= 0)
    {
        UART_Exit(m_fd);
        m_fd = -1;
    }
}

void Motor::operator()()
{
    ListenThread();
}

void Motor::ListenThread()
{
    uint8_t buffer[BUFFER_SIZE];

    while (m_running)
    {
        ssize_t bytes_read = read(m_fd, buffer, BUFFER_SIZE);

        if (bytes_read > 0)
        {
            if (m_receive_callback)
            {
                m_receive_callback(buffer, bytes_read);
            }
        }
        else if (bytes_read < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Error reading from UART: " << strerror(errno) << std::endl;
                break;
            }
        }

        // 10ms delay
        usleep(10 * 1000);
    }
}

bool Motor::Send(const std::vector<uint8_t>& data)
{
    if (!m_running || m_fd < 0)
    {
        std::cerr << "Motor not running or invalid fd" << std::endl;
        return false;
    }

    if (data.empty())
    {
        std::cerr << "Empty data" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(m_write_mutex);

    size_t total_written = 0;
    while (total_written < data.size())
    {
        ssize_t written = write(m_fd, data.data() + total_written, data.size() - total_written);

        if (written < 0)
        {
            if (errno == EINTR)
            {
                // If it is interrupted by a signal, continue writing
                continue;
            }
            std::cerr << "Failed to write to serial port: " << strerror(errno) << std::endl;
            return false;
        }

        total_written += written;
    }

    // Waiting for data to be sent
    tcdrain(m_fd);

    return true;
}

void Motor::SetCallback(std::function<void(const uint8_t*, size_t)> callback)
{
    m_receive_callback = callback;
}