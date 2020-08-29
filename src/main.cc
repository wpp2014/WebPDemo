#include <stdio.h>
#include <string.h>

#include <memory>

#include "src/webp.h"

static void Help() {
  printf("Usage: webp_demo --in-file in_file --out-file out_file --format img_type\n");
}

int main(int argc, char** argv) {
  const char* in_file = NULL;
  const char* out_file = NULL;
  const char* img_format = NULL;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
      Help();
      return 0;
    } else if (!strcmp(argv[i], "--in-file") && i < argc - 1) {
      in_file = (const char*)argv[++i];
    } else if (!strcmp(argv[i], "--out-file") && i < argc - 1) {
      out_file = (const char*)argv[++i];
    } else if (!strcmp(argv[i], "--format") && i < argc - 1) {
      img_format = (const char*)argv[++i];
    }
  }

  if (!in_file) {
    fprintf(stderr, "Missing input file!\n");
    Help();
    return 0;
  }
  if (!out_file) {
    fprintf(stderr, "Missing output file!\n");
    Help();
    return 0;
  }
  if (!img_format) {
    fprintf(stderr, "Missing image format!\n");
    Help();
    return 0;
  }

  WebP::Format format;
  if (!strcmp(img_format, "png")) {
    format = WebP::IMG_PNG;
  } else if (!strcmp(img_format, "jpeg")) {
    format = WebP::IMG_JPEG;
  } else {
    fprintf(stderr, "Please specify png or jpeg!\n");
    Help();
    return 0;
  }

  std::unique_ptr<WebP> webp(new WebP(in_file));
  if (!webp->IsValid()) {
    fprintf(stderr, "%s is not valid webp image\n", argv[1]);
    return 1;
  }

  bool res = webp->SaveImage(format, out_file);
  if (!res) {
    fprintf(stderr, "Save %s to %s failed\n", in_file, img_format);
  } else {
    fprintf(stderr, "Save %s to %s success\n", in_file, img_format);
  }

  return 0;
}
