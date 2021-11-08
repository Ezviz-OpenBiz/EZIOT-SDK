import os
import sys
#import subprocess
#from building import *
#import re
#import shutil

#cwd = os.path.dirname(os.path.realpath(__file__))

def mk_rtconfig(config_filename, filename): 
    print(config_filename, filename)
    rtconfig = open(filename, 'w')
    rtconfig.write('#ifndef {}_H__\n'.format(os.path.basename(filename)[:-2].upper()))
    rtconfig.write('#define {}_H__\n\n'.format(os.path.basename(filename)[:-2].upper()))

    config = open(config_filename, 'r')

    empty_line = 1

    for line in config:
        line = line.lstrip(' ').replace('\n', '').replace('\r', '')

        if len(line) == 0: continue

        if line[0] == '#':
            if len(line) == 1:
                if empty_line:
                    continue

                rtconfig.write('\n')
                empty_line = 1
                continue

            rtconfig.write('/*%s */\n' % line[1:])
            empty_line = 0
        else:
            empty_line = 0
            setting = line.split('=')
            if len(setting) >= 2:
                if setting[0].startswith('CONFIG_'):
                    setting[0] = setting[0][7:]

                if setting[1] == 'y':
                    rtconfig.write('#define %s\n' % setting[0])
                else:
                    rtconfig.write('#define %s %s\n' % (setting[0], setting[1]))

    config.close()
    rtconfig.write('#endif\n')
    rtconfig.close()

def main(config_path):
    mk_rtconfig(config_path + '/' + '.config', config_path + '/' + 'ezconfig.h')

if __name__ == '__main__':
    if 2 > len(sys.argv):
        main('.')
    else:
        main(sys.argv[1])