#include "listen.h"

Listen::Listen()
{
    m_mqtt_handle = mqtt_new(const_cast<char*>("127.0.0.1"), 1883, const_cast<char*>("pre"));

    if(m_mqtt_handle == NULL)
    {
        litelog.log.fatal("MQTT new fail.");
        exit(EXIT_FAILURE);
    }

    if (mqtt_connect(m_mqtt_handle, nullptr, nullptr) != MQTT_SUCCESS)
    {
        litelog.log.fatal("MQTT connect fail.");
        exit(EXIT_FAILURE);
    }

    int ret = mqtt_subscribe(m_mqtt_handle, const_cast<char*>("ui"), QOS_EXACTLY_ONCE);

    litelog.log.debug("MQTT subscribe ui: %d", ret);
}

void Listen::operator()()
{
    while (1)
    {
        std::cout << "Hello World" << mqtt_publish(m_mqtt_handle, const_cast<char*>("pre"), const_cast<char*>("Hello World"), QOS_EXACTLY_ONCE) << std::endl;
        sleep(1);
    }
}