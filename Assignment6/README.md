```bash
gcc -o test test.c `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0`

./test 300


```