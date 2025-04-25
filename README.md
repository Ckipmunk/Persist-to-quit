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

Hiện tại có 4 loại cây có sẵn và 3 loại upgrade. Sẽ cập nhật nhiều hơn ở bản cập nhật gần nhất

|                                                                                               | Tên cây	     | Chức năng                                                                                                                    |
|-----------------------------------------------------------------------------------------------|--------------|------------------------------------------------------------------------------------------------------------------------------|
| ![Peashooter](https://github.com/user-attachments/assets/87f800ae-422a-44b3-b2a8-3a88810638f2)| Peashooter	 | Mỗi lần bắn một đậu vào zombie.                                                                                              |
| ![Sunflower](https://github.com/user-attachments/assets/cf9a2b82-76f4-451f-9e32-31d286cba6fe) | Sunflower	   | Tạo thêm mặt trời để trồng cây                                                                                               |
| ![Walnut](https://github.com/user-attachments/assets/d220fdf4-481b-4242-ab86-a8f87ecde935)    | Wall-nut	   | Chịu được nhiều phát ăn của zombie để bảo vệ các cây khác.                                                                   |
| ![frame_0](https://github.com/user-attachments/assets/7f0b4c8a-f4d8-4bbf-9b02-e69b193f2c89)   | Sunflower LV2| Sản lượng gấp đôi sunflower
| ![frame_2](https://github.com/user-attachments/assets/76e26558-ca43-4cb6-a403-00a6b98b3574)   | Peashooter LV2| Sức mạnh gấp 2 lần peashooter                                                             |
|![frame_0](https://github.com/user-attachments/assets/31f00e18-3ca4-4001-bcf4-5d79bfffe0f3)    | Peashooter LV3| Sức mạnh gấp 3 lần peashooter                                                              |
| ![Cherrybomb](https://github.com/user-attachments/assets/03675863-d568-4b4e-984e-c518f0ae3e4d)| Cherry Bomb	 | Ngay lập tức tiêu diệt tất cả zombie trong khu vực 3x3.                                                                      |
## b. Các loại zombie

- Có 5 loại zombie:

|                                         | Tên zombie 	       | Chức năng                                                                                        |
|-----------------------------------------|--------------------|--------------------------------------------------------------------------------------------------|
| ![frame_0](https://github.com/user-attachments/assets/b93ba0c1-e28f-4a81-b309-4bf07048a5e0)| ZombieNormal	  | Ăn cây và não của bạn.                                                                           |
| ![frame_1](https://github.com/user-attachments/assets/c91b04e6-2b64-4dd0-8365-3eaceaf47d01)| Conehead Zombie	   | Trâu gấp 2 lần zombie thường                                                                 |
| ![frame_1](https://github.com/user-attachments/assets/d568e697-5148-4241-96fb-b3def831b697)| Boss Zombie | Cực trâu , dame cực to, có chế độ truyền nhiễm                                                                 |

# 4. Chiến thắng và thất bại

- Bạn sẽ không thể chiến thắng game này.

- Bạn sẽ thất bại nếu có một zombie nào đó đi hết sân để vào nhà bạn.

<div style="text-align: center;">

![ZombiesWon](https://github.com/user-attachments/assets/88e96e3a-6589-4867-9d1b-96bf17dda696)

</div>

---

### Về đồ họa của game:
- Ta có thể tìm kiếm hình ảnh hoặc gif từ một số trang web/mã nguồn mở như sau:
     - https://plantsvszombies.fandom.com/wiki/Main_Page
     - https://tenor.com/search/plants-vs-zombies-gifs
     - https://pvz-fusion.fandom.com/wiki/PvZ:_Fusion_Wiki
     - https://github.com/nmhienbn/PVZ-Kaito-NMH-Edition
     - https://github.com/wahajnb/PlantsVZombies
- Ta cũng có thể nhờ sự trợ giúp của AI để chuyển từ gif thành file frames zip hoặc 1 số trang web như ezgif.com
- AI cũng có thể vẽ 1 loạt sprite sheet hộ bạn nữa đấy. Nếu bạn muốn tự vẽ có thể sử dụng app www.piskelapp.com or adobe Photoshop adobe illustrator
### Về source code game
- src:
    - ui:
       - FadeEffect.cpp/.h: Xử lý hiệu ứng fadein,fadeout,screen khi thua game
    - core:
       - sound: Xử lý về phần âm thanh
       - screenImage: Xử lý load hình ảnh như FadeEffect
       - Region: Xử lý về khu vực trồng cây
- main:
    - Các hằng số và biến toàn cục:
         - Các hằng số như kích thước màn hình, thời gian animation, chi phí cây trồng...
         - Các biến trạng thái game như số lượng ánh sáng mặt trời, trạng thái thua...
         - Các biến SDL như window, renderer, font...
         - Các đối tượng game như lưới trồng cây, danh sách các cây, zombie, đạn...
    - Các cấu trúc dữ liệu
         - Plant: Lớp cơ sở cho các loại cây
         - Sunflower, Walnut, PeaShooter, CherryBomb: Các loại cây cụ thể  
         - Zombie: Đại diện cho các loại zombie với các trạng thái di chuyển và ăn 
         - Pea: Đạn do PeaShooter bắn ra 
         - Sun: Ánh sáng mặt trời để thu thập 
         - Grid: Lưới trồng cây
    - Các hàm chức năng chính
         - Khởi tạo
           - initGrid(): Khởi tạo lưới trồng cây
           - Các hàm load...Frames(): Tải các frame animation
           - loadResources(): Tải hình ảnh, âm thanh
           - close(): Giải phóng tài nguyên
         - Logic game
           - canPlacePlant(): Kiểm tra có thể đặt cây tại vị trí không
           - produceSunFromSunflowers(): Tạo ánh sáng từ hoa hướng dương
           - spawnSun(): Tạo ánh sáng ngẫu nhiên
           - updateSuns(): Cập nhật vị trí ánh sáng
           - collectSun(): Thu thập ánh sáng
           - spawnZombie(), spawnBossZombie(): Tạo zombie
           - updateZombies(): Cập nhật trạng thái zombie
           - shootPea(): PeaShooter bắn đạn
           - updatePeas(): Cập nhật đạn
           - updateCherryBombs(): Cập nhật cherrybomb
         - Render
           - Các hàm render...(): Vẽ các đối tượng lên màn hình
           - Các hàm preview: Hiển thị preview khi kéo thả cây
         - Xử lý input
           - handleMouseEvent(): Xử lý sự kiện chuột để đặt cây, thu thập ánh sáng        
    - Vòng lặp game chính
         - Xử lý sự kiện
         - Cập nhật trạng thái game
         - Render các đối tượng
         - Xử lý va chạm và tương tác
