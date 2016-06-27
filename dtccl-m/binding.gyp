{
  'targets': [
    {
      'target_name': 'dtccl-n',
      'link_settings': {
            'libraries': [
                '/usr/local/lib/libopencv_adas.so',
                '/usr/local/lib/libopencv_bgsegm.so',
                '/usr/local/lib/libopencv_bioinspired.so',
                '/usr/local/lib/libopencv_calib3d.so',
                '/usr/local/lib/libopencv_ccalib.so',
                '/usr/local/lib/libopencv_core.so',
                '/usr/local/lib/libopencv_datasets.so',
                '/usr/local/lib/libopencv_face.so',
                '/usr/local/lib/libopencv_features2d.so',
                '/usr/local/lib/libopencv_flann.so',
                '/usr/local/lib/libopencv_highgui.so',
                '/usr/local/lib/libopencv_imgcodecs.so',
                '/usr/local/lib/libopencv_imgproc.so',
                '/usr/local/lib/libopencv_latentsvm.so',
                '/usr/local/lib/libopencv_line_descriptor.so',
                '/usr/local/lib/libopencv_ml.so',
                '/usr/local/lib/libopencv_objdetect.so',
                '/usr/local/lib/libopencv_optflow.so',
                '/usr/local/lib/libopencv_photo.so',
                '/usr/local/lib/libopencv_reg.so',
                '/usr/local/lib/libopencv_rgbd.so',
                '/usr/local/lib/libopencv_saliency.so',
                '/usr/local/lib/libopencv_shape.so',
                '/usr/local/lib/libopencv_stereo.so',
                '/usr/local/lib/libopencv_stitching.so',
                '/usr/local/lib/libopencv_superres.so',
                '/usr/local/lib/libopencv_surface_matching.so',
                '/usr/local/lib/libopencv_text.so',
                '/usr/local/lib/libopencv_tracking.so',
                '/usr/local/lib/libopencv_videoio.so',
                '/usr/local/lib/libopencv_video.so',
                '/usr/local/lib/libopencv_videostab.so',
                '/usr/local/lib/libopencv_xfeatures2d.so',
                '/usr/local/lib/libopencv_ximgproc.so',
                '/usr/local/lib/libopencv_xobjdetect.so',
                '/usr/local/lib/libopencv_xphoto.so',

            ]
          },
      'sources': [ 'dtccl-n.cc','myobject.h','myobject.cc','DTCCL.cpp','DTCCL.h','fastcluster/fastcluster.cpp',
                'fastcluster/fastcluster.h','base64/src/cdecode.c','base64/src/cencode.c',
                'base64/include/b64/cdecode.h','base64/include/b64/decode.h','base64/include/b64/cencode.h',
                'base64/include/b64/encode.h'],
      'include_dirs': [ '<!(node -e \"require(\'nan\')\")' ,'<!@(pkg-config --cflags opencv)','base64/include' ],
      'cflags' : [ '-std=c++11','-Ibase64/include','-Werror','-pedantic','-DBUFFERSIZE=65536'],
      'cflags!' :['-fno-exceptions'],
      'cflags_cc!': [ '-fno-rtti',  '-fno-exceptions']
    }
  ]
}
