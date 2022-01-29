import os


def mal_encode(key, clear):
    enc = ""
    for i in range(0, len(clear)):
        key_c = key[i % len(key)]
        placeholder = ord(clear[i]) + ord(key_c)
        enc_c = placeholder % 127
        enc += chr(enc_c)
    return enc


def mal_decode(key, enc):
    dec = ""
    for i in range(0, len(enc)):
        key_c = key[i % len(key)]
        placeholder = 127 + (enc[i]) - ord(key_c)
        dec_c = (placeholder % 127)
        dec += chr(dec_c)
    return dec


def file_decrypt(dir, filename, key):
    """

    :param dir: directory of file
    :param filename: name of file
    :param key: XOR key
    :return: None
    """
    inpath = os.path.join(dir, filename)
    outpath = os.path.join(dir, "decrypted_" + filename)

    CHUNKSIZE = 1000

    with open(inpath, 'rb') as source:
        with open(outpath, 'wb') as dest:
            bytes_read = source.read(CHUNKSIZE)
            while bytes_read:
                cleartext = bytearray(len(bytes_read))
                for i in range(len(bytes_read)):
                    cleartext[i] = bytes_read[i] ^ key
                dest.write(cleartext)
                bytes_read = source.read(CHUNKSIZE)
    os.remove(inpath)
    return