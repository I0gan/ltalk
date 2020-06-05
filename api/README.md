 

# 客户端与服务端交互API

采用http协议与JSON文本传输, 无加密.

## 客户端交互http基本头部信息

```http
POST /?platform=linux HTTP/1.1
Host: 192.168.100.4
Origin: http://lyxf.xyz
Referer: http://ltalk.lyxf.xyz
Content-Type: application/json
User-Agent: Ltalk Client For Linux x64
Accept: application/json
Cookie: None
Date: 20-06-05 17:55:11
Content-Length: 153
Connection: Keep-Alive
Accept-Encoding: gzip, deflate
Accept-Language: en-US,*
```

必须信息必须包含 Referer, User-Agent, Date, 否则会拒绝请求

### URL

在进行客户端交互的时候, 参数值必须包含platform变量, 改变量值主要有三个: linux, windows, android

```
/?platform=linux
```

## Json交互格式

基本交互都采用Json进行传输交互, Json交互基本信息

请求格式

例如请求登录

```json
{
	"cmd" : 4,
	"uid" : "none"
	"date" : "2020-06-5 18:00:33",
	"token" : "none",
	"content_type", "user info"
	"content" : 
    [
		{
    	"account": "418894113", 
    	"password" : "******",
  		"nickname": "i0gan",
        "head_image" : "none",
    	"network_state" : "online"
        }
	]
}
```



服务端回复

例如获取好友列表

```json
{
	"cmd" : 3,
	"code" : 0,
	"date" : "2020-06-5 18:00:33",
	"token" : "none",
	"content_type", "user info"
	"content" : 
    [
		{
    	"account": "418894113", 
    	"nickname" : "I0gan",
  		"remark": "i0gan",
        "head_image" : "http://lyxf.xyz/",
    	"network_state" : "online"
        }
	]
}
```



## 登录模块API

### POST请求

```json
{
	"cmd" : 4,
	"uid" : "none"
	"date" : "2020-06-5 18:00:33",
	"token" : "none",
	"content_type", "user info"
	"content" : 
    [
		{
    	"account": "418894113", 
    	"password" : "******",
  		"nickname": "i0gan",
        "head_image" : "none",
    	"network_state" : "online"
        }
	]
}
```
