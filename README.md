# Persist-to-quit

- Thử nghiệm game: LINK_VIDEO

# Giới thiệu game

Persist to quit là 1 game thuộc thể loại tower defense. Đắm mình vào cuộc chiến giữa cây cối và zombie, bạn hãy cố
gắng hết sức để bảo vệ  mình bằng những loại cây cối kì dị. Trồng cây để tiêu diệt các loại zombie đang cố gắng tấn công bạn.
Thỏa mãn sự sáng tạo bằng cách cho đột biến cây tuy nhiên game không có thể chiến thắng được.Nếu bạn có sự sáng tạo vô hạn thì game cũng sẽ vô hạn.

- [1. Bắt đầu game](#1bắt-đầu-game)
- [2. Các thành phần trong game:](#2-các-thành-phần-trong-game)
- [3. Cách chơi](#3-cách-chơi)
    * [a. Các loại cây](#acác-loại-cây)
    * [b. Các loại zombie](#bcác-loại-zombie)
- [4. Chiến thắng và thất bại](#4-chiến-thắng-và-thất-bại)
- [Về đồ họa của game:](#về-đồ-họa-của-game)
- [Về source code game](#về-source-code-game)

# 1. Bắt đầu game

Chờ một chút rồi sẽ vào game.

![begin](https://github.com/user-attachments/assets/b1e9ba0e-0bac-4abb-8832-66135e151446)

Sau khi logo này chạy xong thì game sẽ chính thức bắt đầu

# 2. Các thành phần trong game:

-    Sun (Mặt trời) ![sun](https://github.com/user-attachments/assets/3c73b174-eb58-4f5b-aaab-99864297f890)

     : là đơn vị để mua plant (cây) trong game. Nhấp vào để thu.
     
-    Sun bar (Thanh mặt trời) ![sun_bar](https://github.com/user-attachments/assets/a8895acb-e23b-4850-867a-82e400f23f6c)

     : là bảng đếm số mặt trời mà người chơi đang có

- Plant seeds: tất cả các giống cây là người chơi đang sở hữu. Để chọn một giống cây mang đi trồng, người chơi phải
  click vào plant seed để chọn cây rồi trồng trên sân. Tất nhiên,
  bạn phải đủ sun để mua cây và plant seed phải có thời gian chờ khác nhau để trồng cây giống đó lần nữa.

<div style="text-align: center;">

![Screenshot 2025-04-26 000733](https://github.com/user-attachments/assets/98615da3-b3aa-4ad5-ba6c-02ece73545f3)

</div>

- Shovel (Xẻng): Là vật dụng vô cùng hữu hiệu để bạn có thể đào đi những cây không cần thiết và thay vào đó cây mới.

<div style="text-align: center;">

![Shovel](https://github.com/user-attachments/assets/d4387718-2fea-4799-b465-135d84f0de57)

</div>

- Bố cục game cơ bản:

<div style="text-align: center;">

![background1a](https://github.com/user-attachments/assets/403674b4-6ee1-44e8-b9a9-08b19b9d4953)

</div>

 

# 3. Cách chơi

Trồng cây để tấn công và phòng thủ, không cho zombie bước vào nhà bạn.<br/>
Để trồng cây, Chọn hạt giống của cây đó rồi đặt vào ô muốn trồng. <br/>
Có thể đào những cây đang sử dụng bằng xẻng.<br/>
Cây sẽ bị zombie ăn mất.

## a. Các loại cây

Có 6 loại cây, lần lượt theo thứ tự có thể nhận được trong game là:

|                                                                                               | Tên cây	     | Chức năng                                                                                                                    |
|-----------------------------------------------------------------------------------------------|--------------|------------------------------------------------------------------------------------------------------------------------------|
| ![Peashooter](https://github.com/user-attachments/assets/87f800ae-422a-44b3-b2a8-3a88810638f2)| Peashooter	 | Mỗi lần bắn một đậu vào zombie.                                                                                              |
| ![Sunflower](https://github.com/user-attachments/assets/cf9a2b82-76f4-451f-9e32-31d286cba6fe) | Sunflower	   | Tạo thêm mặt trời để trồng cây                                                                                               |
| ![Walnut](https://github.com/user-attachments/assets/d220fdf4-481b-4242-ab86-a8f87ecde935)    | Wall-nut	   | Chịu được nhiều phát ăn của zombie để bảo vệ các cây khác.                                                                   |
| <img src="resources/images/Plant_sheet/snowpea/SnowPea.png" width=52>        | 	    | Mỗi lần bắn một đậu băng vào zombie. Đậu băng không tăng sát thương nhưng làm chậm tốc độ di chuyển và tốc độ ăn của zombie. |
| <img src="resources/images/Plant_sheet/potatomine/Potato_Mine.png" width=56> | Potato Mine  | Tiêu diệt zombie khi chạm nó nhưng cần thời gian để kích hoạt.                                                               |
| <img src="resources/images/Plant_sheet/cherrybomb/Cherrybomb.png" width=48>  | Cherry Bomb	 | Ngay lập tức tiêu diệt tất cả zombie trong khu vực 3x3.                                                                      |

## b. Các loại zombie

- Có 5 loại zombie:

|                                         | Tên zombie 	       | Chức năng                                                                                        |
|-----------------------------------------|--------------------|--------------------------------------------------------------------------------------------------|
| ![image](resources/images/Choose_level/normal.png) | Browncoat Zombie	  | Ăn cây và não của bạn.                                                                           |
| ![image](resources/images/Choose_level/flag.png)   | Flag Zombie	       | Báo hiệu một lượng lớn zombie đang chuẩn bị ra.                                                  |
| ![image](resources/images/Choose_level/cone.png)   | Conehead Zombie	   | Trâu gấp 3 lần zombie thường                                                                     |
| ![image](resources/images/Choose_level/bucket.png) | Buckethead Zombie	 | Trâu gấp 6.5 lần zombie thường                                                                   |
| ![image](resources/images/Choose_level/door.png)   | Door Zombie	       | Trâu gấp 6.5 lần zombie thường. Không bị ảnh hưởng bởi sự làm chậm của đạn băng nếu vẫn còn cửa. |

# 4. Chiến thắng và thất bại

- Bạn sẽ không thể chiến thắng game này.

- Bạn sẽ thất bại nếu có một zombie nào đó đi hết sân để vào nhà bạn.

<div style="text-align: center;">

![ZombiesWon](https://github.com/user-attachments/assets/88e96e3a-6589-4867-9d1b-96bf17dda696)

</div>

---

### Về đồ họa của game:

[[Cách làm hiệu ứng từng cây, zombie trong game]](about_graphics.md)

### Về source code game:
