#!/usr/bin/env python

#########################################################################################
# Copyright © 2023 Dmitry Kuznetsov.                                                    #
#                                                                                       #
# All rights reserved. No part of this software may be reproduced, distributed,         #
# or transmitted in any form or by any means, including photocopying, recording,        #
# or other electronic or mechanical methods, without the prior written permission       #
# of the copyright owner.                                                               #
# Any unauthorized use, reproduction, or distribution of                                #
# this software is strictly prohibited and may # result in severe civil and criminal    #
# penalties.                                                                            #
#                                                                                       #
#########################################################################################

import os
import sys
import subprocess

from ShareUtils import *


class VcpkgBuilder(object):
    def __init__(self, is_x64: bool, is_static: bool):
        self.is_x64 = is_x64
        self.is_static = is_static
        self.root_path = default=os.path.join(os.path.realpath(os.path.dirname(__file__)), "..")

        if not os.path.exists(os.path.join(self.root_path, 'build')):
            os.makedirs(os.path.join(self.root_path, 'build'))
        self.log_file = os.path.join(self.root_path, 'build', 'log_BuildDepends.txt')
        if os.path.exists(self.log_file):
            os.unlink(self.log_file)

    @property
    def vcpkg_triplet(self) -> str:
        if self.is_static:
            return 'x64-windows-static' if self.is_x64 else 'x86-windows-static'
        else:
            return 'x64-windows' if self.is_x64 else 'x86-windows'

    def __clone_vcpkg(self, dest_dir: str):
        os.chdir(dest_dir)

        git_cmd = 'git clone https://github.com/Microsoft/vcpkg.git'
        call_cmd(cmd_string=git_cmd, log_file=self.log_file)

    def __init_vcpkg(self):
        log_to_file(log_file=self.log_file,
                    description=None,
                    message='Init vcpkg')
        depends_path = os.path.join(self.root_path, 'dependencies')
        if not os.path.exists(depends_path):
            log_to_file(log_file=self.log_file,
                        description=None,
                        message='Creates folder for dependencies')
            os.makedirs(depends_path)

        vcpkg_path = os.path.join(depends_path, 'vcpkg')
        if not os.path.exists(vcpkg_path):
            log_to_file(log_file=self.log_file,
                        description=None,
                        message='Clone vcpkg')
            self.__clone_vcpkg(dest_dir=depends_path)
        os.chdir(vcpkg_path)
        call_cmd(cmd_string='.\\bootstrap-vcpkg.bat', log_file=self.log_file)

    def __build_dep(self, package_name: str):
        log_to_file(self.log_file, None, f'Install {package_name} package')
        cmd = f'vcpkg install {package_name} --triplet {self.vcpkg_triplet}'
        call_cmd(cmd_string=cmd, log_file=self.log_file)

    def process(self):
        log_to_file(log_file=self.log_file,
                    description=None,
                    message='Start BuildDepends script for {0} {1}'.format('x64' if self.is_x64 else 'x32',
                                                                           self.vcpkg_triplet))
        self.__init_vcpkg()
        self.__build_dep(package_name='boost-program-options')


def main():
    cwd = os.getcwd()
    package_builder_x64 = VcpkgBuilder(is_x64=True, is_static=True)
    package_builder_x64.process()

    os.chdir(cwd)
    package_builder_x32 = VcpkgBuilder(is_x64=False, is_static=True)
    package_builder_x32.process()


if __name__ == '__main__':
    main()
