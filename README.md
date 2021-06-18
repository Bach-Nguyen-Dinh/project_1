# **Đề tài: điều khiển thiết bị và cập nhật dữ liệu cảm biến từ xa thông qua web site**
<br/>
<br/>
<br/>

Phần cứng:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NodeMCU ESP8266  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Cảm biến DHT11  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Đèn LED

Phần mềm:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Broker Mosquitto  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NodeJS  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;OpenSSL

Ngôn ngữ:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;C++  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;JavaScript, html, CSS

Giao thức:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;MQTT, WebSockets, SSL/TLS

<br/>

# **Cách thức hoạt động của giao thức MQTT**
Giả sử có 2 client sử dụng giao thức MQTT để giao tiếp với nhau, chúng sẽ không trực tiếp trao đổi các message mà các message sẽ được gửi tới, xử lí và phân loại bởi broker.

Quá trình gửi và nhận message giữa 2 client sẽ xảy ra như sau: 

\- Client_1 tạo một payload bao gồm:

- topic (có sẵn hoặc tạo mới) để publish lên đó

- message cần publish

- QoS và retain

\- Client_1 gửi payload cho broker.

\- Broker xác nhận kết nối với client\_1, nhận và xử lí payload:

- QoS của payload có 3 loại:

  - QoS = 0: message được gửi đi đúng 1 lần, không được đảm bảo đã gửi đến nơi, không có xác nhận từ người nhận là đã nhận được.

  - QoS = 1: message được đảm bảo đã gửi đến nới ít nhất 1 lần, không có xác nhận từ người nhận là đã nhận được.

  - QoS = 2: message được đảm bảo đã gửi đến nơi và người nhận xác nhận là đã nhận được.

- Retain có 2 loại:

  - Retain = 1: broker sẽ lưu lại message kèm theo giá trị của QoS để coi như message là giá trị tốt cuối cùng nhận được. Mỗi khi có một client mới subsribe tới topic sẽ nhận được message.

  - Retain = 0: mặc định đối với mọi message, ngược lại của retain = 1

  - Broker sẽ kiểm tra xem có client nào subscribe tới topic và gửi message cho client đó.

\- Client_2 muốn nhận được message chỉ cần thực hiện subsribe tới topic mà message được publish lên.

<br/>

# **Cách thức hoạt động của giao thức Websockets**
Thông thường, client và server kết nối thông qua htttp, sau đó chuyển sang sử dụng Websockets.

Trong trường hợp sử dụng MQTT thông qua WebSockets, WebSockets tạo thành một đường ống bên trong là MQTT truyền qua lại.

Broker sẽ đóng gói MQTT packet bên trong một WebSockets packet và gửi nói tới client.

Client nhận và mở gói MQTT packet bên trong WebSockets packet và tiếp tục xử lí MQTT packet.

<br/>

# **Cách thức hoạt động của giao thức SSL/TLS**
Trong trường hợp cụ thể này, chỉ cần sử dụng 3 file:

- CA certicate file

- Server certificate file

- Server key file

Đầu tiên, client kết nối với server sử dụng SSL/TLS (https). Server sẽ phản hồi lại client Server certificate file trong đó có đính kèm Server key file.  

Tiếp theo, client phải chứa CA certicate file để xác nhận Server certificate file. Sau khi xác nhận, client cùng với server sử dụng Server key file để tạo ra một private key.  

Cuối cùng, client và server sẽ sử dụng mã hóa dựa trên private key đó.

<br/>

# **OpenSSL**  
Sử dụng OpenSSL để tạo ra các self-signed certificate  
  
Bước 1: Tạo một **CA key**  
Command:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;openssl genrsa -des3 -out ca.key 2048  
  
Bước 2: Tạo một **CA certificate** bằng **CA key** ở bước 1  
Command:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;openssl req -new -x509 -days 1826 -key ca.key -out ca.crt  
  
Bước 3: Tạo một **server key**  
Command:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;openssl genrsa -out server.key 2048  
  
Bước 4: Tạo một **server certificate request** bằng **server key** ở bước 3  
Command:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;openssl req -new -out server.csr -key server.key  
  
Bước 5: Sử dụng **CA certificate** ở bước 2 để xác nhận **server certificate request** ở bước 4 từ đó tạo ra một **server certificate** tương ứng  
Command:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 360
