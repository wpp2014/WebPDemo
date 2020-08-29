# WebPDemo
将webp格式的图片转成其他格式图片的demo

- [编译](#编译)
  - [Debug](#Debug)
  - [Release](#Release)

## 编译

### Debug

```sh
git clone https://github.com/wpp2014/WebPDemo.git
cd WebPDemo
mkdir -p out/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
make
```

### Release

```sh
git clone https://github.com/wpp2014/WebPDemo.git
cd WebPDemo
mkdir -p out/Release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

## 运行

```sh
webp_demo --in-file <img path> --out-file <output path> --format png/jpeg
```
