# 17. Custom Animation: GIF to C-Array (RGB565)

Panduan membuat animasi custom untuk Digital Pet dari GIF, lalu mengubah setiap frame menjadi C-array RGB565 yang dapat disimpan di header Arduino (.h).

## 1. Ezgif — Edit, Bersihkan Background, Resize, dan Split GIF

**URL:** https://ezgif.com/split

### Hapus background gelap menjadi transparan

1. Buka GIF di Ezgif.
2. Masuk ke tab **Effects** → **Replace color with transparency**.
3. Pilih **custom (HEX)** dan masukkan warna gelap GIF, misalnya `#0d0d18`.
4. Centang **don't stack frames**.
5. Atur **Fuzz %** ke kisaran `8`–`12`.
6. **Jangan centang** opsi **clean up edges**.
7. Klik **Apply selected**.

### Isi background dengan hitam pekat

1. Masuk lagi ke tab **Effects** → **Color presets**.
2. Centang **change background color**.
3. Isi warna `#000000`.
4. Klik **Apply selected**.

### Pangkas jumlah frame (opsional)

Jika jumlah frame terlalu banyak:

1. Masuk ke tab **Optimize**.
2. Pada **Optimization method**, pilih **Remove every 2nd frame**.
3. **Jangan centang** **Adjust speed to match the original (50%)**.
4. Klik **Optimize GIF!**.

### Resize ke 240 × 240

1. Masuk ke tab **Resize**.
2. Isi Width `240` dan Height `240`.
3. Pada **If aspect ratio doesn't match**, pilih **Center and pad to fit**.
4. Atur warna padding menjadi `#000000`.
5. Pada **Resize method**, pilih **Nearest neighbor** atau **Gifsicle**.
6. Klik **Resize image!**.

### Ambil semua frame

1. Masuk ke tab **Split**.
2. Klik **Split to frames!**.
3. Download hasil split dalam format ZIP.

## 2. image2cpp — Konversi Frame ke C-Array RGB565

**URL:** https://javl.github.io/image2cpp/

### Upload frame

1. Extract ZIP hasil split dari Ezgif.
2. Pada **Select Images**, upload semua gambar frame sekaligus (batch upload).

### Image Settings

| Setting | Nilai |
|---|---|
| Canvas size | `240 × 240` |
| Background color | Black (`#000000`) |
| Scaling | Original size (jika frame sudah 240 × 240) |
| Dithering | None |

Dithering diset ke **None** agar latar belakang hitam tetap murni sebagai nilai RGB565 `0x0000`.

### Output Settings

| Setting | Nilai |
|---|---|
| Code output format | Arduino code |
| Draw mode | Horizontal - 2 bytes per pixel (565) |

### Catatan endianness / byte order

Jika warna di layar ST7789 tampak terbalik saat diuji:

- Aktifkan opsi **Swap bytes** di image2cpp; atau
- Ubah parameter ke-4 pada pemanggilan `tft.writePixels(..., true, false)` di file `.ino` menjadi `true`.

Uji hasil pada layar setelah mengubah byte order untuk memastikan warna sudah benar.

### Generate code

1. Klik **Generate code**.
2. Salin hasil C-array ke file header `.h`.
3. Pastikan nama konstanta dan pointer array sesuai dengan yang dipakai file `.ino`, termasuk:
   - `INLINE_SKATE_FRAME_COUNT`
   - `INLINE_SKATE_FRAME_INTERVAL_MS`
   - Pointer frame yang disimpan dengan `PROGMEM`

Nama pada contoh di atas mengikuti pola aset Inline Skate. Sesuaikan prefix/nama konstanta jika membuat animasi dengan nama lain.

## 3. Checklist sebelum dipakai di firmware

- [ ] Semua frame sudah berukuran 240 × 240.
- [ ] Urutan frame sudah benar.
- [ ] Background sudah sesuai dengan warna target.
- [ ] Output image2cpp menggunakan RGB565 / 2 bytes per pixel.
- [ ] Nama konstanta, pointer array, dan jumlah frame cocok dengan kode di `.ino`.
- [ ] Data frame disimpan di `PROGMEM`.
- [ ] Byte order sudah diuji pada LCD ST7789.
