#include "event.h"

static bool is_vis = true;

EventListener::EventListener(Motor& p_motor) 
    : running_(false)
    , m_motor(p_motor)
{
    // do nothing
}

EventListener::~EventListener() {
    Stop();
}

bool EventListener::Start() {
    if (running_) {
        return false;
    }

    running_ = true;
    listen_thread_ = std::thread(&EventListener::ListenThread, this);
    return true;
}

void EventListener::Stop() {
    if (running_) {
        running_ = false;
        if (listen_thread_.joinable()) {
            listen_thread_.join();
        }
    }
}

void EventListener::PrintKeyEvent(const std::string& device, int code, int value) {
    printf("[%s] Key Event - Code: %d, Value: %d\n", 
           device.c_str(), code, value);

    // 对焦，远离
    if (code == KEY_F5 && value == 1)
    {
        std::vector<uint8_t> data = {0x24, 0x02, 0x04, 0x00, 0x0C, 0xFE, 0xFF, 0xFF, 0xD0};
        m_motor.Send(data);
    }
    // 对焦，拉近
    else if (code == KEY_F4 && value == 1)
    {
        std::vector<uint8_t> data = {0x24, 0x02, 0x04, 0x00, 0xF4, 0x01, 0x00, 0x00, 0xD7};
        m_motor.Send(data);
    }
    else if (code == KEY_F3 && value == 1)
    {
        usr.pseudo++;
        usr.pseudo %= PSEUDO_NUMS;
    }
    else if (code == KEY_F2 && value == 1)
    {
        if (is_vis)
            system("/root/app/utils/vis.sh");
        else
            system("killall gst-launch-1.0");
        is_vis = !is_vis;
    }
    else
    {
        std::cout << "Others" << std::endl;
    }
}

void EventListener::ProcessDevice(const std::string& device) {
    int fd = open(device.c_str(), O_RDONLY);
    if (fd == -1) {
        printf("Failed to open device: %s\n", device.c_str());
        return;
    }

    struct input_event ev;
    ssize_t size;

    while (running_) {
        size = read(fd, &ev, sizeof(struct input_event));
        
        if (size == sizeof(struct input_event)) {
            if (ev.type == EV_KEY) {
                PrintKeyEvent(device, ev.code, ev.value);
            }
        }
    }

    close(fd);
}

void EventListener::ListenThread() {
    const int MAX_DEVICES = 2;
    struct pollfd fds[MAX_DEVICES];
    const char* devices[MAX_DEVICES] = {
        "/dev/input/event2",  // adc-key
        "/dev/input/event4"   // gpio-key
    };

    // 打开设备
    for (int i = 0; i < MAX_DEVICES; i++) {
        fds[i].fd = open(devices[i], O_RDONLY);
        if (fds[i].fd == -1) {
            printf("Failed to open %s\n", devices[i]);
            // 关闭之前已经打开的设备
            for (int j = 0; j < i; j++) {
                close(fds[j].fd);
            }
            return;
        }
        fds[i].events = POLLIN;
    }

    // 事件循环
    while (running_) {
        int ret = poll(fds, MAX_DEVICES, 100); // 100ms timeout

        if (ret > 0) {
            for (int i = 0; i < MAX_DEVICES; i++) {
                if (fds[i].revents & POLLIN) {
                    struct input_event ev;
                    ssize_t size = read(fds[i].fd, &ev, sizeof(ev));
                    
                    if (size == sizeof(ev)) {
                        if (ev.type == EV_KEY) {
                            PrintKeyEvent(devices[i], ev.code, ev.value);
                        }
                    }
                }
            }
        }
    }

    // 关闭设备
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }
}