 

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
	"request" : 4,
	"uid" : "none",
	"date" : "2020-06-5 18:00:33",
	"token" : "none",
	"content_type" : "user info",
	"content" : 
    [
		{
    	"account": "418894113", 
    	"password" : "******",
  		"nickname": "i0gan",
        "head_image_url" : "none",
    	"network_state" : "windows_online"
        }
	]
}
```



## 注册

### 请求

#### post

json数据格式为

```json
{
	"request" : 0,
	"datetime" : "2020-06-5 18:00:33",
	"token" : "none",
    "uid" : "none",
	"content_type" : "register_info",
	"content" : 
	{
    	"name": "I0gan", 
    	"email" : "418894113@qq.com",
  		"phone_number": "1571807****",
        "address" : "GuiZhou",
    	"occupation" : "programer",
    	"password" : "******"
  	}
}
```

### 响应

#### 错误响应

响应码1代表失败, 11代表帐号已经存在

```json
{
 	"server": "Ltalk Server Debian Linux x64",
	"code" : "1",
	"datetime" : "2020-06-5 18:00:33"
}
```


#### 成功响应

响应代码code 为0 代表成功, 这时会返回以下内容, uid, token, access_url (用于页面跳转)

```json
{
	"server" : "Ltalk Server Debian Linux x64",
    "code" : 0,
	"datetime" : "2020-06-5 18:00:33",
	"uid" : "***********",
	"token" : "**********",
	"access_url" : "/?request=register_success&platform=web"
}
```

## 登录

### 请求

#### url:

```
http://ltalk.co/?request=login&platform=linux
```
#### post

json数据格式为

```json
{
	"request" : 1,
	"datetime" : "2020-06-5 18:00:33",
	"token" : "none",
    "uid" : "none",
	"content_type" : "login_info",
	"content" : 
	{
    	"account": "I0gan", 
    	"email" : "418894113@qq.com",
  		"phone_number": "1571807****",
        "address" : "GuiZhou",
    	"occupation" : "programer",
    	"password" : "******"
  	}
}

```


### 响应

#### 失败
响应码为1代表密码错误, 12代表帐号不存在, 13已经登录了

```json
{
 	"server": "Ltalk Server Debian Linux x64",
	"code" : "1",
    "request" : "login",
	"datetime" : "2020-06-5 18:00:33"
}
```



成功响应, 响应代码code 为0 代表成功, 这时会返回以下内容, uid, token

```json
{
	"server" : "Ltalk Server Debian Linux x64",
    "code" : 0,
    "request" : "login",
	"datetime" : "2020-06-5 18:00:33",
	"uid" : "***********",
	"token" : "**********",
}
```



## 登录成功后获取各种信息

### GET请求

#### url:

```
http://ltalk.co/?request=get_info&platform=linux&token=*****&uid=****
```


