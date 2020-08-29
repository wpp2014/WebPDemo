#include "src/webp.h"

#include <assert.h>
#include <string.h>

#include "png.h"
#include "jpeglib.h"
#include "src/file_util.h"
#include "webp/decode.h"

static void PNGAPI PNGErrorFunction(png_structp png, png_const_charp dummy) {
  (void)dummy;  // remove variable-unused warning
  longjmp(png_jmpbuf(png), 1);
}

WebP::WebP(const std::string& path)
    : path_(path),
      is_valid_(false) {
  is_valid_ = Initialize();
}

WebP::WebP(const WebP& other)
    : path_(other.path_),
      is_valid_(other.is_valid_),
      img_size_(other.img_size_) {
  if (is_valid_) {
    img_data_.reset(new uint8_t[img_size_]);
    std::copy(&other.img_data_[0], &other.img_data_[0] + img_size_, &img_data_[0]);
  }
  memcpy(&bitstream_, &other.bitstream_, sizeof(WebPBitstreamFeatures));
}

WebP::WebP(WebP&& other)
    : path_(std::move(other.path_)),
      is_valid_(other.is_valid_),
      img_data_(std::move(other.img_data_)),
      img_size_(other.img_size_) {
  memcpy(&bitstream_, &other.bitstream_, sizeof(WebPBitstreamFeatures));
  memset(&other.bitstream_, 0, sizeof(WebPBitstreamFeatures));
}

WebP::~WebP() {
  if (img_data_.get()) {
    img_data_.reset(nullptr);
  }
  img_size_ = 0;

  memset(&bitstream_, 0, sizeof(WebPBitstreamFeatures));
}

WebP& WebP::operator=(const WebP& other) {
  if (&other == this) {
    return *this;
  }

  path_ = other.path_;
  is_valid_ = other.is_valid_;
  if (is_valid_) {
    img_data_.reset(new uint8_t[img_size_]);
    std::copy(&other.img_data_[0], &other.img_data_[0] + img_size_, &img_data_[0]);
  }
  memcpy(&bitstream_, &other.bitstream_, sizeof(WebPBitstreamFeatures));

  return *this;
}

WebP& WebP::operator=(WebP&& other) {
  path_ = std::move(other.path_);
  img_data_ = std::move(img_data_);
  img_size_ = other.img_size_;
  memcpy(&bitstream_, &other.bitstream_, sizeof(WebPBitstreamFeatures));

  other.img_size_ = 0;
  memset(&bitstream_, 0, sizeof(WebPBitstreamFeatures));

  return *this;
}

bool WebP::SaveImage(Format format, const std::string& out_file) {
  if (!is_valid_) {
    fprintf(stderr, "Invalid webp image.\n");
    return false;
  }

  if (bitstream_.has_animation) {
    printf("Animation is not currently supported\n");
    assert(0);
    return false;
  }

  VP8StatusCode status = VP8_STATUS_OK;
  WebPDecoderConfig config;
  WebPDecBuffer* const output_buffer = &config.output;
  WebPBitstreamFeatures* const bitstream = &config.input;

  if (!WebPInitDecoderConfig(&config)) {
    fprintf(stderr, "Library version mismatch!\n");
    return false;
  }
  memcpy(bitstream, &bitstream_, sizeof(WebPBitstreamFeatures));

  output_buffer->colorspace = bitstream->has_alpha ? MODE_RGBA : MODE_RGB;
  status = WebPDecode(img_data_.get(), img_size_, &config);
  if (status != VP8_STATUS_OK) {
    WebPFreeDecBuffer(output_buffer);
    return false;
  }

  FILE* file = fopen(out_file.c_str(), "w");
  if (file == NULL) {
    fprintf(stderr, "Open %s failed\n", out_file.c_str());
    WebPFreeDecBuffer(output_buffer);
    return false;
  }

  bool res = false;
  switch (format) {
    case IMG_PNG:
      res = WritePNG(file, output_buffer);
      break;
    case IMG_JPEG:
      res = WriteJPEG(file, output_buffer);
      break;
    default:
      fprintf(stderr, "Please specify correct format.\n");
  }

  fclose(file);
  WebPFreeDecBuffer(output_buffer);
  return res;
}

bool WebP::Initialize() {
  std::string img_data;
  if (!ReadFileToString(path_, &img_data) || img_data.size() == 0) {
    return false;
  }

  int width = 0;
  int height = 0;
  int res = WebPGetInfo((const uint8_t*)img_data.c_str(), img_data.size(), &width, &height);
  if (!res) {
    return false;
  }

  VP8StatusCode status = WebPGetFeatures(
      (const uint8_t*)img_data.c_str(), img_data.size(), &bitstream_);
  if (status != VP8_STATUS_OK) {
    return false;
  }

  img_size_ = img_data.size();
  img_data_.reset(new uint8_t[img_size_]);
  std::copy(&img_data[0], &img_data[0] + img_size_, &img_data_[0]);

  return true;
}

bool WebP::WritePNG(FILE* out_file, const WebPDecBuffer* const buffer) {
  assert(out_file && buffer);
  assert(is_valid_);

  volatile png_structp png;
  volatile png_infop info;

  png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                NULL, PNGErrorFunction, NULL);
  if (png == NULL) {
    return false;
  }
  info = png_create_info_struct(png);
  if (info == NULL) {
    png_destroy_write_struct((png_structpp)&png, NULL);
    return false;
  }
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_write_struct((png_structpp)&png, (png_infopp)&info);
    return false;
  }
  png_init_io(png, out_file);
  {
    const uint32_t width = buffer->width;
    const uint32_t height = buffer->height;
    png_bytep row = buffer->u.RGBA.rgba;
    const int stride = buffer->u.RGBA.stride;
    const int has_alpha = WebPIsAlphaMode(buffer->colorspace);
    uint32_t y;

    png_set_IHDR(png, info, width, height, 8,
                 has_alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for (y = 0; y < height; ++y) {
      png_write_rows(png, &row, 1);
      row += stride;
    }
  }

  png_write_end(png, info);
  png_destroy_write_struct((png_structpp)&png, (png_infopp)&info);
  return true;
}

bool WebP::WriteJPEG(FILE* out_file, const WebPDecBuffer* const buffer) {
  assert(out_file && buffer);
  assert(is_valid_);

  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, out_file);

  cinfo.input_components = WebPIsAlphaMode(buffer->colorspace) ? 4 : 3;
  cinfo.image_width = buffer->width;
  cinfo.image_height = buffer->height;
  cinfo.in_color_space = WebPIsAlphaMode(buffer->colorspace) ? JCS_EXT_RGBA :JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 100, TRUE);

  jpeg_start_compress(&cinfo, TRUE);
  {
    const int stride = buffer->u.RGBA.stride;
    uint8_t* const img_data = buffer->u.RGBA.rgba;
    JSAMPROW row[1];
    while (cinfo.next_scanline < cinfo.image_height) {
      row[0] = img_data + cinfo.next_scanline * stride;
      jpeg_write_scanlines(&cinfo, row, 1);
    }
  }
  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);
  return true;
}
