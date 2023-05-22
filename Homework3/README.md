# 141337 - Lập trình mạng - IT4060

1. Buổi 3 - BVN:

- Bài 1: Sử dụng hàm select()/poll(), viết chương trình **chat_server** thực hiện các chức năng sau: Nhận kết nối từ các client, và vào hỏi tên client cho đến khi client gửi đúng cú pháp: **“client_id: client_name”** trong đó client_name là tên của client, xâu ký tự viết liền. Sau đó nhận dữ liệu từ một client và gửi dữ liệu đó đến các client còn lại, ví dụ: client có id “abc” gửi “xinchao” thì các client khác sẽ nhận được: “abc: xin chao”hoặc có thể thêm thời gian vào trước ví dụ: “2023/05/06 11:00:00PM abc: xin chao”.

- Bài 2: Sử dụng hàm select()/poll(), viết chương trình **telnet_server** thực hiện các chức năng sau: Khi đã kết nối với 1 client nào đó, yêu cầu client gửi user và pass, so sánh với file cơ sở dữ liệu là một file text, mỗi dòng chứa một cặp user + pass. vídụ:
  + admin admin
  + guest nopass...
  
Nếu so sánh sai, không tìm thấy tài khoản thì báo lỗi đăng nhập. Nếu đúng thì đợi lệnh từ client, thực hiện lệnh và trả kết quả cho client. Dùng hàm system(“dir > out.txt”) để thực hiện lệnh.dir là ví dụ lệnh dir mà client gửi.
- > out.txt để định hướng lại dữ liệu ra từ lệnh dir, khi đó kết quả lệnh dir sẽ được ghi vào file văn bản.
