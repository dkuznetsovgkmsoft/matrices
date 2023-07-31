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


def log_to_file(log_file: str, description, message, copy_to_std_out=True):
    message = message.split('\\n')
    with open(log_file, 'a') as fp:
        if description:
            fp.write(f'---- {description}: \n')
            if copy_to_std_out:
                print(f'---- {description}:\n')
        for line in message:
            fp.write(f'{line}\n')
            if copy_to_std_out:
                print(f'{line}')


def call_cmd(cmd_string: str, log_file: str):
    process = subprocess.Popen(cmd_string, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    std_out, std_err = process.communicate()
    rc = process.wait()
    log_to_file(log_file, 'std_out', std_out.decode('utf-8'), copy_to_std_out=False)
    if std_err:
        log_to_file(log_file, 'std_err', std_err.decode('utf-8'), copy_to_std_out=False)
    if rc:
        print(f'Something went wrong. More information in "{log_file}"')
        sys.exit()


def clone_from_github(remote_path: str, destination_dir: str, log_file: str):
    log_to_file(log_file=log_file,
                description=None,
                message=f"Clone '{remote_path}' repo")

    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)
    os.chdir(destination_dir)

    cmd = f"git clone {remote_path}"

    call_cmd(cmd, log_file)


def reset_hard_to_master_branch(destination_dir: str, repo_name: str, log_file: str):
    log_to_file(log_file=log_file,
                description=None,
                message=f"Reset hard master branch of the '{repo_name}' repository")
    os.chdir(os.path.join(destination_dir, repo_name))

    cmd = f"git checkout master"
    call_cmd(cmd, log_file)

    cmd = f"git reset --hard origin/master"
    call_cmd(cmd, log_file)

    cmd = f"git pull --rebase origin master"
    call_cmd(cmd, log_file)


def apply_patches_for_repository(repo_folder: str, patches_dir: str, repo_name: str, log_file: str):
    log_to_file(log_file=log_file,
                description=None,
                message=f"Apply patches for '{repo_name}' repo")

    if not os.path.exists(repo_folder):
        print(f"Not found the folder '{repo_name}', "
              f"please check this folder and run the script again.")
        sys.exit()

    if not os.path.exists(patches_dir):
        print(f"Not found the folder with patches for {repo_name}, "
              f"please check this folder and run the script again.")
        sys.exit()

    os.chdir(repo_folder)
    git_patch_cmd = "git am --3way --ignore-whitespace {filename}"

    for patch_filename in os.listdir(patches_dir):
        abs_path = os.path.join(patches_dir, patch_filename)
        if not os.path.isfile(os.path.join(patches_dir, patch_filename)):
            continue

        log_to_file(log_file=log_file,
                    description=None,
                    message=f"Apply patch '{patch_filename}'")
        current_cmd = git_patch_cmd.format(filename=abs_path)
        call_cmd(current_cmd, log_file)


