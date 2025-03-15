#ifndef __Oss_H__
#define __Oss_H__

#include "mai.h"
#include <alibabacloud/oss/OssClient.h>
#include <alibabacloud/oss/client/ClientConfiguration.h>
#include <alibabacloud/oss/model/Bucket.h>
#include <sys/socket.h>
using namespace AlibabaCloud::OSS;

using namespace std;

struct ossInfo{
    string EndPoint_out ="oss-cn-shenzhen.aliyuncs.com";
    string EndPoint_in = "oss-cn-shenzhen-internal.aliyuncs.com";
    string AccessKey;
    string AccessKeySecret;
    string Bucket = "25-3-7";
};

class Oss
{
public:
    Oss(){};

    int putFile(string remotePath, string localPath);
    string getFileUrl(string remotePath);


    ~Oss(){};

};
#endif


