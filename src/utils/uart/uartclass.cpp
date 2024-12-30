#include "uartclass.h"

UART::UART(const char* device_name, int baudrate, int buffer_size)
    : m_device_name(device_name)
    , m_baudrate(baudrate)
    , m_buffer_size(buffer_size)
    , m_fd(-1)
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

UART::~UART()
{
    Stop();
}

bool UART::Start()
{
    if (m_running)
    {
        std::cout << "UART already running" << std::endl;
        return false;
    }

    m_fd = UART_Init(const_cast<char*>(m_device_name), m_baudrate);
    if (m_fd < 0)
    {
        std::cout << "Failed to initialize UART" << std::endl;
        return false;
    }

    m_running = true;
    m_listen_thread = std::thread(&UART::ListenThread, this);

    return true;
}

void UART::Stop()
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

void UART::ListenThread()
{
    uint8_t buffer[m_buffer_size];

    while (m_running)
    {
        ssize_t bytes_read = read(m_fd, buffer, m_buffer_size);

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

        usleep(10 * 1000);
    }
}

bool UART::Send(const std::vector<uint8_t>& data)
{
    if (!m_running || m_fd < 0)
    {
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
                continue;
            }
            std::cerr << "Failed to write to serial port: " << strerror(errno) << std::endl;
            return false;
        }

        total_written += written;
    }

    tcdrain(m_fd);

    return true;
}

void UART::SetCallback(std::function<void(const uint8_t*, size_t)> callback)
{
    m_receive_callback = callback;
}