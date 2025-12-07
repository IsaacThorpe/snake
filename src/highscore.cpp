#include "highscore.h"

namespace fs = std::filesystem;

static fs::path get_user_home_dir() {
    // 1. Linux/macOS (Preferred: HOME)
    if (const char* home_dir = std::getenv("HOME")) {
        return fs::path(home_dir);
    }
    
    // 2. Windows (Fall back to HOMEDRIVE + HOMEPATH)
    if (const char* drive = std::getenv("HOMEDRIVE")) {
        if (const char* path = std::getenv("HOMEPATH")) {
            return fs::path(drive) / path;
        }
    }

    // 3. Nothing/Error: Return an empty path
    return fs::path();
}

// TODO: Make this not throw. It should just output to cerr.
static fs::path get_snake_highscore_path() {
    // Get ~/.snake_highscore path
    fs::path home_dir = get_user_home_dir();
    if (home_dir.empty()) throw std::runtime_error("Error: Could not determine the user's home directory.");
    fs::path highscore_path = home_dir / ".snake_highscore";

    return highscore_path;
}

// Encrypt plaintext with AES-256-CBC
static std::vector<unsigned char> aes_encrypt(const std::vector<unsigned char>& plaintext,
                                              const std::array<unsigned char, 32>& key,
                                              std::array<unsigned char, 16>& iv_out)
{
    if(!RAND_bytes(iv_out.data(), iv_out.size()))
        throw std::runtime_error("Failed to generate IV");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv_out.data());

    std::vector<unsigned char> ciphertext(plaintext.size() + 16); // extra for padding
    int outlen = 0, tmplen = 0;

    EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen, plaintext.data(), plaintext.size());
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &tmplen);
    outlen += tmplen;
    ciphertext.resize(outlen);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// Decrypt AES-256-CBC ciphertext
static std::vector<unsigned char> aes_decrypt(const std::vector<unsigned char>& ciphertext,
                                              const std::array<unsigned char, 32>& key,
                                              const std::array<unsigned char, 16>& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());

    std::vector<unsigned char> plaintext(ciphertext.size());
    int outlen = 0, tmplen = 0;

    EVP_DecryptUpdate(ctx, plaintext.data(), &outlen, ciphertext.data(), ciphertext.size());
    EVP_DecryptFinal_ex(ctx, plaintext.data() + outlen, &tmplen);
    outlen += tmplen;
    plaintext.resize(outlen);

    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

static std::array<unsigned char, 32>& get_key() {
    static std::array<unsigned char, 32> arr;
    static bool initialized = false;
    if (!initialized) {
        for (auto& k : arr) {
            k = random_int_compile_time_seeded(0, 255);
        }
        initialized = true;
    }
    return arr;
}

static constexpr std::array<unsigned char, 32> tamper_signature = {238, 6, 209, 233, 120, 88, 33, 91, 96, 64, 93, 58, 209, 114, 161, 242, 120, 156, 47, 203, 114, 55, 238, 64, 242, 83, 213, 197, 161, 51, 100, 193 };

// TODO: Make this not throw. It should just output to cerr.
void save_highscore(u32 highscore) {

    fs::path highscore_path = get_snake_highscore_path();

    std::vector<unsigned char> plainbytes;
    plainbytes.push_back((highscore >> 24) & 0xFF);
    plainbytes.push_back((highscore >> 16) & 0xFF);
    plainbytes.push_back((highscore >> 8) & 0xFF);
    plainbytes.push_back(highscore & 0xFF);

    plainbytes.insert(plainbytes.end(), tamper_signature.begin(), tamper_signature.end());

    std::array<unsigned char, 16> iv;
    std::vector<unsigned char> encrypted_bytes = aes_encrypt(plainbytes, get_key(), iv);

    std::ofstream out(highscore_path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open highscore file");

    out.write(reinterpret_cast<const char*>(iv.data()), iv.size());
    out.write(reinterpret_cast<const char*>(encrypted_bytes.data()), encrypted_bytes.size());

    out.close();
}

u32 get_highscore() {

    fs::path highscore_path = get_snake_highscore_path();

    std::ifstream in(highscore_path, std::ios::binary);
    if (!in)
        return 0;
    std::array<unsigned char, 16> iv;
    in.read(reinterpret_cast<char*>(iv.data()), iv.size());

    std::vector<unsigned char> encrypted_bytes((std::istreambuf_iterator<char>(in)), {});

    std::vector<unsigned char> plainbytes = aes_decrypt(encrypted_bytes, get_key(), iv);

    if (plainbytes.size() < tamper_signature.size() + 4) return 0;
    for (int i = 0; i < 32; ++i) {
        if (plainbytes[plainbytes.size() - 1 - i] != tamper_signature[tamper_signature.size() - 1 - i]) {
            return 0;
        }
    }

    u32 highscore = (plainbytes[0] << 24) |
        (plainbytes[1] << 16) |
        (plainbytes[2] << 8)  |
        plainbytes[3];

    return highscore;
}

