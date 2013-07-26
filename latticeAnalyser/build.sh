#export PKG_CONFIG_PATH='/media/atul/a4c78183-78c2-4a82-9985-21937168d57f/Summer13/opencv/release2/unix-install'
# export LD_LIBRARY_PATH=/usr/local/src/opencv-2.4.5/release/lib
#export LD_LIBRARY_PATH=/usr/local/src/opencv-2.4.5/DieHard/lib
export LD_LIBRARY_PATH=/usr/local/lib/

#/usr/local/lib/libopencv_calib3d.so /usr/local/lib/libopencv_contrib.so /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_features2d.so /usr/local/lib/libopencv_flann.so /usr/local/lib/libopencv_gpu.so /usr/local/lib/libopencv_highgui.so /usr/local/lib/libopencv_imgproc.so /usr/local/lib/libopencv_legacy.so /usr/local/lib/libopencv_ml.so /usr/local/lib/libopencv_nonfree.so /usr/local/lib/libopencv_objdetect.so /usr/local/lib/libopencv_photo.so /usr/local/lib/libopencv_stitching.so /usr/local/lib/libopencv_superres.so /usr/local/lib/libopencv_ts.so /usr/local/lib/libopencv_video.so /usr/local/lib/libopencv_videostab.so
# /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_calib3d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_contrib.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_core.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_features2d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_flann.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_gpu.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_highgui.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_imgproc.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_legacy.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ml.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_nonfree.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_objdetect.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_photo.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_stitching.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_superres.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ts.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_video.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_videostab.so

#export LD_LIBRARY_PATH=/usr/local/src/opencv-2.4.5/opencvFFMPEGbuild/lib

#g++ hello-world.cpp -o hello-world -I /usr/local/include/opencv -L /usr/local/lib -lm -lcv -lhighgui -lcvaux

#g++ -std=c++0x -pthread -ggdb `pkg-config --cflags opencv` -o latticeAnalyser latticeAnalyser.cpp `pkg-config --libs opencv`

g++ -std=c++0x -pthread -ggdb -Iinclude/ `pkg-config --cflags opencv` `libusb-config --cflags` -o latticeAnalyser latticeAnalyser.cpp `pkg-config --libs opencv` `libusb-config --libs`

#g++ -std=c++0x -pthread -ggdb `pkg-config --cflags opencv` -o latticeAnalyser latticeAnalyser.cpp /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_calib3d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_contrib.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_core.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_features2d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_flann.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_gpu.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_highgui.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_imgproc.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_legacy.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ml.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_nonfree.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_objdetect.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_photo.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_stitching.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_superres.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ts.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_video.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_videostab.so

#this is for an IPP build
#g++ -std=c++0x -pthread -ggdb `pkg-config --cflags opencv` -o latticeAnalyser latticeAnalyser.cpp /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_calib3d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_contrib.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_core.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_features2d.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_flann.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_gpu.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_highgui.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_imgproc.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_legacy.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ml.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_nonfree.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_objdetect.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_photo.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_stitching.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_superres.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_ts.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_video.so /usr/local/src/opencv-2.4.5/DieHard/lib/libopencv_videostab.so
#this is for a  standard build
# g++ -std=c++0x -pthread -ggdb `pkg-config --cflags opencv` -o latticeAnalyser latticeAnalyser.cpp /usr/local/src/opencv-2.4.5/release/lib/libopencv_calib3d.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_contrib.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_core.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_features2d.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_flann.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_gpu.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_highgui.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_imgproc.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_legacy.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_ml.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_nonfree.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_objdetect.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_photo.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_stitching.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_superres.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_ts.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_video.so /usr/local/src/opencv-2.4.5/release/lib/libopencv_videostab.so

# libopencv_calib3d.so
# libopencv_contrib.so
# libopencv_core.so
# libopencv_features2d.so
# libopencv_flann.so
# libopencv_gpu.so
# libopencv_highgui.so
# libopencv_imgproc.so
# libopencv_legacy.so
# libopencv_ml.so
# libopencv_nonfree.so
# libopencv_objdetect.so
# libopencv_photo.sos
# libopencv_stitching.so
# libopencv_superres.so
# libopencv_ts.so
# libopencv_video.so
# libopencv_videostab.so