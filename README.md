# SoalShiftSISOP20_modul4_B05
Di suatu perusahaan, terdapat pekerja baru yang super jenius, ia bernama jasir. Jasir baru bekerja selama seminggu di perusahan itu, dalam waktu seminggu tersebut ia selalu terhantui oleh ketidak amanan dan ketidak efisienan file system yang digunakan perusahaan tersebut. Sehingga ia merancang sebuah file system yang sangat aman dan efisien. Pada segi keamanan, Jasir telah menemukan 2 buah metode enkripsi file. Pada mode enkripsi pertama, nama file-file pada direktori terenkripsi akan dienkripsi menggunakan metode substitusi. Sedangkan pada metode enkripsi yang kedua, file-file pada direktori terenkripsi akan di-split menjadi file-file kecil. Sehingga orang-orang yang tidak menggunakan filesystem rancangannya akan kebingungan saat mengakses direktori terenkripsi tersebut. Untuk segi efisiensi, dikarenakan pada perusahaan tersebut sering dilaksanakan sinkronisasi antara 2 direktori, maka jasir telah merumuskan sebuah metode agar filesystem-nya mampu mengsingkronkan kedua direktori tersebut secara otomatis. Agar integritas filesystem tersebut lebih terjamin, maka setiap command yang dilakukan akan dicatat kedalam sebuah file log.
(catatan: filesystem berfungsi normal layaknya linux pada umumnya)
(catatan: mount source (root) filesystem adalah direktori /home/[user]/Documents)

Berikut adalah detail filesystem rancangan jasir:
Enkripsi versi 1:
a. Jika sebuah direktori dibuat dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
b. Jika sebuah direktori di-rename dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
c. Apabila sebuah direktori terenkripsi di-rename menjadi tidak terenkripsi, maka isi adirektori tersebut akan terdekrip.
d. Setiap pembuatan direktori terenkripsi baru (mkdir ataupun rename) akan tercatat ke sebuah database/log berupa file.
e. Semua file yang berada dalam direktori ter enkripsi menggunakan caesar cipher dengan key.
``` 9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO ```
f. Metode enkripsi pada suatu direktori juga berlaku kedalam direktori lainnya yang ada didalamnya.

Langkah-langkah :
- Pertama kami ambil template dari modul, yaitu yang memiliki fuse_operations ``` getattr, readdir, read ```
- Kami menambahkan fuse_operation yaitu ``` mkdir ``` (agar dapat membuat folder baru), ``` write ``` (agar dapat mengedit file), ``` create ``` (agar dapat membuat file), ``` open ``` (agar dapat membuka file), ``` access ``` (untuk systemcall), ``` utimens ``` (untuk mengupdate last access time yg akan digunakan di nomor 4), dan ``` truncate ``` (dibutuhkan untuk read/write filesystem karena membuat ulang file pertama akan memotongnya/truncate).
- Membuat enkripsi dan dekripsi sesuai ketentuan dan key-nya
- Untuk tiap function xmp_ akan memanggil function dekripsi
- Untuk function xmp_readdir juga memanggil enkripsi
- Disini kenapa dekripsi yang dipanggil, karena saat enkripsi yang dipanggil malah salah (harusnya bergerak 10 karakter ke kanan, tetapi malah ke kiri).
