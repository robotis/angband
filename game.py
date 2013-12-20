import os, sys
root = os.path.dirname(os.path.abspath(__file__))
for v in ('x86_64', 'i686'):
    for pv in ('2', '3'):
        path = '%s/build/lib.linux-%s-3.%s' % (root, v, pv)
        if os.path.exists(path):
            sys.path.append(path)
        else:
            path += '-pydebug'
            if os.path.exists(path):
                sys.path.append(path)
                
import angband

angpath = os.path.expanduser('~') + '/.angband'
if not os.path.exists(angpath):
    os.mkdir(angpath + '/Angband')
    os.mkdir(angpath + '/Angband/save')

angband.run()