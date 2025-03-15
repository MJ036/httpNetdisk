#include "Oss.h"
#include <alibabacloud/oss/OssClient.h>
#include <alibabacloud/oss/OssFwd.h>
#include <alibabacloud/oss/client/ClientConfiguration.h>
#include <ctime>
#include <iostream>
#include <sys/socket.h>


int Oss::putFile(string remotePath, string localPath){
        InitializeSdk();
    ossInfo info;
    ClientConfiguration conf;
    conf.maxConnections = 30;
    conf.connectTimeoutMs = 3000;
    conf.requestTimeoutMs = 3000;
    OssClient client(info.EndPoint_out,info.AccessKeyID,info.AccessKeySecret,conf);
    PutObjectOutcome outcome = client.PutObject(info.Bucket,remotePath,localPath);
      if(!outcome.isSuccess()){
          cerr << "code = " << outcome.error().Code()
               << ",message = " << outcome.error().Message() << "\n";
      }
        ShutdownSdk();
    return 0;




}

string Oss::getFileUrl(string remotePath){
    InitializeSdk();
    ossInfo info;
    ClientConfiguration conf;
    conf.maxConnections = 30;
    conf.connectTimeoutMs = 3000;
    conf.requestTimeoutMs = 3000;

    OssClient client(info.EndPoint_out,info.AccessKeyID,info.AccessKeySecret,conf);
    time_t expire = time(nullptr) + 600;

    StringOutcome outcome = client.GeneratePresignedUrl(info.Bucket,remotePath,expire,Http::Get);
    if(outcome.isSuccess()){
        return outcome.result();
    }
    else{
        cerr << "error!\n";
        return "error";
    }

    ShutdownSdk();

}
