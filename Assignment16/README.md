g++ -o demo main.c camera.c img_processing_cv.cpp fpga.c \
    20sim_files/panController/*.c \
    20sim_files/tiltController/*.c \
    -I 20sim_files/panController \
    -I 20sim_files/tiltController \
    $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0) \
    $(pkg-config --cflags --libs opencv4) \
    -lm -lpthread



gcc -o demo main.c camera.c img_processing.c fpga.c \
    20sim_files/panController/*.c \
    20sim_files/tiltController/*.c \
    -I 20sim_files/panController \
    -I 20sim_files/tiltController \
    $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0) \
    -lm -lpthread