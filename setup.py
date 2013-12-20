# -*- coding: utf-8 -*-
from distutils.core import setup
from distutils.extension import Extension
import os
ROOT = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../')

glpyext = Extension(
    'angband'
    ,sources = [
        'src/pybind.c',
        'src/attack.c',
        'src/birth.c',
        'src/buildid.c',
        'src/cave.c',
        'src/cmd-cave.c',
        'src/cmd-context.c',
        'src/cmd-know.c',
        'src/cmd-misc.c',
        'src/cmd-obj.c',
        'src/cmd-pickup.c',
        'src/cmd-process.c',
        'src/death.c',
        'src/debug.c',
        'src/dungeon.c',
        'src/effects.c',
        'src/files.c',
        'src/game-cmd.c',
        'src/game-event.c',
        'src/generate.c',
        'src/grafmode.c',
        'src/guid.c',
        'src/history.c',
        'src/init.c',
        'src/keymap.c',
        'src/load.c',
        'src/main-sdl.c',
        'src/monster/mon-init.c',
        'src/monster/melee1.c',
        'src/monster/melee2.c',
        'src/monster/mon-list.c',
        'src/monster/mon-lore.c',
        'src/monster/mon-make.c',
        'src/monster/mon-msg.c',
        'src/monster/mon-power.c',
        'src/monster/mon-spell.c',
        'src/monster/mon-timed.c',
        'src/monster/mon-util.c',
        'src/object/chest.c',
        'src/object/identify.c',
        'src/object/obj-desc.c',
        'src/object/obj-flag.c',
        'src/object/obj-info.c',
        'src/object/obj-list.c',
        'src/object/obj-make.c',
        'src/object/obj-power.c',
        'src/object/obj-ui.c',
        'src/object/obj-util.c',
        'src/object/pval.c',
        'src/object/randart.c',
        'src/object/slays.c',
        'src/option.c',
        'src/parser.c',
        'src/randname.c',
        'src/pathfind.c',
        'src/prefs.c',
        'src/player/calcs.c',
        'src/player/class.c',
        'src/player/player.c',
        'src/player/race.c',
        'src/player/spell.c',
        'src/player/timed.c',
        'src/player/p-util.c',
        'src/quest.c',
        'src/score.c',
        'src/signals.c',
        'src/save.c',
        'src/savefile.c',
        'src/spells1.c',
        'src/spells2.c',
        'src/squelch.c',
        'src/store.c',
        'src/tables.c',
        'src/target.c',
        'src/trap.c',
        'src/ui.c',
        'src/ui-birth.c',
        'src/ui-event.c',
        'src/ui-knowledge.c',
        'src/ui-menu.c',
        'src/ui-options.c',
        'src/ui-spell.c',
        'src/util.c',
        'src/variable.c',
        'src/wiz-spoil.c',
        'src/wiz-stats.c',
        'src/wizard.c',
        'src/x-spell.c',
        'src/xtra2.c',
        'src/xtra3.c',
        'src/z-bitflag.c',
        'src/z-file.c',
        'src/z-form.c',
        'src/z-msg.c',
        'src/z-quark.c',
        'src/z-queue.c',
        'src/z-rand.c',
        'src/z-set.c',
        'src/z-term.c',
        'src/z-textblock.c',
        'src/z-type.c',
        'src/z-util.c',
        'src/z-virt.c',
        'src/borg/borg1.c',
        'src/borg/borg2.c',
        'src/borg/borg3.c',
        'src/borg/borg4.c',
        'src/borg/borg5.c',
        'src/borg/borg6.c',
        'src/borg/borg7.c',
        'src/borg/borg8.c',
        'src/borg/borg9.c',
    ]
    ,libraries=[
        'SDL', 'SDL_image', 'SDL_ttf',
    ]
    # Debug options
    ,define_macros = [
        ('__DEBUG__', 1), ('USE_SDL', 1)
    ]
    ,extra_link_args = ['-g']
    ,extra_compile_args = ['-g', '-std=gnu99']
    ,library_dirs=[
        
    ]
    ,include_dirs=[
        'src',
    ]
)

setup(
    name                = 'angband',
    version             = '0.1a',
    author              = 'FIXME',
    author_email        = 'FIXME',
    keywords            = ["template"],
    license             = 'FIXME',
    description         = 'FIXME',
    long_description    = 'FIXME',
    ext_modules         = [glpyext],
#     packages            = ['angbandpyborg'],
    # Add data
#     package_dir         = {'glpy': 'glpy'},
#     package_data        = {'glpy': ['shaders/*/*.s']},
)

