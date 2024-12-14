#include "event.h"

EventListener::EventListener() : running_(false) {}

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