# zephyr_conan_sysbuild: the same app, built with sysbuild + MCUboot

Companion of [`../zephyr_conan`](../zephyr_conan/README.md). Same sources,
same profiles, same Conan graph; the difference is that `build()` runs
`west build --sysbuild`, so Zephyr's sysbuild configures **two images** on the
STM32 board (MCUboot + this app, the app image signed for MCUboot) and one
image on `native_sim`.

```text
conan build . -pr:h profiles/nucleo_f429zi
  └── west build --sysbuild -b nucleo_f429zi -d build/nucleo_f429zi/zephyr .
        ├── image mcuboot                 bootloader/mcuboot (+ mbedtls, tf-psa-crypto) from the zephyr package
        └── image zephyr_conan_sysbuild   this app, find_package(fmt) from Conan, signed -> zephyr.signed.bin
```

## What had to change compared to the single-image build

Almost nothing in the Conan part, and that is the point of handing everything
over through the environment:

* Sysbuild does **not** forward `-DCMAKE_*` cache variables to the images it
  configures (only `CMAKE_BUILD_TYPE`/`CMAKE_VERBOSE_MAKEFILE`, and only when
  prefixed with the image name). `CMAKE_PREFIX_PATH` and
  `ZEPHYR_EXTRA_MODULES` are environment variables, which every image
  inherits, so `find_package(fmt)` and the build-type glue module work in the
  app image without any sysbuild-specific plumbing. Other `-D` variables are
  written to a sysbuild cache file that images only consult through Zephyr's
  `zephyr_get()`, i.e. not for arbitrary variables.
* `mcuboot` is just another west project in the pinned `zephyr` package; its
  signature verification needs the `mbedtls` and `tf-psa-crypto` projects too
  (Mbed TLS 4 splits the crypto into TF-PSA-Crypto), hence the
  `zephyr/*:projects` pin. Signing needs `imgtool`'s Python requirements
  (`bootloader/mcuboot/scripts/requirements.txt`) in the west venv.
* `Kconfig.sysbuild` decides whether MCUboot is part of the build. It
  re-opens the `BOOTLOADER` choice *before* sourcing Zephyr's sysbuild Kconfig
  (the Kconfig.defconfig trick: the first satisfied default wins, and the
  stock `default BOOTLOADER_NONE` is unconditional) and enables MCUboot for
  every board except `native_sim`. `sysbuild/mcuboot.conf` and
  `sysbuild/mcuboot.overlay` configure the MCUboot image only (see below).

## Build

```bash
export PATH=~/.venvs/zephyr/bin:$PATH          # west, ninja, imgtool deps (see ../zephyr_conan/README.md)

conan build . -pr:h profiles/native_sim --build=missing
./build/native_sim_native_64/zephyr/zephyr_conan_sysbuild/zephyr/zephyr.exe

conan build . -pr:h profiles/nucleo_f429zi --build=missing
ls build/nucleo_f429zi/zephyr/mcuboot/zephyr/zephyr.bin \
   build/nucleo_f429zi/zephyr/zephyr_conan_sysbuild/zephyr/zephyr.signed.bin
west flash -d build/nucleo_f429zi/zephyr        # flashes both domains (see domains.yaml)
```

Expect one Kconfig warning from the app image about `MCUBOOT_UPDATE_FOOTER_SIZE`:
MCUboot's sysbuild integration passes it to the app, which has no image
manager (`CONFIG_IMG_MANAGER`) in this demo. It is harmless.

## Flash layout: a 48 KB bootloader partition

The board's devicetree reserves 32 KB for MCUboot. MCUboot with Mbed TLS 4
RSA verification needs about 32.2 KB with its default UART console and
info-level logging, so it overflows by a few hundred bytes. Instead of
silencing the bootloader, the demo grows the partition to 48 KB (three 16 KB
sectors; slot0/slot1 start at 128 KB and are unaffected):

| File | Applies to | Content |
| --- | --- | --- |
| `boards/nucleo_f429zi.overlay` | app image | `&boot_partition { reg = <0 DT_SIZE_K(48)>; }` |
| `sysbuild/mcuboot.overlay` | MCUboot image | the same, **plus** `zephyr,code-partition = &boot_partition` |
| `sysbuild/mcuboot.conf` | MCUboot image | keeps info-level logging |

The duplicated `chosen` node is not optional: sysbuild hands
`sysbuild/<image>.overlay` to the image as `DTC_OVERLAY_FILE`, which
*replaces* MCUboot's own `boot/zephyr/app.overlay` rather than extending it.
Without it MCUboot is silently linked into slot0 (visible as a 256 KB `FLASH`
region in its memory report). With the overlay MCUboot uses ~34 KB of 48 KB
and the app 204 KB of its 256 KB slot.

The app image uses MCUboot's development signing key
(`bootloader/mcuboot/root-rsa-2048.pem`); set `SB_CONFIG_BOOT_SIGNATURE_KEY_FILE`
in `sysbuild.conf` for anything beyond a demo.
