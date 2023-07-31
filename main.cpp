/****************************************************************************************
* Copyright © 2023 Dmitry Kuznetsov.                                                    *
*                                                                                       *
* All rights reserved. No part of this software may be reproduced, distributed,         *
* or transmitted in any form or by any means, including photocopying, recording,        *
* or other electronic or mechanical methods, without the prior written permissin        *
* of the copyright owner.                                                               *
* Any unauthorized use, reproduction, or distribution of this software is strictly      *
* prohibited and may # result in severe civil and criminal penalties.                   *
*                                                                                       *
****************************************************************************************/

#include <iostream>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cctype>
#include <memory>

#include "program_options.h"

int main(int argc, char** argv) {
    boost::program_options::options_description options;
    boost::program_options::variables_map parsed_options;

    bool parsed = matrices::program_options::parse_command_line(argc, argv, options, parsed_options);
    if (!parsed) {
        return 0;
    }

    if (parsed_options.contains("help")) {
        matrices::program_options::print_help(options);
        return 1;
    }

    return 1;
}