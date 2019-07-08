/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, Robert Bosch LLC.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Robert Bosch nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/
#ifndef USB_CAM_USB_CAM_H
#define USB_CAM_USB_CAM_H

#include <asm/types.h> /* for videodev2.h */

extern "C"
{
#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/mem.h>
}

// legacy reasons
#include <libavcodec/version.h>
#if LIBAVCODEC_VERSION_MAJOR < 55
#define AV_CODEC_ID_MJPEG CODEC_ID_MJPEG
#endif

#include <string>
#include <sstream>

#include <sensor_msgs/Image.h>
#include "datainfile/ImageDataFile.h"
#include "datainfile/datafile.h"

namespace usb_cam
{

  class UsbCam
  {
  public:
    typedef enum
    {
      IO_METHOD_READ,
      IO_METHOD_MMAP,
      IO_METHOD_USERPTR,
      IO_METHOD_UNKNOWN,
    } io_method;

    typedef enum
    {
      PIXEL_FORMAT_YUYV,
      PIXEL_FORMAT_UYVY,
      PIXEL_FORMAT_MJPEG,
      PIXEL_FORMAT_YUVMONO10,
      PIXEL_FORMAT_RGB24,
      PIXEL_FORMAT_UNKNOWN
    } pixel_format;

    UsbCam();
    ~UsbCam();

    // start camera
    void start(const std::string &dev, io_method io, pixel_format pf,
              int image_width, int image_height, int framerate);
    // shutdown camera
    void shutdown(void);

    // grabs a new image from the camera
    int grab_image(sensor_msgs::Image *image, int timeout);
    int grab_image(datainfile::ImageDataFile *img_data_file_, DataFile *dataFileObj, sensor_msgs::Image *img_, int timeout);

    // enables/disable auto focus
    void set_auto_focus(int value);

    // Set video device parameters
    void set_v4l_parameter(const std::string &param, int value);
    void set_v4l_parameter(const std::string &param, const std::string &value);

    static io_method io_method_from_string(const std::string &str);
    static pixel_format pixel_format_from_string(const std::string &str);

    void stop_capturing(void);
    void start_capturing(void);
    bool is_capturing();
    int fetch_device_path(const std::string usb_port, std::string &device_path,
                          const std::string camera_id_string_, const std::string dev_map_yml_);

  private:
    struct CameraImage
    {
      int width;
      int height;
      int bytes_per_pixel;
      int image_size;
      char *image;
      int is_new;
      int tv_sec;
      int tv_usec;
    };

    struct Buffer
    {
      void *start;
      size_t length;
    };

    int init_mjpeg_decoder(int image_width, int image_height);
    void mjpeg2rgb(char *MJPEG, int len, char *RGB, int NumPixels);
    bool process_image(const void *src, int len, boost::shared_ptr<CameraImage> dest);
    int read_frame();
    void uninit_device(void);
    void init_read(unsigned int buffer_size);
    void init_mmap(void);
    void init_userp(unsigned int buffer_size);
    void init_device(int image_width, int image_height, int framerate);
    void close_device(void);
    void open_device(void);
    // TODO
    //void reset_device(void);
    int grab_image(int timeout);

    bool convertYuyv2Rgb_;
    bool is_capturing_;
    std::string camera_dev_;
    std::string dump_path_;
    std::string camera_id_string_;
    unsigned int pixelformat_;
    bool monochrome_;
    io_method io_;
    int fd_;
    std::vector<Buffer> buffers_;
    unsigned int n_buffers_;
    AVFrame *avframe_camera_;
    AVFrame *avframe_rgb_;
    AVCodec *avcodec_;
    AVDictionary *avoptions_;
    AVCodecContext *avcodec_context_;
    int avframe_camera_size_;
    int avframe_rgb_size_;
    struct SwsContext *video_sws_;
    boost::shared_ptr<CameraImage> image_;
  };

} // namespace usb_cam

#endif
