```bash
gcc -o gst_yuv_recorder yuv_recorder.c `pkg-config --cflags --libs gstreamer-1.0`

./gst_yuv_recorder output.yuv

ls -lh output.yuv


```