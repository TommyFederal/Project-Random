# Project-Random
Disini adalah hasil kegabutan gw bilamana tak ada kerjaan.

Berikut instruksi buat install file2 kampret diatas.
Oh ya kalo ad yg nanya "Nih kok kek ai readme.md nya bang" hooh, soalnya gw mager bet buat bikin sendiri, +pada 9/14/2026 gua bikinnya 0422 subuh, itupun gw lagi persiapan buat ke sekolah AOKWOAKWOKAOW

# Cincin & Fotow Kita Blur 🎵📸

Dua project C++ gw yang seru-seruan buat bikin konten yang keren. Satu buat visualisasi lirik lagu, satu buat blur effect pake gesture. Yuk cek!

---

## Project 1: Cincin - Lyrics Visualizer 🎵

Yo, ini project C++/SFML buat visualisasi lirik lagu dengan animasi kotak yang bergerak-gerak, catatan musik yang jatuh, dan efek tabrakan yang keren.

### Yang seru:
- Lirik muncul dalam kotak yang pop in dan fade out (smooth banget!)
- Setiap kata muncul satu per satu (bisa diatur kecepatannya)
- Musik notes (♪ ♫) yang ngambang dan hilang
- Kotak tabrakan sama kotak lain terus ledak dengan efek hati, cincin, sama sparkle
- Bisa sesuaiin timing setiap baris buat cocok sama lagu
- Background gradient sama border bercahaya (bagus banget 👀)

### Yang perlu:
- **Windows**: MSYS2 MinGW64
- **g++**: 13.3.0 atau lebih
- **SFML**: 3.x
- Font file `.ttf` (taruh `font.ttf` di folder project)

### Setup SFML
```bash
pacman -S mingw-w64-x86_64-sfml
```

### Compile
```bash
g++ cincin_gui.cpp -o cincin_gui.exe -lsfml-graphics -lsfml-window -lsfml-system
```

### Run
```bash
./cincin_gui.exe
```

### Customize

**Ganti Lirik:**
```cpp
std::vector<std::string> liriknya = {
    "Lirik lo yang pertama",
    "Lirik lo yang kedua",
};
```

**Atur Timing (detik per baris):**
```cpp
std::vector<float> lineHoldOverride = {
    3.5f, 2.1f, 4.2f, ...
};
```

**Tweak Visual:**
- `WORD_INTERVAL`: Detik tiap kata muncul
- `POP_DURATION`: Seberapa cepat kotak terbang masuk
- `CLOSE_DURATION`: Seberapa cepat kotak hilang

---

## Project 2: Fotow Kita Blur 📸

Project C++/OpenCV yang blur webcam lo saat ngangkat tangan. Ada hati-hati yang ngambang sama text yang fade in/out. Inspo dari trend "Foto Kita Blur" yang lagi viral.

### Yang seru:
- Deteksi tangan pake skin color (real-time!)
- Blur langsung jalan pas tangan di atas
- Blur fade in/out yang mulus (bisa diatur kecepatannya)
- Hati emoji yang ngambang kayak confetti
- Text "Foto Kita Blurrr 🤍" di tengah yang fade
- Bisa tuning sensitivity deteksi tangan

### Yang perlu:
- **Windows**: MSYS2 MinGW64
- **g++**: 13.3.0 atau lebih
- **OpenCV**: 5.x
- **Qt6**: `mingw-w64-x86_64-qt6-base`
- PNG emoji hati (taruh `heart.png` di folder project)

### Setup OpenCV & Qt
```bash
pacman -S mingw-w64-x86_64-opencv mingw-w64-x86_64-qt6-base
```

### Compile
```bash
g++ fotow_kitaa_blurrr.cpp -o fotow_kitaa_blurrr.exe -I"C:/msys64/mingw64/include/opencv5" -L"C:/msys64/mingw64/lib" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs
```

### Run
```bash
./fotow_kitaa_blurrr.exe
```

**Jangan lupa `heart.png` harus di folder yang sama!**

### Customize

**Ganti Hati:**
Ganti `heart.png` dengan PNG apa aja yang punya transparent background.

**Tuning Deteksi Tangan (Ketat):**
```cpp
cv::Scalar lower1 = cv::Scalar(0, 60, 160);
cv::Scalar upper1 = cv::Scalar(8, 90, 255);
cv::Scalar lower2 = cv::Scalar(172, 60, 160);
cv::Scalar upper2 = cv::Scalar(180, 90, 255);
if (whitePixels > 12000) {
```

**Tuning Deteksi Tangan (Longgar):**
```cpp
cv::Scalar lower1 = cv::Scalar(0, 30, 120);
cv::Scalar upper1 = cv::Scalar(15, 60, 255);
cv::Scalar lower2 = cv::Scalar(165, 30, 120);
cv::Scalar upper2 = cv::Scalar(180, 60, 255);
if (whitePixels > 5000) {
```

**Atur Blur Fade:**
```cpp
const float BLUR_FADE_IN_SPEED = 1.0f;    // 1 = 1 detik fade in
const float BLUR_FADE_OUT_SPEED = 0.3f;   // 0.3 = 3 detik fade out
```

**Atur Posisi Window:**
```cpp
cv::resizeWindow("Foto Kita Blur", 1000, 700);
cv::moveWindow("Foto Kita Blur", 200, 100);
```

---

## VS Code Code Runner Setup

**Untuk Cincin:**
```json
"cpp": "cd $dir && g++ $fileName -o $fileNameWithoutExt.exe -lsfml-graphics -lsfml-window -lsfml-system && $dir$fileNameWithoutExt.exe"
```

**Untuk Fotow Kita Blur:**
```json
"cpp": "cd $dir && g++ $fileName -o $fileNameWithoutExt.exe -I\"C:/msys64/mingw64/include/opencv5\" -L\"C:/msys64/mingw64/lib\" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs && $dir$fileNameWithoutExt.exe"
```

---

Hanya gw dan TUHAN yang tau tu kode2 gw hasilin apaan soalnya gw masukin sesuai selera 🙏(dengan sedikit cicipan claude)
