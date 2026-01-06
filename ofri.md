The HTTP message you get looks like:

```
GET /ipsum.html HTTP/1.1\r\n
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n
Accept-Encoding: gzip, deflate, br, zstd\r\n
Accept-Language: en-US,en;q=0.9\r\n
Cache-Control: no-cache\r\n
Connection: keep-alive\r\n
DNT: 1\r\n
Host: localhost:4221\r\n
Pragma: no-cache\r\n
Referer: http://localhost:4221/\r\n
Sec-Fetch-Dest: document\r\n
Sec-Fetch-Mode: navigate\r\n
Sec-Fetch-Site: same-origin\r\n
Sec-Fetch-User: ?1\r\n
Upgrade-Insecure-Requests: 1\r\n
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36\r\n
sec-ch-ua: "Chromium";v="143", "Not A(Brand";v="24"\r\n
sec-ch-ua-mobile: ?0\r\n
sec-ch-ua-platform: "Linux"\r\n
\r\n
```
```
```

you need to parse this request into (while checking it is in the format specified):
```c
struct http_request_t {
  METHOD - GET
  FILE - /ipsum.html
  VERSION - HTTP/1.1
};
```

then you need to check if the file is in the root directory, i.e. use get_safe_path (like in handler.c).
you need to make a type


```c
struct http_response_t {
  VERSION - HTTP/1.1
  STATUS CODE - 200 if file is ok, 404 otherwise
  REASON - "OK" for 200, "NOT FOUND" for 404.
  Content-Type - (See HTTP Protocol for content type)
  Content-Length: length of the BODY (see HTTP Protocol)
  uint8_6 BODY - the file contents
};
```

you should make a function send_response that sends it in the format

```
VERSION STATUS CODE REASON\r\n
Content-Type: %s\r\n
Content-Length: %zu\r\n
\r\n
BODY
```
```
```
