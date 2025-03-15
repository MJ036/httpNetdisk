#include <SimpleAmqpClient/Channel.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <nlohmann/detail/meta/type_traits.hpp>
#include <string>
#include <nlohmann/json_fwd.hpp>
#include <nlohmann/json.hpp>

class Rabbitmq{
public:
    Rabbitmq(){}

    void publish(std::string);

    void consumer();

    ~Rabbitmq(){};
};

