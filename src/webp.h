#ifndef WEBP_H_
#define WEBP_H_

#include <memory>
#include <string>

#include "webp/decode.h"

class WebP {
 public:
  enum Format {
    IMG_PNG = 0,
    IMG_JPEG,
  };

  WebP(const std::string& path);
  WebP(const WebP& other);
  WebP(WebP&& other);
  ~WebP();

  WebP& operator=(const WebP& other);
  WebP& operator=(WebP&& other);

  bool IsValid() const {
    return is_valid_;
  }

  uint8_t* data() const { return img_data_.get(); }
  size_t size() const { return img_size_; }
  int width() const { return bitstream_.width; }
  int height() const { return bitstream_.height; }

  // 将webp图片保存为其它格式的图片
  // 如果has_alpha为true，保存为png；否则，保存为jpg。
  bool SaveImage(Format format, const std::string& out_file);

 private:
  bool Initialize();

  // webp to png
  bool WritePNG(FILE* out_file, const WebPDecBuffer* const buffer);

  // webp to jpeg
  bool WriteJPEG(FILE* out_file, const WebPDecBuffer* const buffer);

  std::string path_;

  bool is_valid_;

  std::unique_ptr<uint8_t[]> img_data_;
  size_t img_size_;

  WebPBitstreamFeatures bitstream_;
};  // class WebP

#endif  // WEBP_H_
