#include "listen.h"

Listen::Listen()
    : m_topic("pre")
    , m_target("host")
{
    m_mqtt_handle = mqtt_new(const_cast<char*>("127.0.0.1"), 1883, const_cast<char*>(m_topic.c_str()));

    if (m_mqtt_handle == NULL)
    {
        litelog.log.fatal("MQTT new fail.");
        exit(EXIT_FAILURE);
    }

    if (mqtt_connect(m_mqtt_handle, nullptr, nullptr) != MQTT_SUCCESS)
    {
        litelog.log.fatal("MQTT connect fail.");
        exit(EXIT_FAILURE);
    }

    int ret = mqtt_subscribe(m_mqtt_handle, const_cast<char*>(m_target.c_str()), QOS_EXACTLY_ONCE);

    litelog.log.info("MQTT subscribe %s: %d", m_target.c_str(), ret);
}

void Listen::operator()()
{
    while (1)
    {
        if (mqtt_receive(m_mqtt_handle, 200) == MQTT_SUCCESS)
        {
            // litelog.log.debug("MQTT receive:\n%s", std::string(m_mqtt_handle->received_message, m_mqtt_handle->received_message_len).c_str());
            // litelog.log.debug("MQTT receive from:%s", std::string(m_mqtt_handle->received_topic, m_mqtt_handle->received_topic_len).c_str());

            std::cout << ("MQTT receive:\n%s", std::string(m_mqtt_handle->received_message, m_mqtt_handle->received_message_len).c_str()) << std::endl;

            if (m_mqtt_handle->received_message_len < 0)
            {
                litelog.log.warning("Listen received message length < 0");
                continue;
            }

            JWrap jw(std::string{m_mqtt_handle->received_message, static_cast<size_t>(m_mqtt_handle->received_message_len)});

            int retval = 0;
            pthread_mutex_lock(&usr.mutex);
            switch (jw.GetCodeEnum())
            {
            case JWrap::CODE_ENUM::PSEUDO:
                retval = Set_Pseudo(jw.GetValue());
                break;
            case JWrap::CODE_ENUM::GAS_ENHANCEMENT:
                retval = Set_Gas_Enhancement(jw.GetValue());
                break;
            case JWrap::CODE_ENUM::AUTOFOCUS_IR:
                retval = Set_IR_Focus(jw.GetValue());
                break;
            default:
                litelog.log.warning("Unknown code in json.");
                retval = Controller::ErrorCode::INVALID_CODE;
                break;
            }
            pthread_mutex_unlock(&usr.mutex);

            mqtt_publish(m_mqtt_handle, const_cast<char*>(m_topic.c_str()), const_cast<char*>(jw.CreateReturnJson(std::to_string(retval)).c_str()), QOS_EXACTLY_ONCE);
        }
    }
}