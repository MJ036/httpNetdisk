#include "Rabbitmq.h"
#include <SimpleAmqpClient/BasicMessage.h>
#include <SimpleAmqpClient/Channel.h>
#include <SimpleAmqpClient/Envelope.h>
#include <iostream>
#include "Oss.h"
void Rabbitmq::publish(std::string filename){
    //建立和rabbitmq的连接
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create();
    //创建一个消息
    AmqpClient::BasicMessage::ptr_t messages = AmqpClient::BasicMessage::Create(filename);
    //发布消息
    //三个参数：交换器、key、消息正文
    channel->BasicPublish("file-exchange","file",messages);
    std::cout << "message is sent\n";
}

void Rabbitmq::consumer(){
    using Json = nlohmann::json;
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create();
    channel->BasicConsume("file-queue");

    AmqpClient::Envelope::ptr_t envelop;
    Json msg;
    std::string filepath;
    while(1){
        bool flag = channel->BasicConsumeMessage(envelop,3000);
        if(flag){
            msg = nlohmann::json::parse(envelop->Message()->Body());
            filepath = msg["filepath"];
            Oss oss;
            string remote_path = filepath;
            oss.putFile(remote_path,filepath);
            std::cout << "onf file has been put\n";
        }else{
            std::cerr << "timeout!\n";
        }
    }

    //  while(1){
    //      bool flag = channel->BasicConsumeMessage(envelop,3000);
    //      if(flag){
    //          std::cout << "Body = " << envelop->Message()->Body() << "\n"; 
    //      }else{
    //          std::cerr << "timeout!\n";
    //      }
    //  }
}
