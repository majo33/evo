top = "."
out = "build"


def options(ctx):
    ctx.load('compiler_c')


def configure(ctx):
    ctx.load('compiler_c')
    ctx.check_cc(uselib_store="cups", header_name="cups/raster.h", lib=["cupsimage"])


def build(ctx):
    ctx.program(
      name='evo',
      target='evo',
      features='c cprogram',
      source = ctx.path.ant_glob('src/*.c'),
      cflags=['-O2'],
      use=["cups"]
    )
