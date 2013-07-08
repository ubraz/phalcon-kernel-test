# Unit Tests for Phalcon Kernel

**WARNING:** work in progress

## Goal

Write unit tests that will check the functionality of the [Phalcon Kernel](https://github.com/phalcon/cphalcon/blob/master/ext/kernel/README.md).

## Dependencies

  * Embedded SAPI for PHP built with `--enable-debug` and optionally `--enable-maintainer-zts` (consider using [phpenv](https://github.com/CHH/phpenv) and [phpbuild](http://github.com/chh/php-build))
  * [CUnit](http://cunit.sourceforge.net/)

## Build

Please check [Makefile](https://github.com/sjinks/phalcon-kernel-test/blob/master/Makefile) first.
It currently uses `phpenv` (mainly because this is what I use to test Phalcon against different versions of PHP and because the system version of PHP is built without `--enable-debug`).
If you don't want to use `phpenv`, please make sure to update `PHPCONFIG` and `LDFLAGS` variables (`CFLAGS` variable is probably fine).

Build with `make`, then run `test-phalcon-kernel` and enjoy :-)

