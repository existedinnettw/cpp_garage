

"Dynamic" means type(static) casting at runtime. "Static" means casting at compile time.

You can uncomment `// std::vector<Shape *> items = std::vector<Shape *>(2);` in dyna_poly.hpp to see speed different.

# result
for dynamic polymorphism with array container
```bash
2023-12-18T10:28:38+08:00
Running ./bench
Run on (8 X 2112.01 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 1280 KiB (x4)
  L3 Unified 12288 KiB (x1)
Load Average: 0.09, 0.04, 0.05
------------------------------------------------------------
Benchmark                  Time             CPU   Iterations
------------------------------------------------------------
bench_dyna_poly         4.27 ns         4.24 ns    167894898
bench_static_poly       4.20 ns         4.20 ns    167192886
```

```bash
2023-12-18T10:28:43+08:00
Running ./bench
Run on (8 X 2112.01 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 1280 KiB (x4)
  L3 Unified 12288 KiB (x1)
Load Average: 0.16, 0.06, 0.05
------------------------------------------------------------
Benchmark                  Time             CPU   Iterations
------------------------------------------------------------
bench_dyna_poly         4.19 ns         4.19 ns    168608434
bench_static_poly       4.21 ns         4.21 ns    167306415
```

---

for dynamic polymorphism with vector container
```bash
2023-12-18T10:23:17+08:00
Running ./bench
Run on (8 X 2112.01 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 1280 KiB (x4)
  L3 Unified 12288 KiB (x1)
Load Average: 0.00, 0.04, 0.05
------------------------------------------------------------
Benchmark                  Time             CPU   Iterations
------------------------------------------------------------
bench_dyna_poly         6.69 ns         6.69 ns    101958031
bench_static_poly       4.10 ns         4.10 ns    170099973
```


# conclusion

- Static (CRTP through template) and dynamic polymorphism actually no visible excution time difference at this simple case. (which function call stack is shallow.)
- At dynamic polymorphism case, container in vector will be about 50% slower than array one even memory already allocated, so stay in array as long as you can.

# my choice
- At this point I will choose dynamic one, since it's eazier to write and able to hide your implementation. If I need ultra performance just open LTO.

# todo
- I am not sure if there is any boost for static(template) one if there are large but reasonable function call, which will be benificial with inline. 
  - Of course dynamic one have LTO (link-time optimization).

# reference
* [Header only or splitting files for libraries](https://www.reddit.com/r/cpp/comments/wyyddj/comment/ilzlk5v/?utm_source=share&utm_medium=web2x&context=3)
  * > If performance is a big concern, just turn on LTO for your release builds, and the performance advantages of header-only (mostly) go away (traded for a significantly longer linking step). This would fix the "code bloat" issue as well.
* [C++ 的靜態多型：CRTP](https://viml.nchc.org.tw/curiously-recurring-template-pattern/)
* 

