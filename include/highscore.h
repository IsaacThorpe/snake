#pragma once
#include <vector>
#include <array>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstdlib>
#include <filesystem>

#include "globals.h"
#include "random_utils.h"

void save_highscore(u32 highscore);

u32 get_highscore();
